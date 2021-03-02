#include "CIFCodec.h"

#include <thread>

#include <windows.h>
#include <tchar.h>

#include <opencv2/imgcodecs.hpp>
#include <tiff.h>

#include <utils/File.h>
#include <utils/Debug.h>
#include "CloseWindow.h"

namespace fs = std::filesystem;

namespace
{
    bool executeCommand(std::wstring& wsCmd)
    {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        BOOL status = CreateProcessW(NULL,           // No module name (use command line)
                                     &wsCmd[0],      // Command line
                                     NULL,           // Process handle not inheritable
                                     NULL,           // Thread handle not inheritable
                                     FALSE,          // Set handle inheritance to FALSE
                                     0,              // No creation flags
                                     NULL,           // Use parent's environment block
                                     NULL,           // Use parent's starting directory
                                     &si,            // Pointer to STARTUPINFO structure
                                     &pi);           // Pointer to PROCESS_INFORMATION structure
        if (status != TRUE)
        {
            MV_CLOGD("CreateProcess failed");
            return false;
        }

        // Wait until child process exits.
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles.
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return true;
    }

} // unnamed namespace

bool CIFCodec::KmpToCIF(const fs::path& rDstPath,
                        const fs::path& rKmpPath,
                        const fs::path& rEtlPath,
                        const fs::path& rDarkComponentPath,
                        const fs::path& rLightComponentPath)
{
    fs::path inputFilePath = rKmpPath;
    fs::path outputFilePath = rEtlPath;
    if (compressKmp(rDstPath.parent_path(),
                    inputFilePath,
                    outputFilePath) == false)
    {
        return false;
    }

    inputFilePath = outputFilePath;
    inputFilePath.replace_extension(".bds");
    if (compressFinal(rDstPath,
                      inputFilePath,
                      outputFilePath,
                      rDarkComponentPath,
                      rLightComponentPath) == false)
    {
        return false;
    }

    return true;
}

bool CIFCodec::StrToCIF(const fs::path& rDstPath,
                        const fs::path& rStrPath,
                        const fs::path& rDarkComponentPath,
                        const fs::path& rLightComponentPath)
{
    fs::path inputFilePath = rStrPath;
    fs::path outputFilePath = rDstPath;
    outputFilePath.replace_extension(".kmp");
    if (compressStr(outputFilePath, inputFilePath) == false)
    {
        return false;
    }

    inputFilePath = outputFilePath;
    outputFilePath.replace_extension(".etl");
    if (KmpToCIF(rDstPath, inputFilePath, outputFilePath,
                 rDarkComponentPath, rLightComponentPath) == false)
    {
        return false;
    }

    return true;
}

bool CIFCodec::ImageToCIF(const std::filesystem::path& rDstPath,
                          const std::filesystem::path& rImageFilePath)
{
    if ((rDstPath.extension() != ".CIF") || !fs::exists(rImageFilePath))
    {
        MV_CLOGD("Incorrect input parameters for the image compression:");
        MV_CLOGD(rDstPath);
        MV_CLOGD(rImageFilePath);
        return false;
    }

    fs::path darkComponentPath;
    fs::path lightComponentPath;
    if (params_.bCompressBg || params_.bCompressFg)
    {
        fs::path outputComponentPath = rDstPath.parent_path() / rDstPath.stem();
        darkComponentPath = outputComponentPath;
        lightComponentPath = outputComponentPath;
        darkComponentPath.concat("_dark.bmp");
        lightComponentPath.concat("_light.bmp");
        if (extractColor(rImageFilePath,
                         darkComponentPath,
                         lightComponentPath) == false)
        {
            return false;
        }

        if (params_.bCompressFg)
        {
            outputComponentPath = darkComponentPath;
            outputComponentPath.replace_extension(".Ti");
            if (compressColor(outputComponentPath, darkComponentPath,
                              params_.iSizeFg, params_.cFilter) == false)
            {
                return false;
            }
            darkComponentPath.replace_extension(".Ti");
        }
        else
        {
            if (fs::remove(darkComponentPath) == false)
            {
                return false;
            }
        }

        if (params_.bCompressBg)
        {
            outputComponentPath = lightComponentPath;
            outputComponentPath.replace_extension(".Ti");
            if (compressColor(outputComponentPath, lightComponentPath,
                              params_.iSizeBg, params_.cFilter) == false)
            {
                return false;
            }
            lightComponentPath.replace_extension(".Ti");
        }
        else
        {
            if (fs::remove(lightComponentPath) == false)
            {
                return false;
            }
        }
    }

    fs::path outputFilePath = rDstPath;
    outputFilePath.replace_extension(".str");
    if (extractStr(outputFilePath, rImageFilePath) == false)
    {
        return false;
    }

    if (StrToCIF(rDstPath, outputFilePath,
                 darkComponentPath, lightComponentPath) == false)
    {
        return false;
    }

    return true;
}

bool CIFCodec::extractColor(const std::filesystem::path& rImageFilePath,
                            const std::filesystem::path& rDstDarkComponentPath,
                            const std::filesystem::path& rDstLightComponentPath)
{
    if (!fs::exists(rImageFilePath)
        || rDstDarkComponentPath.empty()
        || rDstLightComponentPath.empty())
    {
        MV_CLOGD("Incorrect input parameters");
        return false;
    }
    cv::Mat image = cv::imread(rImageFilePath.string());
    if (image.empty())
    {
        MV_CLOGD("Image reading failed");
        return false;
    }

    std::wstring wsCmd = L"ColorExtractor.exe "
            + rImageFilePath.wstring() + L" "
            + rDstDarkComponentPath.wstring() + L" "
            + rDstLightComponentPath.wstring();
    if (executeCommand(wsCmd) == false)
    {
        return false;
    }

    return true;
}

bool CIFCodec::compressColor(const std::filesystem::path& rDstPath,
                             const std::filesystem::path& rImageFilePath,
                             int iDesiredSize, char cFilter)
{
    if (!fs::exists(rImageFilePath)
        || rDstPath.extension() != ".Ti")
    {
        MV_CLOGD("Incorrect input parameters");
        return false;
    }
    cv::Mat image = cv::imread(rImageFilePath.string());
    if (image.empty())
    {
        MV_CLOGD("Image reading failed");
        return false;
    }

    std::wstring wsCmd = L"TiCodec.exe c "
            + rImageFilePath.wstring() + L" "
            + rDstPath.wstring() + L" "
            + std::to_wstring(iDesiredSize);
    if (cFilter == 'D')
    {
        wsCmd += L" D";
    }
    else if (cFilter == 'B')
    {
        wsCmd += L" B";
    }
    if (executeCommand(wsCmd) == false)
    {
        return false;
    }

    if (params_.bRemoveIntermediates)
    {
        if (fs::remove(rImageFilePath) == false)
        {
            MV_CLOGD("Generated color image removing failed");
            return false;
        }
    }

    return true;
}

bool CIFCodec::extractStr(const std::filesystem::path& rDstPath,
                          const std::filesystem::path& rImageFilePath)
{
    if ((rDstPath.extension() != ".str") || !fs::exists(rImageFilePath))
    {
        MV_CLOGD("Incorrect input parameters");
        return false;
    }
    cv::Mat image = cv::imread(rImageFilePath.string());
    if (image.empty())
    {
        MV_CLOGD("Image reading failed");
        return false;
    }
    fs::path outputFilePath = rImageFilePath;
    outputFilePath.replace_filename("converted.tiff");
    std::vector<int> compressionParams = {
        TIFFTAG_COMPRESSION, COMPRESSION_NONE,
        TIFFTAG_ROWSPERSTRIP, image.rows
    };
    if (cv::imwrite(outputFilePath.string(), image, compressionParams) == false)
    {
        MV_CLOGD("Image conversion failed");
        return false;
    }

    std::wstring wsCmd = L"BTR.exe " + outputFilePath.wstring();
    bool bRunning = true;
    auto messagesCloser = [&bRunning]()
    {
        while(bRunning)
        {
            closeWindowByClassName(L"TMessageForm");
        }
    };
    std::thread thread(messagesCloser);
    if (executeCommand(wsCmd) == false)
    {
        return false;
    }
    bRunning = false;
    thread.join();

    if (params_.bRemoveIntermediates)
    {
        if (fs::remove(outputFilePath) == false)
        {
            MV_CLOGD("Generated image removing failed");
            return false;
        }
    }

    outputFilePath.replace_extension(".str");
    fs::rename(outputFilePath, rDstPath);

    return true;
}

bool CIFCodec::compressStr(const std::filesystem::path& rDstPath, const std::filesystem::path& rStrFilePath)
{
    if ((rDstPath.extension() != ".kmp")
        || (rStrFilePath.extension() != ".str")
        || !fs::exists(rStrFilePath))
    {
        MV_CLOGD("Incorrect input parameters");
        return false;
    }

    std::wstring wsCmd = L"kom_2017.exe " + rStrFilePath.wstring();

    if (executeCommand(wsCmd) == false)
    {
        return false;
    }

    fs::path kmpFilePath = rStrFilePath;
    kmpFilePath.replace_extension(".kmp");
    fs::rename(kmpFilePath, rDstPath);

    if (params_.bRemoveIntermediates)
    {
        if (fs::remove(rStrFilePath) == false)
        {
            MV_CLOGD("Generated str file removing failed");
            return false;
        }
    }

    return true;
}

bool CIFCodec::compressKmp(const fs::path& rDstPath, const fs::path& rKmpFilePath, const fs::path& rEtlFilePath)
{
    if (!fs::exists(rDstPath)
        || (rKmpFilePath.extension() != ".kmp")
        || !fs::exists(rKmpFilePath)
        || (rEtlFilePath.extension() != ".etl"))
    {
        MV_CLOGD("Incorrect input parameters for KMP compression:");
        MV_CLOGD(rDstPath);
        MV_CLOGD(rKmpFilePath);
        MV_CLOGD(rEtlFilePath);
        return false;
    }

    // files must be in the same directory as IFc.exe
    fs::path temporaryFolder("temporary_files");
    fs::path temporaryFolderAbs = getExeFolderW() / temporaryFolder;
    fs::path kmpFileName = rKmpFilePath.filename();

    fs::create_directory(temporaryFolderAbs);
    fs::copy(rKmpFilePath, temporaryFolderAbs, fs::copy_options::overwrite_existing);
    if (fs::exists(rEtlFilePath))
    {
        fs::copy(rEtlFilePath, temporaryFolderAbs, fs::copy_options::overwrite_existing);
    }

    std::wstring wsCmd = L"IFc.exe " + (temporaryFolder / kmpFileName).wstring()
            + L" " + (temporaryFolder / rEtlFilePath.filename()).wstring();
    if (executeCommand(wsCmd) == false)
    {
        return false;
    }

    fs::path outputFilePath;
    for(const auto& rEntity: fs::directory_iterator(temporaryFolder))
    {
        fs::path currentPath = rEntity.path();
        if (currentPath.extension() == ".etl")
        {
            if (outputFilePath.empty() || fs::file_size(outputFilePath) <= rEntity.file_size())
            {
                outputFilePath = std::move(currentPath);
            }
        }
    }
    kmpFileName.replace_extension(".etl");
    fs::path dstFilePath = rDstPath / kmpFileName;
    fs::rename(outputFilePath, dstFilePath);

    outputFilePath.replace_extension(".bds");
    dstFilePath.replace_extension(".bds");
    fs::rename(outputFilePath, dstFilePath);

    if (params_.bRemoveIntermediates)
    {
        if (fs::remove(rKmpFilePath) == false)
        {
            return false;
        }
    }

    return fs::remove_all(temporaryFolderAbs) > 0 ? true : false;
}

bool CIFCodec::compressFinal(const fs::path& rDstPath,
                             const fs::path& rBdsFilePath, const fs::path& rEtlFilePath,
                             const fs::path& rDarkColorFilePath, const fs::path& rLightColorFilePath)
{
    if ((rDstPath.extension() != ".CIF")
        || (rBdsFilePath.extension() != ".bds")
        || !fs::exists(rBdsFilePath)
        || (rEtlFilePath.extension() != ".etl")
        || !fs::exists(rEtlFilePath))
    {
        MV_CLOGD("Incorrect input parameters");
        return false;
    }

    std::wstring wsCmd = L"7z.exe a " + rDstPath.wstring()
            + L" " + rBdsFilePath.wstring()
            + L" " + rEtlFilePath.wstring();
    if (fs::exists(rDarkColorFilePath))
    {
        wsCmd += L" " + rDarkColorFilePath.wstring();
    }
    if (fs::exists(rLightColorFilePath))
    {
        wsCmd += L" " + rLightColorFilePath.wstring();
    }
    if (executeCommand(wsCmd) == false)
    {
        return false;
    }

    if (params_.bRemoveIntermediates)
    {
        if (fs::remove(rBdsFilePath) == false)
        {
            return false;
        }
        if (fs::remove(rEtlFilePath) == false)
        {
            return false;
        }
        if (fs::exists(rDarkColorFilePath))
        {
            if (fs::remove(rDarkColorFilePath) == false)
            {
                return false;
            }
        }
        if (fs::exists(rLightColorFilePath))
        {
            if (fs::remove(rLightColorFilePath) == false)
            {
                return false;
            }
        }
    }

    return true;
}

bool CIFCodec::decompressImage(const fs::path& rDstPath,
                               const fs::path& rCIFFilePath)
{
    if (rDstPath.empty()
        || (rCIFFilePath.extension() != ".CIF")
        || !fs::exists(rCIFFilePath))
    {
        MV_CLOGD("Incorrect input parameters for image decompession:");
        MV_CLOGD(rDstPath);
        MV_CLOGD(rCIFFilePath);
        return false;
    }
    fs::path outputFolderPath = rDstPath.parent_path();
    if (decompressCIF(outputFolderPath,
                      rCIFFilePath) == false)
    {
        return false;
    }

    fs::path outputDarkComponentPath;
    fs::path outputLightComponentPath;
    if (true)
    {
        const fs::path inputFilePath = outputFolderPath / rCIFFilePath.stem();
        fs::path inputComponentPath = inputFilePath;
        inputComponentPath.concat("_dark.Ti");
        outputDarkComponentPath = inputComponentPath;
        outputDarkComponentPath.replace_extension(".bmp");
        if (restoreColor(outputDarkComponentPath,
                         inputComponentPath) == false)
        {
            return false;
        }
        inputComponentPath = inputFilePath;
        inputComponentPath.concat("_light.Ti");
        outputLightComponentPath = inputComponentPath;
        outputLightComponentPath.replace_extension(".bmp");
        if (restoreColor(outputLightComponentPath,
                         inputComponentPath) == false)
        {
            return false;
        }
    }

    fs::path inputFilePath = rCIFFilePath;
    fs::path outputFilePath = rDstPath;
    outputFilePath.replace_extension(".kmr");
    {
        fs::path bdsFilePath = inputFilePath;
        bdsFilePath.replace_extension(".bds");
        fs::path etlFilePath = inputFilePath;
        etlFilePath.replace_extension(".etl");
        if (restoreKmp(outputFilePath,
                       bdsFilePath,
                       etlFilePath) == false)
        {
            return false;
        }
    }

    inputFilePath = outputFilePath;
    outputFilePath.replace_extension(".str");
    if (restoreStr(outputFilePath,
                   inputFilePath) == false)
    {
        return false;
    }

    inputFilePath = outputFilePath;
    outputFilePath.replace_extension(".raw");
    if (restoreRaw(outputFilePath,
                   inputFilePath) == false)
    {
        return false;
    }

    // TODO: add choice of color
    const fs::path& backgroundColorPath = outputLightComponentPath;
    const fs::path& foregroundColorPath = outputDarkComponentPath;
    if (restoreImage(rDstPath,
                     outputFilePath,
                     backgroundColorPath,
                     foregroundColorPath) == false)
    {
        return false;
    }

    return true;
}

bool CIFCodec::decompressCIF(const fs::path& rDstPath, const fs::path& rCIFFilePath)
{
    if (rDstPath.empty()
        || !fs::is_directory(rDstPath)
        || (rCIFFilePath.extension() != ".CIF")
        || !fs::exists(rCIFFilePath))
    {
        MV_CLOGD("Incorrect input parameters for CIF decompression");
        MV_CLOGD(rDstPath);
        MV_CLOGD(rCIFFilePath);
        return false;
    }

    std::wstring wsCmd = L"7z.exe e " + rCIFFilePath.wstring()
            + L" -o" + rDstPath.wstring();
    if (executeCommand(wsCmd) == false)
    {
        return false;
    }

    return true;
}

bool CIFCodec::restoreColor(const fs::path& rDstPath, const fs::path& rTiColorFilePath)
{
    if (rDstPath.empty()
        || (rTiColorFilePath.extension() != ".Ti")
        || !fs::exists(rTiColorFilePath))
    {
        MV_CLOGD("Incorrect input parameters");
        return false;
    }

    std::wstring wsCmd = L"TiCodec.exe d "
            + rTiColorFilePath.wstring() + L" "
            + rDstPath.wstring();
    if (executeCommand(wsCmd) == false)
    {
        return false;
    }

    if (params_.bRemoveIntermediates)
    {
        if (fs::remove(rTiColorFilePath) == false)
        {
            return false;
        }
    }

    return true;
}

bool CIFCodec::restoreKmp(const fs::path& rDstPath,
                          const fs::path& rBdsFilePath,
                          const fs::path& rEtlFilePath)
{
    if ((rDstPath.extension() != ".kmr")
        || (rBdsFilePath.extension() != ".bds")
        || !fs::exists(rBdsFilePath)
        || (rEtlFilePath.extension() != ".etl")
        || !fs::exists(rEtlFilePath))
    {
        MV_CLOGD("Incorrect input parameters:");
        MV_CLOGD(rDstPath);
        MV_CLOGD(rBdsFilePath);
        MV_CLOGD(rEtlFilePath);
        return false;
    }

    // files must be in the same directory as RestoreKmpAppl.exe
    fs::path temporaryFolder("temporary_files");
    fs::path temporaryFolderAbs = getExeFolderW() / temporaryFolder;
    fs::path etlFileName = rEtlFilePath.filename();

    fs::create_directory(temporaryFolderAbs);
    fs::copy(rBdsFilePath, temporaryFolderAbs, fs::copy_options::overwrite_existing);
    fs::copy(rEtlFilePath, temporaryFolderAbs, fs::copy_options::overwrite_existing);

    std::wstring wsCmd = L"RestoreKmpAppl.exe "
            + (temporaryFolder / rBdsFilePath.filename()).wstring() + L" "
            + (temporaryFolder / etlFileName).wstring() + L" ";
    if (executeCommand(wsCmd) == false)
    {
        return false;
    }

    etlFileName.replace_extension(".kmr");
    fs::rename(temporaryFolder / etlFileName, rDstPath);

    if (params_.bRemoveIntermediates)
    {
        if (fs::remove(rBdsFilePath) == false)
        {
            return false;
        }
        if (fs::remove(rEtlFilePath) == false)
        {
            return false;
        }
    }

    return fs::remove_all(temporaryFolderAbs) > 0 ? true : false;
}

bool CIFCodec::restoreStr(const fs::path& rDstPath, const fs::path& rKmrFilePath)
{
    if ((rDstPath.extension() != ".str")
        || (rKmrFilePath.extension() != ".kmr")
        || !fs::exists(rKmrFilePath))
    {
        MV_CLOGD("Incorrect input parameters:");
        MV_CLOGD(rDstPath);
        MV_CLOGD(rKmrFilePath);
        return false;
    }

    std::wstring wsCmd = L"RAK_2017.exe "
            + rKmrFilePath.wstring();
    if (executeCommand(wsCmd) == false)
    {
        return false;
    }

    fs::path outputStrFilePath = rKmrFilePath;
    outputStrFilePath.replace_extension(".str");
    fs::rename(outputStrFilePath, rDstPath);

    if (params_.bRemoveIntermediates)
    {
        if (fs::remove(rKmrFilePath) == false)
        {
            return false;
        }
    }

    return true;
}

bool CIFCodec::restoreRaw(const fs::path& rDstPath, const fs::path& rStrFilePath)
{
    if ((rDstPath.extension() != ".raw")
        || (rStrFilePath.extension() != ".str")
        || !fs::exists(rStrFilePath))
    {
        MV_CLOGD("Incorrect input parameters:");
        MV_CLOGD(rDstPath);
        MV_CLOGD(rStrFilePath);
        return false;
    }

    // files must be in the same directory as StrToRaw.exe
    fs::path temporaryFolder("temporary_files");
    fs::path temporaryFolderAbs = getExeFolderW() / temporaryFolder;
    fs::path strFileName = rStrFilePath.filename();

    fs::create_directory(temporaryFolderAbs);
    fs::copy(rStrFilePath, temporaryFolderAbs, fs::copy_options::overwrite_existing);

    std::wstring wsCmd = L"StrToRaw.exe "
            + (temporaryFolder / strFileName).wstring();
    if (executeCommand(wsCmd) == false)
    {
        return false;
    }

    strFileName.replace_extension(".raw");
    fs::rename(temporaryFolder / strFileName, rDstPath);

    if (params_.bRemoveIntermediates)
    {
        if (fs::remove(rStrFilePath) == false)
        {
            return false;
        }
    }

    return fs::remove_all(temporaryFolderAbs) > 0 ? true : false;
}

bool CIFCodec::restoreImage(const fs::path& rDstPath,
                            const fs::path& rRawImageFilePath,
                            const fs::path& rBackgroundColorFilePath,
                            const fs::path& rForegroundColorFilePath)
{
    if (rDstPath.empty()
        || (rRawImageFilePath.extension() != ".raw")
        || !fs::exists(rRawImageFilePath))
    {
        MV_CLOGD("Incorrect input parameters for image restoring:");
        MV_CLOGD(rDstPath);
        MV_CLOGD(rRawImageFilePath);
        MV_CLOGD(rBackgroundColorFilePath);
        MV_CLOGD(rForegroundColorFilePath);
        return false;
    }

    std::wstring wsCmd = L"RestoreImage.exe "
            + rDstPath.wstring() + L" "
            + rRawImageFilePath.wstring() + L" "
            + rBackgroundColorFilePath.wstring() + L" "
            + rForegroundColorFilePath.wstring();
    if (executeCommand(wsCmd) == false)
    {
        return false;
    }

    if (params_.bRemoveIntermediates)
    {
        if (fs::remove(rRawImageFilePath) == false)
        {
            return false;
        }
        if (fs::remove(rBackgroundColorFilePath) == false)
        {
            return false;
        }
        if (fs::remove(rForegroundColorFilePath) == false)
        {
            return false;
        }
    }

    return true;
}

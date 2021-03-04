#ifndef CIF_CODEC_H_
#define CIF_CODEC_H_

#include <filesystem>

class CIFCodec
{
public:
    CIFCodec() = default;
    ~CIFCodec() = default;

    struct Params
    {
        bool bRemoveIntermediates = true;
        bool bRemoveColorInfo = true;
        bool bCompressBg = false;
        bool bCompressFg = false;
        int iSizeBg = 0;
        int iSizeFg = 0;
        char cFilter = 'B';
    };

    Params& getParams() { return params_; }
    void setParams(const Params& rParams) { params_ = rParams; }

    bool KmpToCIF(const std::filesystem::path& rDstPath,
                  const std::filesystem::path& rKmpPath,
                  const std::filesystem::path& rEtlPath,
                  const std::filesystem::path& rDarkComponentPath = "",
                  const std::filesystem::path& rLightComponentPath = "");

    bool StrToCIF(const std::filesystem::path& rDstPath,
                  const std::filesystem::path& rStrPath,
                  const std::filesystem::path& rDarkComponentPath = "",
                  const std::filesystem::path& rLightComponentPath = "");

    bool ImageToCIF(const std::filesystem::path& rDstPath,
                    const std::filesystem::path& rImageFilePath);

    bool extractColor(const std::filesystem::path& rImageFilePath,
                      const std::filesystem::path& rDstDarkComponentPath,
                      const std::filesystem::path& rDstLightComponentPath);

    bool compressColor(const std::filesystem::path& rDstPath,
                       const std::filesystem::path& rImageFilePath,
                       int iDesiredSize, char cFilter);

    bool extractStr(const std::filesystem::path& rDstPath,
                    const std::filesystem::path& rImageFilePath);

    bool compressStr(const std::filesystem::path& rDstPath,
                     const std::filesystem::path& rStrFilePath);

    bool compressKmp(const std::filesystem::path& rDstPath,
                     const std::filesystem::path& rKmpFilePath,
                     const std::filesystem::path& rEtlFilePath);

    bool compressFinal(const std::filesystem::path& rDstPath,
                       const std::filesystem::path& rBdsFilePath,
                       const std::filesystem::path& rEtlFilePath,
                       const std::filesystem::path& rDarkColorFilePath = "",
                       const std::filesystem::path& rLightColorFilePath = "");

    bool CIFToKmp(const std::filesystem::path& rDstPath,
                  const std::filesystem::path& rCIFFilePath);

    bool CIFToStr(const std::filesystem::path& rDstPath,
                  const std::filesystem::path& rCIFFilePath);

    bool CIFToImage(const std::filesystem::path& rDstPath,
                    const std::filesystem::path& rCIFFilePath,
                    const std::filesystem::path& rBackgroundComponentPath = "",
                    const std::filesystem::path& rForegroundComponentPath = "");

    bool decompressCIF(const std::filesystem::path& rDstPath,
                       const std::filesystem::path& rCIFFilePath);

    bool restoreColor(const std::filesystem::path& rDstPath,
                      const std::filesystem::path& rTiColorFilePath);

    bool restoreKmp(const std::filesystem::path& rDstPath,
                    const std::filesystem::path& rBdsFilePath,
                    const std::filesystem::path& rEtlFilePath);

    bool restoreStr(const std::filesystem::path& rDstPath,
                    const std::filesystem::path& rKmrFilePath);

    bool restoreRaw(const std::filesystem::path& rDstPath,
                    const std::filesystem::path& rStrFilePath);

    bool restoreImage(const std::filesystem::path& rDstPath,
                      const std::filesystem::path& rRawImageFilePath,
                      const std::filesystem::path& rBackgroundColorFilePath = "",
                      const std::filesystem::path& rForegroundColorFilePath = "");

private:
    bool prepareColorComponent(std::filesystem::path& rResultColorPath,
                               const std::filesystem::path& rDstPath,
                               const std::filesystem::path& rCIFFilePath,
                               const std::filesystem::path& rColorComponentPath,
                               const std::string& sPostfix);
private:
    Params params_;
};

#endif // CIF_CODEC_H_

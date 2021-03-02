#include <QApplication>

#include "mainwindow.h"
#include "CIFCodec.h"
#include <opencv2/imgcodecs.hpp>
#include <filesystem>
#include <tiff.h>
#include <utils/Debug.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();

    CIFCodec codec;
    auto& rParams = codec.getParams();
    rParams.bCompressBg = true;
    rParams.bCompressFg = true;
    rParams.iSizeBg = 10240;
    rParams.iSizeFg = 102400;
    codec.ImageToCIF("C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\compressed.CIF",
                     "C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002n.tif");

//    codec.compressImage("C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\compressed.CIF",
//                              "C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\inv0002n.tiff");
//    codec.decompressImage("C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\restored.tiff",
//                              "C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\compressed.CIF");
    return 0;
    //    CIFCodec::decompressCIF("C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002n",
    //                            "C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002.CIF");
//    CIFCodec::restoreKmp("C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002\\0002.kmr",
//                         "C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002\\0002.bds",
//                         "C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002\\0002.etl");
//    CIFCodec::restoreStr("C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002\\0002.str",
//                         "C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002\\0002.kmr");
//    CIFCodec::restoreRaw("C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002\\0002.raw",
//                         "C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002\\0002.str");
//    CIFCodec::restoreColor("C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002\\0002_light.bmp",
//                         "C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002\\0002_light.Ti");
//    CIFCodec::restoreColor("C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002\\0002_dark.bmp",
//                         "C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002\\0002_dark.Ti");
//    CIFCodec::restoreImage("C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002\\0002_restored.tiff",
//                           "C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002\\0002.raw",
//                         "C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002\\0002_dark.bmp",
//                           "C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002\\0002_light.bmp");
//    return 0;
//    CIFCodec::compressImage("C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002.CIF",
//                            "C:\\Users\\Mikhail\\Desktop\\repo\\CIFCodec\\build\\Debug\\bin\\results\\0002n.tif");
//    return 0;

//    CIFCodec::compressImage("C:\\Users\\Mikhail\\Desktop\\repo\\IFcCodec\\build\\Debug\\bin\\results",
//                            //"C:\\Users\\Mikhail\\Desktop\\repo\\IFcCodec\\build\\Debug\\bin\\results/test.tiff");
//                            "C:/Users/Mikhail/Desktop/repo/IFcCodecConsole/test/0002n.tif");
//    return 0;
//    CIFCodec::compressStr("C:\\Users\\Mikhail\\Desktop\\repo\\IFcCodec\\build\\Debug\\bin\\results",
//                          "C:\\Users\\Mikhail\\Desktop\\repo\\IFcCodec\\build\\Debug\\bin\\results/0002n.str");

//    CIFCodec::compressKmp("C:\\Users\\Mikhail\\Desktop\\repo\\IFcCodec\\build\\Debug\\bin\\results",
//                          "C:\\Users\\Mikhail\\Desktop\\repo\\IFcCodec\\build\\Debug\\bin\\results\\0002n.kmp",
//                          "C:/Users/Mikhail/Desktop/repo/IFcCodecConsole/test/0002n.etl");

    //    IFcCodec::compressFile();
//    return a.exec();
    return 0;
}

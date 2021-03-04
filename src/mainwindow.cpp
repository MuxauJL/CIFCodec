#include "mainwindow.h"

#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QLineEdit>

namespace fs = std::filesystem;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , pUi(new Ui::MainWindow)
{
    pUi->setupUi(this);
    pUi->bg_check_box->setChecked(false);
    pUi->fg_check_box->setChecked(false);
    pUi->bg_size_text_line->setEnabled(false);
    pUi->fg_size_text_line->setEnabled(false);
    pUi->color_group->setEnabled(false);

    pColorSizeValidator = new QIntValidator(this);
    constexpr int iMinimalColorFileSize = 28;
    pColorSizeValidator->setBottom(iMinimalColorFileSize);
    pUi->bg_size_text_line->setValidator(pColorSizeValidator);
    pUi->fg_size_text_line->setValidator(pColorSizeValidator);
}

MainWindow::~MainWindow()
{
    delete pColorSizeValidator;
    delete pUi;
}

QString MainWindow::getInputForCompression()
{
    QString sFileName = QFileDialog::getOpenFileName(
        this, tr("Выберите файл для сжатия"), "",
        tr("Все файлы (*.*);;"
        "Изображения (*.tif *tiff *.png *.jpg *jpeg *.bmp);;"
        "STR файлы (*.str);;"
        "KMP файлы (*.kmp *.kmr)"));
    return sFileName;
}

QString MainWindow::getOutputForCompression()
{
    QString sFileName = QFileDialog::getSaveFileName(
        this, tr("Выберите выходной файл"), "",
        tr("CIF файлы (*.CIF)"));
    return sFileName;
}

QString MainWindow::getInputForDecompression()
{
    QString sFileName = QFileDialog::getOpenFileName(
        this, tr("Выберите файл для восстановления"), "",
        tr("CIF файлы (*.CIF)"));
    return sFileName;
}

QString MainWindow::getOutputForDecompression()
{
    QString sFileName = QFileDialog::getSaveFileName(
        this, tr("Выберите выходное изображение"), "",
        tr("Изображения (*.tif *tiff *.png *.jpg *jpeg *.bmp);;"
        "KMP файлы (*.kmr);;"
        "STR файлы (*.str)"));
    return sFileName;
}

void MainWindow::on_choose_input_file_triggered()
{
    QWidget* pCurrentTab = pUi->mode_tabs->currentWidget();
    if (pCurrentTab == pUi->compression_tab)
    {
        QString sInputFileName = getInputForCompression();
        if (!sInputFileName.isEmpty())
        {
            QLineEdit* pTextLine = pCurrentTab->findChild<QLineEdit*>("compression_input_text_line");
            if (pTextLine)
            {
                pTextLine->setText(sInputFileName);
            }
            int iFileSize = fs::file_size(sInputFileName.toStdWString());
            pColorSizeValidator->setTop(iFileSize);
        }
    }
    else if (pCurrentTab == pUi->decompression_tab)
    {
        QString sInputFileName = getInputForDecompression();
        if (!sInputFileName.isEmpty())
        {
            QLineEdit* pTextLine = pCurrentTab->findChild<QLineEdit*>("decompression_input_text_line");
            if (pTextLine)
            {
                pTextLine->setText(sInputFileName);
            }
        }
    }
}

void MainWindow::on_choose_output_file_triggered()
{
    QWidget* pCurrentTab = pUi->mode_tabs->currentWidget();
    if (pCurrentTab == pUi->compression_tab)
    {
        QString sOutputFileName = getOutputForCompression();
        QLineEdit* pTextLine = pCurrentTab->findChild<QLineEdit*>("compression_output_text_line");
        if (pTextLine)
        {
            pTextLine->setText(sOutputFileName);
        }
    }
    else if (pCurrentTab == pUi->decompression_tab)
    {
        QString sOutputFileName = getOutputForDecompression();
        QLineEdit* pTextLine = pCurrentTab->findChild<QLineEdit*>("decompression_output_text_line");
        if (pTextLine)
        {
            pTextLine->setText(sOutputFileName);
        }
    }
}

QString MainWindow::getColorForDecompression()
{
    QString sFileName = QFileDialog::getOpenFileName(
        this, tr("Выберите файл, содержащий информацию о цвете"), "",
        tr("Изображения (*.tif *tiff *.png *.jpg *jpeg *.bmp)"));
    return sFileName;
}

void MainWindow::on_choose_bg_color_file_triggered()
{
    QString sColorFileName = getColorForDecompression();
    QLineEdit* pTextLine = pUi->decompression_tab->findChild<QLineEdit*>("decompression_bg_color_text_line");
    if (pTextLine)
    {
        pTextLine->setText(sColorFileName);
    }
}

void MainWindow::on_choose_fg_color_file_triggered()
{
    QString sColorFileName = getColorForDecompression();
    QLineEdit* pTextLine = pUi->decompression_tab->findChild<QLineEdit*>("decompression_fg_color_text_line");
    if (pTextLine)
    {
        pTextLine->setText(sColorFileName);
    }
}

void MainWindow::on_color_check_box_stateChanged(int iState)
{
    if (iState == Qt::Unchecked)
    {
        pUi->color_group->setEnabled(false);
    }
    else if (iState == Qt::Checked)
    {
        pUi->color_group->setEnabled(true);
    }
}

void MainWindow::on_bg_check_box_stateChanged(int iState)
{
    if (iState == Qt::Unchecked)
    {
        pUi->bg_size_text_line->setEnabled(false);
    }
    else if (iState == Qt::Checked)
    {
        pUi->bg_size_text_line->setEnabled(true);
    }
}

void MainWindow::on_fg_check_box_stateChanged(int iState)
{
    if (iState == Qt::Unchecked)
    {
        pUi->fg_size_text_line->setEnabled(false);
    }
    else if (iState == Qt::Checked)
    {
        pUi->fg_size_text_line->setEnabled(true);
    }
}

void MainWindow::on_compress_button_clicked()
{
    fs::path inputFilePath(pUi->compression_input_text_line->text().toStdWString());
    fs::path outputCIFPath(pUi->compression_output_text_line->text().toStdWString());
    auto& rParams = cifCodec_.getParams();
    if (pUi->color_check_box->isChecked())
    {
        rParams.bCompressBg = pUi->bg_check_box->isChecked();
        rParams.bCompressFg = pUi->fg_check_box->isChecked();
        if (rParams.bCompressBg)
        {
            rParams.iSizeBg = pUi->bg_size_text_line->text().toInt();
        }
        if (rParams.bCompressFg)
        {
            rParams.iSizeFg = pUi->fg_size_text_line->text().toInt();
        }
        int iFilterIndex = pUi->filter_combo_box->currentIndex();
        if (iFilterIndex == 0)
        {
            rParams.cFilter = 'B';
        }
        else
        {
            rParams.cFilter = 'D';
        }
    }
    else
    {
        rParams.bCompressBg = false;
        rParams.bCompressFg = false;
    }

    pUi->statusbar->showMessage("Выполняется сжатие");
    const auto inputFileExtension = inputFilePath.extension();
    if (inputFileExtension == ".kmp")
    {
        fs::path etlFileName = outputCIFPath;
        etlFileName.replace_extension(".etl");
        if (cifCodec_.KmpToCIF(outputCIFPath, inputFilePath, etlFileName) == false)
        {
            pUi->statusbar->showMessage("Произошла ошибка при сжатии KMP файла");
            return;
        }
    }
    else if (inputFileExtension == ".str")
    {
        if (cifCodec_.StrToCIF(outputCIFPath, inputFilePath) == false)
        {
            pUi->statusbar->showMessage("Произошла ошибка при сжатии STR файла");
            return;
        }
    }
    else
    {
        if (cifCodec_.ImageToCIF(outputCIFPath, inputFilePath) == false)
        {
            pUi->statusbar->showMessage("Произошла ошибка при сжатии изображения");
            return;
        }
    }
    pUi->statusbar->showMessage("Сжатие успешно завершено");
}

void MainWindow::on_decompress_button_clicked()
{
    fs::path inputCIFPath(pUi->decompression_input_text_line->text().toStdWString());
    fs::path outputFilePath(pUi->decompression_output_text_line->text().toStdWString());

    pUi->statusbar->showMessage("Выполняется восстановление");
    const auto outputFileExtension = outputFilePath.extension();
    if (outputFileExtension == ".kmr")
    {
        if (cifCodec_.CIFToKmp(outputFilePath, inputCIFPath) == false)
        {
            pUi->statusbar->showMessage("Произошла ошибка при восстановлении KMP файла");
            return;
        }
    }
    else if (outputFileExtension == ".str")
    {
        if (cifCodec_.CIFToStr(outputFilePath, inputCIFPath) == false)
        {
            pUi->statusbar->showMessage("Произошла ошибка при восстановлении STR файла");
            return;
        }
    }
    else
    {
        auto& rParams = cifCodec_.getParams();
        rParams.bRemoveColorInfo = true;
        fs::path backgroundColorPath(pUi->decompression_bg_color_text_line->text().toStdWString());
        if (fs::exists(backgroundColorPath))
        {
            rParams.bRemoveColorInfo = false;
        }
        else
        {
            backgroundColorPath = "";
        }
        fs::path foregroundColorPath(pUi->decompression_fg_color_text_line->text().toStdWString());
        if (fs::exists(foregroundColorPath))
        {
            rParams.bRemoveColorInfo = false;
        }
        else
        {
            foregroundColorPath = "";
        }
        if (cifCodec_.CIFToImage(outputFilePath, inputCIFPath,
                                 backgroundColorPath, foregroundColorPath) == false)
        {
            pUi->statusbar->showMessage("Произошла ошибка при восстановлении изображения");
            return;
        }
    }
    pUi->statusbar->showMessage("Восстановление успешно завершено");
}

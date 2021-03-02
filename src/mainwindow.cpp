#include "mainwindow.h"

#include <limits>

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

    pColorSizeValidator = new QIntValidator(0, std::numeric_limits<int>::max(), this);
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
        tr("CIF файлы (*.CIF);;"
        "KMP файлы (*.kmp);;"
        "STR файлы (*.str)"));
    return sFileName;
}

QString MainWindow::getOutputForDecompression()
{
    QString sFileName = QFileDialog::getSaveFileName(
        this, tr("Выберите выходное изображение"), "",
        tr("Изображения (*.tif *tiff *.png *.jpg *jpeg *.bmp)"));
    return sFileName;
}

void MainWindow::on_choose_input_file_triggered()
{
    QWidget* pCurrentTab = pUi->mode_tabs->currentWidget();
    if (pCurrentTab == pUi->compression_tab)
    {
        QString sInputFileName = getInputForCompression();
        QLineEdit* pTextLine = pCurrentTab->findChild<QLineEdit*>("compression_input_text_line");
        if (pTextLine)
        {
            pTextLine->setText(sInputFileName);
        }
    }
    else if (pCurrentTab == pUi->decompression_tab)
    {
        QString sInputFileName = getInputForDecompression();
        QLineEdit* pTextLine = pCurrentTab->findChild<QLineEdit*>("decompression_input_text_line");
        if (pTextLine)
        {
            pTextLine->setText(sInputFileName);
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
    fs::path inputFilePath(pUi->compression_input_text_line->text().toStdString());
    fs::path outputCIFPath(pUi->compression_output_text_line->text().toStdString());
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

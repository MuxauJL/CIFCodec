#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QIntValidator>

#include "CIFCodec.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QString getInputForCompression();
    QString getOutputForCompression();
    QString getInputForDecompression();
    QString getOutputForDecompression();
    QString getColorForDecompression();

private slots:

    void on_choose_input_file_triggered();

    void on_choose_output_file_triggered();

    void on_color_check_box_stateChanged(int arg1);

    void on_bg_check_box_stateChanged(int arg1);

    void on_fg_check_box_stateChanged(int arg1);

    void on_compress_button_clicked();

    void on_decompress_button_clicked();

    void on_choose_bg_color_file_triggered();

    void on_choose_fg_color_file_triggered();

private:
    Ui::MainWindow *pUi;
    QIntValidator *pColorSizeValidator;
    CIFCodec cifCodec_;
};
#endif // MAINWINDOW_H

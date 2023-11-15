#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QProcess>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class UsbListener;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void readConfig();



private slots:
    void on_pushButton_clicked();
    void detect_usb_arrival();
    void detect_usb_remove();
    void readAppVersionOutput();
    void readAppVersionErrOutput();


private:
    Ui::MainWindow *ui;
    UsbListener *usb_listener{nullptr};
    QString andriod_path;
    QString work_path;
    QString adb_absolute_path;

    QProcess *app_version_process{nullptr};

    qint64 app_ver{0};
};
#endif // MAINWINDOW_H

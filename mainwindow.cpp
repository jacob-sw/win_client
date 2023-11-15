#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "usblistener.h"
#include <QDir>
#include <QSettings>
#include <QGraphicsDropShadowEffect>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    QGraphicsDropShadowEffect *shadow_upload = new QGraphicsDropShadowEffect(this);
    shadow_upload->setOffset(5,5);
    shadow_upload->setColor(QColor(43, 43, 43));
    shadow_upload->setBlurRadius(8);
    //ui->pushButton_upload->setGraphicsEffect(shadow_upload);


    QGraphicsDropShadowEffect *shadow_download = new QGraphicsDropShadowEffect(this);
    shadow_download->setOffset(5,5);
    shadow_download->setColor(QColor(43, 43, 43));
    shadow_download->setBlurRadius(8);
    //ui->pushButton_download->setGraphicsEffect(shadow_download);

    usb_listener = new UsbListener(this);
    connect(usb_listener, &UsbListener::usbConn,
            this, &MainWindow::detect_usb_arrival);

    connect(usb_listener, &UsbListener::usbRemove,
            this, &MainWindow::detect_usb_remove);

    qApp->installNativeEventFilter(usb_listener);

    work_path = QDir::currentPath();
    readConfig();
    adb_absolute_path = work_path + "/third-tools/adb.exe";

    app_version_process = new QProcess(this);
    connect(app_version_process, &QProcess::readyReadStandardOutput,
            this, &MainWindow::readAppVersionOutput);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::readConfig()
{
    QSettings config(work_path + "/" + "config/config.ini", QSettings::IniFormat);
    andriod_path = config.value("setting/andriod_path", "").toString();
}





void MainWindow::detect_usb_arrival()
{
    qDebug() << "detect_usb_arrival";
    //app_version_process->start(adb_absolute_path, QStringList()  << "devices");
}

void MainWindow::detect_usb_remove()
{
    qDebug() << "detect_usb_remove";
}



void MainWindow::on_pushButton_clicked()
{
    qDebug() << "on_pushButton_clicked";
    qDebug() << "adb_absolute_path:" << adb_absolute_path;
    app_version_process->start(work_path + "/scripts/get_app_ver.bat", QStringList() << adb_absolute_path);
}



void MainWindow::readAppVersionOutput()
{
    QString out = app_version_process->readAllStandardOutput();
    qDebug() << "readAllStandardOutput:" << out;
}


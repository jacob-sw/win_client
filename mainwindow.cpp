#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "usblistener.h"
#include <QDir>
#include <QSettings>
#include <QGraphicsDropShadowEffect>
#include <QThread>
#include <QTimer>


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
    ui->pushButton_upload->setGraphicsEffect(shadow_upload);


    QGraphicsDropShadowEffect *shadow_download = new QGraphicsDropShadowEffect(this);
    shadow_download->setOffset(5,5);
    shadow_download->setColor(QColor(43, 43, 43));
    shadow_download->setBlurRadius(8);
    ui->pushButton_download->setGraphicsEffect(shadow_download);

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

    connect(app_version_process, &QProcess::readyReadStandardError,
            this, &MainWindow::readAppVersionErrOutput);


    on_pushButton_clicked();
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
    on_pushButton_clicked();
}

void MainWindow::detect_usb_remove()
{
    qDebug() << "detect_usb_remove";
    on_pushButton_clicked();
}



void MainWindow::on_pushButton_clicked()
{
    ui->lineEdit_app_conn->setStyleSheet("color:black");
    ui->lineEdit_app_conn->setText(tr("检测中......."));

    qDebug() << "adb_absolute_path:" << adb_absolute_path;
    QTimer::singleShot(1000, this, [this](){
        this->app_version_process->start(work_path + "/scripts/get_app_ver.bat", QStringList() << adb_absolute_path);
    });
}



void MainWindow::readAppVersionOutput()
{
    QString out = app_version_process->readAllStandardOutput();
    qDebug() << "readAppVersionOutput:" << out;
    qsizetype start_index = out.indexOf('=');
    QString app_ver_str = out.sliced(start_index + 1);
    qsizetype first_space_index = app_ver_str.indexOf(" ");

    app_ver = app_ver_str.sliced(0, first_space_index).toInt();
    ui->lineEdit_app_conn->setStyleSheet("color:blue");
    ui->lineEdit_app_conn->setText(QString(tr("连接APP成功,APP版本:V%1")).arg(app_ver));
}

void MainWindow::readAppVersionErrOutput()
{
    QString err_out = app_version_process->readAllStandardError();
    qDebug() << "readAppVersionErrOutput:" << err_out;
    ui->lineEdit_app_conn->setText(tr("连接失败......."));
    ui->lineEdit_app_conn->setStyleSheet("color:red");
}





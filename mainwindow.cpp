#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "usblistener.h"
#include <QDir>
#include <QSettings>
#include <QGraphicsDropShadowEffect>
#include <QThread>
#include <QTimer>
#include "setconfig.h"
#include <QMessageBox>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include "progressdialog.h"
#include <QHttpMultiPart>



QString MainWindow::task_download_path = "/api/windows/task/download";



static std::optional<QJsonObject> byteArrayToJsonObject(const QByteArray& data)
{
    QJsonParseError parseError;
    const auto json = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error) {
        qDebug() << "Response data not JSON:" << parseError.errorString()
                 << "at" << parseError.offset << data;
    }
    return json.isObject() ? json.object() : std::optional<QJsonObject>(std::nullopt);
}


static std::unique_ptr<QFile> openFileForWrite(const QString &fileName)
{
    std::unique_ptr<QFile> file = std::make_unique<QFile>(fileName);
    if (!file->open(QIODevice::WriteOnly))
    {
        return nullptr;
    }
    return file;
}


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
    adb_absolute_path = work_path + "/third-tools/adb.exe";
    app_version_process = new QProcess(this);
    app_upgrade_process = new QProcess(this);
    task_download_process = new QProcess(this);
    task_upload_process = new QProcess(this);
    delete_app_uploadfile_process = new QProcess(this);

    connect(app_version_process, &QProcess::readyReadStandardOutput,
            this, &MainWindow::readAppVersionOutput);
    connect(app_version_process, &QProcess::readyReadStandardError,
            this, &MainWindow::readAppVersionErrOutput);
    connect(app_version_process, &QProcess::finished,
            this, &MainWindow::readAppVersionFinish);


    connect(app_upgrade_process, &QProcess::readyReadStandardOutput,
            this, &MainWindow::readAppUpgradeOutput);
    connect(app_upgrade_process, &QProcess::readyReadStandardError,
            this, &MainWindow::readAppUpgradeErrOutput);
    connect(app_upgrade_process, &QProcess::finished,
            this, &MainWindow::upgradeFinish);


    connect(task_download_process, &QProcess::readyReadStandardOutput,
            this, &MainWindow::downloadOutput);
    connect(task_download_process, &QProcess::readyReadStandardError,
            this, &MainWindow::downloadErrOutput);
    connect(task_download_process, &QProcess::finished,
            this, &MainWindow::downloadFinish);

    connect(task_upload_process, &QProcess::readyReadStandardOutput,
            this, &MainWindow::uploadOutput);
    connect(task_upload_process, &QProcess::readyReadStandardError,
            this, &MainWindow::uploadErrOutput);
    connect(task_upload_process, &QProcess::finished,
            this, &MainWindow::uploadFinish);


    connect(delete_app_uploadfile_process, &QProcess::readyReadStandardOutput,
            this, &MainWindow::deleteOutput);
    connect(delete_app_uploadfile_process, &QProcess::readyReadStandardError,
            this, &MainWindow::deleteErrOutput);
    connect(delete_app_uploadfile_process, &QProcess::finished,
            this, &MainWindow::deleteFinish);


    config = new SetConfig(this);
    connect(config, &SetConfig::configChange,
            this, &MainWindow::configChanged);



    QTimer::singleShot(1000, this, [this](){
        on_pushButton_clicked();
        on_pushButton_test_serv_conn_clicked();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
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
    app_conn_success = true;
    app_is_installed = true;
}

void MainWindow::readAppVersionErrOutput()
{
    QString err_out = app_version_process->readAllStandardError();
    qDebug() << "readAppVersionErrOutput:" << err_out;
    ui->lineEdit_app_conn->setText(tr("连接失败......."));
    ui->lineEdit_app_conn->setStyleSheet("color:red");
    app_conn_success = false;
    app_version_process_already_error = true;
    app_ver = 0;
}


void MainWindow::readAppVersionFinish(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "readAppVersionFinish,exitCode:" << exitCode << ",exitStatus:" << exitStatus;
    if(app_version_process_already_error)
    {
        app_version_process_already_error = false;
        return;
    }

    if(0 != exitCode && exitStatus == QProcess::NormalExit)
    {
        ui->lineEdit_app_conn->setText(tr("移动端未安装电缆运维APP，请点击升级按钮进行安装...."));
        ui->lineEdit_app_conn->setStyleSheet("color:red");
        app_conn_success = true;
        app_is_installed = false;
        app_ver = 0;
    }
}


void MainWindow::on_pushButton_set_clicked()
{
    config->show();
}


void MainWindow::configChanged()
{
    on_pushButton_test_serv_conn_clicked();
}


void MainWindow::on_pushButton_test_serv_conn_clicked()
{
    QUrl url(config->getServUrl());
    if(!url.isValid())
    {
        QMessageBox::warning(this, tr(""), tr("服务端地址未配置或无效，请点击设置按钮进行配置"));
        return;
    }

    ui->lineEdit_serv_conn->setText("连接服务器中....");
    ui->lineEdit_serv_conn->setStyleSheet("color:black");

    url.setPath(config->getServApiPath());
    QUrlQuery paras;
    paras.addQueryItem("systemType", "1");
    url.setQuery(paras);
    auto request = QNetworkRequest(url);

    QNetworkReply* reply = m_accessManager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, reply, [reply,this](){
        int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        bool isReplyError = (reply->error() != QNetworkReply::NoError);
        if (isReplyError || !(httpStatusCode >= 200 && httpStatusCode < 300))
        {
            qDebug() << "Error" << reply->error();
            ui->lineEdit_serv_conn->setText(QString("连接服务器失败:%1....").arg(reply->errorString()));
            ui->lineEdit_serv_conn->setStyleSheet("color:red");
            serv_conn_success = false;
            return;
        }

        QByteArray all_data = reply->readAll();
        qDebug() << "on_pushButton_test_serv_conn_clicked get data:" << QString::fromLocal8Bit(all_data);
        std::optional<QJsonObject> json = byteArrayToJsonObject(all_data);
        if (json)
        {
            bool success = json->value("success").toBool();
            QString msg = json->value("msg").toString();
            if(!success)
            {
                ui->lineEdit_serv_conn->setText(QString("连接服务器失败:%1").arg(msg));
                ui->lineEdit_serv_conn->setStyleSheet("color:red");
                serv_conn_success = false;
                delete reply;
                return;
            }

            QJsonObject data = json->value("data").toObject();
            serv_ver = data.value("versionCode").toInt();
            latest_apk_path = data.value("systemFilePath").toString();
            ui->lineEdit_serv_conn->setText(QString("连接服务器成功,移动端最新版本:%1").arg(serv_ver));
            ui->lineEdit_serv_conn->setStyleSheet("color:blue");
            qDebug() << "versionCode is:" << serv_ver << ",systemFilePath is:" << latest_apk_path;
            serv_conn_success = true;
        }
        else
        {
            ui->lineEdit_serv_conn->setText("连接服务器失败....");
            ui->lineEdit_serv_conn->setStyleSheet("color:red");
            serv_conn_success = false;
        }

        delete reply;
    });
}


void MainWindow::on_pushButton_upgrade_clicked()
{
    on_pushButton_clicked();
    on_pushButton_test_serv_conn_clicked();


    if(!app_conn_success)
    {
        QMessageBox::warning(this, tr(""), tr("APP连接失败,请确认USB线是否已经连接好"));
        return;
    }

    if(!serv_conn_success || latest_apk_path.isEmpty())
    {
        QMessageBox::warning(this, tr(""), tr("服务端连接失败,请点击设置按钮并确认服务端配置是否正确"));
        return;
    }

    upgrade_start_time = QDateTime::currentDateTime();

    ui->textEdit->clear();
    ui->textEdit->append(QString("开始准备升级.启动时间: %1\n"
                                 "正在获取服务端最新版本……获取成功。最新版本值：%2\n"
                                 "正在获取移动端当前版本……获取成功。当前版本值：%3")
                             .arg(upgrade_start_time.toString("yyyy-MM-dd HH:mm:ss"))
                             .arg(serv_ver)
                             .arg(app_ver));

    if(serv_ver == app_ver)
    {
        ui->textEdit->append(tr("经版本值对比：当前已是最新版，无需升级"));
        return;
    }

    ui->textEdit->append(tr("经版本值对比：存在新版本，需要升级"));
    ui->textEdit->append(tr("正在从服务端下载新版本APK文件"));

    QUrl newUrl(config->getServUrl() + "/" + latest_apk_path);
    QString fileName = newUrl.fileName();
    fileName.prepend(work_path + "/temp/");

    upgrade_apk_abs_path = fileName;
    if (QFile::exists(fileName))
    {
        QFile::remove(fileName);
    }

    file = openFileForWrite(fileName);
    if (!file)
    {
        ui->textEdit->append(QString("打开文件失败,文件路径:%1").arg(fileName));
        return;
    }

    is_upgrade_process = true;
    ui->pushButton_upgrade->setEnabled(false);
    startRequest(newUrl);
}



void MainWindow::cancelDownload()
{
    httpRequestAborted = true;
    reply->abort();
}


void MainWindow::httpReadyRead()
{
    if (file)
        file->write(reply->readAll());
}


void MainWindow::processRedirected(const QUrl &requestedUrl)
{
    QUrlQuery query(requestedUrl);
    QString temp = query.queryItemValue("filePath");
    QString fileName = temp.sliced(temp.lastIndexOf('/') + 1);
    download_instruction_file_abs_path = work_path + "/temp/" + fileName;

    qDebug() << "processRedirected, requestedUrl is:" << requestedUrl;
    qDebug() << "download_instruction_file_abs_path is:" << download_instruction_file_abs_path;
    if (QFile::exists(download_instruction_file_abs_path))
    {
        QFile::remove(download_instruction_file_abs_path);
    }

    file = openFileForWrite(download_instruction_file_abs_path);
    if (!file)
    {
        ui->textEdit->append(QString("打开文件失败,文件路径:%1").arg(fileName));
        return;
    }
}



void MainWindow::httpUpLoadFinished()
{
    QNetworkReply::NetworkError error = reply->error();
    const QString &errorString = reply->errorString();

    if(upload_file->isOpen())
    {
        upload_file->close();
    }
    upload_file->reset();

    if (error != QNetworkReply::NoError)
    {
        qDebug() << "httpUpLoadFinished with error:" << errorString;
        ui->textEdit->append(QString("文件%1上传失败:%2")
                                 .arg(zip_file_name, errorString));
        reply.reset();
        QFile::remove(work_path + "/temp/" + zip_file_name);
        ui->pushButton_upload->setEnabled(true);
        return;
    }

    QByteArray all_data = reply->readAll();
    qDebug() << "httpUpLoadFinished all_data is:" << QString::fromLocal8Bit(all_data);
    std::optional<QJsonObject> json = byteArrayToJsonObject(all_data);
    bool success = json->value("success").toBool();
    QString msg = json->value("msg").toString();
    if(!success)
    {
        ui->textEdit->append(QString("文件%1上传失败:%2")
                                 .arg(zip_file_name, msg));
        reply.reset();
        QFile::remove(work_path + "/temp/" + zip_file_name);
        ui->pushButton_upload->setEnabled(true);
        return;
    }

    reply.reset();

    QString delete_path = work_path + "/temp/" + zip_file_name;
    QFile::remove(delete_path);

    ui->textEdit->append(QString("上传完成"));
    ui->textEdit->append(QString("删除移动端文件,避免重复上传...."));

    this->delete_app_uploadfile_process->start(work_path + "/scripts/delete_upload_file.bat", QStringList()
        << adb_absolute_path
        << config->getAppPath() + "/" + zip_file_name);
}

void MainWindow::httpFinished()
{
    QFileInfo fi;
    if (file) {
        fi.setFile(file->fileName());
        file->close();
        file.reset();
    }

    QNetworkReply::NetworkError error = reply->error();
    const QString &errorString = reply->errorString();
    reply.reset();
    if (error != QNetworkReply::NoError)
    {
        QFile::remove(fi.absoluteFilePath());
        if (!httpRequestAborted)
        {
            ui->textEdit->append(QString("文件%1下载失败:%2")
                                     .arg(fi.fileName(), errorString));
        }
        else
        {
            ui->textEdit->append(QString("用户取消文件%1下载").arg(fi.fileName()));
        }

        if(is_upgrade_process)
        {
            is_upgrade_process = false;
            ui->pushButton_upgrade->setEnabled(true);
        }

        if(is_task_download_process)
        {
            is_task_download_process = false;
            ui->pushButton_download->setEnabled(true);
        }
        return;
    }

    ui->textEdit->append(QString("文件%1下载成功").arg(fi.fileName()));
    if(is_upgrade_process)
    {
        ui->textEdit->append(QString("正在执行升级APP命令……"));
        this->app_upgrade_process->start(work_path + "/scripts/install_app.bat", QStringList() << adb_absolute_path
            << work_path + "/temp/" + fi.fileName());
    }

    if(is_task_download_process)
    {
        ui->textEdit->append(QString("正在写入移动端目录……"));
        this->task_download_process->start(work_path + "/scripts/download_instruction.bat", QStringList()
            << adb_absolute_path
            <<  config->getAppPath()
            << download_instruction_file_abs_path);
    }
}


void MainWindow::readAppUpgradeOutput()
{
    QString out = app_upgrade_process->readAllStandardOutput();
    //QString::contains(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    if(out.contains("pushed", Qt::CaseInsensitive))
    {
        QMessageBox::information(this, tr(""), tr("请在移动端操作安装电缆运维APP"));
    }

    ui->textEdit->append(out);
}


void MainWindow::readAppUpgradeErrOutput()
{
    ui->textEdit->append(app_upgrade_process->readAllStandardError());
}


void MainWindow::upgradeFinish(int exitCode, QProcess::ExitStatus exitStatus)
{
    upgrade_end_time = QDateTime::currentDateTime();
    if(0 == exitCode && exitStatus == QProcess::NormalExit)
    {
        ui->textEdit->append("升级成功.");
        on_pushButton_clicked();
    }
    else
    {
        ui->textEdit->append("升级失败.");
    }

    ui->textEdit->append(QString("完成时间: %1,耗时:%2毫秒")
                             .arg(upgrade_end_time.toString("yyyy-MM-dd HH:mm:ss"))
                             .arg(upgrade_start_time.msecsTo(upgrade_end_time)));

    is_upgrade_process = false;
    ui->pushButton_upgrade->setEnabled(true);
    QFile::remove(upgrade_apk_abs_path);
}




void MainWindow::downloadOutput()
{
    QString out = task_download_process->readAllStandardOutput();
    ui->textEdit->append(out);
}

void MainWindow::downloadErrOutput()
{
    QString err = task_download_process->readAllStandardError();
    ui->textEdit->append(err);
}

void MainWindow::downloadFinish(int exitCode, QProcess::ExitStatus exitStatus)
{
    task_download_end_time = QDateTime::currentDateTime();
    if(0 == exitCode && exitStatus == QProcess::NormalExit)
    {
        ui->textEdit->append("写入成功");
        ui->textEdit->append("下载指令结束,请在APP端读取指令文件");
    }
    else
    {
        ui->textEdit->append("写入失败");
    }

    ui->textEdit->append(QString("完成时间: %1,耗时:%2毫秒")
                             .arg(task_download_end_time.toString("yyyy-MM-dd HH:mm:ss"))
                             .arg(task_download_start_time.msecsTo(task_download_end_time)));

    is_task_download_process = false;
    ui->pushButton_download->setEnabled(true);
    QFile::remove(download_instruction_file_abs_path);
}


void MainWindow::deleteOutput()
{
    QString out = delete_app_uploadfile_process->readAllStandardOutput();
    ui->textEdit->append(out);
}

void MainWindow::deleteErrOutput()
{
    QString out = delete_app_uploadfile_process->readAllStandardError();
    ui->textEdit->append(out);
}

void MainWindow::deleteFinish(int exitCode, QProcess::ExitStatus exitStatus)
{
    task_upload_end_time = QDateTime::currentDateTime();
    if(0 == exitCode && exitStatus == QProcess::NormalExit)
    {
        ui->textEdit->append("删除成功");
    }
    else
    {
        ui->textEdit->append(QString("移动端删除文件:%1失败").arg(zip_file_name));
    }

    ui->textEdit->append(QString("上传任务结束.\n完成时间: %1,耗时:%2毫秒")
                             .arg(task_upload_end_time.toString("yyyy-MM-dd HH:mm:ss"))
                             .arg(task_upload_start_time.msecsTo(task_upload_end_time)));

    ui->pushButton_upload->setEnabled(true);
}


void MainWindow::uploadOutput()
{
    QString out = task_upload_process->readAllStandardOutput();
    ui->textEdit->append(out);
}

void MainWindow::uploadErrOutput()
{
    QString err = task_upload_process->readAllStandardError();
    ui->textEdit->append(err);
}

void MainWindow::uploadFinish(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(0 == exitCode && exitStatus == QProcess::NormalExit)
    {
        //上传文件至服务器
        QDir temp(work_path + "/temp/");
        QStringList zip_list = temp.entryList(QStringList() << "*.zip");
        if(zip_list.size() == 1)
        {
            ui->textEdit->append("开始上传文件到服务端...");
            zip_file_name = zip_list[0];
            qDebug() << "zip_file_path is:" << zip_file_name;
            upload_file = new QFile(work_path + "/temp/" + zip_file_name);
            ui->textEdit->append(QString("已读取到文件:%1,文件大小:%2 bytes")
                                     .arg(zip_file_name)
                                     .arg(upload_file->size()));


            QUrl upload_url(config->getServUrl() + "/api/uploadFile");
            QNetworkRequest http_req = QNetworkRequest(upload_url);
            http_req.setRawHeader("userName", config->getUser().toStdString().c_str());
            http_req.setRawHeader("deviceType", "6");
            http_req.setRawHeader("token", config->getToken().toStdString().c_str());

            QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
            QHttpPart filePart;
            filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/zip"));
            filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"file\"; filename=\"%1\"")
                                                                                   .arg(zip_file_name)));
            upload_file->open(QIODevice::ReadOnly);
            filePart.setBodyDevice(upload_file);
            multiPart->append(filePart);

            // 发送HTTP POST请求
            reply.reset(m_accessManager.post(http_req, multiPart));
            multiPart->setParent(reply.get());

            connect(reply.get(), &QNetworkReply::finished, this, &MainWindow::httpUpLoadFinished);

            ProgressDialog *progressDialog = new ProgressDialog(zip_file_name, false, this);
            progressDialog->setAttribute(Qt::WA_DeleteOnClose);
            connect(progressDialog, &QProgressDialog::canceled, this, &MainWindow::cancelDownload);

            connect(reply.get(), &QNetworkReply::uploadProgress,
                    progressDialog, &ProgressDialog::networkReplyProgress);
            connect(reply.get(), &QNetworkReply::finished, progressDialog, &ProgressDialog::hide);
        }
        else
        {
            qDebug() << "temp directory has more than one zip:" << zip_list;
            ui->textEdit->append("未读取到文件,请先在移动端打包任务数据");
            task_upload_end_time = QDateTime::currentDateTime();
            ui->textEdit->append(QString("完成时间: %1,耗时:%2毫秒")
                                     .arg(task_upload_end_time.toString("yyyy-MM-dd HH:mm:ss"))
                                     .arg(task_upload_start_time.msecsTo(task_upload_end_time)));
            ui->pushButton_upload->setEnabled(true);
        }

    }
    else
    {
        ui->textEdit->append("读取文件失败");
        ui->textEdit->append("上传任务失败");
        task_upload_end_time = QDateTime::currentDateTime();
        ui->textEdit->append(QString("完成时间: %1,耗时:%2毫秒")
                                 .arg(task_upload_end_time.toString("yyyy-MM-dd HH:mm:ss"))
                                 .arg(task_upload_start_time.msecsTo(task_upload_end_time)));

        ui->pushButton_upload->setEnabled(true);
    }
}





void MainWindow::startRequest(const QUrl &requestedUrl)
{
    QUrl url = requestedUrl;
    httpRequestAborted = false;

    QNetworkRequest http_req = QNetworkRequest(url);
    http_req.setRawHeader("userName", config->getUser().toStdString().c_str());
    http_req.setRawHeader("deviceType", "6");
    http_req.setRawHeader("token", config->getToken().toStdString().c_str());

    reply.reset(m_accessManager.get(http_req));
    connect(reply.get(), &QNetworkReply::finished, this, &MainWindow::httpFinished);
    connect(reply.get(), &QNetworkReply::redirected, this, &MainWindow::processRedirected);
    connect(reply.get(), &QIODevice::readyRead, this, &MainWindow::httpReadyRead);


    QString download_file_name;
    if(is_upgrade_process)
    {
        download_file_name = requestedUrl.fileName();
    }
    ProgressDialog *progressDialog = new ProgressDialog(download_file_name, true, this);
    progressDialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(progressDialog, &QProgressDialog::canceled, this, &MainWindow::cancelDownload);
    connect(reply.get(), &QNetworkReply::downloadProgress,
            progressDialog, &ProgressDialog::networkReplyProgress);
    connect(reply.get(), &QNetworkReply::finished, progressDialog, &ProgressDialog::hide);
    connect(reply.get(), &QNetworkReply::redirected, progressDialog, &ProgressDialog::setDownloadFileName);
    progressDialog->show();
}



void MainWindow::on_pushButton_download_clicked()
{
    on_pushButton_clicked();
    on_pushButton_test_serv_conn_clicked();

    if(!app_conn_success)
    {
        QMessageBox::warning(this, tr(""), tr("APP连接失败,请确认USB线是否已经连接好"));
        return;
    }

    if(!serv_conn_success)
    {
        QMessageBox::warning(this, tr(""), tr("服务端连接失败,请点击设置按钮并确认服务端配置是否正确"));
        return;
    }

    if(!app_is_installed)
    {
        QMessageBox::warning(this, tr(""), tr("未安装APP,请先安装APP"));
        return;
    }

    task_download_start_time = QDateTime::currentDateTime();
    ui->textEdit->clear();
    ui->textEdit->append(QString("开始准备下载.启动时间: %1\n"
                                 "正在获取服务端最新版本……获取成功。最新版本值：%2\n"
                                 "正在获取移动端当前版本……获取成功。当前版本值：%3")
                             .arg(task_download_start_time.toString("yyyy-MM-dd HH:mm:ss"))
                             .arg(serv_ver)
                             .arg(app_ver));

    if(serv_ver != app_ver)
    {
        ui->textEdit->append(tr("版本不一致,不能进行操作...."));
        QMessageBox::information(this, tr(""), tr("版本号不一致,请先升级APP"));
        return;
    }
    ui->textEdit->append(tr("正在从服务端下载指令文件"));

    QUrl newUrl(config->getServUrl() + "/" + task_download_path);
    is_task_download_process = true;
    ui->pushButton_download->setEnabled(false);
    startRequest(newUrl);
}


void MainWindow::on_pushButton_upload_clicked()
{
    on_pushButton_clicked();
    on_pushButton_test_serv_conn_clicked();

    if(!app_conn_success)
    {
        QMessageBox::warning(this, tr(""), tr("APP连接失败,请确认USB线是否已经连接好"));
        return;
    }

    if(!serv_conn_success)
    {
        QMessageBox::warning(this, tr(""), tr("服务端连接失败,请点击设置按钮并确认服务端配置是否正确"));
        return;
    }

    if(!app_is_installed)
    {
        QMessageBox::warning(this, tr(""), tr("未安装APP,请先安装APP"));
        return;
    }

    task_upload_start_time = QDateTime::currentDateTime();
    ui->textEdit->clear();
    ui->textEdit->append(QString("开始准备上传.启动时间: %1\n"
                                 "正在获取服务端最新版本……获取成功。最新版本值：%2\n"
                                 "正在获取移动端当前版本……获取成功。当前版本值：%3")
                             .arg(task_upload_start_time.toString("yyyy-MM-dd HH:mm:ss"))
                             .arg(serv_ver)
                             .arg(app_ver));

    if(serv_ver != app_ver)
    {
        ui->textEdit->append(tr("版本不一致,不能进行操作...."));
        QMessageBox::information(this, tr(""), tr("版本号不一致,请先升级APP"));
        return;
    }

    ui->pushButton_upload->setEnabled(false);
    ui->textEdit->append(tr("正在读取移动端文件"));
    this->task_upload_process->start(work_path + "/scripts/upload.bat", QStringList()
        << adb_absolute_path
        << config->getAppPath() + "/*.zip"
        << work_path + "/temp");

}


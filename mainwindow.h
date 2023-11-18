#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QProcess>
#include <QUrl>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class UsbListener;
class SetConfig;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();



private slots:
    void on_pushButton_clicked();
    void detect_usb_arrival();
    void detect_usb_remove();
    void readAppVersionOutput();
    void readAppVersionErrOutput();
    void readAppVersionFinish(int exitCode, QProcess::ExitStatus exitStatus);

    void readAppUpgradeOutput();
    void readAppUpgradeErrOutput();
    void upgradeFinish(int exitCode, QProcess::ExitStatus exitStatus);


    void downloadOutput();
    void downloadErrOutput();
    void downloadFinish(int exitCode, QProcess::ExitStatus exitStatus);


    void uploadOutput();
    void uploadErrOutput();
    void uploadFinish(int exitCode, QProcess::ExitStatus exitStatus);


    void deleteOutput();
    void deleteErrOutput();
    void deleteFinish(int exitCode, QProcess::ExitStatus exitStatus);


    void on_pushButton_set_clicked();
    void on_pushButton_test_serv_conn_clicked();
    void configChanged();
    void on_pushButton_upgrade_clicked();
    void cancelDownload();
    void httpReadyRead();
    void httpFinished();
    void startRequest(const QUrl &requestedUrl);
    void processRedirected(const QUrl &requestedUrl);

    void on_pushButton_download_clicked();

    void on_pushButton_upload_clicked();

    void httpUpLoadFinished();

private:
    Ui::MainWindow *ui;
    UsbListener *usb_listener{nullptr};
    QString andriod_path;
    QString work_path;
    QString adb_absolute_path;

    QProcess *app_version_process{nullptr};
    bool app_version_process_already_error{false};

    QProcess *app_upgrade_process{nullptr};

    bool app_conn_success{false};
    bool app_is_installed{false};
    int app_ver{0};

    bool serv_conn_success{false};
    int serv_ver{0};
    QString latest_apk_path;

    SetConfig *config{nullptr};
    QNetworkAccessManager m_accessManager;



    QDateTime upgrade_start_time;
    QDateTime upgrade_end_time;

    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> reply;
    std::unique_ptr<QFile> file;
    bool httpRequestAborted{false};
    bool is_upgrade_process{false};

    QString upgrade_apk_abs_path;

    static QString task_download_path;
    QDateTime task_download_start_time;
    QDateTime task_download_end_time;
    bool is_task_download_process{false};
    QProcess *task_download_process{nullptr};
    QString download_instruction_file_abs_path;

    QDateTime task_upload_start_time;
    QDateTime task_upload_end_time;
    bool is_task_upload_process{false};
    QProcess *task_upload_process{nullptr};
    QString zip_file_name;


    QProcess *delete_app_uploadfile_process{nullptr};
    QFile *upload_file{nullptr};
};
#endif // MAINWINDOW_H

#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QProgressDialog>


class ProgressDialog : public QProgressDialog {
    Q_OBJECT

public:
    explicit ProgressDialog(QString filename, bool is_download = true, QWidget *parent = nullptr);

public slots:
    void networkReplyProgress(qint64 bytesRead, qint64 totalBytes);
    void setDownloadFileName(const QUrl &requestedUrl);
};

#endif // PROGRESSDIALOG_H

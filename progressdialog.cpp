#include "progressdialog.h"
#include <QUrlQuery>

ProgressDialog::ProgressDialog(QString filename, QWidget *parent)
    : QProgressDialog(parent)
{
    setWindowTitle(tr("Download Progress"));
    setLabelText(tr("Downloading %1.").arg(filename));
    setMinimum(0);
    setValue(0);
    setMinimumDuration(0);
    setMinimumSize(QSize(400, 75));
}


void ProgressDialog::networkReplyProgress(qint64 bytesRead, qint64 totalBytes)
{
    setMaximum(totalBytes);
    setValue(bytesRead);
}


void ProgressDialog::setDownloadFileName(const QUrl &requestedUrl)
{
    QUrlQuery query(requestedUrl);
    QString temp = query.queryItemValue("filePath");
    QString fileName = temp.sliced(temp.lastIndexOf('/') + 1);
    setLabelText(tr("Downloading %1.").arg(fileName));
}



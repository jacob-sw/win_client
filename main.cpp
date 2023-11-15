#include "mainwindow.h"
#include <QDateTime>
#include <QApplication>
#include <QFile>


#define FILE_SIZE_10M (1024 * 1024 * 10)


void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    QString dt = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString txt = QString("[%1] ").arg(dt);
    QByteArray localMsg = msg.toLocal8Bit();
    QString file = context.file ? context.file : "";
    qsizetype start_index = file.lastIndexOf('\\');
    QString strip_file = file.sliced(start_index + 1);

    switch (type)
    {
    case QtDebugMsg:
        txt += QString("{Debug}{%1:%2} \t\t %3").arg(strip_file).arg(context.line).arg(localMsg.constData());
        break;

    case QtWarningMsg:
        txt += QString("{Warning}{%1:%2} \t\t %3").arg(strip_file).arg(context.line).arg(localMsg.constData());
        break;

    case QtCriticalMsg:
        txt += QString("{Critical}{%1:%2} \t\t %3").arg(strip_file).arg(context.line).arg(localMsg.constData());
        break;

    case QtFatalMsg:
        txt += QString("{Fatal}{%1:%2} \t\t %3").arg(strip_file).arg(context.line).arg(localMsg.constData());
        abort();
        break;

    case QtInfoMsg:
        txt += QString("{Info}{%1:%2} \t\t %3").arg(strip_file).arg(context.line).arg(localMsg.constData());
        break;
    }

    QFile *outFile = new QFile("./log/LogFile.log");
    qint64 s = outFile->size();
    if(s >= FILE_SIZE_10M)
    {
        QFile outFileBak("./log/LogFile.log.bak");
        outFileBak.remove();
        outFileBak.close();
        outFile->rename("./log/LogFile.log.bak");
        outFile->close();
        outFile = new QFile("./log/LogFile.log");
    }

    outFile->open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream textStream(outFile);
    textStream << txt << Qt::endl;

    outFile->flush();
    outFile->close();
}


int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

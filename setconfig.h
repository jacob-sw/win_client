#ifndef SETCONFIG_H
#define SETCONFIG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class SetConfig;
}

class SetConfig : public QDialog
{
    Q_OBJECT

public:
    explicit SetConfig(QWidget *parent = nullptr);
    ~SetConfig();
    void readConfig();
    QString getAppPath() const;
    QString getServUrl() const;
    QString getUser() const;
    QString getToken() const;

    QString getServApiPath() const;

Q_SIGNALS:
    void configChange();

private slots:
    void on_pushButton_close_clicked();
    void on_pushButton_save_clicked();

private:
    Ui::SetConfig *ui;
    QSettings *config_set{nullptr};
    QString serv_api_path;

};

#endif // SETCONFIG_H

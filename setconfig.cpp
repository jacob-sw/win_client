#include "setconfig.h"
#include "ui_setconfig.h"

SetConfig::SetConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetConfig)
{
    ui->setupUi(this);
    config_set = new QSettings("./config/config.ini", QSettings::IniFormat, this);

    readConfig();
}

SetConfig::~SetConfig()
{
    delete ui;
}


void SetConfig::readConfig()
{
    ui->lineEdit_app_path->setText(config_set->value("setting/app_path", "").toString());
    ui->lineEdit_serv_url->setText(config_set->value("setting/serv_url", "").toString());
    ui->lineEdit_user->setText(config_set->value("setting/user", "").toString());
    ui->lineEdit_token->setText(config_set->value("setting/token", "").toString());

    serv_api_path = config_set->value("setting/apk_latest_ver_api", "").toString();
    return;
}

void SetConfig::on_pushButton_close_clicked()
{
    this->close();
}


void SetConfig::on_pushButton_save_clicked()
{
    config_set->setValue("setting/app_path", ui->lineEdit_app_path->text());
    config_set->setValue("setting/serv_url", ui->lineEdit_serv_url->text());
    config_set->setValue("setting/user", ui->lineEdit_user->text());
    config_set->setValue("setting/token", ui->lineEdit_token->text());

    emit configChange();
    return;
}


QString SetConfig::getAppPath() const
{
    return ui->lineEdit_app_path->text();
}


QString SetConfig::getServUrl() const
{
    return ui->lineEdit_serv_url->text();
}

QString SetConfig::getUser() const
{
    return ui->lineEdit_user->text();
}

QString SetConfig::getToken() const
{
    return ui->lineEdit_token->text();
}


QString SetConfig::getServApiPath() const
{
    return serv_api_path;
}







#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QMessageBox>
#include <QHostAddress>
#include <QJsonObject>
#include <QDebug>
#include "bind.h"
#include "player.h"


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_clicked();
    void server_reply_solt();

    void on_pushButton_2_clicked();

private:
    void widget_send_date(QJsonObject &);
    void widget_recv_date(QByteArray&);
    void app_register_handle(QJsonObject &);
    void app_login_handle(QJsonObject &);
private:
    Ui::Widget *ui;
    QTcpSocket*socket;
    QString m_appid;
};
#endif // WIDGET_H

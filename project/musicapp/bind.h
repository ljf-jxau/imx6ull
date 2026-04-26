#ifndef BIND_H
#define BIND_H

#include <QWidget>
#include <QTcpSocket>
#include <QJsonObject>

namespace Ui {
class Bind;
}

class Bind : public QWidget
{
    Q_OBJECT

public:
    explicit Bind(QTcpSocket*,QString ,QWidget *parent = nullptr);
    ~Bind();
private slots:
    void server_reply_slot();

    void on_pushButton_clicked();

private:
    void bind_send_date(QJsonObject &);
    void Bind_recv_date(QByteArray &);
private:
    Ui::Bind *ui;
    QString appid;
    QTcpSocket* socket;
    QString deviceid;
};

#endif // BIND_H

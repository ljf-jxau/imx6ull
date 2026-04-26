#ifndef PLAYER_H
#define PLAYER_H

#include <QWidget>
#include <QTcpSocket>
#include <QJsonObject>
#include <QCloseEvent>

namespace Ui {
class player;
}

#define SEQUENCE 1
#define CIRCLE 2

class player : public QWidget
{
    Q_OBJECT

public:
    explicit player(QTcpSocket*,QString,QString ,QWidget *parent = nullptr);
    ~player();
private slots:
    void player_reply_solt();
    void timeout_slot();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_4_clicked();

    void on_radioButton_clicked();

    void on_radioButton_2_clicked();

private:
    void player_send_date(QJsonObject &);
    void player_recv_date(QByteArray &);
    void player_update_info(QJsonObject &);
    void player_update_music(QJsonObject &);
    void player_start_reply(QJsonObject &);
    void player_suspend_reply(QJsonObject &);
    void player_continue_reply(QJsonObject &);
    void player_next_reply(QJsonObject &);
    void player_prior_reply(QJsonObject &);
    void player_voice_reply(QJsonObject &);
    void player_mode_reply(QJsonObject &);
    void player_get_music();
    void closeEvent(QCloseEvent*);
private:
    Ui::player *ui;
    QString appid;
    QTcpSocket* socket;
    QTimer*timer;
    QString deviceid;
    quint8 flag;
    quint8 start_flag;
    quint8 suspend_flag;
};

#endif // PLAYER_H

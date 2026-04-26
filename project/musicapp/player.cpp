#include "player.h"
#include "ui_player.h"
#include <QMessageBox>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFont>
#include <QDebug>
#include <QJsonArray>

player::player(QTcpSocket*socket,QString id,QString deviceid,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::player)
{
    ui->setupUi(this);
    this->socket=socket;
    this->appid=id;
    this->deviceid=deviceid;
    this->setWindowTitle("小李的智能音箱");
    flag=1;
    start_flag=0;//没有播放
    suspend_flag=0;//没有暂停
    connect(socket,&QTcpSocket::connected,this,[=]{
        QMessageBox::information(this,"连接提示","连接服务器成功");
    });

    connect(socket,&QTcpSocket::disconnected,this,[=]{
        QMessageBox::warning(this,"连接提示","连接服务器断开");
    });

    connect(socket,&QTcpSocket::readyRead,this,&player::player_reply_solt);//如果有数据可读
    timer=new QTimer();
    timer->start(2000);
    connect(timer,&QTimer::timeout,this,&player::timeout_slot);
}

player::~player()
{
    delete ui;
}

void player::player_reply_solt()
{
    QByteArray recvDate;
    player_recv_date(recvDate);//读数组数据到recvdate
    QJsonObject obj=QJsonDocument::fromJson(recvDate).object();//转化为json格式
    QString cmd=obj.value("cmd").toString();
    if(cmd=="info")
    {
       player_update_info(obj);
       if(flag)
       {
           player_get_music();
           flag=0;
       }
    }
    else if(cmd=="upload_music")
    {
        player_update_music(obj);
    }
    else if(cmd=="app_start_reply")
    {
        player_start_reply(obj);
    }
    else if(cmd=="app_suspend_reply")
    {
        player_suspend_reply(obj);
    }
    else if(cmd=="app_continue_reply")
    {
        player_continue_reply(obj);
    }
    else if(cmd=="app_next_reply")
    {
        player_next_reply(obj);
    }
    else if(cmd=="app_prior_reply")
    {
        player_prior_reply(obj);
    }
    else if(cmd=="app_voice_up_reply" || cmd=="app_voice_down_reply")
    {
        player_voice_reply(obj);
    }
    else if(cmd=="app_circle_reply" || cmd=="app_sequence_reply")
    {
        player_mode_reply(obj);
    }
}

void player::timeout_slot()
{
    QJsonObject obj;
    obj.insert("cmd","app_info");
    obj.insert("appid",appid);
    obj.insert("deviceid",deviceid);
    player_send_date(obj);

}

void player::player_send_date(QJsonObject &obj)
{
    QByteArray sendDate;
    QByteArray ba=QJsonDocument(obj).toJson();

    int size=ba.size();
    sendDate.insert(0,(char*)&size,4);
    sendDate.append(ba);
    socket->write(sendDate);
}

void player::player_recv_date(QByteArray &ba)
{
    char buf[1024]={0};
    qint64 size=0;

    while(true)
    {
        size+=socket->read(buf+size,sizeof(int)-size);
        if(size>=4) break;
    }
    int len=*(int*)buf;
    size=0;
    memset(buf,0,sizeof(buf));
    while(true)
    {
        size+=socket->read(buf+size,len-size);
        if(size>=len) break;
    }
    ba.append(buf);
    qDebug()<<"服务器返回的为"<<buf<<endl;
}

void player::player_update_info(QJsonObject &obj)
{
    QString cur_music=obj.value("cur_music").toString();
    QString dev_id=obj.value("deviceid").toString();
    int volume=obj.value("volume").toInt();
    int mode=obj.value("mode").toInt();
    QString status=obj.value("status").toString();
    QFont f("黑体",14);
    ui->label_4->setFont(f);
    ui->label_4->setText(dev_id);

//    ui->label_3->setFont(f);
//    ui->label_3->setText(cur_music);

    ui->label->setFont(f);
    ui->label->setText(QString::number(volume));

    if(mode==SEQUENCE)
    {
        ui->radioButton_2->setChecked(true);
    }
    else if(mode==CIRCLE)
    {
        ui->radioButton->setChecked(true);
    }

    ui->pushButton_2->setFont(f);
    if(status=="start")
    {
        ui->label_5->setText("开始播放中....");
        ui->label_3->setFont(f);
        ui->label_3->setText(cur_music);
        start_flag=1;
        suspend_flag=0;
    }
    else if(status=="stop")
    {
        ui->label_5->setText("停止播放中....");
        start_flag=0;
        suspend_flag=0;
    }
    else if(status=="suspend")
    {
        ui->label_5->setText("停止播放中....");
        start_flag=1;
        suspend_flag=1;
    }
}

void player::player_update_music(QJsonObject &obj)
{
    QJsonArray arr=obj.value("music").toArray();
    QFont f("黑体",14);
    ui->textEdit->setFont(f);
    QString result;
    ui->textEdit->setReadOnly(true);
    for(int i=0;i<arr.size();i++)
    {
        result.append(arr.at(i).toString());
        result.append('\n');
        ui->textEdit->setText(result);
    }

}

void player::player_start_reply(QJsonObject &obj)
{
    QString result=obj.value("result").toString();
    if(result=="offline")
    {
        QMessageBox::warning(this,"播放提示","音箱离线");
    }
    else if(result=="failure")
    {
        QMessageBox::warning(this,"播放提示","音箱启动失败");
    }
    else if(result=="success")
    {
        start_flag=1;
        ui->label_5->setText("开始播放中....");
    }

}

void player::player_suspend_reply(QJsonObject &obj)
{
    QString result=obj.value("result").toString();
    if(result=="offline")
    {
        QMessageBox::warning(this,"播放提示","音箱离线");
    }
    else if(result=="success")
    {
        suspend_flag=1;
        ui->label_5->setText("停止播放中....");
    }

}

void player::player_continue_reply(QJsonObject &obj)
{
    QString result=obj.value("result").toString();
    if(result=="offline")
    {
        QMessageBox::warning(this,"播放提示","音箱离线");
    }
    else if(result=="success")
    {
        suspend_flag=0;
        ui->label_5->setText("开始播放中....");
    }

}

void player::player_next_reply(QJsonObject &obj)
{
    QString result=obj.value("result").toString();
    if(result=="offline")
    {
        QMessageBox::warning(this,"播放提示","音箱离线");
    }
    else if(result=="success")
    {
        QString music=obj.value("music").toString();
        ui->label_3->setText(music);
    }
}

void player::player_prior_reply(QJsonObject &obj)
{
    QString result=obj.value("result").toString();
    if(result=="offline")
    {
        QMessageBox::warning(this,"播放提示","音箱离线");
    }
    else if(result=="success")
    {
        QString music=obj.value("music").toString();
        ui->label_3->setText(music);
    }
}

void player::player_voice_reply(QJsonObject &obj)
{
    QString result=obj.value("result").toString();
    if(result=="offline")
    {
        QMessageBox::warning(this,"播放提示","音箱离线");
    }
    else if(result=="success")
    {
        int volume=obj.value("voice").toInt();
        ui->label->setText(QString::number(volume));
    }
}

void player::player_mode_reply(QJsonObject &obj)
{
    QString result=obj.value("result").toString();
    if(result=="offline")
    {
        ui->radioButton->setChecked(false);
        ui->radioButton_2->setChecked(true);
        QMessageBox::warning(this,"播放提示","音箱离线");
    }
}

void player::player_get_music()
{
    QJsonObject obj;
    obj.insert("cmd","app_get_music");
    player_send_date(obj);
}

void player::closeEvent(QCloseEvent *e)
{
    QJsonObject obj;
    obj.insert("cmd","app_offline");
    player_send_date(obj);
    e->accept();
}

void player::on_pushButton_2_clicked()
{
    if(start_flag==0)
    {
        QJsonObject obj;
        obj.insert("cmd","app_start");
        player_send_date(obj);
    }
    else if(start_flag==1 && suspend_flag==1)
    {
        QJsonObject obj;
        obj.insert("cmd","app_continue");
        player_send_date(obj);
    }
    else if(start_flag==1 && suspend_flag==0)
    {
        QJsonObject obj;
        obj.insert("cmd","app_suspend");
        player_send_date(obj);
    }

}

void player::on_pushButton_3_clicked()
{
    QJsonObject obj;
    obj.insert("cmd","app_next");
    player_send_date(obj);
}

void player::on_pushButton_clicked()
{
    QJsonObject obj;
    obj.insert("cmd","app_prior");
    player_send_date(obj);
}

void player::on_pushButton_5_clicked()//增加音量
{
    QJsonObject obj;
    obj.insert("cmd","app_voice_up");
    player_send_date(obj);
}

void player::on_pushButton_4_clicked()//降低音量
{
    QJsonObject obj;
    obj.insert("cmd","app_voice_down");
    player_send_date(obj);
}

void player::on_radioButton_clicked()
{
    QJsonObject obj;
    obj.insert("cmd","app_circle");
    player_send_date(obj);
    ui->radioButton->setChecked(true);
}

void player::on_radioButton_2_clicked()
{
    QJsonObject obj;
    obj.insert("cmd","app_sequence");
    player_send_date(obj);
    ui->radioButton_2->setChecked(true);
}

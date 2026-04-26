#include "bind.h"
#include "ui_bind.h"
#include <QWidget>
#include <QMessageBox>
#include <QJsonDocument>
#include "player.h"

Bind::Bind(QTcpSocket*s,QString id ,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Bind)
{
    ui->setupUi(this);
    this->setWindowTitle("智能音箱");
    this->appid=id;
    this->socket=s;
    QFont f("黑体",14);
    ui->lineEdit->setFont(f);

    connect(socket,&QTcpSocket::connected,this,[=]{
          QMessageBox::information(this,"连接提示","连接服务器成功");
      });

      connect(socket,&QTcpSocket::disconnected,this,[this]{
          QMessageBox::warning(this,"连接提示","连接服务器断开");
      });

      connect(socket,&QTcpSocket::readyRead,this,&Bind::server_reply_slot);//如果有数据可读
}

Bind::~Bind()
{
    delete ui;
}

void Bind::server_reply_slot()
{
    QByteArray recvDate;
    Bind_recv_date(recvDate);
    QJsonObject obj=QJsonDocument::fromJson(recvDate).object();//转化为json格式
    QString cmd=obj.value("cmd").toString();
    if(cmd=="app_bind_reply")
    {
        QString result=obj.value("result").toString();
        if(result=="success")
        {
            socket->disconnect(SIGNAL(connected()));
            socket->disconnect(SIGNAL(disconnected()));
            socket->disconnect(SIGNAL(readyRead()));
            player*m_player=new player(socket,appid,deviceid);
            this->hide();//当前界面隐藏起来
            m_player->show();//显示新页面
        }
    }
}

void Bind::bind_send_date(QJsonObject &v)
{
    QByteArray sendDate;
    QByteArray ba=QJsonDocument(v).toJson();
    int size=ba.size();
    sendDate.insert(0,(char*)&size,4);
    sendDate.append(ba);
    socket->write(sendDate);
}

void Bind::Bind_recv_date(QByteArray &ba)
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
    qDebug()<<"bind服务器返回的为"<<buf<<endl;
}

void Bind::on_pushButton_clicked()
{
    QString deviceid=ui->lineEdit->text();
    this->deviceid=deviceid;
    QJsonObject obj;
    obj.insert("cmd","app_bind");
    obj.insert("deviceid",deviceid);
    obj.insert("appid",appid);

    bind_send_date(obj);

}

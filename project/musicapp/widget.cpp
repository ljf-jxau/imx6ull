#include "widget.h"
#include "ui_widget.h"

#include <QJsonDocument>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowTitle("智能音箱");
    QFont f("黑体",14);
    ui->lineEdit->setFont(f);
    ui->lineEdit_2->setFont(f);
    socket=new QTcpSocket;
    socket->connectToHost(QHostAddress("192.168.10.100"),8001);
    connect(socket,&QTcpSocket::connected,this,[=]{
        QMessageBox::information(this,"连接提示","连接服务器成功");
    });

    connect(socket,&QTcpSocket::disconnected,this,[=]{
        QMessageBox::warning(this,"连接提示","连接服务器断开");
    });

    connect(socket,&QTcpSocket::readyRead,this,&Widget::server_reply_solt);//如果有数据可读
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_clicked()//注册
{
    //获取appid和passwod
    QString appid=ui->lineEdit->text();
    QString password=ui->lineEdit_2->text();

   //发送给服务器{"cmd":"app_register","appid":"1001","password":"1111"}
    QJsonObject obj;
    obj.insert("cmd","app_register");
    obj.insert("appid",appid);
    obj.insert("password",password);

    widget_send_date(obj);

}

void Widget::server_reply_solt()//数据可读执行函数
{
    QByteArray recvDate;
    widget_recv_date(recvDate);//读数组数据到recvdate
    QJsonObject obj=QJsonDocument::fromJson(recvDate).object();//转化为json格式
    QString cmd=obj.value("cmd").toString();
    if(cmd=="app_register_reply")
    {
        app_register_handle(obj);//对服务器返回的值进行提示
    }
    else if(cmd=="app_login_reply")
    {
        app_login_handle(obj);
    }
}

void Widget::widget_send_date(QJsonObject &obj)//把json格式转换为字符数组发送给服务器
{
    QByteArray sendDate;
    QByteArray ba=QJsonDocument(obj).toJson();

    int size=ba.size();
    sendDate.insert(0,(char*)&size,4);
    sendDate.append(ba);
    socket->write(sendDate);

}

void Widget::widget_recv_date(QByteArray &ba)//接收到的数据转换为字符数组的格式
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

void Widget::app_register_handle(QJsonObject &obj)//对服务器返回的值进行提示
{
    QString result=obj.value("result").toString();
    if(result=="success")
    {
        QMessageBox::information(this,"注册提示","注册成功");
    }
    else if(result=="failure")
    {
         QMessageBox::warning(this,"注册提示","注册失败");
    }
}

void Widget::app_login_handle(QJsonObject &obj)
{
    QString result =obj.value("result").toString();
    if(result=="not_exist")
    {
        QMessageBox::warning(this,"登录提示","用户不存在,请先注册");
    }
    else if(result=="password_error")
    {
        QMessageBox::warning(this,"登录提示","密码错误，请检查密码");
    }
    else if(result=="not_bind")
    {
        QMessageBox::information(this,"登入提示","登入成功，未绑定");
        socket->disconnect(SIGNAL(connected()));
        socket->disconnect(SIGNAL(disconnected()));
        socket->disconnect(SIGNAL(readyRead()));
        //创建新界面
        Bind*m_bind=new Bind(socket,m_appid);
        this->hide();//当前界面隐藏起来
        m_bind->show();//显示新页面
    }
    else if(result=="bind")
    {
        //QMessageBox::information(this,"登入提示","登入成功,已绑定");
       QString deviceid=obj.value("deviceid").toString();
       socket->disconnect(SIGNAL(connected()));
       socket->disconnect(SIGNAL(disconnected()));
       socket->disconnect(SIGNAL(readyRead()));
       player*m_player=new player(socket,m_appid,deviceid);
       this->hide();//当前界面隐藏起来
       m_player->show();//显示新页面
    }

}

void Widget::on_pushButton_2_clicked()
{
    //获取appid和passwod
     QString appid=ui->lineEdit->text();
     QString password=ui->lineEdit_2->text();
     this->m_appid=appid;
     QJsonObject obj;
     obj.insert("cmd","app_login");
     obj.insert("appid",m_appid);
     obj.insert("password",password);

     widget_send_date(obj);
}

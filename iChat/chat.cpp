#include "chat.h"
#include "ui_chat.h"
#include <QDataStream>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QHostInfo>
#include <QDateTime>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QScrollBar>
#include "Structor.h"

Chat::Chat(QString _IP ,QString _hostName,QWidget *parent) :
    QDialog(parent),\
    ui(new Ui::Chat),\
    IP(_IP),\
    hostName(_hostName)
{
    ui->setupUi(this);
    ui->textEdit->installEventFilter(this);
    setWindowTitle(tr("IM:%1,Chating with:%2")\
                   .arg(QHostInfo::localHostName())\
                   .arg(_hostName));
    senderSocket  = NULL;
    sender = new Sender(NULL);
}

Chat::~Chat()
{
    delete ui;
}

void Chat::readMessage(Recorder _recoders)
{
    qDebug()<<"read message"<<endl;
    ui->textBrowser->setCurrentFont(QFont("Times new Roman",12));
    ui->textBrowser->append(tr("[%1] [%2]")\
                            .arg(_recoders.srcIP)
                            .arg(_recoders.time));
    if(_recoders.type == Image)
        ui->textBrowser->insertHtml(_recoders.data);
    if(_recoders.type == Message)
        ui->textBrowser->append(_recoders.data);
    if(_recoders.type == Refuse)
    {
        sender->refused();
    }
    ui->textEdit->setFocus();
    ui->textBrowser->verticalScrollBar()->setValue\
            (ui->textBrowser->verticalScrollBar()->maximum());

}
void Chat::readRecoder(QVector<Recorder> *_recoders)
{
    recoders = _recoders;
    QString tIP =getIP();
    qDebug()<<"tip="<<tIP;
    qDebug()<<"ip="<<IP;
    for ( int i= 0 ; i < _recoders->size() ;++i)
    {
        if( ( (tIP == _recoders->at(i).srcIP) && (IP == _recoders->at(i).destIP) )\
                || ((tIP == _recoders->at(i).destIP) && (IP == _recoders->at(i).srcIP))  )
        {
            this->readMessage(_recoders->at(i));
        }
    }
}

void Chat::on_sendBtn_clicked()
{

    if(ui->textEdit->toPlainText() =="")
    {
        QMessageBox::warning(0,tr("waring"),tr("must send at least one word"),QMessageBox::Ok);
        return;
    }
    QByteArray data;
    QDataStream  out(&data,QIODevice::WriteOnly);
    //主机名，IP
    QString msg = ui->textEdit->toHtml();
    if(msg.size() > 4096)
    {
        QMessageBox::warning(0,tr("waring"),tr("too much words"),QMessageBox::Ok);
        msg = msg.left(4096);
    }
    out<<(int)Message<<getIP()<<QHostInfo::localHostName()<<IP<<msg;
    if(senderSocket)
    {
        qDebug()<<"server";
        senderSocket->write(data);
    }
    Recorder re ={Message,getIP(),IP,msg,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")};
    recoders->push_back(re);

    //更新自己的面板，将自己发送的消息打印在上面
    ui->textBrowser->setCurrentFont(QFont("Times new Roman",12));
    ui->textBrowser->append(tr("[%1] [%2]")\
                            .arg(getIP())
                            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:dd")));
    ui->textBrowser->append(ui->textEdit->toHtml());
    ui->textEdit->clear();
    ui->textEdit->setFocus();


}
QString Chat::getIP()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list)
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol)
            return address.toString();

    }
    return 0;
}

void Chat::closeEvent(QCloseEvent *)
{
    qDebug()<<"exiting .. ";
}
void Chat::displayError(QAbstractSocket::SocketError)
{
    // QMessageBox::warning(0,tr("connection error"),tr("can't connect to server"),QMessageBox::Ok);
}
void Chat::setSenderSocket(QTcpSocket *socket)
{
    senderSocket = socket;
}
bool Chat::eventFilter(QObject *obj, QEvent *e)
{
    Q_ASSERT(obj == ui->textEdit);
    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent *event = static_cast<QKeyEvent*>(e);
        if (event->key() == Qt::Key_Return && (event->modifiers() & Qt::ControlModifier))
        {
            on_sendBtn_clicked();
            return true;
        }
    }
    return false;
}
void Chat::sendImage(QByteArray *_data)
{
    QByteArray data;
    QDataStream  out(&data,QIODevice::WriteOnly);
    out<<(int)Image<<getIP()<<QHostInfo::localHostName()<<IP;
    data.append(*_data);
    qDebug()<<"caht data size"<<data.size();
    if(senderSocket)
    {
        qDebug()<<"server";
        senderSocket->write(data);
    }

}

void Chat::on_pushButton_clicked()
{
    QByteArray imageAr;
    QString path = QFileDialog::getOpenFileName(this,\
                                                tr("Open Image"),"", tr("Image Files (*.png *.jpg *.bmp)"));
    QFile imageFile(path);
    imageFile.open(QIODevice::ReadOnly);
    imageAr=imageFile.readAll();
    QByteArray tmp;
    QDataStream out(&tmp,QIODevice::WriteOnly);
    out<<(int)imageAr.size();
    tmp.append(imageAr);
    sendImage(&tmp);

    //加入聊天记录
    path = QString("<img src=\"%1\"/>").arg(path);
    Recorder re ={Image,getIP(),IP,path,\
                  QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")};
    recoders->push_back(re);
    //更新自己的面板，将自己发送的图片打印在上面
    ui->textBrowser->setCurrentFont(QFont("Times new Roman",12));
    ui->textBrowser->append(tr("[%1] [%2]")\
                            .arg(getIP())
                            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:dd")));
    ui->textBrowser->insertHtml(path);
    ui->textEdit->clear();
    ui->textEdit->setFocus();
}

void Chat::on_sendFileButton_clicked()
{
    sender->show();
    sender->initServer();
    connect(sender,SIGNAL(sendFileName(QString)),\
            this,SLOT(sendFileName(QString)));
}
void Chat::sendFileName(QString fileName)
{
    QByteArray data;
    QDataStream  out(&data,QIODevice::WriteOnly);
    out<<(int)FileName<<getIP()<<QHostInfo::localHostName()<<IP<<fileName;
    if(senderSocket)
    {
        qDebug()<<"server";
        senderSocket->write(data);
    }

}

void Chat::on_spinBox_valueChanged(int arg1)
{
    ui->textEdit->setFontPointSize(arg1*1.0);
    ui->textEdit->setFocus();
}

void Chat::on_comboBox_currentIndexChanged(const QString &arg1)
{

    if( tr("red") == arg1)
        ui->textEdit->setTextColor(Qt::red);
    if( tr("blue") == arg1)
        ui->textEdit->setTextColor(Qt::blue);
    if(  tr("black") == arg1)
        ui->textEdit->setTextColor(Qt::black);
    if( tr("green") == arg1)
        ui->textEdit->setTextColor(Qt::green);
    ui->textEdit->setFocus();
}

void Chat::on_fontComboBox_currentFontChanged(const QFont &f)
{
    ui->textEdit->setCurrentFont(f);
    ui->textEdit->setFocus();
}

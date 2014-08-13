#include "chat.h"
#include "ui_chat.h"
#include <QDataStream>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QHostInfo>
#include <QDateTime>
#include <QMessageBox>
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
    ui->textBrowser->append(_recoders.data);
    ui->textEdit->setFocus();

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
    QString msg = ui->textEdit->toPlainText();
    if(msg.size() > 4096)
    {
        QMessageBox::warning(0,tr("waring"),tr("most 4096 words"),QMessageBox::Ok);
        return;
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
    ui->textBrowser->append(ui->textEdit->toPlainText());
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

#include "thread.h"
#include <QHostAddress>
#include <QMessageBox>
#include <QList>
#include <QDataStream>
#include <QStringList>

Thread::Thread(QVector<Item> *_items,QTcpServer * tcpServer,QTableWidget *_tableWidget)
{
    items = _items;
    tcpSocket = tcpServer->nextPendingConnection();
    tableWidget = _tableWidget;
}
void Thread::run()
{
    connect(tcpSocket,SIGNAL(readyRead()),\
            this,SLOT(on_tcpSocket_readyRead()));
}
void Thread::on_tcpSocket_readyRead()
{
    QString srcHostName,srcIP,destIP;
    int msgeType;
    QByteArray inBlock;
    inBlock = tcpSocket->readAll();
    QDataStream in(&inBlock,QIODevice::ReadOnly);
    in>>msgeType>>srcIP>>srcHostName>>destIP;
    qDebug()<<msgeType;
    qDebug()<<srcIP;
    qDebug()<<srcHostName;
    qDebug()<<destIP;

    switch (msgeType)
    {
    case NewParticipant:
    {
        this->newParticipant(srcHostName,srcIP);
        this->sendUserList(NewParticipant,destIP,srcIP,srcHostName);
        break;
    }
    case ParticipantLeft:
    {
        this->participantLeft(srcIP);
        this->sendUserList(NewParticipant,destIP,srcIP,srcHostName);
        break;
    }
    case Message:
    {
        QString msge;
        in>>msge;
        sendMessage(Message,destIP,srcIP,srcHostName,msge);
        break;
    }
    default:
        break;
    }

}
void Thread::newParticipant(QString hostName, QString IP)
{
    //更新用户列表
    bool isEmpty = tableWidget->findItems(IP,Qt::MatchExactly).isEmpty();
    if(isEmpty)
    {
        //更新table widget
        QTableWidgetItem * host = new QTableWidgetItem(hostName);
        QTableWidgetItem * ip = new QTableWidgetItem(IP);
        QTableWidgetItem * Socket = new QTableWidgetItem(QString("%1").arg((int)tcpSocket));
        tableWidget->insertRow(0);
        tableWidget->setItem(0,0,ip);
        tableWidget->setItem(0,1,host);
        tableWidget->setItem(0,2,Socket);
        //更新用户列表的数据结构
        Item item={IP,hostName,tcpSocket};
        items->push_back(item);
    }

}
void Thread::sendUserList(\
        MessageType type,\
        QString destIP,\
        QString srcIP,\
        QString srcHostName)
{
    QStringList hostNameList,IPList;
    QVector<Item>::iterator it = items->begin();
    for(; it != items->end() ; ++it)
    {
        hostNameList.push_back(it->HostName);
        IPList.push_back(it->IP);
    }
    QByteArray data;
    QDataStream  out(&data,QIODevice::WriteOnly);
    out<<type<<srcIP<<srcHostName<<destIP<<IPList<<hostNameList;
    tcpSocket->write(data);
}

void Thread::participantLeft(QString IP)
{
   // QMessageBox::information(0,"",IP);
    //依据IP查找和删除某个用户
    QList<QTableWidgetItem *> tmp = tableWidget->findItems(IP,Qt::MatchExactly);
    if (tmp.isEmpty() == false)
    {
        int rowNum = tmp.first()->row();
        tableWidget->removeRow(rowNum);
       // tableWidget->(tr("onlines:%1").arg(ui->userTableWidget->rowCount()));
    }

}
void Thread::sendMessage(\
        MessageType type,\
        QString destIP,\
        QString srcIP,\
        QString srcHostName,\
        QString msge)
{
    QTcpSocket * destSocket = NULL;
    QVector<Item>::iterator it = items->begin();
    for(; it != items->end() ; ++it)
    {
        if(it->IP == destIP)
        {
            destSocket = it->tcpSocket;
        }
    }
    if(destSocket != NULL)
    {
        //发送消息
        QByteArray data;
        QDataStream  out(&data,QIODevice::WriteOnly);
        out<<type<<srcIP<<srcHostName<<destIP<<msge;
 //       QMessageBox::information(0,"",type+srcIP+srcHostName+destIP+msge);
        destSocket->write(data);

    }
    else
    {
        QMessageBox::warning(0,"","can't find socket");
        //如果没有找到destIP 对应的SOCKET，那么把错误返回给发送者
    }
}

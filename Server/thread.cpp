#include "thread.h"
#include <QHostAddress>
#include <QMessageBox>
#include <QList>
#include <QDataStream>
#include <QStringList>
#include <QMutexLocker>

Thread::Thread(QVector<Item> *_items, QTcpServer * tcpServer, QTableWidget *_tableWidget, QMutex * _mutex)
{
    items = _items;
    tcpSocket = tcpServer->nextPendingConnection();
    tcpSocket->setReadBufferSize(64*1024);
    tableWidget = _tableWidget;
    mutex = _mutex;
    bRecvingImage = false;
}
void Thread::run()
{
    connect(tcpSocket,SIGNAL(readyRead()),\
            this,SLOT(on_tcpSocket_readyRead()));
}
void Thread::on_tcpSocket_readyRead()
{
    qDebug()<<"tcp byteAvaiable:"<<tcpSocket->bytesAvailable();
    QByteArray inBlock;
    inBlock = tcpSocket->readAll();
    QDataStream in(&inBlock,QIODevice::ReadOnly);
    if(!bRecvingImage)
    {
        in>>msgeType>>srcIP>>srcHostName>>destIP;
        if(msgeType == Image || msgeType == ImageBraodCast)
        {
            in>>totalBytes;
            recedBYtes = 0;
            bRecvingImage = true;
            QByteArray tmp;
            QDataStream out(&tmp,QIODevice::WriteOnly);
            out<<msgeType<<srcIP<<srcHostName<<destIP<<totalBytes;
            totalBytes += tmp.size();
            recvedByteAr.append(inBlock);
            recedBYtes += inBlock.size();
            if(recedBYtes < totalBytes)
                return;
        }
    }
    else
    {
        recvedByteAr.append(inBlock);
        recedBYtes += inBlock.size();
        if(recedBYtes < totalBytes)
            return;
    }
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
    case BroadCast:
    {
        QString msge;
        in>>msge;
        sendMessageToallUser(BroadCast,destIP,srcIP,srcHostName,msge);
        break;
    }
    case Image:
    {
        QByteArray data;
        data.append(recvedByteAr);
        sendImage(Image,destIP,srcIP,srcHostName,data);
        qDebug()<<"image send datasize="<<data.size();
        recvedByteAr.clear();
        bRecvingImage = false;
        break;
    }
    case ImageBraodCast:
    {
        QByteArray data;
        data.append(recvedByteAr);
        qDebug()<<"data size = "<<data.size();
        sendImageToallUser(ImageBraodCast,destIP,srcIP,srcHostName,data);
        recvedByteAr.clear();
        bRecvingImage = false;
        break;
    }
    case FileName:
    {
        QString fileName;
        in>>fileName;
        sendMessage(FileName,destIP,srcIP,srcHostName,fileName);
        break;
    }
    default:
        break;
    }

}
void Thread::newParticipant(QString hostName, QString IP)
{
    bool isEmpty;
    {
        QMutexLocker locker(mutex);
        //更新用户列表
        isEmpty = tableWidget->findItems(IP,Qt::MatchExactly).isEmpty();
    }
    if(isEmpty)
    {
        //更新table widget
        QTableWidgetItem * host = new QTableWidgetItem(hostName);
        QTableWidgetItem * ip = new QTableWidgetItem(IP);
        QTableWidgetItem * Socket = new QTableWidgetItem(QString("%1").arg((int)tcpSocket));
        {
            QMutexLocker locker(mutex);
            tableWidget->insertRow(0);
            tableWidget->setItem(0,0,ip);
            tableWidget->setItem(0,1,host);
            tableWidget->setItem(0,2,Socket);
            //更新用户列表的数据结构
            Item item={IP,hostName,tcpSocket};
            items->push_back(item);
        }
    }

}
void Thread::sendUserList(\
        MessageType type,\
        QString destIP,\
        QString srcIP,\
        QString srcHostName)
{
    QStringList hostNameList,IPList;
    {
        QMutexLocker locker(mutex);
        QVector<Item>::iterator it = items->begin();
        for(; it != items->end() ; ++it)
        {
            hostNameList.push_back(it->HostName);
            IPList.push_back(it->IP);
        }
    }
    QByteArray data;
    QDataStream  out(&data,QIODevice::WriteOnly);
    out<<type<<srcIP<<srcHostName<<destIP<<IPList<<hostNameList;
    {
        QMutexLocker locker(mutex);
        for(int i = 0 ; i< items->size() ;++i)
        {
            QTcpSocket * sock = items->at(i).tcpSocket;
            sock->write(data);
        }
    }
}

void Thread::participantLeft(QString IP)
{
    qDebug()<<"quit:"<<IP;
    QMutexLocker locker(mutex);
    //依据IP查找和删除某个用户
    QList<QTableWidgetItem *> tmp = tableWidget->findItems(IP,Qt::MatchExactly);
    if (tmp.isEmpty() == false)
    {
        int rowNum = tmp.first()->row();
        tableWidget->removeRow(rowNum);
    }
    QVector<Item>::iterator it = items->begin();
    for(int index =0; it != items->end() ; ++it,index++)
    {
        if(it->IP == IP)
        {
            items->remove(index);
            break;
        }
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
    {
        QMutexLocker locker(mutex);
        QVector<Item>::iterator it = items->begin();
        for(; it != items->end() ; ++it)
        {
            if(it->IP == destIP)
            {
                destSocket = it->tcpSocket;
            }
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
void Thread::sendMessageToallUser(MessageType type, QString destIP, QString srcIP, QString srcHostName, QString msge)
{
    QTcpSocket * destSocket = NULL;

    {
        QMutexLocker locker(mutex);
        QVector<Item>::iterator it = items->begin();
        for(; it != items->end() ; ++it)
        {
           destSocket = it->tcpSocket;
           //发送消息
           QByteArray data;
           QDataStream  out(&data,QIODevice::WriteOnly);
           out<<(int)type<<srcIP<<srcHostName<<it->IP<<msge;
           destSocket->write(data);
        }
    }
}
void Thread::sendImage(MessageType type, QString destIP, QString srcIP, QString srcHostName, QByteArray _data)
{
    QTcpSocket * destSocket = NULL;
    {
        QMutexLocker locker(mutex);
        QVector<Item>::iterator it = items->begin();
        for(; it != items->end() ; ++it)
        {
            if(it->IP == destIP)
            {
                destSocket = it->tcpSocket;
            }
        }
    }
    if(destSocket != NULL)
    {
        destSocket->write(_data);
    }
    else
    {
        QMessageBox::warning(0,"","can't find socket");
        //如果没有找到destIP 对应的SOCKET，那么把错误返回给发送者
    }
}
void Thread::sendImageToallUser(MessageType type, QString destIP, QString srcIP, QString srcHostName, QByteArray _data)
{
    QTcpSocket * destSocket = NULL;
    {
        QMutexLocker locker(mutex);
        QVector<Item>::iterator it = items->begin();
        for(; it != items->end() ; ++it)
        {
            destSocket = it->tcpSocket;
            destSocket->write(_data);
            qDebug()<<"broad case :"<<_data.size();
        }
    }
}

#ifndef THREAD_H
#define THREAD_H
#include <QThread>
#include <QTcpSocket>
#include <QVector>
#include <QTableWidget>
#include <QTcpServer>
#include <QMutex>
#include "Structor.h"

class Thread:public QThread
{
    Q_OBJECT
public:
    Thread(QVector<Item> *_items, QTcpServer *tcpserver, QTableWidget *_tableWidget, QMutex *_mutex);
protected:
    void run();
private:
    QVector<Item> * items;
    QTcpSocket * tcpSocket;
    QTableWidget * tableWidget;
    QMutex * mutex;
    //当有新用户加入，更新用户列表
    void newParticipant(QString hostName,QString IP);
    //当有用户离开，更新用户列表
    void participantLeft(QString IP);
    //转发消息函数
    void sendMessage(\
            MessageType type,\
            QString destIP,\
            QString srcIP,\
            QString srcHostName,\
            QString data);
    //返回用户的列表
    void sendUserList(\
            MessageType type,\
            QString destIP,\
            QString srcIP,\
            QString srcHostName);
    //发送给所有用户
    void sendMessageToallUser(\
            MessageType type,\
            QString destIP,\
            QString srcIP,\
            QString srcHostName,\
            QString data);
private slots:
    void on_tcpSocket_readyRead();
};

#endif // THREAD_H

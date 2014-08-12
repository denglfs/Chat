#ifndef STRUCTOR_H
#define STRUCTOR_H
#include <QString>
#include <QTcpSocket>
//消息类型
enum MessageType
{
    Message = 0,
    ReplyNewParticipant,
    NewParticipant,
    ParticipantLeft,
    FileName,
    Refuse,
    xChat,
    Error
};

struct Item
{
    QString IP;
    QString HostName;
    QTcpSocket *tcpSocket;
};

#endif // STRUCTOR_H

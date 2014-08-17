#ifndef STRUCTOR_H
#define STRUCTOR_H
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
    BroadCast,
    Image,
    ImageBraodCast,
    Error
};

//消息记录数据结构
struct Recorder
{
    MessageType type;
    QString srcIP;
    QString destIP;
    QString data;
    QString time;
};

#endif // STRUCTOR_H

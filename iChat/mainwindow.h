#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QFile>
#include "sender.h"
#include "receiver.h"
#include "chat.h"
#include "Structor.h"

namespace Ui {
class MainWindow;
}

//枚举，分别为消息，新用户加入，用户退出，文件名，拒绝接受文件

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    unsigned xCount;
    ~MainWindow();
protected:
    void newParticipant(QStringList hostNameList, QStringList IPList);
    void sendMessage(MessageType type , QString serverAddress="", QByteArray *sendAr =NULL);
    QString getIP();
    QString getMessaget();
    void hasPendingFile(QString _srcHostName, QString _srcIP, QString _destIP, QString _fileName);
private:
    bool bRecvingImage;
    QByteArray recvedByteAr;
    int totalBytes;
    int recedBYtes;
    int msgeType;
    QString srcIP,srcHostName,destIP;
private:
    QTcpSocket * xsock;
    void closeEvent(QCloseEvent *);
    Ui::MainWindow *ui;
    qint16 port;
    QString filename;
    QVector<Recorder> recoders;
private slots:
    void on_sendButton_clicked();
    void on_xChat_clicked();
    void on_refButton_clicked();
    void on_xsock_readyRead();
    void on_pushButton_clicked();
signals:
    void newMessageSignal(Recorder  _recoders);
};

#endif // MAINWINDOW_H

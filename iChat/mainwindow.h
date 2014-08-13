#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
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
    ~MainWindow();
protected:
    void newParticipant(QStringList hostNameList, QStringList IPList);
    void participantLeft(QString userName,QString localhostName,QString time);
    void sendMessage(MessageType type ,QString serverAddress="");
    QString getIP();
    QString getMessaget();
    void hasPendingFile(QString userName,QString serverAddress,QString clientAddress,QString filename);

private:
    QTcpSocket * xsock;
    void closeEvent(QCloseEvent *);
    Ui::MainWindow *ui;
    qint16 port;
    QString filename;
    Sender * server;
    QVector<Recorder> recoders;
private slots:
    void processPendingDatagrams();
    void on_sendButton_clicked();
    void getFileName(QString);
    void on_sendFileBtn_clicked();
    void on_xChat_clicked();
    void on_exitButton_clicked();
    void on_refButton_clicked();
    void on_xsock_readyRead();

signals:
    void newMessageSignal(Recorder  _recoders);
};

#endif // MAINWINDOW_H

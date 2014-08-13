#ifndef SENDER_H
#define SENDER_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QByteArray>
#include <QTime>
#include <QFileDialog>

namespace Ui {
class Sender;
}

class Sender : public QDialog
{
    Q_OBJECT

public:
    explicit Sender(QWidget *parent = 0);
   // void sendMessage();
    void refused();
    void initServer();
    ~Sender();

private:
    Ui::Sender *ui;
    QTcpServer * tcpServer;
    QTcpSocket * clientConnection;
    qint64 payLoadSize;
    qint64 totalBytes;
    qint64 bytesWriten;
    qint64 bytesToWrite;
    int tcpPort;
    QString theFileName;
    QString filename;
    QFile *localFile;
    QByteArray outBlock;
    QTime time;

private slots:
    void sendMessage();
    void updateClientProgress(qint64);
    void on_serverOpenBtn_clicked();
    void on_serverSendBtn_clicked();
    void on_serverCloseBtn_clicked();

signals:
    void sendFileName(QString theFilename);
};

#endif // SENDER_H

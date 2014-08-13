#ifndef RECEIVER_H
#define RECEIVER_H

#include <QDialog>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QFile>
#include <QTime>
#include <QMessageBox>
#include <QHostAddress>


namespace Ui {
class receiver;
}

class receiver : public QDialog
{
    Q_OBJECT

public:
    explicit receiver(QWidget *parent = 0);
    void setFileName(QString filename);
    void setHostAddress(QHostAddress address);

    ~receiver();

private:
    Ui::receiver *ui;
    qint64 TotalBytes;
    qint64 bytesReceived;
    qint64 bytesToReceive;
    qint64 fileNameSize;
    qint16 blockSize;
    QString filename;
    QFile *localFile;
    QTcpSocket * tcpClient;
    QHostAddress hostAddress;
    QTime time;
    QByteArray inBlock;
    void newConnect();
    int tcpProt;

private slots:
    void readMessage();
    void displayError(QAbstractSocket::SocketError socketError);
    void on_calcelBtn_clicked();
    void on_closeBtn_clicked();
};

#endif // RECEIVER_H

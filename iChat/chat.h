#ifndef CHAT_H
#define CHAT_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QDateTime>
#include <QKeyEvent>
#include "Structor.h"
#include "sender.h"


namespace Ui {
class Chat;
}

class Chat : public QDialog
{
    Q_OBJECT

public:
    explicit Chat(QString IP ,QString hostName,QWidget *parent = 0);
    void setSenderSocket(QTcpSocket * socket);
    void readRecoder(QVector<Recorder> *_recoders);
    ~Chat();
private:
    Ui::Chat *ui;
    QString hostName;
    QString IP;
    QTcpSocket * senderSocket;
    QVector<Recorder> *recoders;
    Sender * sender;
    QString getIP();
    void sendImage(QByteArray * data);
    void stringToHtmlFilter(QString &str);

    void closeEvent(QCloseEvent *);
    bool eventFilter(QObject *obj, QEvent *e);
private slots:
    void on_sendBtn_clicked();
    void displayError (QAbstractSocket::SocketError);
    void on_pushButton_clicked();
    void on_sendFileButton_clicked();
    void sendFileName(QString fileName);
    void on_spinBox_valueChanged(int arg1);

    void on_comboBox_currentIndexChanged(const QString &arg1);

    void on_fontComboBox_currentFontChanged(const QFont &f);

public slots:
    void readMessage(Recorder _recoders);

};

#endif // CHAT_H

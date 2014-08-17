#include "sender.h"
#include "ui_sender.h"
#include <QMessageBox>

Sender::Sender(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Sender)
{
    ui->setupUi(this);
  //  this->setWindowTitle("发送文件");
    this->setFixedSize(360,180);
    tcpPort = 6666;
    tcpServer = new QTcpServer(this);
    //如果有新的连接，说明有接收端已经确认开始接收，那么开始发送文件
    connect(tcpServer,SIGNAL(newConnection()),\
            this,SLOT(sendMessage()));
    initServer();
}
void Sender::initServer()
{
    payLoadSize = 64*1024; //64k
    totalBytes = 0;
    bytesToWrite = 0;
    bytesWriten = 0;
    ui->progressBar->reset();
    ui->serverSendBtn->setEnabled(false);
    ui->serverOpenBtn->setEnabled(true);
    ui->label->setText("Choose file!");
    tcpServer ->close();
}
void Sender::sendMessage()
{
    ui->serverSendBtn->setEnabled(false);
    //获取连接的SOCKET
    clientConnection = tcpServer->nextPendingConnection();
    //This signal is emitted every time a payload of data has been written to the device.
    //The bytes argument is set to the number of bytes that were written in this payload.
    //bytesWritten() is not emitted recursively;
    connect(clientConnection,SIGNAL(bytesWritten(qint64)),\
            this,SLOT(updateClientProgress(qint64)));
    ui->label->setText((tr("开始发送文件 %1").arg(theFileName)));
    //获取要发送文件的文件名
    localFile = new QFile(filename);
    if(! localFile->open(QFile::ReadOnly))
    {
        QMessageBox::warning(this,tr("ERROR"),tr("Can't read file %1:\n%2").arg(filename).arg(localFile->errorString()));
        return;
    }
    //获取文件的大小
    totalBytes = localFile->size();
    QDataStream sendOut(&outBlock,QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_4_7);
    //设置开始时间
    time.start();
    //获取文件名
    QString currentFile = filename.right(filename.size()-filename.lastIndexOf('/')-1);
    sendOut<<qint64(0)<<qint64(0)<<currentFile;
    totalBytes += outBlock.size();
    sendOut.device()->seek(0);
    sendOut<<totalBytes<<qint64((outBlock.size()-sizeof(qint64)*2));
    bytesToWrite = totalBytes-clientConnection->write(outBlock);
    outBlock.resize(0);
}

void Sender::updateClientProgress(qint64 numBytes)
{
    qApp->processEvents();
    bytesWriten += int(numBytes);
    //如果还有文件要发出去
    if(bytesToWrite >0)
    {
        //读取文件
        outBlock = localFile->read(qMin(bytesToWrite,payLoadSize));
        bytesToWrite -= (int)clientConnection->write(outBlock);
        outBlock.resize(0);
    }
    else
    {
        localFile->close();
    }

    ui->progressBar->setMaximum(totalBytes);
    ui->progressBar->setValue(bytesWriten);

    float useTime = time.elapsed();
    double speed = bytesWriten / useTime;
    ui->label->setText(tr("complete %1MB(%2MB/S) \n total:%3MB time:%4秒 \n remain:%5秒")\
                       .arg(bytesWriten/(1024*1024))\
                       .arg(speed*1000/(1024*1024))
                       .arg(totalBytes/(1024*1024))
                       .arg(useTime/1000)
                       .arg(totalBytes/speed/1000 - useTime/1000));

    if(bytesWriten == totalBytes)
    {
        localFile->close();
        tcpServer->close();
        ui->label->setText("complete!");
        this->close();
    }
}

Sender::~Sender()
{
    delete ui;
}

void Sender::on_serverOpenBtn_clicked()
{
    filename = QFileDialog::getOpenFileName(this);
    if(! filename.isEmpty())
    {
        theFileName = filename.right(filename.size() - filename.lastIndexOf('/')-1);
        ui->label->setText(tr("File Name:%1").arg(theFileName));
        ui->serverSendBtn->setEnabled(true);
        ui->serverOpenBtn->setEnabled(false);
    }
}

void Sender::on_serverSendBtn_clicked()
{
   if( !tcpServer->listen(QHostAddress::Any,tcpPort))
   {
       close();
       return;
   }
   ui->label->setText("waiting receiver...");

   //发出消息，通知对方接收
   emit sendFileName(theFileName);
}

void Sender::on_serverCloseBtn_clicked()
{
//    if(tcpServer->isListening())
//    {
//        tcpServer->close();
//        if(localFile->isOpen())
//        {
//            localFile->close();
//        }
//        clientConnection->abort();
//    }
    close();
}
void Sender::refused()
{
    tcpServer->close();
    ui->label->setText("receiver refused!");
}

#include "receiver.h"
#include "ui_receiver.h"

receiver::receiver(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::receiver)
{
    ui->setupUi(this);
    this->setWindowTitle("接收文件");
    setFixedSize(350,180);
    TotalBytes = 0;
    bytesReceived = 0;
    fileNameSize = 0;
    tcpClient = new QTcpSocket(this);
    tcpProt = 6666;
    connect(tcpClient,SIGNAL(readyRead()),\
            this,SLOT(readMessage()));
    connect(tcpClient,SIGNAL(error(QAbstractSocket::SocketError)),\
            this,SLOT(displayError(QAbstractSocket::SocketError)));
}

void receiver::setFileName(QString filename)
{

    localFile = new QFile(filename);
}
void receiver::setHostAddress(QHostAddress address)
{
    hostAddress = address;
    newConnect();
}
void receiver::newConnect()
{
    blockSize = 0;
    tcpClient->abort();
    tcpClient->connectToHost(hostAddress,tcpProt);
    qDebug()<<"receiver:"<<hostAddress<<":"<<tcpProt;
    time.start();
}
void receiver::readMessage()
{
    QDataStream in(tcpClient);
    in.setVersion(QDataStream::Qt_4_7);
    float useTime = time.elapsed();
    double speed = bytesReceived / useTime;
    //如果是发送的文件名，那么接收文件名
    if(bytesReceived <= sizeof(qint64)*2)
    {
        if((tcpClient->bytesAvailable() >= sizeof(qint64)*2) && (fileNameSize == 0))
        {
            in>>TotalBytes>>fileNameSize;
            bytesReceived += sizeof(qint64)*2;

        }
        if((tcpClient->bytesAvailable() >= fileNameSize) && (fileNameSize !=0))
        {
            in>>filename;
            ui->label->setText(tr("receive:%1").arg(filename));
            bytesReceived += fileNameSize;
            if(!localFile->open(QFile::WriteOnly))
            {
                QMessageBox::warning(this,tr("waring"),tr("can't read file:%1").arg(filename));
                return;
            }

        }
        else
        {
            return;
        }

    }
    if(bytesReceived < TotalBytes)
    {
        bytesReceived += tcpClient->bytesAvailable();
        inBlock = tcpClient->readAll();
        localFile->write(inBlock);
        inBlock.resize(0);
    }
    ui->progressBar->setMaximum(TotalBytes);
 //   qDebug()<<"byte received = "<<bytesReceived;
    ui->progressBar->setValue(bytesReceived);

    //补充传输过程信息

    if(bytesReceived == TotalBytes)
    {
        localFile->close();
        tcpClient->close();
        ui->label->setText("Compelete!");
        this->close();
    }


}
void receiver::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    default:
        qDebug()<<tcpClient->errorString();
        break;
    }
}

receiver::~receiver()
{
    delete ui;
}

void receiver::on_calcelBtn_clicked()
{
    tcpClient->abort();
    if(localFile->isOpen())
    {
        localFile->close();
    }
}

void receiver::on_closeBtn_clicked()
{
    tcpClient->abort();
    if(localFile->isOpen())
    {
        localFile->close();
    }
    close();
}

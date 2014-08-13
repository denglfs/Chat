#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QByteArray>
#include <QHostInfo>
#include <QProcess>
#include <QNetworkInterface>
#include <QDateTime>
#include <QScrollBar>
#include <QUdpSocket>
#include <QMessageBox>
#include <QHostAddress>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->userTableWidget->setShowGrid(false);
    xsock = new QTcpSocket(this);
   // QString ip = getIP();
    QString ip = "192.168.1.100";
    qDebug()<<"ip="<<ip;
    xsock->connectToHost(QHostAddress(ip),5001);
    connect(xsock,SIGNAL(readyRead()),\
            this,SLOT(on_xsock_readyRead()));
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    out<<NewParticipant<<getIP()<<QHostInfo::localHostName()<<tr("127.0.0.1");
    xsock->write(data);
    setWindowIcon(QIcon(":/new/prefix1/icon.ico"));
    setWindowTitle("iChat");

    server = new Sender();
    connect(server,SIGNAL(sendFileName(QString)),\
            this,SLOT(getFileName(QString)));
    connect(ui->userTableWidget,SIGNAL(itemDoubleClicked(QTableWidgetItem*)),\
            this,SLOT(on_xChat_clicked()));
}

void MainWindow::getFileName(QString name)
{
    filename = name;
    sendMessage(FileName);
}

//依据发送的消息类型发送数据
void MainWindow::sendMessage(MessageType type, QString destIP)
{
    QByteArray data;
    //将data绑定到数据流
    QDataStream out(&data,QIODevice::WriteOnly);
    //获取主机名
    QString localHostName = QHostInfo::localHostName();
    //获取主机IP
    QString address = getIP();
    //消息类型，用户名，主机名必须写入
    out<<(int)type<<address<<localHostName<<destIP;
    qDebug()<<type<<"+"<<address<<"+"<<localHostName<<"+"<<destIP;
    switch (type)
    {
    case BroadCast:
    case Message:
    {
        if(ui->messageTextEdit->toPlainText() == "")
        {
            QMessageBox::warning(0,tr("waring"),tr("must send one word"),QMessageBox::Ok);
            return;
        }
        //如果是消息，还要写入发送地址和消息
        out<<getMessaget();
        ui->messageBrowser->verticalScrollBar()->setValue(ui->messageBrowser->verticalScrollBar()->maximum());
        break;
    }
    case NewParticipant:
        break;
    case ReplyNewParticipant:
        break;
    case ParticipantLeft:
        break;
    case FileName:
    {
        int row = ui->userTableWidget->currentRow();
        QString clientAddress = ui->userTableWidget->item(row,2)->text();
        out<<address<<clientAddress<<filename;
        break;
    }
    case Refuse:
        break;
    default:
        break;
    }
    //发送数据出去
    xsock->write(data);

}

void MainWindow::on_xsock_readyRead()
{
    qDebug()<<"ready read";
    QDataStream in(xsock);
    int msgeType;
    QString srcIP,srcHostName,destIP;
    in>>msgeType>>srcIP>>srcHostName>>destIP;

    switch (msgeType)
    {
    case Message:
    {
        QString message;
        in>>message;
        Recorder re = {Message,srcIP,destIP,message,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")};
        recoders.push_back(re);
        newMessageSignal(re);
        ui->messageBrowser->append(srcIP);
        ui->messageBrowser->append(destIP);
        ui->messageBrowser->append(message);
        ui->messageBrowser->append(re.time);
        break;
    }
    case BroadCast:
    {
        QString message;
        in>>message;
        ui->messageBrowser->append("=====================");
        ui->messageBrowser->append(srcIP);
        ui->messageBrowser->append(destIP);
        ui->messageBrowser->append(message);
        ui->messageBrowser->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        break;

    }
    case NewParticipant:
    {
        QStringList hostNameList;
        QStringList IPList;
        in>>IPList>>hostNameList;
        newParticipant(hostNameList,IPList);
        break;
    }
    default:
        break;
    }
}


void MainWindow::hasPendingFile(QString userName, QString serverAddress, QString clientAddress, QString filename)
{
    QString ipAddress = getIP();
    if(ipAddress == clientAddress)
    {
        int btn = QMessageBox::information(this,tr("Receive File"),tr("Save the file?"),QMessageBox::Yes,QMessageBox::No);
        if(btn == QMessageBox::Yes)
        {
            //获取要保存的文件名
            QString name = QFileDialog::getSaveFileName(0,tr("Save"),filename);
            if( !name.isEmpty())
            {
                receiver * client = new receiver(this);
                client->setFileName(name);
                client->setHostAddress(QHostAddress(serverAddress));
                client->show();
            }
            else
            {
                sendMessage(Refuse,serverAddress);
            }

        }
        if(btn == QMessageBox::No)
        {
            sendMessage(Refuse,serverAddress);
        }
    }
}

void MainWindow::newParticipant(QStringList hostNameList, QStringList IPList)
{
    //小清空表格
    int rowcount = ui->userTableWidget->rowCount();
    for ( int i = 0 ; i< rowcount ;++i)
        ui->userTableWidget->removeRow(0);

    for(int i = 0 ; i < hostNameList.size() ; ++i)
    {
        //更新table widget
        QTableWidgetItem * host = new QTableWidgetItem(hostNameList.at(i));
        QTableWidgetItem * ip = new QTableWidgetItem(IPList.at(i));
        ui->userTableWidget->insertRow(0);
        ui->userTableWidget->setItem(0,0,ip);
        ui->userTableWidget->setItem(0,1,host);
        ui->userTableWidget->item(0,0)->setTextAlignment(Qt::AlignHCenter);
        ui->userTableWidget->item(0,1)->setTextAlignment(Qt::AlignHCenter);
    }
}
void MainWindow::participantLeft(QString userName, QString localhostName, QString time)
{
    QList<QTableWidgetItem *> tmp = ui->userTableWidget->findItems(localhostName,Qt::MatchExactly);
    if (tmp.isEmpty() == false)
    {
        int rowNum = tmp.first()->row();
        qDebug()<<"rowNum = "<<rowNum;
        ui->userTableWidget->removeRow(rowNum);
        ui->userNumLabel->setText(tr("onlines:%1").arg(ui->userTableWidget->rowCount()));
    }
}
QString MainWindow::getIP()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list)
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol)
            return address.toString();

    }
    return 0;
}

//获取消息
QString MainWindow::getMessaget()
{
    QString msg = ui->messageTextEdit->toPlainText();
    return msg;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_sendButton_clicked()
{
    sendMessage(BroadCast,tr("0.0.0.0")); //随便设置一个目的地址，无用地址
    ui->messageTextEdit->clear();
    ui->messageTextEdit->setFocus();
}

void MainWindow::on_sendFileBtn_clicked()
{
    if(ui->userTableWidget->selectedItems().isEmpty())
    {
        QMessageBox::warning(0,tr("Select a receiver"),tr("Please select a receiver!"),QMessageBox::Ok);
        return;
    }
    server->show();
    server->initServer();
}

void MainWindow::on_xChat_clicked()
{
    qDebug()<<"xxxx";
    if(ui->userTableWidget->selectedItems().isEmpty())
    {
        QMessageBox::warning(0,tr("Select a receiver"),tr("Please select a receiver!"),QMessageBox::Ok);
        return;
    }
    int row = ui->userTableWidget->currentRow();
    QString destIP = ui->userTableWidget->item(row,0)->text();
    QString destHostName = ui->userTableWidget->item(row,1)->text();
    Chat * xchat = new Chat(destIP,destHostName,NULL);
    connect(this,SIGNAL(newMessageSignal(Recorder)),\
            xchat,SLOT(readMessage(Recorder)));
    xchat->readRecoder(&recoders);
    xchat->setSenderSocket(xsock);
    xchat->show();
}

void MainWindow::on_exitButton_clicked()
{
    //发送用户离开消息
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    out<<(int)ParticipantLeft<<getIP()<<QHostInfo::localHostName()<<tr("127.0.0.1");
    xsock->write(data);
    qDebug()<<"main quit:"<<getIP();
    this->hide();
    delete server;
    close();
}
void MainWindow::closeEvent(QCloseEvent *)
{
    on_exitButton_clicked();

}

void MainWindow::on_refButton_clicked()
{

}

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

    xsock = new QTcpSocket(this);
    QString ip = getIP();
    xsock->connectToHost(QHostAddress(ip),5001);
    connect(xsock,SIGNAL(readyRead()),\
            this,SLOT(on_xsock_readyRead()));

    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    out<<NewParticipant<<getIP()<<QHostInfo::localHostName()<<tr("127.0.0.1");
    xsock->write(data);


    setWindowIcon(QIcon(":/new/prefix1/icon.ico"));
    setWindowTitle("iChat");

    udpSocket = new QUdpSocket(this);
    port = 15254;
    udpSocket->bind(port,QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    tcpServer = new QTcpServer(this);
    tcpServer->listen(QHostAddress::Any,port+1);
    connect(tcpServer,SIGNAL(newConnection()),\
            this,SLOT(newXchat()));

    server = new Sender(this);
    //双击用户弹出私聊窗口
    connect(ui->userTableWidget,SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
            this,SLOT(on_xChat_clicked()));
    //获取要发出文件的名字
    connect(server,SIGNAL(sendFileName(QString)),\
            this,SLOT(getFileName(QString)));
    //接收得到消息的处理方式
    connect(udpSocket,SIGNAL(readyRead()),\
            this,SLOT(processPendingDatagrams()));
    //发送上线消息

    counter = 0;
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
    case Message:
        if(ui->messageTextEdit->toPlainText() == "")
        {
            QMessageBox::warning(0,tr("waring"),tr("must send one word"),QMessageBox::Ok);
            return;
        }
        //如果是消息，还要写入发送地址和消息
        out<<getMessaget();
        ui->messageBrowser->verticalScrollBar()->setValue(ui->messageBrowser->verticalScrollBar()->maximum());
        break;
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
        out<<destIP;
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

//接收到消息后的处理方式
void MainWindow::processPendingDatagrams()
{
    while(udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(),datagram.size());
        QDataStream in(&datagram,QIODevice::ReadOnly);
        int messageType;
        in>>messageType;
        QString UserName,localHostName,Ipaddress;//对方的：用户名，主机名，本机IP
        QString Time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        switch (messageType)
        {
        case Message:
        {
            QString message;
            in>>UserName>>localHostName>>Ipaddress>>message;
            if((counter++)%2)
                ui->messageBrowser->setTextColor(Qt::blue);
            else
                ui->messageBrowser->setTextColor(Qt::red);
            ui->messageBrowser->setCurrentFont(QFont("Times new Roman",12));
            ui->messageBrowser->append("["+localHostName+"]   "+Time);
            ui->messageBrowser->append(message);
            break;
        }
         case NewParticipant:
            in>>UserName>>localHostName>>Ipaddress;
            break;
        case ReplyNewParticipant:
            in>>UserName>>localHostName>>Ipaddress;
            break;
        case ParticipantLeft:
            in>>UserName>>localHostName;
            participantLeft(UserName,localHostName,Time);
            break;
        case FileName:
        {
            in>>UserName>>localHostName>>Ipaddress;
            QString clientAddress,filename;
            in>>clientAddress>>filename;
            hasPendingFile(UserName,Ipaddress,clientAddress,filename);

            break;
        }
        case Refuse:
        {
            in>>UserName>>localHostName;
            QString serverAddress;
            in>>serverAddress;
            QString ipAddress = getIP();
            if(ipAddress == serverAddress)
            {
                server->refused();
            }
            break;
        }
//        case xChat:
//        {
//            in>>UserName>>localHostName>>Ipaddress;
//            QString clientAddress;
//            in>>clientAddress;//只有选择的用户才能弹出私聊对话框
//            if(clientAddress == getIP())
//            {
//                Chat * chat = new Chat(NULL);
//                chat->setPort(port+1);
//                chat->m_connect(Ipaddress);
//                chat->setTitle(QHostInfo::localHostName(),localHostName);
//                chat->show();
//                connect(this,SIGNAL(exitSignal()),\
//                        chat,SLOT(close()));
//            }
//        }
        default:
            break;
        }
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
    for(int i = 0 ; i < hostNameList.size() ; ++i)
    {
        //更新table widget
        QTableWidgetItem * host = new QTableWidgetItem(hostNameList.at(i));
        QTableWidgetItem * ip = new QTableWidgetItem(IPList.at(i));
        ui->userTableWidget->insertRow(0);
        ui->userTableWidget->setItem(0,0,ip);
        ui->userTableWidget->setItem(0,1,host);
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
    ui->messageTextEdit->clear();
    ui->messageTextEdit->setFocus();
    return msg;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_sendButton_clicked()
{
    sendMessage(Message,getIP());
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
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    out<<ParticipantLeft<<getIP()<<QHostInfo::localHostName()<<tr("127.0.0.1");
    xsock->write(data);


    this->hide();
    delete server;
    delete udpSocket;
    close();
}
void MainWindow::closeEvent(QCloseEvent *)
{
    exitSignal();
    on_exitButton_clicked();

}
//void MainWindow::newXchat()
//{
//    QTcpSocket * tcpSocket = tcpServer->nextPendingConnection();
//    Chat * chat = new Chat(NULL);
//    chat->setSenderSocket(tcpSocket);

//    int row = ui->userTableWidget->currentRow();
//    QString clientName = ui->userTableWidget->item(row,1)->text();
//    chat->setTitle(QHostInfo::localHostName(),clientName);
//    chat->show();
//    connect(this,SIGNAL(exitSignal()),\
//            chat,SLOT(close()));
//}

void MainWindow::on_refButton_clicked()
{
//    int count = ui->userTableWidget->rowCount();
//    for ( int i = 0; i<count;++i)
//         ui->userTableWidget->removeRow(i);
//    for( int i = 0 ; i < 30 ;i++)
//    {
//        sendMessage(NewParticipant);
//    }
}

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
#include <QBuffer>
#include <QPixmap>
#include <QImageReader>
#include <QFile>
#include <QTimer>
#include<QEventLoop>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    xCount = 0;
    bRecvingImage = false;
    ui->userTableWidget->setShowGrid(false);
    ui->messageTextEdit->installEventFilter(this); //安装事件过滤器
    setWindowIcon(QIcon(":/new/prefix1/icon.ico"));
    setWindowTitle("iChat");

    QString ip = getServerIpFromInpuDialog();
    xsock = new QTcpSocket(this);
    xsock->connectToHost(QHostAddress(ip),5001);

    sendMessage(NewParticipant,tr("127.0.0.1")); //发送自己到来的消息

    connect(xsock,SIGNAL(readyRead()),\
            this,SLOT(my_on_xsock_readyRead()));
    connect(ui->userTableWidget,SIGNAL(itemDoubleClicked(QTableWidgetItem*)),\
            this,SLOT(my_on_xChat_clicked()));
}
QString MainWindow::getServerIpFromInpuDialog()
{
    bool isOK;
    QString text = QInputDialog::getText(NULL,\
                                         "Input Dialog",\
                                         "Please input server's ip",\
                                         QLineEdit::Normal,\
                                         getIP(),\
                                         &isOK);
    if(isOK)
        return text;
    else
        exit(0);
}

//依据发送的消息类型发送数据
void MainWindow::sendMessage(MessageType type, QString destIP,QByteArray *sendAr)
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
    {
        //如果是消息，还要写入发送地址和消息
        QString msg = getMessaget();
        if(msg == tr(""))
            return;
        out<<msg;
        ui->messageBrowser->verticalScrollBar()->setValue(ui->messageBrowser->verticalScrollBar()->maximum());
        break;
    }
    case NewParticipant:
        break;
    case ParticipantLeft:
        break;
    case Image:
    {
        qDebug()<<"send preFix:"<<data.size();
        data.append(*sendAr);
        qDebug()<<"send image:"<<sendAr->size();
        qDebug()<<"send tatal size:"<<data.size();
        break;
    }
    case ImageBraodCast:
    {
        qDebug()<<"send preFix:"<<data.size();
        data.append(*sendAr);
        qDebug()<<"send image:"<<sendAr->size();
        qDebug()<<"send tatal size:"<<data.size();
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

void MainWindow::my_on_xsock_readyRead()
{
    qDebug()<<"ready read";
    QByteArray inBlock;
    inBlock=xsock->readAll();
    QDataStream in(&inBlock,QIODevice::ReadOnly);
    if(!bRecvingImage)
    {
        in>>msgeType>>srcIP>>srcHostName>>destIP;
        if(msgeType == Image || msgeType == ImageBraodCast)
        {
            in>>totalBytes;
            recedBYtes = 0;
            bRecvingImage = true;
            QByteArray tmp;
            QDataStream out(&tmp,QIODevice::WriteOnly);
            out<<msgeType<<srcIP<<srcHostName<<destIP<<totalBytes;
            inBlock.remove(0,tmp.size());
            recvedByteAr.append(inBlock);
            recedBYtes += inBlock.size();
            if(recedBYtes < totalBytes)
                return;
        }
    }
    else
    {
        recvedByteAr.append(inBlock);
        recedBYtes += inBlock.size();
        if(recedBYtes < totalBytes)
            return;
    }

    switch (msgeType)
    {
    case Message:
    {
        QString message;
        in>>message;
        Recorder re = {Message,srcIP,destIP,message,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")};
        recoders.push_back(re);
        newMessageSignal(re);
        break;
    }
    case BroadCast:
    {
        QString message;
        in>>message;
        ui->messageBrowser->append("===============================================");
        ui->messageBrowser->append(QString("[%1] [%2]")\
                                   .arg(srcHostName)\
                                   .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
        ui->messageBrowser->append(message);
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
    case Image:
    {
       // QByteArray data = xsock->read(8192);
        //保存文件
        QString path = tr("%1").arg(xCount++);
        QFile file(path);
        file.resize(0);
        file.open(QIODevice::WriteOnly);
        file.resize(0);
        file.write(recvedByteAr);
        file.close();
        //通知给chat
        path = QString("<img src=\"%1\"/>").arg(path);
        Recorder re = {Image,srcIP,destIP,path,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")};
        recoders.push_back(re);
        newMessageSignal(re);
        recvedByteAr.clear();
        bRecvingImage = false;
        break;
    }
    case ImageBraodCast:
    {
        //QByteArray data = xsock->read(8192);
        //保存文件
        QString path = tr("%1").arg(xCount++);
        QFile file(path);
        file.resize(0);
        file.open(QIODevice::WriteOnly);
        file.resize(0);
        file.write(recvedByteAr);
        file.close();
        path = QString("<img src=\"%1\"/>").arg(path);
        ui->messageBrowser->append("===============================================");
        ui->messageBrowser->append(QString("[%1] [%2]")\
                                   .arg(srcHostName)\
                                   .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
        ui->messageBrowser->insertHtml(path);
        ui->messageBrowser->verticalScrollBar()->setValue\
                (ui->messageBrowser->verticalScrollBar()->maximum());
        recvedByteAr.clear();
        bRecvingImage = false;
        break;
    }
    case FileName:
    {
        QString fileName;
        in>>fileName;
        hasPendingFile(srcHostName,srcIP,destIP,fileName);
        break;
    }
    case Refuse:
    {
        QString placeHolder;
        in>>placeHolder;
        Recorder re = {Refuse,srcIP,destIP,placeHolder,QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")};
        recoders.push_back(re);
        newMessageSignal(re);
        break;
    }
    default:
        break;
    }
}


void MainWindow::hasPendingFile(QString _srcHostName, QString _srcIP, QString _destIP, QString _fileName)
{

    int btn = QMessageBox::information(this,\
                                       tr("Receive File"),\
                                       tr("Save the file?"),\
                                       QMessageBox::Yes,QMessageBox::No);
    if(btn == QMessageBox::Yes)
    {
        //获取要保存的文件名
        QString name = QFileDialog::getSaveFileName(0,tr("Save"),_fileName);
        if( !name.isEmpty())
        {
            receiver * client = new receiver(this);
            client->setFileName(name);
            client->setHostAddress(QHostAddress(_srcIP));
            client->show();
        }
        else
        {
            sendMessage(Refuse,srcIP);
            //添加拒绝接收的代码
        }

    }
    if(btn == QMessageBox::No)
    {
        sendMessage(Refuse,srcIP);
        //添加拒绝接收的代码
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
    ui->userNumLabel->setText(tr("User Count:%1").arg(hostNameList.size()));
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
    if(ui->messageTextEdit->toPlainText() == "")
    {
        QMessageBox::warning(0,tr("waring"),tr("must send one word"),QMessageBox::Ok);
        return tr("");
    }
    QString msg = ui->messageTextEdit->toHtml();
    if(msg.size() > 4096)
    {
        msg = msg.left(4096);
        QMessageBox::warning(NULL,"waring","too much words",QMessageBox::Ok);
    }
    return msg;
}

MainWindow::~MainWindow()
{
    qDebug()<<"destrutor";
    delete ui;
}

void MainWindow::on_sendButton_clicked()
{
    sendMessage(BroadCast,tr("0.0.0.0")); //随便设置一个目的地址，无用地址
    ui->messageTextEdit->clear();
    ui->messageTextEdit->setFocus();
}

void MainWindow::my_on_xChat_clicked()
{
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
void MainWindow::closeEvent(QCloseEvent *)
{
    //发送用户离开消息
    this->hide();
    sendMessage(ParticipantLeft,tr("127.0.0.1"));
    QEventLoop eventloop;
    QTimer::singleShot(2000, &eventloop, SLOT(quit()));
    eventloop.exec();
    qDebug()<<"main quit:"<<getIP();

}

void MainWindow::on_refButton_clicked()
{
    sendMessage(NewParticipant,tr("127.0.0.1")); //发送自己到来的消息,用来获取用户列表
}

void MainWindow::on_pushButton_clicked()
{
    QString fileName =\
            QFileDialog::getOpenFileName(this,\
                                         tr("Open Image"),"", tr("Image Files (*.png *.jpg *.bmp)"));
    QFile imageFile(fileName);
    imageFile.open(QIODevice::ReadOnly);
    QByteArray imageAr=imageFile.readAll();
    QByteArray tmp;
    QDataStream out(&tmp,QIODevice::WriteOnly);
    out<<(int)imageAr.size();
    tmp.append(imageAr);
    sendMessage(ImageBraodCast,getIP(),&tmp);

}

void MainWindow::on_fontComboBox_currentFontChanged(const QFont &f)
{
    ui->messageTextEdit->setCurrentFont(f);
    ui->messageTextEdit->setFocus();
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    ui->messageTextEdit->setFontPointSize(arg1*1.0);
    ui->messageTextEdit->setFocus();
}


void MainWindow::on_comboBox_currentIndexChanged(const QString &arg1)
{
    if( tr("red") == arg1)
        ui->messageTextEdit->setTextColor(Qt::red);
    if( tr("blue") == arg1)
        ui->messageTextEdit->setTextColor(Qt::blue);
    if(  tr("black") == arg1)
        ui->messageTextEdit->setTextColor(Qt::black);
    if( tr("green") == arg1)
        ui->messageTextEdit->setTextColor(Qt::green);
    ui->messageTextEdit->setFocus();
}
bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
    Q_ASSERT(obj == ui->messageTextEdit);
    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent *event = static_cast<QKeyEvent*>(e);
        if (event->key() == Qt::Key_Return && (event->modifiers() & Qt::ControlModifier))
        {
            on_sendButton_clicked();
            return true;
        }
    }
    return false;
}

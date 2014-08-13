#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QByteArray>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    port = 5001;
    tcpServer = new QTcpServer(this);
    connect(tcpServer,SIGNAL(newConnection()),\
            this,SLOT(on_newConnection()));
    tcpServer->listen(QHostAddress::Any,port);
}
void MainWindow::on_newConnection()
{
    qDebug()<<"new connection";
    thread = new Thread(&items,tcpServer,ui->tableWidget,&mutex);
    if(thread->isRunning() == false)
    {
        thread->start();
    }
    threads.push_back(thread);
}


MainWindow::~MainWindow()
{
    delete ui;
    delete tcpServer;
    delete tcpSocket;
}

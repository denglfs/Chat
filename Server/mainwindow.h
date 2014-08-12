#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include "thread.h"
#include "Structor.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    QTcpServer * tcpServer;
    QTcpSocket * tcpSocket;
    QVector<Item> items;
    QVector<Thread*> threads;
    Thread * thread;

    int port;
    Ui::MainWindow *ui;
private slots:
    void on_newConnection();
};

#endif // MAINWINDOW_H

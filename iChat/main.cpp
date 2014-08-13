#include "mainwindow.h"
#include <QApplication>
#include <QApplication>
#include <QTextCodec>
#include <qtextcodec.h>
#if _MSC_VER >= 1600
    #pragma execution_character_set("utf-8")
#endif
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.show();

    return a.exec();
}

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#include "mainwindow.h"
#include "qgsapplication.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QgsApplication a(argc, argv, true);
    MainWindow w;
    w.show();
    return a.exec();
}

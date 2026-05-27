#include <QApplication>
#include "mainwindow.h"
#include <vd2/system/text.h>
#include <iostream>

int main(int argc, char *argv[])
{
    // Initialize VirtualDub system core
    // In actual VD, there's init code, we'll try a basic setup or just let it be for now.
    // VDInit(); // We would call initialization if VD exposes a simple one.

    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    int ret = app.exec();

    // VDDeinit();
    return ret;
}

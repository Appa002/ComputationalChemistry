#include "ui/mainwindow.h"
#include "rendering/globalprogrammemanager.h"
#include <QApplication>
#include <iostream>
int main(int argc, char *argv[])
{
    std::cout << "hi" << std::endl;

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    int out = a.exec();
    GlobalProgrammeManager::del();
    return out;
}

#include "mainwindow.h"
#include <QApplication>


Q_DECLARE_METATYPE(MidiMessage) // defined in workerthread.h

int main(int argc, char *argv[])
{
    qRegisterMetaType<MidiMessage>();

    QApplication a(argc, argv);
    MainWindow w;

    // Disable titleBar
    w.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    w.show();
    return a.exec();
}

#include <QCoreApplication>
#include "cUSBDetectHotplug.h"
#include <QThread>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QThread *usbDetectThread = new QThread();
    cUSBDetectHotplug *usbDetector = new cUSBDetectHotplug();
    usbDetector->moveToThread(usbDetectThread);
    QObject::connect(usbDetectThread, SIGNAL(started()), usbDetector, SLOT(mainLoop()));
    QObject::connect(usbDetector, SIGNAL(finished()), usbDetectThread, SLOT(quit()));
    usbDetectThread->start();
    return a.exec();
}

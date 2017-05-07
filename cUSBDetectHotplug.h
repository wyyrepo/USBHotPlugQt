#ifndef CUSBDETECTHOTPLUG_H
#define CUSBDETECTHOTPLUG_H

#include <QObject>
#include <QThread>
#include <QMutex>

class cUSBDetectHotplug : public QObject
{
    Q_OBJECT
public:
    explicit cUSBDetectHotplug(QObject *parent = 0);
    ~cUSBDetectHotplug();
    void abort();
private:
    int usbdevice_init();
    int usbdevice_deinit();
private:
    bool m_Abort;
    QMutex m_Mutex;
signals:
    void finished();
    void sigUSBInitFailed();
    void sigUSBDeviceDetected(quint16 PID, quint16 VID);
    void sigUSBDeviceRemoved();
public slots:
    void mainLoop();
};

#endif // CUSBDETECTHOTPLUG_H

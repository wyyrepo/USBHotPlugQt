#include "cUSBDetectHotplug.h"
#include <libusb-1.0/libusb.h>
#include <QDebug>

#define USB_ATTACHED                            0x00
#define USB_DETACHED                            0x01

libusb_device_handle *ghandle = NULL;
bool m_Presented;
quint16 m_PID, m_VID;

static int LIBUSB_CALL hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    Q_UNUSED(ctx);
    Q_UNUSED(event);
    Q_UNUSED(user_data);
    struct libusb_device_descriptor desc;
    int rc;

    rc = libusb_get_device_descriptor(dev, &desc);
    m_Presented = true;
    if (LIBUSB_SUCCESS != rc)
    {
        qDebug() << "USB Device Detected But Error getting device descriptor";
        m_PID = 0xFF;
        m_VID = 0xFF;
    }
    else
    {
        qDebug() << "Device attached: " << desc.idVendor << desc.idProduct;
        m_PID = desc.idProduct;
        m_VID = desc.idVendor;
    }
    if (ghandle) {
        libusb_close (ghandle);
        ghandle = NULL;
    }
    return 0;
}

static int LIBUSB_CALL hotplug_callback_detach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    Q_UNUSED(ctx);
    Q_UNUSED(dev);
    Q_UNUSED(event);
    Q_UNUSED(user_data);
    m_Presented = false;
    if (ghandle) {
        libusb_close (ghandle);
        ghandle = NULL;
    }
    qDebug() << "Deivice detached";
    return 0;
}

cUSBDetectHotplug::cUSBDetectHotplug(QObject *parent) : QObject(parent)
{

}

cUSBDetectHotplug::~cUSBDetectHotplug()
{

}

void cUSBDetectHotplug::abort()
{
    QMutexLocker locker(&m_Mutex);
    m_Abort = true;
}

int cUSBDetectHotplug::usbdevice_init()
{
    libusb_hotplug_callback_handle hp[2];
    int product_id, vendor_id, class_id;
    int rc;
    vendor_id  = LIBUSB_HOTPLUG_MATCH_ANY;
    product_id = LIBUSB_HOTPLUG_MATCH_ANY;
    class_id   = LIBUSB_HOTPLUG_MATCH_ANY;

    rc = libusb_init (NULL);
    if (rc < 0)
    {
        qDebug() << "failed to initialise libusb:  " << libusb_error_name(rc);
        return -1;
    }

    if (!libusb_has_capability (LIBUSB_CAP_HAS_HOTPLUG)) {
        qDebug() << "Hotplug capabilites are not supported on this platform";
        libusb_exit (NULL);
        return -1;
    }

    rc = libusb_hotplug_register_callback (NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, LIBUSB_HOTPLUG_NO_FLAGS, vendor_id,
        product_id, class_id, hotplug_callback, NULL, &hp[0]);
    if (LIBUSB_SUCCESS != rc) {
        qDebug() << "Error registering callback";
        libusb_exit (NULL);
        return -1;
    }

    rc = libusb_hotplug_register_callback (NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, LIBUSB_HOTPLUG_NO_FLAGS, vendor_id,
        product_id,class_id, hotplug_callback_detach, NULL, &hp[1]);
    if (LIBUSB_SUCCESS != rc) {
        qDebug() << "Error registering callback";
        libusb_exit (NULL);
        return -1;
    }
    return 0;
}

int cUSBDetectHotplug::usbdevice_deinit()
{
    if (ghandle) {
        libusb_close (ghandle);
    }

    libusb_exit (NULL);
    return 0;
}



void cUSBDetectHotplug::mainLoop()
{
    int rc;
    if (usbdevice_init() < 0)
    {
        emit sigUSBInitFailed();
        abort();
    }
    forever
    {
        m_Mutex.lock();
        if (m_Abort)
        {
            emit finished();
            usbdevice_deinit();
            m_Mutex.unlock();
            return;
        }
        m_Mutex.unlock();
        rc = libusb_handle_events (NULL);
        if (rc < 0)
            qDebug() << "libusb_handle_events() failed: " <<  libusb_error_name(rc);
        else
        {
            if (m_Presented)
                emit sigUSBDeviceDetected(m_PID, m_VID);
            else
                emit sigUSBDeviceRemoved();
        }
    }
}

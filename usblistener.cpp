#include "usblistener.h"
#include <QWidget>

UsbListener::UsbListener(QWidget *parent)
    : QWidget{parent},
    QAbstractNativeEventFilter()
{

}


bool UsbListener::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    MSG* msg = reinterpret_cast<MSG*>(message);
    int msgType = msg->message;
    if(msgType == WM_DEVICECHANGE)
    {
        PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)msg->lParam;
        switch (msg->wParam)
        {
        case DBT_DEVICEARRIVAL:
            if(lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                if(lpdbv->dbcv_flags ==0)
                {
                    emit usbConn();
                }
            }
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            if(lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                if(lpdbv->dbcv_flags == 0)
                {
                    emit usbRemove();
                }
            }
            break;

        default:
            break;
        }
    }
    return QWidget::nativeEvent(eventType, message, result);
}

#ifndef USBLISTENER_H
#define USBLISTENER_H

#include <QAbstractNativeEventFilter>
#include <QWidget>
#include <windows.h>
#include <dbt.h>

class UsbListener : public QWidget, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit UsbListener(QWidget *parent = nullptr);
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result);

Q_SIGNALS:
    void usbConn();
    void usbRemove();


signals:

};

#endif // USBLISTENER_H

/*
 * This file was generated by dbusxml2cpp version 0.6
 * Command line was: dbusxml2cpp -m -a resourceadaptor -i resourcebase.h -l Akonadi::ResourceBase ../../include/org.kde.Akonadi.Resource.xml
 *
 * dbusxml2cpp is Copyright (C) 2006 Trolltech ASA. All rights reserved.
 *
 * This is an auto-generated file and manually modified file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef RESOURCEADAPTOR_H_1171814422
#define RESOURCEADAPTOR_H_1171814422

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "resourcebase.h"
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;

/*
 * Adaptor class for interface org.kde.Akonadi.Resource
 */
class ResourceAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Akonadi.Resource")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.kde.Akonadi.Resource\" >\n"
"    <signal name=\"statusChanged\" >\n"
"      <arg direction=\"out\" type=\"i\" name=\"status\" />\n"
"      <arg direction=\"out\" type=\"s\" name=\"message\" />\n"
"    </signal>\n"
"    <signal name=\"progressChanged\" >\n"
"      <arg direction=\"out\" type=\"u\" name=\"progress\" />\n"
"      <arg direction=\"out\" type=\"s\" name=\"message\" />\n"
"    </signal>\n"
"    <signal name=\"nameChanged\" >\n"
"      <arg direction=\"out\" type=\"s\" name=\"name\" />\n"
"    </signal>\n"
"    <method name=\"quit\" />\n"
"    <method name=\"status\" >\n"
"      <arg direction=\"out\" type=\"i\" />\n"
"    </method>\n"
"    <method name=\"statusMessage\" >\n"
"      <arg direction=\"out\" type=\"s\" />\n"
"    </method>\n"
"    <method name=\"progress\" >\n"
"      <arg direction=\"out\" type=\"u\" />\n"
"    </method>\n"
"    <method name=\"progressMessage\" >\n"
"      <arg direction=\"out\" type=\"s\" />\n"
"    </method>\n"
"    <method name=\"requestItemDelivery\" >\n"
"      <arg direction=\"out\" type=\"b\" />\n"
"      <arg direction=\"in\" type=\"i\" name=\"uid\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"remotedId\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"collection\" />\n"
"      <arg direction=\"in\" type=\"i\" name=\"type\" />\n"
"    </method>\n"
"    <method name=\"configure\" >\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"synchronize\" >\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"setName\" >\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"name\" />\n"
"    </method>\n"
"    <method name=\"name\" >\n"
"      <arg direction=\"out\" type=\"s\" />\n"
"    </method>\n"
"    <method name=\"cleanup\" />\n"
"    <method name=\"enableChangeRecording\" >\n"
"      <arg direction=\"in\" type=\"b\" />\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    ResourceAdaptor(Akonadi::ResourceBase *parent);
    virtual ~ResourceAdaptor();

    inline Akonadi::ResourceBase *parent() const
    { return static_cast<Akonadi::ResourceBase *>(QObject::parent()); }

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void cleanup();
    Q_NOREPLY void configure();
    void enableChangeRecording(bool in0);
    QString name();
    uint progress();
    QString progressMessage();
    void quit();
    bool requestItemDelivery(int uid, const QString &remotedId, const QString &collection, int type, const QDBusMessage &msg);
    Q_NOREPLY void setName(const QString &name);
    int status();
    QString statusMessage();
    Q_NOREPLY void synchronize();
Q_SIGNALS: // SIGNALS
    void nameChanged(const QString &name);
    void progressChanged(uint progress, const QString &message);
    void statusChanged(int status, const QString &message);
};

#endif

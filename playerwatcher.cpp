#include "playerwatcher.h"

#include "player_interface.h"
#include "properties_interface.h"

#include <QDBusServiceWatcher>
#include <QDBusConnection>

#include <iostream>

using namespace std;

namespace DBusSoundRecorder {

inline ostream &operator <<(ostream &stream, const QString &str)
{
    stream << str.toLocal8Bit().data();
    return stream;
}

PlayerWatcher::PlayerWatcher(const QString &appName, QObject *parent) :
    QObject(parent),
    m_service(QStringLiteral("org.mpris.MediaPlayer2.%1").arg(appName)),
    m_serviceWatcher(new QDBusServiceWatcher(m_service, QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange, this)),
    m_propertiesInterface(new OrgFreedesktopDBusPropertiesInterface(m_service, QStringLiteral("/org/mpris/MediaPlayer2"), QDBusConnection::sessionBus(), this)),
    m_playerInterface(new OrgMprisMediaPlayer2PlayerInterface(m_service, QStringLiteral("/org/mpris/MediaPlayer2"), QDBusConnection::sessionBus(), this)),
    m_trackNumber(0)
{
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &PlayerWatcher::serviceOwnerChanged);
    connect(m_propertiesInterface, &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged, this, &PlayerWatcher::propertiesChanged);
}

void PlayerWatcher::serviceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner)
{
    if(oldOwner.isEmpty()) {
        cerr << "MPRIS service \"" << service << "\" is now online" << endl;
    }
    if(newOwner.isEmpty()) {
        cerr << "MPRIS service \"" << service << "\" went offline" << endl;
    }
}

void PlayerWatcher::propertiesChanged(const QString &, const QVariantMap &, const QStringList &)
{
    // get meta data
    QVariantMap metadata = m_playerInterface->metadata();
    QString title = metadata.value(QStringLiteral("xesam:title")).toString();
    QString album = metadata.value(QStringLiteral("xesam:album")).toString();
    QString artist = metadata.value(QStringLiteral("xesam:artist")).toString();
    // use title, album and artist to identify song
    if(m_title != title || m_album != album || m_artist != artist) {
        // next song playing
        m_title = title;
        m_album = album;
        m_artist = artist;
        // read additional meta data
        m_year = metadata.value(QStringLiteral("xesam:contentCreated")).toString();
        m_genre = metadata.value(QStringLiteral("xesam:genre")).toString();
        m_trackNumber = metadata.value(QStringLiteral("xesam:tracknumber")).toUInt();
        // notify
        cerr << "Next song: " << m_title << endl;
        emit nextSong();
    }
}

}

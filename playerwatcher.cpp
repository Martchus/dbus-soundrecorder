#include "playerwatcher.h"

#include "playerinterface.h"
#include "propertiesinterface.h"

#include <QDBusConnection>
#include <QDBusServiceWatcher>

#include <iostream>

using namespace std;
using namespace CppUtilities;

namespace DBusSoundRecorder {

inline ostream &operator<<(ostream &stream, const QString &str)
{
    return stream << str.toLocal8Bit().data();
}

PlayerWatcher::PlayerWatcher(const QString &appName, bool ignorePlaybackStatus, QObject *parent)
    : QObject(parent)
    , m_mediaPlayerInterfaceName(QStringLiteral("org.mpris.MediaPlayer2.%1").arg(appName))
    , m_mediaPlayerServiceWatcher(
          new QDBusServiceWatcher(m_mediaPlayerInterfaceName, QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange, this))
    , m_propertiesInterface(new OrgFreedesktopDBusPropertiesInterface(
          m_mediaPlayerInterfaceName, QStringLiteral("/org/mpris/MediaPlayer2"), QDBusConnection::sessionBus(), this))
    , m_playerInterface(new OrgMprisMediaPlayer2PlayerInterface(
          m_mediaPlayerInterfaceName, QStringLiteral("/org/mpris/MediaPlayer2"), QDBusConnection::sessionBus(), this))
    , m_isPlaying(false)
    , m_isAd(false)
    , m_trackNumber(0)
    , m_diskNumber(0)
    , m_silent(false)
    , m_ignorePlaybackStatus(ignorePlaybackStatus)
{
    if (!connect(m_mediaPlayerServiceWatcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &PlayerWatcher::serviceOwnerChanged)) {
        cout << "Warning: Unable to connect \"serviceOwnerChanged\" signal of service watcher." << endl;
    }
    // The code below might not work with some players:
    //if(!connect(m_propertiesInterface, &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged, this, &PlayerWatcher::propertiesChanged)) {
    //    cout << "Warning: Unable to connect \"PropertiesChanged\" signal of properties interface." << endl;
    //}
    // However, the following seems to work always:
    if (!QDBusConnection::sessionBus().connect(m_mediaPlayerInterfaceName, QStringLiteral("/org/mpris/MediaPlayer2"),
            QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("PropertiesChanged"), this, SLOT(propertiesChanged()))) {
        cout << "Warning: Unable to connect \"PropertiesChanged\" signal of properties interface." << endl;
    }
    propertiesChanged();
}

void PlayerWatcher::play()
{
    m_playerInterface->Play();
}

void PlayerWatcher::stop()
{
    m_playerInterface->Stop();
}

void PlayerWatcher::pause()
{
    m_playerInterface->Pause();
}

void PlayerWatcher::playPause()
{
    m_playerInterface->PlayPause();
}

void PlayerWatcher::serviceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner)
{
    if (oldOwner.isEmpty()) {
        cerr << "MPRIS service \"" << service << "\" is now online" << endl;
    }
    if (newOwner.isEmpty()) {
        cerr << "MPRIS service \"" << service << "\" went offline" << endl;
    }
}

void PlayerWatcher::propertiesChanged()
{
    // get meta data
    QVariantMap metadata = m_playerInterface->metadata();
    m_isAd = metadata.value(QStringLiteral("mpris:trackid")).toString().startsWith(QLatin1String("spotify:ad"));
    QString title = metadata.value(QStringLiteral("xesam:title")).toString();
    QString album = metadata.value(QStringLiteral("xesam:album")).toString();
    QString artist = metadata.value(QStringLiteral("xesam:artist")).toString();
    bool isPlaying;
    if (m_ignorePlaybackStatus) {
        // determine playback status by checking whether there is a song title
        isPlaying = !title.isEmpty();
    } else {
        isPlaying = !m_playerInterface->playbackStatus().compare(QLatin1String("playing"), Qt::CaseInsensitive);
    }
    if (isPlaying) {
        if (!m_isPlaying) {
            cerr << "Playback started" << endl;
        }
        // use title, album and artist to identify song
        if (m_title != title || m_album != album || m_artist != artist) {
            // next song playing
            m_title = title;
            m_album = album;
            m_artist = artist;
            // read additional meta data
            m_year = metadata.value(QStringLiteral("xesam:contentCreated")).toString();
            m_genre = metadata.value(QStringLiteral("xesam:genre")).toString();
            m_trackNumber = metadata.value(QStringLiteral("xesam:tracknumber")).toUInt();
            if (!m_trackNumber) {
                m_trackNumber = metadata.value(QStringLiteral("xesam:trackNumber")).toUInt();
            }
            m_diskNumber = metadata.value(QStringLiteral("xesam:discnumber")).toUInt();
            if (!m_diskNumber) {
                m_diskNumber = metadata.value(QStringLiteral("xesam:discNumber")).toUInt();
            }
            m_length = TimeSpan(metadata.value(QStringLiteral("mpris:length")).toULongLong() * 10);
            // notify
            cerr << "Next song: " << m_title << endl;
            if (!m_isPlaying && !m_silent) {
                m_isPlaying = true;
                emit playbackStarted();
            }
            if (!m_silent) {
                m_isPlaying = true;
                emit nextSong();
            }
        } else if (!m_isPlaying) {
            if (!m_silent) {
                m_isPlaying = true;
                emit playbackStarted();
            }
        }
    } else if (m_isPlaying) {
        m_isPlaying = false;
        cerr << "Playback stopped" << endl;
        if (!m_silent) {
            emit playbackStopped();
        }
    }
}

void PlayerWatcher::notificationReceived()
{
    cout << "It works!" << endl;
}

void PlayerWatcher::seeked(qlonglong pos)
{
    cerr << "Seeked: " << pos << endl;
}
} // namespace DBusSoundRecorder

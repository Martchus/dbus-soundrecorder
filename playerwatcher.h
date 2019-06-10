#ifndef PLAYERWATCHER_H
#define PLAYERWATCHER_H

#include <c++utilities/chrono/timespan.h>

#include <QObject>

QT_FORWARD_DECLARE_CLASS(QDBusServiceWatcher)

class OrgFreedesktopDBusPropertiesInterface;
class OrgMprisMediaPlayer2PlayerInterface;

namespace DBusSoundRecorder {

class PlayerWatcher : public QObject {
    Q_OBJECT
public:
    explicit PlayerWatcher(const QString &appName, bool ignorePlaybackStatus = false, QObject *parent = nullptr);

    void play();
    void stop();
    void pause();
    void playPause();

    bool isPlaying() const;
    bool isPlaybackStatusIgnored() const;
    bool isAd() const;
    const QString &title() const;
    const QString &album() const;
    const QString &artist() const;
    const QString &year() const;
    const QString &genre() const;
    unsigned int trackNumber() const;
    unsigned int diskNumber() const;
    CppUtilities::TimeSpan length() const;
    void setSilent(bool silent);

signals:
    void nextSong();
    void playbackStarted();
    void playbackStopped();

private slots:
    void serviceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner);
    void propertiesChanged();
    void notificationReceived();
    void seeked(qlonglong pos);

private:
    QString m_mediaPlayerInterfaceName;
    QDBusServiceWatcher *m_mediaPlayerServiceWatcher;
    QString m_notifyInterfaceName;
    QDBusServiceWatcher *m_notifyServiceWatcher;
    OrgFreedesktopDBusPropertiesInterface *m_propertiesInterface;
    OrgMprisMediaPlayer2PlayerInterface *m_playerInterface;
    bool m_isPlaying;
    bool m_isAd;
    QString m_title;
    QString m_album;
    QString m_artist;
    QString m_year;
    QString m_genre;
    unsigned int m_trackNumber;
    unsigned int m_diskNumber;
    CppUtilities::TimeSpan m_length;
    bool m_silent;
    bool m_ignorePlaybackStatus;
};

inline bool PlayerWatcher::isPlaying() const
{
    return m_isPlaying;
}

inline bool PlayerWatcher::isPlaybackStatusIgnored() const
{
    return m_ignorePlaybackStatus;
}

inline bool PlayerWatcher::isAd() const
{
    return m_isAd;
}

inline const QString &PlayerWatcher::title() const
{
    return m_title;
}

inline const QString &PlayerWatcher::album() const
{
    return m_album;
}

inline const QString &PlayerWatcher::artist() const
{
    return m_artist;
}

inline const QString &PlayerWatcher::year() const
{
    return m_year;
}

inline const QString &PlayerWatcher::genre() const
{
    return m_genre;
}

inline unsigned int PlayerWatcher::trackNumber() const
{
    return m_trackNumber;
}

inline unsigned int PlayerWatcher::diskNumber() const
{
    return m_diskNumber;
}

inline CppUtilities::TimeSpan PlayerWatcher::length() const
{
    return m_length;
}

inline void PlayerWatcher::setSilent(bool silent)
{
    m_silent = silent;
}
}

#endif // PLAYERWATCHER_H

#ifndef PLAYERWATCHER_H
#define PLAYERWATCHER_H

#include <QObject>

QT_FORWARD_DECLARE_CLASS(QDBusServiceWatcher)

class OrgFreedesktopDBusPropertiesInterface;
class OrgMprisMediaPlayer2PlayerInterface;

namespace DBusSoundRecorder {

class PlayerWatcher : public QObject
{
    Q_OBJECT
public:
    explicit PlayerWatcher(const QString &appName, QObject *parent = nullptr);

    const QString &title() const;
    const QString &album() const;
    const QString &artist() const;
    const QString &year() const;
    const QString &genre() const;
    unsigned int trackNumber() const;

signals:
    void nextSong();

private slots:
    void serviceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner);
    void propertiesChanged(const QString &, const QVariantMap &, const QStringList &);

private:
    QString m_service;
    QDBusServiceWatcher *m_serviceWatcher;
    OrgFreedesktopDBusPropertiesInterface *m_propertiesInterface;
    OrgMprisMediaPlayer2PlayerInterface *m_playerInterface;
    QString m_title;
    QString m_album;
    QString m_artist;
    QString m_year;
    QString m_genre;
    unsigned int m_trackNumber;
};

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

}

#endif // PLAYERWATCHER_H

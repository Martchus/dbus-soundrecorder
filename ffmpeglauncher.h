#ifndef FFMPEGLAUNCHER_H
#define FFMPEGLAUNCHER_H

#include <QObject>
#include <QProcess>
#include <QDir>

namespace DBusSoundRecorder {

class PlayerWatcher;

class FfmpegLauncher : public QObject
{
    Q_OBJECT
public:
    explicit FfmpegLauncher(PlayerWatcher &watcher, QObject *parent = nullptr);

    void setSink(const QString &sinkName);
    void setFfmpegBinary(const QString &path);
    void setFfmpegOptions(const QString &options);
    void setTargetDir(const QString &path);
    void setTargetExtension(const QString &extension);

private slots:
    void nextSong();
    void ffmpegStarted();
    void ffmpegError();
    void ffmpegFinished(int exitCode);

private:
    PlayerWatcher &m_watcher;
    QString m_sink;
    QStringList m_options;
    QDir m_targetDir;
    QString m_targetExtension;
    QProcess *m_ffmpeg;
};

inline void FfmpegLauncher::setSink(const QString &sinkName)
{
    m_sink = sinkName;
}

inline void FfmpegLauncher::setFfmpegBinary(const QString &path)
{
    m_ffmpeg->setProgram(path);
}

inline void FfmpegLauncher::setFfmpegOptions(const QString &options)
{
    m_options = options.split(QChar(' '), QString::SkipEmptyParts);
}

inline void FfmpegLauncher::setTargetDir(const QString &path)
{
    m_targetDir = QDir(path);
}

inline void FfmpegLauncher::setTargetExtension(const QString &extension)
{
    m_targetExtension = extension.startsWith(QChar('.')) ? extension : QStringLiteral(".") + extension;
}

}

#endif // FFMPEGLAUNCHER_H

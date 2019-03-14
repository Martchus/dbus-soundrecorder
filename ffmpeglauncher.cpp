#include "ffmpeglauncher.h"
#include "playerwatcher.h"

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/inifile.h>

#include <QStringBuilder>

#include <fstream>
#include <iostream>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;

namespace DBusSoundRecorder {

inline ostream &operator<<(ostream &stream, const QString &str)
{
    stream << str.toLocal8Bit().data();
    return stream;
}

inline QString validFileName(const QString &text)
{
    QString copy(text);
    copy.replace(QChar('\\'), QLatin1String(" - "))
        .replace(QChar('/'), QLatin1String(" - "))
        .replace(QChar('\n'), QString())
        .replace(QChar('\r'), QString())
        .replace(QChar('\f'), QString())
        .replace(QChar('<'), QString())
        .replace(QChar('>'), QString())
        .replace(QChar('?'), QString())
        .replace(QChar('*'), QString())
        .replace(QChar('!'), QString())
        .replace(QChar('|'), QString())
        .replace(QLatin1String(": "), QLatin1String(" - "))
        .replace(QChar(':'), QChar('-'));
    return copy;
}

FfmpegLauncher::FfmpegLauncher(PlayerWatcher &watcher, QObject *parent)
    : QObject(parent)
    , m_watcher(watcher)
    , m_sink(QStringLiteral("default"))
    , m_inputOptions()
    , m_options()
    , m_targetDir(QStringLiteral("."))
    , m_targetExtension(QStringLiteral(".m4a"))
    , m_ffmpeg(new QProcess(this))
{
    connect(&watcher, &PlayerWatcher::nextSong, this, &FfmpegLauncher::nextSong);
    connect(&watcher, &PlayerWatcher::playbackStopped, this, &FfmpegLauncher::stopFfmpeg);
    connect(m_ffmpeg, &QProcess::started, this, &FfmpegLauncher::ffmpegStarted);
    connect(m_ffmpeg, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error), this, &FfmpegLauncher::ffmpegError);
    connect(m_ffmpeg, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, &FfmpegLauncher::ffmpegFinished);
    m_ffmpeg->setProgram(QStringLiteral("ffmpeg"));
    m_ffmpeg->setProcessChannelMode(QProcess::ForwardedChannels);
}

void addMetaData(QStringList &args, const QString &field, const QString &value)
{
    if (!value.isEmpty()) {
        args << QStringLiteral("-metadata");
        args << QStringLiteral("%1=%2").arg(field, value);
    }
}

void FfmpegLauncher::nextSong()
{
    // skip ads
    if (m_watcher.isAd()) {
        return;
    }
    // pause player until ffmpeg has been started
    m_watcher.setSilent(true);
    if (m_watcher.isPlaying()) {
        m_watcher.pause();
    }
    // terminate/kill the current process
    stopFfmpeg();
    // determine output file, create target directory
    static const QString miscCategory(QStringLiteral("misc"));
    static const QString unknownTitle(QStringLiteral("unknown track"));
    const auto targetDirPath = QStringLiteral("%1/%2").arg(m_watcher.artist().isEmpty() ? miscCategory : validFileName(m_watcher.artist()),
        m_watcher.artist().isEmpty() ? miscCategory : validFileName(m_watcher.album()));
    if (!m_targetDir.mkpath(targetDirPath)) {
        cerr << "Error: Can not create target directory: " << targetDirPath << endl;
        return;
    }
    QDir targetDir(m_targetDir);
    targetDir.cd(targetDirPath);
    // determine track number
    QString number, length, year, genre, totalTracks, totalDisks;
    if (m_watcher.trackNumber()) {
        if (m_watcher.diskNumber()) {
            number = QStringLiteral("%2-%1").arg(m_watcher.trackNumber(), 2, 10, QLatin1Char('0')).arg(m_watcher.diskNumber());
        } else {
            number = QStringLiteral("%1").arg(m_watcher.trackNumber(), 2, 10, QLatin1Char('0'));
        }
    }
    if (!number.isEmpty()) {
        number.append(QStringLiteral(" - "));
    }
    // read additional meta info
    //  - from an INI file called info.ini in the album directory (must be created before recording)
    //  - track lengths might be specified for each track in the [length] section (useful to get rid of advertisements at the end)
    //  - year, genre, total_tracks and total_disks might be specified in the [general] section
    if (targetDir.exists(QStringLiteral("info.ini"))) {
        fstream infoFile;
        infoFile.exceptions(ios_base::badbit | ios_base::failbit);
        try {
            infoFile.open((targetDir.path() + QStringLiteral("/info.ini")).toLocal8Bit().data(), ios_base::in);
            IniFile infoIni;
            infoIni.parse(infoFile);
            for (auto &scope : infoIni.data()) {
                if (scope.first == "length") {
                    if (m_watcher.trackNumber()) {
                        // reading length scope is only possible if track number known because the track number is used for mapping
                        for (const auto &entry : scope.second) {
                            try {
                                if (stringToNumber<unsigned int>(entry.first) == m_watcher.trackNumber()) {
                                    // length entry for this track
                                    length = QString::fromLocal8Bit(entry.second.data());
                                    break;
                                }
                            } catch (const ConversionException &) {
                                cerr << "Warning: Ignoring non-numeric key \"" << entry.first << "\" in [length] section of info.ini." << endl;
                            }
                        }
                    }
                } else if (scope.first == "general") {
                    for (const auto &entry : scope.second) {
                        if (entry.first == "year") {
                            year = QString::fromLocal8Bit(entry.second.data());
                        } else if (entry.first == "genre") {
                            genre = QString::fromLocal8Bit(entry.second.data());
                        } else if (entry.first == "total_tracks") {
                            totalTracks = QString::fromLocal8Bit(entry.second.data());
                        } else if (entry.first == "total_disks") {
                            totalDisks = QString::fromLocal8Bit(entry.second.data());
                        } else {
                            cerr << "Warning: Ignoring unknown property \"" << entry.first << "\" in [general] section of info.ini." << endl;
                        }
                    }
                } else {
                    cerr << "Warning: Ignoring unknown section [" << scope.first << "] in info.ini." << endl;
                }
            }
        } catch (const std::ios_base::failure &failure) {
            cerr << "Warning: Can't parse info.ini because an IO error occured: " << failure.what() << endl;
        }
    }
    // determine target name/path
    QString targetName(
        QStringLiteral("%3%1%2").arg(m_watcher.title().isEmpty() ? unknownTitle : validFileName(m_watcher.title()), m_targetExtension, number));
    unsigned int count = 1;
    while (targetDir.exists(targetName)) {
        ++count;
        targetName
            = QStringLiteral("%3%1 (%4)%2").arg(m_watcher.title().isEmpty() ? unknownTitle : m_watcher.title(), m_targetExtension, number).arg(count);
    }
    auto targetPath = targetDir.absoluteFilePath(targetName);
    // set input device
    QStringList args;
    args << QStringLiteral("-f");
    args << QStringLiteral("pulse");
    args << m_inputOptions;
    args << QStringLiteral("-i");
    args << m_sink;
    // set length if specified in info.ini
    if (!length.isEmpty() || !m_watcher.length().isNull()) {
        args << "-t";
        args << (length.isEmpty() ? QString::number(m_watcher.length().totalSeconds()) : length);
    }
    // set additional options
    args << m_options;
    // set meta data
    addMetaData(args, QStringLiteral("title"), m_watcher.title());
    addMetaData(args, QStringLiteral("album"), m_watcher.album());
    addMetaData(args, QStringLiteral("artist"), m_watcher.artist());
    addMetaData(args, QStringLiteral("genre"), genre.isEmpty() ? m_watcher.genre() : genre);
    addMetaData(args, QStringLiteral("year"), year.isEmpty() ? m_watcher.year() : year);
    if (m_watcher.trackNumber()) {
        addMetaData(args, QStringLiteral("track"),
            totalTracks.isEmpty() ? QString::number(m_watcher.trackNumber()) : QString::number(m_watcher.trackNumber()) % QChar('/') % totalTracks);
    }
    if (m_watcher.diskNumber()) {
        addMetaData(args, QStringLiteral("disk"),
            totalDisks.isEmpty() ? QString::number(m_watcher.diskNumber()) : QString::number(m_watcher.diskNumber()) % QChar('/') % totalDisks);
    }
    // set output file
    args << targetPath;
    m_ffmpeg->setArguments(args);
    // start process
    m_ffmpeg->start();
    // resume player
    m_watcher.play();
    m_watcher.setSilent(false);
}

void FfmpegLauncher::stopFfmpeg()
{
    if (m_ffmpeg->state() != QProcess::NotRunning) {
        m_ffmpeg->terminate();
        m_ffmpeg->waitForFinished(10000);
        if (m_ffmpeg->state() != QProcess::NotRunning) {
            m_ffmpeg->kill();
            m_ffmpeg->waitForFinished(5000);
            if (m_ffmpeg->state() != QProcess::NotRunning) {
                throw runtime_error("Unable to terminate/kill ffmpeg process.");
            }
        }
    }
}

void FfmpegLauncher::ffmpegStarted()
{
    cerr << "Started ffmpeg: ";
    cerr << m_ffmpeg->program();
    for (const auto &arg : m_ffmpeg->arguments()) {
        cerr << ' ' << arg;
    }
    cerr << endl;
}

void FfmpegLauncher::ffmpegError()
{
    cerr << "Failed to start ffmpeg: " << m_ffmpeg->errorString();
}

void FfmpegLauncher::ffmpegFinished(int exitCode)
{
    cerr << "FFmpeg finished with exit code " << exitCode << endl;
}
}

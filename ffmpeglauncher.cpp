#include "ffmpeglauncher.h"
#include "playerwatcher.h"

#include <c++utilities/io/inifile.h>
#include <c++utilities/conversion/stringconversion.h>

#include <iostream>
#include <fstream>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;

namespace DBusSoundRecorder {

inline ostream &operator <<(ostream &stream, const QString &str)
{
    stream << str.toLocal8Bit().data();
    return stream;
}

FfmpegLauncher::FfmpegLauncher(PlayerWatcher &watcher, QObject *parent) :
    QObject(parent),
    m_watcher(watcher),
    m_sink(QStringLiteral("default")),
    m_options(),
    m_targetDir(QStringLiteral(".")),
    m_targetExtension(QStringLiteral(".m4a")),
    m_ffmpeg(new QProcess(this))
{
    connect(&watcher, &PlayerWatcher::nextSong, this, &FfmpegLauncher::nextSong);
    connect(m_ffmpeg, &QProcess::started, this, &FfmpegLauncher::ffmpegStarted);
    connect(m_ffmpeg, static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error), this, &FfmpegLauncher::ffmpegError);
    connect(m_ffmpeg, static_cast<void(QProcess::*)(int)>(&QProcess::finished), this, &FfmpegLauncher::ffmpegFinished);
    m_ffmpeg->setProgram(QStringLiteral("ffmpeg"));
    m_ffmpeg->setProcessChannelMode(QProcess::ForwardedChannels);
}

void addMetaData(QStringList &args, const QString &field, const QString &value)
{
    if(!value.isEmpty()) {
        args << QStringLiteral("-metadata");
        args << QStringLiteral("%1=%2").arg(field, value);
    }
}

void FfmpegLauncher::nextSong()
{
    // terminate/kill the current process
    if(m_ffmpeg->state() != QProcess::NotRunning) {
        m_ffmpeg->terminate();
        m_ffmpeg->waitForFinished(250);
        if(m_ffmpeg->state() != QProcess::NotRunning) {
            m_ffmpeg->kill();
            m_ffmpeg->waitForFinished(250);
            if(m_ffmpeg->state() != QProcess::NotRunning) {
                throw runtime_error("Unable to terminate/kill ffmpeg process.");
            }
        }
    }
    if(m_watcher.isPlaying()) {
        // determine output file, create target directory
        static const QString miscCategory(QStringLiteral("misc"));
        static const QString unknownTitle(QStringLiteral("unknown track"));
        const auto targetDirPath = QStringLiteral("%1/%2").arg(m_watcher.artist().isEmpty() ? miscCategory : m_watcher.artist(), m_watcher.artist().isEmpty() ? miscCategory : m_watcher.album());
        if(!m_targetDir.mkpath(targetDirPath)) {
            cerr << "Error: Can not create target directory: " << targetDirPath << endl;
            return;
        }
        QDir targetDir(m_targetDir);
        targetDir.cd(targetDirPath);
        // determine track number
        QString number, length;
        if(m_watcher.trackNumber()) {
            if(m_watcher.diskNumber()) {
                number = QStringLiteral("%2-%1").arg(m_watcher.trackNumber(), 2, 10, QLatin1Char('0')).arg(m_watcher.diskNumber());
            } else {
                number = QStringLiteral("%1").arg(m_watcher.trackNumber(), 2, 10, QLatin1Char('0'));
            }
        }
        if(!number.isEmpty()) {
            number.append(QStringLiteral(" - "));
        }
        // determine additional info
        //  - from a file called info.ini in the album directory
        //  - currently only track length is supported (used to get rid of advertisements at the end)
        if(targetDir.exists(QStringLiteral("info.ini"))) {
            fstream infoFile;
            infoFile.exceptions(ios_base::badbit | ios_base::failbit);
            try {
                infoFile.open((targetDir.path() +  QStringLiteral("/info.ini")).toLocal8Bit().data(), ios_base::in);
                IniFile infoIni;
                infoIni.parse(infoFile);
                // read length scope, only possible if track number known because the track number is used for mapping
                if(m_watcher.trackNumber()) {
                    for(auto &scope : infoIni.data()) {
                        if(scope.first == "length") {
                            for(const auto &entry : scope.second) {
                                try {
                                    if(stringToNumber<unsigned int>(entry.first) == m_watcher.trackNumber()) {
                                        // length entry for this track
                                        length = QString::fromLocal8Bit(entry.second.data());
                                        break;
                                    }
                                } catch( const ConversionException &) {
                                    cerr << "Warning: Ignoring non-numeric key \"" << entry.first << "\" in info.ini." << endl;
                                }
                            }
                        }
                    }
                }
            } catch(const ios_base::failure &) {
                cerr << "Warning: Can't parse info.ini because an IO error occured." << endl;
            }
        }
        // determine target name/path
        QString targetName(QStringLiteral("%3%1%2").arg(m_watcher.title().isEmpty() ? unknownTitle : m_watcher.title(), m_targetExtension, number));
        unsigned int count = 1;
        while(targetDir.exists(targetName)) {
            ++count;
            targetName = QStringLiteral("%3%1 (%4)%2").arg(m_watcher.title().isEmpty() ? unknownTitle : m_watcher.title(), m_targetExtension, number).arg(count);
        }
        auto targetPath = targetDir.absoluteFilePath(targetName);
        // set input device
        QStringList args;
        args << QStringLiteral("-f");
        args << QStringLiteral("pulse");
        args << QStringLiteral("-i");
        args << m_sink;
        // set length
        if(!length.isEmpty()) {
            args << "-t";
            args << length;
        }
        // set additional options
        args << m_options;
        // set meta data
        addMetaData(args, QStringLiteral("title"), m_watcher.title());
        addMetaData(args, QStringLiteral("album"), m_watcher.album());
        addMetaData(args, QStringLiteral("artist"), m_watcher.artist());
        addMetaData(args, QStringLiteral("genre"), m_watcher.genre());
        addMetaData(args, QStringLiteral("year"), m_watcher.year());
        if(m_watcher.trackNumber()) {
            addMetaData(args, QStringLiteral("track"), QString::number(m_watcher.trackNumber()));
        }
        if(m_watcher.diskNumber()) {
            addMetaData(args, QStringLiteral("disk"), QString::number(m_watcher.diskNumber()));
        }
        // set output file
        args << targetPath;
        m_ffmpeg->setArguments(args);
        // start process
        m_ffmpeg->start();
    }
}

void FfmpegLauncher::ffmpegStarted()
{
    cerr << "Started ffmpeg: ";
    cerr << m_ffmpeg->program();
    for(const auto &arg : m_ffmpeg->arguments()) {
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

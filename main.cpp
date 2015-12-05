#include "playerwatcher.h"
#include "ffmpeglauncher.h"

// include configuration from separate header file when building with CMake
#ifndef APP_METADATA_AVAIL
#include "config.h"
#endif

#include <c++utilities/application/argumentparser.h>
#include <c++utilities/application/failure.h>

#include <QCoreApplication>

#include <iostream>

using namespace std;
using namespace ApplicationUtilities;
using namespace DBusSoundRecorder;

int main(int argc, char *argv[])
{
    // setup the argument parser
    SET_APPLICATION_INFO;
    ArgumentParser parser;
    HelpArgument helpArg(parser);
    Argument recordArg("record", "r", "starts recording");
    recordArg.setDenotesOperation(true);
    Argument applicationArg("application", "a", "specifies the application providing meta information via D-Bus interface");
    applicationArg.setRequired(true);
    applicationArg.setValueNames({"name"});
    applicationArg.setRequiredValueCount(1);
    Argument sinkArg("sink", "s", "specifies the Pulse Audio sink to be recorded (see pactl list short sinks)");
    sinkArg.setValueNames({"sink"});
    sinkArg.setRequiredValueCount(1);
    sinkArg.setCombinable(true);
    Argument ffmpegInputOptions("ffmpeg-input-options", "i", "specifies options for the ffmpeg input device");
    ffmpegInputOptions.setRequiredValueCount(1);
    ffmpegInputOptions.setCombinable(true);
    Argument targetDirArg("target-dir", "t", "specifies the target directory");
    targetDirArg.setValueNames({"path"});
    targetDirArg.setRequiredValueCount(1);
    targetDirArg.setCombinable(true);
    Argument targetExtArg("target-extension", "e", "specifies the target extension (default is .m4a)");
    targetExtArg.setValueNames({"extension"});
    targetExtArg.setRequiredValueCount(1);
    targetExtArg.setCombinable(true);
    Argument ignorePlaybackStatusArg("ignore-playback-status", string(), "ignores the playback status (does not call PlaybackStatus())");
    ignorePlaybackStatusArg.setCombinable(true);
    Argument ffmpegBinArg("ffmpeg-bin", "f", "specifies the path to the ffmpeg binary");
    ffmpegBinArg.setValueNames({"path"});
    ffmpegBinArg.setRequiredValueCount(1);
    ffmpegBinArg.setCombinable(true);
    Argument ffmpegOptions("ffmpeg-options", "o", "specifies options for ffmpeg");
    ffmpegOptions.setValueNames({"options"});
    ffmpegOptions.setRequiredValueCount(1);
    ffmpegOptions.setCombinable(true);
    recordArg.setSecondaryArguments({&applicationArg, &sinkArg, &ffmpegInputOptions, &targetDirArg, &targetExtArg, &ignorePlaybackStatusArg, &ffmpegBinArg, &ffmpegOptions});
    parser.setMainArguments({&recordArg, &helpArg});
    parser.setIgnoreUnknownArguments(false);
    // parse command line arguments
    try {
        parser.parseArgs(argc, argv);
    } catch (const Failure &e) {
        cerr << "Unable to parse arguments: " << e.what() << endl;
        return 1;
    }
    try {
        if(recordArg.isPresent()) {
            // start watching/recording
            cerr << "Watching MPRIS service of the specified application \"" << applicationArg.values().front() << "\" ..." << endl;
            // create app loop, player watcher and ffmpeg launcher
            QCoreApplication app(argc, argv);
            PlayerWatcher watcher(QString::fromLocal8Bit(applicationArg.values().front().data()), ignorePlaybackStatusArg.isPresent());
            FfmpegLauncher ffmpeg(watcher);
            // pass specified args to ffmpeg launcher
            if(sinkArg.isPresent()) {
                ffmpeg.setSink(QString::fromLocal8Bit(sinkArg.values().front().data()));
            }
            if(ffmpegInputOptions.isPresent()) {
                ffmpeg.setFFmpegInputOptions(QString::fromLocal8Bit(ffmpegInputOptions.values().front().data()));
            }
            if(ffmpegBinArg.isPresent()) {
                ffmpeg.setFFmpegBinary(QString::fromLocal8Bit(ffmpegBinArg.values().front().data()));
            }
            if(ffmpegOptions.isPresent()) {
                ffmpeg.setFFmpegOptions(QString::fromLocal8Bit(ffmpegOptions.values().front().data()));
            }
            if(targetDirArg.isPresent()) {
                ffmpeg.setTargetDir(QString::fromLocal8Bit(targetDirArg.values().front().data()));
            }
            if(targetExtArg.isPresent()) {
                ffmpeg.setTargetExtension(QString::fromLocal8Bit(targetExtArg.values().front().data()));
            }
            // enter app loop
            return app.exec();
        } else if(!helpArg.isPresent()) {
            cerr << "No operation specified." << endl;
            return 2;
        }
    } catch(const runtime_error &e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 3;
    }
}

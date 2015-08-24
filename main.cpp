#include "playerwatcher.h"

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
    ArgumentParser parser;
    HelpArgument helpArg(parser);
    Argument applicationArg("application", "a", "specifies the application providing meta information via D-Bus interface");
    applicationArg.setRequired(true);
    applicationArg.setValueNames({"name"});
    applicationArg.setRequiredValueCount(1);
    Argument sinkArg("sink", "s", "specifies the Pulse Audio sink to be recorded (see pactl list short sinks)");
    sinkArg.setValueNames({"sink"});
    sinkArg.setRequiredValueCount(1);
    sinkArg.setCombinable(true);
    Argument targetDirArg("target-dir", "t", "specifies the target directory");
    targetDirArg.setValueNames({"path"});
    targetDirArg.setRequiredValueCount(1);
    targetDirArg.setCombinable(true);
    Argument ffmpegOptions("ffmpeg-options", "o", "specifies options for ffmpeg");
    ffmpegOptions.setValueNames({"options"});
    ffmpegOptions.setRequiredValueCount(1);
    ffmpegOptions.setCombinable(true);
    parser.setMainArguments({&applicationArg, &sinkArg, &targetDirArg, &ffmpegOptions, &helpArg});
    parser.setIgnoreUnknownArguments(false);
    // parse command line arguments
    try {
        parser.parseArgs(argc, argv);
    } catch (Failure &e) {
        cerr << "Unable to parse arguments: " << e.what() << endl;
        return 2;
    }
    QCoreApplication app(argc, argv);
    PlayerWatcher watcher(QString::fromLocal8Bit(applicationArg.values().front().data()));
    return app.exec();
}

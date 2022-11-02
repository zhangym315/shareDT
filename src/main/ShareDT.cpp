/*
 * ShareDT
 * Show all of the windows/monitors that can be shared
 */
#include <QApplication>
#include <QImage>
#include <QDesktopWidget>

#include "ShareDT.h"
#include "ShareDTWindow.h"

#ifdef __SHAREDT_WIN__
#include <Shlobj.h>
#include <windows.h>

// Hide console for windows
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#endif

static void initShareDT(const char * argv0)
{
    ShareDTHome::instance()->set(argv0);
#ifdef __SHAREDT_WIN__
    WCHAR * filepath;
    if (!SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &filepath))) {
    }
    std::wstring ws(filepath);
    String varRun = String(ws.begin(), ws.end()) + String(PATH_SEP_STR) +
                    String(SHAREDT_KEYWORD) + String(PATH_SEP_STR) +  String(VAR_RUN);
    if (!fs::exists(varRun)) fs::create_directories(varRun);
    //set log file to var/run/ShareDT.log
    Logger::instance().setLogFile((varRun+String(CAPTURE_LOG)).c_str());
#else
    String varrun = ShareDTHome::instance()->getHome() + String(VAR_RUN);
    if (!fs::exists(varrun)) fs::create_directories(varrun);
    //set log file to var/run/ShareDT.log
    Logger::instance().setLogFile((ShareDTHome::instance()->getHome() +
                                   String(VAR_RUN)+String(CAPTURE_LOG)).c_str());
#endif
}

static int localDisplayer(int argc, char **argv)
{
    QApplication app(argc, argv);
    LocalDisplayer gui(argc, argv);

    if (!gui.isInited()) {
        return -1;
    }

    gui.show();
    gui.startFetcher();
    return QApplication::exec();
}

int
main(int argc, char **argv)
{
    initShareDT(argv[0]);

    if (argc == 1) {
        QApplication app(argc, argv);
        LOGGER.info() << "Starting " << argv[0] << " ...";

        ShareDTWindow gui(argc, argv);
        gui.show();
        return QApplication::exec();
    }

}

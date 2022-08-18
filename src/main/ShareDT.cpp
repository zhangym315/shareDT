/*
 * ShareDT
 * Show all of the windows/monitors that can be shared
 */
#include <QApplication>
#include <QImage>
#include <QAction>
#include <QDesktopWidget>

#include "ShareDT.h"
#include "LocalDisplayer.h"

#ifdef __SHAREDT_WIN__
#include <Shlobj.h>
#include <windows.h>
#endif

ShareDTWindow::ShareDTWindow (int argc, char ** argv, QWidget *parent) :
        _ui(new Ui::ShareDTWindow)
{
#ifndef __SHAREDT_IOS__
    // set program icon, ShareDT.png should be the same directory
    String png = ShareDTHome::instance()->getHome() + String(PATH_SEP_STR) +
                String("bin") + String(PATH_SEP_STR) + String("ShareDT.png");
    setWindowIcon(QIcon(QPixmap(png.c_str())));
#endif

    setMenu();
    setCentralWidget(parent);
    resize(QDesktopWidget().availableGeometry(this).size() * 0.5);
    _ui->setupUi (parent);
}

void ShareDTWindow::setMenu()
{
    auto * menubar = menuBar();
    menubar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    menubar->setGeometry(QRect(0, 0, 800, 50));

    /* Edit */
    auto * menuEdit = new QMenu(menubar);
    menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
    menuEdit->setTitle(QCoreApplication::translate("ShareDTClientWin", "Edit", nullptr));
    auto * newConnect = new QAction();
    newConnect->setObjectName(QString::fromUtf8("new_connection"));
    newConnect->setText(QCoreApplication::translate("ShareDTWindow", "New Connection", nullptr));
    menuEdit->addAction(newConnect);
    /* Edit end*/

    /* Window */
    auto * menuWindow = new QMenu(menubar);
    menuWindow->setObjectName(QString::fromUtf8("menuWindow"));
    menuWindow->setTitle(QCoreApplication::translate("ShareDTWindow", "Window", nullptr));

    auto * freshWin = new QAction();
    freshWin->setObjectName(QString::fromUtf8("fresh_itmes"));
    freshWin->setText(QCoreApplication::translate("ShareDTWindow", "Refresh Items", nullptr));
    menuWindow->addAction(freshWin);
    QObject::connect (freshWin, SIGNAL(triggered()), this, SLOT(actionFreshItems()));
    /* Window end */

    /* Help */
    auto *menuHelp = new QMenu(menubar);
    menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
    menuHelp->setTitle(QCoreApplication::translate("ShareDTWindow", "Help", nullptr));

    auto * aboutWin = new QAction();
    aboutWin->setObjectName(QString::fromUtf8("about_window"));
    aboutWin->setText(QCoreApplication::translate("ShareDTWindow", "About", nullptr));
    menuHelp->addAction(aboutWin);
    /* Help  end */

    menubar->addAction(menuEdit->menuAction());
    menubar->addAction(menuWindow->menuAction());
    menubar->addAction(menuHelp->menuAction());
}

ShareDTWindow::~ShareDTWindow()
{
    delete _ui;
    LOGGER.info() << "Stopped...";
}

void ShareDTWindow::actionFreshItems()
{
    _ui->refreshLocalBoxGroup(this);
}

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

int
main(int argc, char **argv)
{
    initShareDT(argv[0]);
    QApplication app(argc, argv);

    if (argc == 1) {
        LOGGER.info() << "Starting " << argv[0] << " ...";

        ShareDTWindow gui(argc, argv);
        gui.show();
        return QApplication::exec();
    } else {
#ifdef __SHAREDT_WIN__
//    ::ShowWindow(::GetConsoleWindow(), SW_HIDE ); //hide console window
#endif
        LocalDisplayer gui(argc, argv);

        if (!gui.isInited()) {
            return -1;
        }

        gui.show();
        gui.startFetcher();
        return QApplication::exec();
    }

}

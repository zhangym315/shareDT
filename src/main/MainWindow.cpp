#include <QApplication>
#include <QImage>
#include <QAction>
#include <QDesktopWidget>

#include "MainWindow.h"
#include "LocalDisplayer.h"

ShareDTWindow::ShareDTWindow (int argc, char ** argv, QWidget *parent) :
        _ui(new Ui::ShareDTWindow)
{
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
}

void ShareDTWindow::actionFreshItems()
{
    _ui->refreshLocalBoxGroup(this);
}

int
main(int argc, char **argv)
{
    ShareDTHome::instance()->set(argv[0]);
    QApplication app(argc, argv);

    if (argc == 1) {
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

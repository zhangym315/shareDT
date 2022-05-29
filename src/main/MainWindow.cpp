#include <QApplication>
#include <QImage>

#include "MainWindow.h"
#include "LocalDisplayer.h"

ShareDTWindow::ShareDTWindow (int argc, char ** argv, QWidget *parent) :
        QWidget (parent),
        _ui(new Ui::ShareDTWindow)
{
    _ui->setupUi (this);
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
        LocalDisplayer gui(argc, argv);

        if (!gui.isInited()) {
            return -1;
        }

        gui.show();
        gui.startFetcher();
        return QApplication::exec();
    }

}

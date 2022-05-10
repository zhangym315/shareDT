#include <QApplication>
#include <QImage>
#include "MainWindow.h"

ShareDTWindow::ShareDTWindow (int argc, char ** argv, QWidget *parent) :
        QWidget (parent),
        _ui(new Ui::ShareDTWindow)
{
    _ui->setupUi (this);

    unsigned char buf[100*80*4] = { 50 };
}

ShareDTWindow::~ShareDTWindow()
{
    delete _ui;
}

int
main(int argc, char **argv)
{

    QApplication app(argc, argv);
    ShareDTWindow gui(argc, argv);

    gui.show();

    return QApplication::exec();
}

#include <QApplication>
#include <QImage>
#include <QMessageBox>
#include "MainWindow.h"

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
    QMessageBox msgBox;
    msgBox.setText("Action to refresh!!!");
    msgBox.exec();
}
int
main(int argc, char **argv)
{

    QApplication app(argc, argv);
    ShareDTWindow gui(argc, argv);

    gui.show();

    return QApplication::exec();
}

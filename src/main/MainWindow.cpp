#include <QApplication>
#include <QImage>
#include "MainWindow.h"

ShareDTWindow::ShareDTWindow (int argc, char ** argv, QWidget *parent) :
        QWidget (parent),
        _ui(new Ui::ShareDTWindow)
{
    _ui->setupUi (this);
    _ui->imageLabel->setText (QString("                                      "
                                      "Starting Local Displayer..."));
    _ui->imageLabel1->setText (QString("helloe world"));

    unsigned char buf[100*80*4] = { 50 };

    QImage im(buf, 100, 80, QImage::Format::Format_RGB32);

    _ui->imageLabel2->setPixmap(QPixmap::fromImage(im));
    _ui->imageLabel3->setPixmap(QPixmap::fromImage(im));

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

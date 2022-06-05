#ifndef SHAREDT_MAINWINDOW_H
#define SHAREDT_MAINWINDOW_H
#include "ShareDTWindow.h"
#include <QWidget>
#include <QMainWindow>

class ShareDTWindow : public QMainWindow{
    Q_OBJECT

public:
    explicit ShareDTWindow (int argc, char ** argv,
                             QWidget *parent = new QWidget());
    ~ShareDTWindow () override;

    void setMenu();
private:
    Ui::ShareDTWindow   * _ui;

public slots:
    void actionFreshItems();
//    void putImage(FrameBuffer * data);
};

#endif //SHAREDT_MAINWINDOW_H

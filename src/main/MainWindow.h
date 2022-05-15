#ifndef SHAREDT_MAINWINDOW_H
#define SHAREDT_MAINWINDOW_H
#include "ShareDTWindow.h"
#include <QWidget>

class ShareDTWindow : public QWidget{
    Q_OBJECT

public:
    explicit ShareDTWindow (int argc, char ** argv,
                             QWidget *parent = nullptr);
    ~ShareDTWindow () override;
/*
    bool isInited() const { return _fetcher->isInited(); }
    void startFetcher();

    void closeEvent ( QCloseEvent *event ) override;
    void resizeEvent( QResizeEvent * e ) override;
*/
private:
    Ui::ShareDTWindow   * _ui;

public slots:
//    void putImage(FrameBuffer * data);
};

#endif //SHAREDT_MAINWINDOW_H

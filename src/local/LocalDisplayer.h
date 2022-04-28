#ifndef SHAREDT_LOCALDISPLAYER_H
#define SHAREDT_LOCALDISPLAYER_H
#include <QEvent>
#include <QWidget>
#include <QMouseEvent>
#include <QHoverEvent>

#include "Thread.h"
#include "Capture.h"
#include "SDThread.h"
#include "Buffer.h"
#include "ui_LocalDisplayer.h"

#define RATIO_PRECISION 1000

struct MainWindowResized {
    bool isResized = false;
    QSize curSize;    /* current window size */
    QSize oriSize;    /* original window size */

    int ratioX = RATIO_PRECISION;     /* Ratio of original to X */
    int ratioY = RATIO_PRECISION;     /* Ratio of original to Y */
    int ratioX_Y;
};

class FetchingDataThread : public SDThread {
    Q_OBJECT

public:
    FetchingDataThread(int argc, char **argv);
    ~FetchingDataThread() override = default;

    void run() override;
    [[nodiscard]] bool isInited() const { return _isInited; }

    FrameBuffer *  getFrame() { return _frame; }
    ScreenProvider * getScreenProvide() { return _capture.getScreenProvide();}

signals:
    void sendRect(FrameBuffer* client);

private:
    bool       _isInited;
    FrameBuffer *   _frame{};
    Capture    _capture;
};

class LocalDisplayer : public QWidget{
Q_OBJECT

public:
    explicit LocalDisplayer (int argc, char ** argv,
                               QWidget *parent = nullptr);
    ~LocalDisplayer () override;

    bool isInited() const { return _fetcher->isInited(); }
    void startFetcher();

    void closeEvent ( QCloseEvent *event ) override;
    void resizeEvent( QResizeEvent * e ) override;

private:
    void resetRatioWindow();

    Ui::LocalDisplayer   * _ui;
    FetchingDataThread   * _fetcher;
    MainWindowResized     _winSize;

public slots:
    void putImage(FrameBuffer * data);
    void actionAdjustToOriginSize();
    void actionFixRatioWidthHeight();
};

#endif //SHAREDT_LOCALDISPLAYER_H

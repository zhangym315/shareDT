//
// Created by Yiming Zhang on 4/6/21.
//

#ifndef _SHAREDTCLIENTWIN_H_
#define _SHAREDTCLIENTWIN_H_

#include <QEvent>
#include <QWidget>
#include <QMouseEvent>
#include <QHoverEvent>
#include <chrono>

#include "FetchingData.h"

using namespace std;

/* 30ms (millisecons) */
#define MOUSEMOVEMENT_LATENCY 30000

QT_BEGIN_NAMESPACE
namespace Ui
{
class ShareDTClientWin;
}
QT_END_NAMESPACE

#define RATIO_PRECISION 1000

struct MainWindowResized {
    bool isResized = false;
    QSize curSize;    /* current shareDTClient window size */
    QSize oldSize;    /* last shareDTClient window size    */
    QSize vncSize;    /* original window size on shareDTServer */

    int ratioX = RATIO_PRECISION;     /* Ratio of original to X */
    int ratioY = RATIO_PRECISION;     /* Ratio of original to Y */
};

class ShareDTClientWin : public QWidget{
    Q_OBJECT

  public:
    explicit ShareDTClientWin (int argc, char ** argv,
                               QWidget *parent = nullptr);
    ~ShareDTClientWin () override;

    void startVNC();
    bool isInited();

    void resizeEvent( QResizeEvent * e ) override;
    void closeEvent ( QCloseEvent *event ) override;

    void resizeToNewVNC(int w, int h);

    /* mouse handle */
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    bool event(QEvent * e) override;

    void enterEvent(QEvent * e) override;
    void leaveEvent(QEvent * e) override;

    void hoverEnter(QHoverEvent * event);
    void hoverLeave(QHoverEvent * event);
    void hoverMove(QHoverEvent * event);

    void keyPressEvent(QKeyEvent * event) override;
    void keyReleaseEvent(QKeyEvent * event) override;

    bool checkShouldSendMouseMove();

  private:
    void resetRatioWindow();

    Ui::ShareDTClientWin   * ui;
    FetchingDataFromServer * _fetcher;
    MainWindowResized        _winResize;
    bool                     _closed;  // windows is closed
    chrono::time_point<chrono::high_resolution_clock>  _mouseMoved;

  public slots:
    void putImage(rfbClient* client);
    void actionAdjustToOriginSize();
    void serverConnectionClosed();
};

#endif //_SHAREDTCLIENTWIN_H_

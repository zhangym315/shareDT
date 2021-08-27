//
// Created by Yiming Zhang on 4/6/21.
//

#ifndef _SHAREDTCLIENTWIN_H_
#define _SHAREDTCLIENTWIN_H_

#include <QEvent>
#include <QWidget>
#include <QMouseEvent>
#include <QHoverEvent>

#include "FetchingData.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class ShareDTClientWin;
}
QT_END_NAMESPACE

struct MainWindowResized {
    bool isResized = false;
    QSize curSize;
    QSize oldSize;
    QSize vncSize;
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

  private:
    Ui::ShareDTClientWin   * ui;
    FetchingDataFromServer * _fetcher;
    MainWindowResized        _winResize;
    bool                     _closed;  // windows is closed

  public slots:
    void putImage(rfbClient* client);
    void actionAdjustToOriginSize();
    void serverConnectionClosed();
};

#endif //_SHAREDTCLIENTWIN_H_

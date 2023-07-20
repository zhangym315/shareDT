#include <iostream>
#include <chrono>

#include <QObject>
#include <QColorSpace>
#include <qevent.h>
#include <QMessageBox>

#include "MainGUI.h"
#include "ShareDTClientWin.h"
#include "ui_ShareDTClientWin.h"
#include "InputInterface.h"

ShareDTClientWin::ShareDTClientWin (int argc, char ** argv,
                                    QWidget *parent)
    : _closed(false),
     QWidget (parent), 
     ui (new Ui::ShareDTClientWin),
     _mouseMoved(chrono::high_resolution_clock::now())
{
#ifndef __SHAREDT_IOS__
    ShareDTWindow::setIcon(this);
#endif

    ui->setupUi (this);

    ui->imageLabel->setText (QString("hello from ShareDTClientWin::ShareDTClientWin"));
    _fetcher = new FetchingDataFromServer(argc, argv);

    QObject::connect (_fetcher, SIGNAL(sendRect(rfbClient*)),
                      this, SLOT(putImage(rfbClient*)));
    QObject::connect (ui->actionAdjust_to_original_size, SIGNAL(triggered()),
                      this, SLOT(actionAdjustToOriginSize()));
    QObject::connect (_fetcher, SIGNAL(serverConnectionClosedSend()),
                      this, SLOT(serverConnectionClosed()));

    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
    setFocusPolicy(Qt::StrongFocus);
}

ShareDTClientWin::~ShareDTClientWin ()
{
    delete ui;
}

bool ShareDTClientWin::isInited ()
{
    return _fetcher->isInited();
}

void ShareDTClientWin::resizeEvent( QResizeEvent * e )
{
    _winResize.isResized = true;

    // resize can happened when user resize windows or VNCServer
    // send a different size window(resized on server sid).
    _winResize.oldSize = _winResize.curSize;
    _winResize.curSize = e->size();

    resetRatioWindow();

    QWidget::resizeEvent(e);
}

void ShareDTClientWin::putImage (rfbClient* client)
{
    if (_closed) return;
    auto * fetcher = (FetchingDataFromServer *) client->_fetcher;

    if (fetcher->isShutDown()) return;
    auto & frame = fetcher->getFrame();

    QImage im(frame.frame.getData(), frame.w, frame.h,
              QImage::Format::Format_RGBX8888);

    // resize requested from vnc server
    if (frame.w != _winResize.vncSize.width() ||
        frame.h != _winResize.vncSize.height()) {
        resizeToNewVNC(frame.w, frame.h);
    }

    // scale the image
    if (frame.w != _winResize.curSize.width() ||
        frame.h != _winResize.curSize.height()) {
        im = im.scaled(_winResize.curSize.width(), _winResize.curSize.height());
    }

    ui->imageLabel->setPixmap(QPixmap::fromImage(im));

    if (_winResize.isResized) {
        int w = _winResize.curSize.width();
        int h = _winResize.curSize.height();
        ui->imageLabel->resize (w, h);
        ui->horizontalLayoutWidget->resize(w, h);
        this->resize(w, h);
        _winResize.isResized = false;
    }

    frame.frame.setUsed();
}

void ShareDTClientWin::resetRatioWindow()
{
    if ( _winResize.curSize.width() && _winResize.curSize.height()) {
        _winResize.ratioX = _winResize.vncSize.width() * RATIO_PRECISION/ _winResize.curSize.width();
        _winResize.ratioY = _winResize.vncSize.height() * RATIO_PRECISION/ _winResize.curSize.height();
    }
}

void ShareDTClientWin::resizeToNewVNC(int w, int h)
{
    _winResize.oldSize = _winResize.curSize;
    _winResize.curSize.setWidth (w*2/3);
    _winResize.curSize.setHeight (h*2/3);
    _winResize.vncSize.setWidth (w);
    _winResize.vncSize.setHeight (h);
    _winResize.isResized = true;
    resetRatioWindow();
}

void ShareDTClientWin::actionAdjustToOriginSize()
{
    _winResize.curSize = _winResize.vncSize;
    _winResize.isResized = true;

    _winResize.ratioX = RATIO_PRECISION;
    _winResize.ratioY = RATIO_PRECISION;
}

void ShareDTClientWin::startVNC ()
{
    _fetcher->start ();
}

void ShareDTClientWin::closeEvent (QCloseEvent *event)
{
    _closed = true;

    // make sure fetcher thread is shutdown
    while(!_fetcher->isShutDown()) {
        _fetcher->stop();
    }

    delete _fetcher;

    event->accept();
}

void ShareDTClientWin::serverConnectionClosed()
{
    this->close();
    QMessageBox msgBox;
    msgBox.setText("ShareDTServer closed the connection!");
    msgBox.exec();
}

/* Mouse events started */
void ShareDTClientWin::mousePressEvent(QMouseEvent *event)
{
    const QPoint & locPoint = event->pos();
    const QPoint & gloPoint = event->globalPos();
    int b = (int) event->button() | MouseButton::ButtonDown;

    SendPointerEvent(_fetcher->getRfbClient(),
                     locPoint.x() * _winResize.ratioX / RATIO_PRECISION,
                     locPoint.y() * _winResize.ratioY / RATIO_PRECISION,
                     b);
    QWidget::mousePressEvent(event);
}

void ShareDTClientWin::mouseReleaseEvent(QMouseEvent *event)
{
    const QPointF & locPoint = event->localPos();
    const QPointF & winPoint = event->windowPos();
    const QPointF & gloPoint = event->globalPos();
    int b = (int) event->button() | MouseButton::ButtonUp;

    SendPointerEvent(_fetcher->getRfbClient(),
                     locPoint.x() * _winResize.ratioX / RATIO_PRECISION,
                     locPoint.y() * _winResize.ratioY / RATIO_PRECISION,
                     b);
    QWidget::mouseReleaseEvent(event);
}

void ShareDTClientWin::mouseDoubleClickEvent(QMouseEvent *event)
{
    /*
     * Double mouse click would not send to client
     * since the mouse press/release event will send
     * to client separately.
     */
    QWidget::mouseDoubleClickEvent(event);
}

/*
 * For mouse movement without button clicked, only send event every 30ms, if there
 * are performances requirement, we can reduce the latency.
 */
bool ShareDTClientWin::checkShouldSendMouseMove()
{

    auto elapsed = chrono::high_resolution_clock::now() - _mouseMoved;

    if (chrono::duration_cast<chrono::microseconds>(elapsed).count() < MOUSEMOVEMENT_LATENCY) {
        return false;
    }
    _mouseMoved = chrono::high_resolution_clock::now();
    return true;
}

void ShareDTClientWin::mouseMoveEvent(QMouseEvent *event)
{
    if (!checkShouldSendMouseMove()) return;
    const QPointF & locPoint = event->localPos();
    const QPointF & winPoint = event->windowPos();
    const QPointF & gloPoint = event->globalPos();

    SendPointerEvent(_fetcher->getRfbClient(),
                     locPoint.x() * _winResize.ratioX / RATIO_PRECISION,
                     locPoint.y() * _winResize.ratioY / RATIO_PRECISION,
                     MouseButton::NoButton);
    QWidget::mouseMoveEvent(event);
}

void ShareDTClientWin::wheelEvent(QWheelEvent *event) {
    const QPoint & delta = event->pixelDelta();
    const QPoint & angle = event->angleDelta();

    if (delta.x() == 0 && delta.y() == 0) return;

    SendPointerEvent(_fetcher->getRfbClient(),
                     delta.x(),
                     delta.y(),
                     MouseButton::WheeleMoved);
    QWidget::wheelEvent(event);
}

bool ShareDTClientWin::event(QEvent * e)
{
    switch(e->type())
    {
        case QEvent::HoverEnter:
            hoverEnter(static_cast<QHoverEvent*>(e));
            break;
        case QEvent::HoverLeave:
            hoverLeave(static_cast<QHoverEvent*>(e));
            break;
        case QEvent::HoverMove:
            hoverMove(static_cast<QHoverEvent*>(e));
            break;
        default:
            break;
    }
    return QWidget::event(e);
}

void ShareDTClientWin::enterEvent(QEnterEvent * e)
{
}

void ShareDTClientWin::leaveEvent(QEvent * e)
{
}

void ShareDTClientWin::hoverEnter(QHoverEvent * event)
{
    const QPoint & locPoint = event->pos();

    SendPointerEvent(_fetcher->getRfbClient(),
                     locPoint.x() * _winResize.ratioX / RATIO_PRECISION,
                     locPoint.y() * _winResize.ratioY / RATIO_PRECISION,
                     MouseButton::NoButton);
}

void ShareDTClientWin::hoverLeave(QHoverEvent * event)
{
    const QPoint & locPoint = event->pos();

    SendPointerEvent(_fetcher->getRfbClient(),
                     locPoint.x() * _winResize.ratioX / RATIO_PRECISION,
                     locPoint.y() * _winResize.ratioY / RATIO_PRECISION,
                     MouseButton::NoButton);
}

void ShareDTClientWin::hoverMove(QHoverEvent * event)
{
    if (!checkShouldSendMouseMove()) return;
    const QPoint & locPoint = event->pos();

    SendPointerEvent(_fetcher->getRfbClient(),
                     locPoint.x() * _winResize.ratioX / RATIO_PRECISION,
                     locPoint.y() * _winResize.ratioY / RATIO_PRECISION,
                     MouseButton::NoButton);
}
/* Mouse events ended   */

/* Keyboard events */
void ShareDTClientWin::keyPressEvent(QKeyEvent * event)
{
    SendKeyEvent(_fetcher->getRfbClient(), event->key(), true);
}

void ShareDTClientWin::keyReleaseEvent(QKeyEvent *event)
{
    SendKeyEvent(_fetcher->getRfbClient(), event->key(), false);
}
/* Keyboard events ended */

#include <QApplication>
#include "LocalDisplayer.h"
#include <QObject>
#include <chrono>

FetchingDataThread::FetchingDataThread(int argc, char **argv) :
        _isInited(false),
        _frame(nullptr)
{
    _capture.setCType(Capture::C_LOCALDISPLAYER);
    int ret = _capture.initParsing(argc, argv) ||
              _capture.initSrceenProvider();

    if (ret != RETURN_CODE_SUCCESS || _capture.getScreenProvide() == nullptr) {
        return;
    }

    auto sp = _capture.getScreenProvide();

    if (!sp->startSample()) {
        LOGGER.error() << "FetchingDataThread failed to start SampleProvider" ;
        return ;
    }

    int count = 0;
    while(!sp->isSampleReady() && ++count < 100) {
        std::this_thread::sleep_for(50ms);
    }
    if (!sp->isSampleReady()) return;

    _isInited = true;
}

void FetchingDataThread::run()
{
    auto sp = _capture.getScreenProvide();
    FrameBuffer * fb;

    sp->sampleResume();
    while (!_stopped) {
        this_thread::sleep_for(20ms);  // TODO: can be set through menu edit

        /* get frame and emit to display */
        fb = sp->getFrameBuffer();
        if(!fb) {
            continue;
        }

        emit sendRect(fb);
    }

    _shutdown = true;
}

LocalDisplayer::LocalDisplayer (int argc, char ** argv, QWidget *parent) :
        QWidget (parent),
        _ui(new Ui::LocalDisplayer),
        _fetcher(new FetchingDataThread(argc, argv))
{
    if (!_fetcher->isInited()) return;

    _ui->setupUi (this);

    _ui->imageLabel->setText (QString("                                      "
                                      "Starting Local Displayer..."));

    QObject::connect (_fetcher, SIGNAL(sendRect(FrameBuffer*)),
                      this, SLOT(putImage(FrameBuffer*)));
    QObject::connect (_ui->actionAdjust_to_original_size, SIGNAL(triggered()),
                      this, SLOT(actionAdjustToOriginSize()));

}

LocalDisplayer::~LocalDisplayer()
{
    delete _ui;
}

void LocalDisplayer::resetRatioWindow()
{
    if ( _winSize.curSize.width() && _winSize.curSize.height()) {
        _winSize.ratioX = _winSize.oriSize.width() * RATIO_PRECISION/ _winSize.curSize.width();
        _winSize.ratioY = _winSize.oriSize.height() * RATIO_PRECISION/ _winSize.curSize.height();
    }
}

void LocalDisplayer::resizeEvent( QResizeEvent * e )
{
    _winSize.isResized = true;

    // resize can happened when user resize windows
    // send a different size window.
    _winSize.curSize = e->size();

    resetRatioWindow();

    QWidget::resizeEvent(e);
}

void LocalDisplayer::putImage (FrameBuffer * frame)
{
    if (_fetcher->isShutDown()) return;

    size_t w = frame->getWidth();
    size_t h = frame->getHeight();

    QImage im(frame->getData(), w, h, QImage::Format::Format_RGBX8888);

    // scale the image
    if (w != _winSize.curSize.width() || h != _winSize.curSize.height()) {
        im = im.scaled(_winSize.curSize.width(), _winSize.curSize.height());
    }

    _ui->imageLabel->setPixmap(QPixmap::fromImage(im));

    if (_winSize.isResized) {
        int wt = _winSize.curSize.width();
        int ht = _winSize.curSize.height();
        _ui->imageLabel->resize (wt, ht);
        _ui->horizontalLayoutWidget->resize(wt, ht);
        this->resize(wt, ht);
        _winSize.isResized = false;
    }

    frame->setUsed();
}

void LocalDisplayer::actionAdjustToOriginSize()
{
    _winSize.curSize = _winSize.oriSize;
    _winSize.isResized = true;

    _winSize.ratioX = RATIO_PRECISION;
    _winSize.ratioY = RATIO_PRECISION;
}

void LocalDisplayer::startFetcher()
{
    _winSize.oriSize.setWidth(_fetcher->getScreenProvide()->getWidth());
    _winSize.oriSize.setHeight(_fetcher->getScreenProvide()->getHeight());
    actionAdjustToOriginSize();

    _fetcher->start();
}

void LocalDisplayer::closeEvent (QCloseEvent *event)
{
    // make sure fetcher thread is shutdown
    while(!_fetcher->isShutDown()) {
        _fetcher->stop();
    }

    delete _fetcher;

    event->accept();
}

int
main(int argc, char **argv)
{
    QApplication app(argc, argv);
    LocalDisplayer gui(argc, argv);

    if (!gui.isInited()) {
        return -1;
    }

    gui.show();
    gui.startFetcher();

    return QApplication::exec();
}


#include <QApplication>
#include <QObject>
#include <chrono>
#include <QScreen>
#include <QObject>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QDesktopWidget>

#include "LocalDisplayer.h"
#include "main.h"

#ifdef __SHAREDT_WIN__
#include <windows.h>
#endif

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

    if (sp->startSample()) {

        int count = 0;
        while (!sp->isSampleReady() && ++count < 100) {
            std::this_thread::sleep_for(50ms);
        }
        if (!sp->isSampleReady()) return;

        _isInited = true;
    } else {
        LOGGER.error() << "FetchingDataThread failed to start SampleProvider";
    }
}

void FetchingDataThread::run()
{
    auto sp = _capture.getScreenProvide();
    FrameBuffer * fb;
    std::chrono::microseconds duration(MICROSECONDS_PER_SECOND/_capture.getFrenquency());

    sp->sampleResume();

    while (!_stopped) {
        std::this_thread::sleep_for(duration);
        /* get frame and emit to display */
        fb = sp->getFrameBuffer();
        if(!fb) {
            std::this_thread::sleep_for(duration);
            continue;
        }

        emit sendRect(fb);
    }

    _shutdown = true;
}

LocalDisplayer::LocalDisplayer (int argc, char ** argv) :
        _fetcher(new FetchingDataThread(argc, argv))
{
    if (!_fetcher->isInited()) return;

#ifndef __SHAREDT_IOS__
    // set program icon, ShareDT.png should be the same directory
    std::string png = ShareDTHome::instance()->getHome() + std::string(PATH_SEP_STR) +
                 std::string("bin") + std::string(PATH_SEP_STR) + std::string("ShareDT.png");
    setWindowIcon(QIcon(QPixmap(png.c_str())));
#endif

    setupMenu();
    setupMain();

    setWindowTitle((std::string("ShareDT - ") + _fetcher->getName()).c_str());

    QObject::connect (_fetcher, SIGNAL(sendRect(FrameBuffer*)),
                      this, SLOT(putImage(FrameBuffer*)));
    QObject::connect (this->actionFix_to_ratio_width_height, SIGNAL(triggered()),
                      this, SLOT(actionFixRatioWidthHeight()));
    QObject::connect (this->actionAdjust_to_original_size, SIGNAL(triggered()),
                      this, SLOT(actionAdjustToOriginSize()));

}

LocalDisplayer::~LocalDisplayer()
{
}

void LocalDisplayer::setupMain()
{
    _imageLabel = new QLabel();
    _imageLabel->setObjectName(QString::fromUtf8("ImageLabel"));
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(_imageLabel->sizePolicy().hasHeightForWidth());

    _imageLabel->setSizePolicy(sizePolicy);
    _imageLabel->setFrameShape(QFrame::Box);
    _imageLabel->setFrameShadow(QFrame::Plain);
    _imageLabel->setLineWidth(0);
    _imageLabel->setMinimumSize(1, 1);

    _imageLabel->setText(QCoreApplication::translate("LocalDisplayer", "ImageLabel", nullptr));

    setCentralWidget(_imageLabel);
    resize(QDesktopWidget().availableGeometry(this).size() * 0.5);
}

void LocalDisplayer::setupMenu()
{
    actionFix_to_ratio_width_height = new QAction();
    actionFix_to_ratio_width_height->setObjectName(QString::fromUtf8("actionFix_to_ratio_width_height"));
    actionAdjust_to_original_size = new QAction();
    actionAdjust_to_original_size->setObjectName(QString::fromUtf8("actionAdjust_to_original_size"));
    actionShow_help = new QAction();
    actionShow_help->setObjectName(QString::fromUtf8("actionShow_help"));

    _menubar = menuBar();
    _menubar->setObjectName(QString::fromUtf8("MenuBar"));
    _menubar->setGeometry(QRect(0, 0, 800, 24));
    auto * menuEdit = new QMenu(_menubar);
    menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
    auto * menuWindow = new QMenu(_menubar);
    menuWindow->setObjectName(QString::fromUtf8("menuWindow"));
    auto * menuHelp = new QMenu(_menubar);
    menuHelp->setObjectName(QString::fromUtf8("menuHelp"));

    _menubar->addAction(menuEdit->menuAction());
    _menubar->addAction(menuWindow->menuAction());
    _menubar->addAction(menuHelp->menuAction());
    menuWindow->addAction(actionFix_to_ratio_width_height);
    menuWindow->addAction(actionAdjust_to_original_size);
    menuHelp->addAction(actionShow_help);

    actionFix_to_ratio_width_height->setText(QCoreApplication::translate("LocalDisplayer", "Fixed width and height ratio", nullptr));
    actionAdjust_to_original_size->setText(QCoreApplication::translate("LocalDisplayer", "Adjust to original size", nullptr));
    actionShow_help->setText(QCoreApplication::translate("LocalDisplayer", "Show Help", nullptr));

    menuEdit->setTitle(QCoreApplication::translate("LocalDisplayer", "Edit", nullptr));
    menuWindow->setTitle(QCoreApplication::translate("LocalDisplayer", "Window", nullptr));
    menuHelp->setTitle(QCoreApplication::translate("LocalDisplayer", "Help", nullptr));
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

    QMainWindow::resizeEvent(e);
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

    _imageLabel->setPixmap(QPixmap::fromImage(im));

    if (_winSize.isResized) {
        int wt = _winSize.curSize.width();
        int ht = _winSize.curSize.height();

        _imageLabel->resize (wt, ht);
        resize(wt, ht);

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

void LocalDisplayer::actionFixRatioWidthHeight()
{
    _winSize.curSize = _winSize.oriSize;
    _winSize.isResized = true;

    _winSize.ratioX = RATIO_PRECISION;
    _winSize.ratioY = RATIO_PRECISION;
}

void LocalDisplayer::startFetcher()
{
    MonitorVectorProvider mvp;

    int scale =
#ifdef __SHAREDT_IOS__
    (_fetcher->getType() != SP_WINDOW) ? RATIO_PRECISION : static_cast<int>(mvp.get().begin()->getScale() * RATIO_PRECISION);
#else
    RATIO_PRECISION;
#endif
    _winSize.oriSize.setWidth(_fetcher->getScreenProvide()->getWidth()*RATIO_PRECISION / scale);
    _winSize.oriSize.setHeight(_fetcher->getScreenProvide()->getHeight()*RATIO_PRECISION / scale);

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

int localDisplayer(struct cmdConf * conf)
{
    QApplication app(const_cast<int&> (conf->argc), const_cast<char **>(conf->argv));
    LocalDisplayer gui(const_cast<int&> (conf->argc), const_cast<char **> (conf->argv));

    if (!gui.isInited()) {
        return -1;
    }

    gui.show();
    gui.startFetcher();
    return QApplication::exec();
}

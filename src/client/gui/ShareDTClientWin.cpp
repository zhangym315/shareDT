#include <iostream>
#include <QObject>
#include <QColorSpace>
#include <qevent.h>
#include <QMessageBox>

#include <chrono>
#include "ShareDTClientWin.h"
#include "ui_ShareDTClientWin.h"
#include <png.h>

ShareDTClientWin::ShareDTClientWin (int argc, char ** argv,
                                    QWidget *parent)
    : _closed(false),
     QWidget (parent), ui (new Ui::ShareDTClientWin)
{
    ui->setupUi (this);

    ui->imageLabel->setText (QString("hello from ShareDTClientWin::ShareDTClientWin"));

    _fetcher = new FetchingDataFromServer(argc, argv);

    QObject::connect (_fetcher, SIGNAL(sendRect(rfbClient*)),
                      this, SLOT(putImage(rfbClient*)));
    QObject::connect (ui->actionAdjust_to_original_size, SIGNAL(triggered()),
                      this, SLOT(actionAdjustToOriginSize()));

    QObject::connect (_fetcher, SIGNAL(serverConnectionClosedSend()),
                      this, SLOT(serverConnectionClosed()));

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
std::cout << "resize Event: new size width=" << e->size().width()
            << " height=" << e->size().height() << " old size width="
            << e->oldSize().width() << " height=" << e->oldSize().height() << std::endl;
    _winResize.isResized = true;

    // resize can happened when user resize windows or VNCServer
    // send a different size window(resized on server sid).
    _winResize.oldSize = _winResize.curSize;
    _winResize.curSize = e->size();
    QWidget::resizeEvent(e);
}


static void writeToFile(const char * file, int x, int y, int w, int h, uint8_t * frame)
{
    unsigned int width = w;
    unsigned int height = h;
    FILE *fp = fopen(file, "wb");
    if(!fp) return ;

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) abort();

    png_infop info = png_create_info_struct(png);
    if (!info) abort();

    if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    png_set_IHDR(png,
                 info,
                 width, height,
                 8,
                 PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    for ( int i=0 ; i<height ; i++) {
            uint8_t * ptr = frame + i*width*4;
            for ( int j=0; j<width*4; ) {
                    *(ptr+j+3) = 0xff;
                    j += 4;
                }
            png_write_row(png, (png_bytep)(ptr));
        }

    png_write_end(png, nullptr);
    fclose(fp);

    png_destroy_write_struct(&png, &info);

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

    char path[128] = { 0 };
    sprintf(path, "File_after_sentHandleRect.%llu.png", client->_sequence);
    ui->imageLabel->setPixmap(QPixmap::fromImage(im));

// for debug
//    writeToFile(path, 0, 0, frame.w, frame.h, frame.frame.getData());
//    im.save(QString::asprintf ("File_after_QT_file.%llu.png", client->_sequence));


    if (_winResize.isResized) {
        int w = _winResize.curSize.width();
        int h = _winResize.curSize.height();
        ui->imageLabel->resize (w, h);
        ui->horizontalLayoutWidget->resize(w, h);
        this->resize(w, h);
        _winResize.isResized = false;
    }

//    using namespace std::chrono;
//    uint64_t nowCu = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
//    std::cout << "time=" << nowCu << " ShareDTClientWin::putImage new w=" << frame.w << " h=" << frame.h <<std::endl ;

    frame.frame.setUsed();
}

void ShareDTClientWin::resizeToNewVNC(int w, int h)
{
    _winResize.oldSize = _winResize.curSize;
    _winResize.curSize.setWidth (w);
    _winResize.curSize.setHeight (h);
    _winResize.vncSize.setWidth (w);
    _winResize.vncSize.setHeight (h);
    _winResize.isResized = true;
}

void ShareDTClientWin::actionAdjustToOriginSize()
{
    std::cout << "actionAdjustToOriginSize" << std::endl;
    _winResize.curSize = _winResize.vncSize;
    _winResize.isResized = true;
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
#include "ShareDTWindow.h"
#include "Buffer.h"
#include "WindowProcessor.h"
#include "Path.h"
#include "ExportAll.h"

void UI_ShareDTWindow::newGroupBox()
{
    auto * gb = new QGroupBox("192.168.56.113");
    auto * gbFL = new FlowLayout();
    gb->setLayout(gbFL);
/*
    gbFL->addWidget(newImageBox());
    gbFL->addWidget(newImageBox());
    gbFL->addWidget(newImageBox());
    gbFL->addWidget(newImageBox());
    gbFL->addWidget(newImageBox());
    gbFL->addWidget(newImageBox());
    gbFL->addWidget(newImageBox());
    gbFL->addWidget(newImageBox());
*/
    auto* scrollArea = new QScrollArea();
    scrollArea->setWidget(gb);
    scrollArea->setWidgetResizable( true );

    _mainLayout->addWidget(scrollArea);
    gb->setFont(QFont({"Arial", 10}));
    _remoteGroupBoxes.emplace_back(gb);
}

QWidget * UI_ShareDTWindow::newImageBox(int width, int height, unsigned char * data, const String & name) const
{
    auto * w = new QWidget();

    auto * image = new QLabel();
    auto * text = new QLabel();

    QImage im(data, width, height, QImage::Format::Format_RGBX8888);
    im = im.scaled(_w_unit, _h_unit);

    image->setFixedWidth(_w_unit);
    image->setFixedHeight(_h_unit);
    image->setPixmap(QPixmap::fromImage(im));
    image->setObjectName(QString::fromUtf8("imageLabel"));
    image->setFrameShape(QFrame::Box);
    image->setFrameShadow(QFrame::Plain);
    image->setLineWidth(0);

    String n = name.size() < 30 ? name : name.substr(0, 25) + String(" ...");
    text->setText(QString::fromUtf8(n.c_str()));
    text->setFont(QFont({"Arial", 10}));

    auto * l = new QVBoxLayout();
    l->addWidget(image);
    l->addWidget(text);
    w->setLayout(l);

    return w;
}

void UI_ShareDTWindow::setLocalWindows(QWidget *ShareDTWindow)
{
    _localGroupBox.layout = new FlowLayout();
    _localGroupBox.item = new QGroupBox(QString("Localhost Group Box"));
    _localGroupBox.item->setFont(QFont({"Arial", 22}));
    _localGroupBox.item->setLayout(_localGroupBox.layout);

    CircWRBuf<FrameBuffer>  cwb(2);
    MonitorVectorProvider mvp;
    CapPoint cp(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    FrameBuffer * fb;

    for (auto & m : mvp.get()) {
        ExportAll ea(SP_MONITOR, m.getId());
        if ((fb=ea.getFrameBuffer(cwb)) == nullptr) continue;

        _localGroupBox.layout->addWidget(newImageBox(fb->getWidth(), fb->getHeight(), fb->getData(), m.getName()));
        //set smallest width and height
        if (cp.getX() > fb->getWidth() && cp.getY() > fb->getHeight()) {
            cp.setX((int) fb->getWidth());
            cp.setY((int) fb->getHeight());
        }
    }

    WindowVectorProvider wvp(-1);

    for (const auto & w : wvp.get()) {
        ExportAll ea(SP_WINDOW, w.getHandler());
        if ((fb=ea.getFrameBuffer(cwb)) == nullptr) continue;

        // filter out the unnecessary window
        if ((fb=ea.getFrameBuffer(cwb)) == nullptr ||
                fb->getWidth() < cp.getX()/8 ||
                fb->getHeight() < cp.getY()/8 ||
            ExportAll::filterExportWinName(w.getName()))
            continue;

        _localGroupBox.layout->addWidget(newImageBox(fb->getWidth(),
                                                     fb->getHeight(),
                                                     fb->getData(),
                                                     w.getName()));
    }

    auto * scrollArea = new QScrollArea();
    scrollArea->setWidget(_localGroupBox.item);
    scrollArea->setWidgetResizable( true );

    _mainLayout->addWidget(scrollArea);

}

void UI_ShareDTWindow::setupMainWindow(QWidget *ShareDTWindow)
{
    _mainLayout = new QVBoxLayout(ShareDTWindow);
    ShareDTWindow->setGeometry(600, 100, 1000, 900);
}

void UI_ShareDTWindow::setupUi(QWidget *ShareDTWindow)
{
    setupMainWindow(ShareDTWindow);
    setLocalWindows(ShareDTWindow);
}
#include "ShareDTWindow.h"

void UI_ShareDTWindow::newGroupBox()
{
    auto * gb = new QGroupBox("192.168.56.113");
    auto * gbFL = new FlowLayout();
    gb->setLayout(gbFL);

    gbFL->addWidget(newImageBox());
    gbFL->addWidget(newImageBox());
    gbFL->addWidget(newImageBox());
    gbFL->addWidget(newImageBox());
    gbFL->addWidget(newImageBox());
    gbFL->addWidget(newImageBox());
    gbFL->addWidget(newImageBox());
    gbFL->addWidget(newImageBox());

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidget(gb);
    scrollArea->setWidgetResizable( true );

    _mainLayout->addWidget(scrollArea);
    gb->setFont(QFont({"Arial", 10}));
    _remoteGroupBoxes.emplace_back(gb);
}

QWidget * UI_ShareDTWindow::newImageBox()
{
    auto * w = new QWidget();

    auto * image = new QLabel();
    auto * text = new QLabel();

    unsigned char buf[100*80*4] = { 50 };
    QImage im(buf, 100, 80, QImage::Format::Format_RGB32);

    image->setPixmap(QPixmap::fromImage(im));

    image->setObjectName(QString::fromUtf8("imageLabel"));
    image->setFrameShape(QFrame::Box);
    image->setFrameShadow(QFrame::Plain);
    image->setLineWidth(0);
    image->setFixedWidth(100);
    image->setFixedHeight(80);

    text->setText("Hello New Images");
    text->setFont(QFont({"Arial", 10}));

    auto * l = new QVBoxLayout();
    l->addWidget(image);
    l->addWidget(text);
    w->setLayout(l);

    return w;
}

void UI_ShareDTWindow::setupMainWindow(QWidget *ShareDTWindow)
{
    _mainLayout = new QVBoxLayout(ShareDTWindow);
    ShareDTWindow->setGeometry(600, 100, 1000, 900);
    _localGroupBox.layout = new FlowLayout();
    _localGroupBox.item = new QGroupBox(QString("localhost group box"));
    _localGroupBox.item->setFont(QFont({"Arial", 22}));

    _localGroupBox.layout->addWidget(newImageBox());
    _localGroupBox.layout->addWidget(newImageBox());
    _localGroupBox.layout->addWidget(newImageBox());
    _localGroupBox.layout->addWidget(newImageBox());
    _localGroupBox.layout->addWidget(newImageBox());
    _localGroupBox.layout->addWidget(newImageBox());
    _localGroupBox.layout->addWidget(newImageBox());
    _localGroupBox.layout->addWidget(newImageBox());
    _localGroupBox.layout->addWidget(newImageBox());

    _localGroupBox.item->setLayout(_localGroupBox.layout);

    _mainLayout->addWidget(_localGroupBox.item);
    newGroupBox();
    newGroupBox();
    newGroupBox();
}

void UI_ShareDTWindow::setupUi(QWidget *ShareDTWindow)
{
    return setupMainWindow(ShareDTWindow);
}
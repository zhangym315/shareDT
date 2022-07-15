#include "ShareDTWindow.h"
#include "Buffer.h"
#include "WindowProcessor.h"
#include "Path.h"
#include "ExportAll.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "TimeUtil.h"
#ifdef __cplusplus
}
#endif

#include <QObject>
#include <QMessageBox>
#include <QEvent>
#include <QMouseEvent>
#include <QProcess>
#include <QChar>

void UI_ShareDTWindow::newRemoteGroupBox()
{
    auto * gb = new QGroupBox("192.168.56.113");
    auto * gbFL = new FlowLayout();
    gb->setLayout(gbFL);

    auto* scrollArea = new QScrollArea();
    scrollArea->setWidget(gb);
    scrollArea->setWidgetResizable( true );

    _mainLayout->addWidget(scrollArea);
    gb->setFont(QFont({"Arial", 10}));
    _remoteGroupBoxes.emplace_back(gb);
}

void ImageItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::RightButton) {
    } else {
    }
    QWidget::mousePressEvent(event);
}

void ImageItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::RightButton) {
    } else {
        QString program = ShareDTHome::instance()->getArgv0().c_str();

        std::cout << "Starting display for name=\"" << _info.name << "\" command=\""
            << qPrintable(program) << " "
            << qPrintable(_info.argument.join(QChar::SpecialCharacter::Space)) << "\""
            << std::endl;

        auto * newProcess = new QProcess();
        newProcess->start(program, _info.argument);
    }
    QWidget::mouseReleaseEvent(event);
}

void ImageItem::enterEvent(QEvent *event)
{
    QWidget::setCursor(QCursor(Qt::PointingHandCursor));
}

QWidget * UI_ShareDTWindow::newImageBox(int width, int height, unsigned char * data, const ItemInfo & info) const
{
    auto * w = new ImageItem(info);

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

    String n = info.name.size() < 30 ? info.name : info.name.substr(0, 25) + String(" ...");
    text->setText(QString::fromUtf8(n.c_str()));
    text->setFont(QFont({"Arial", 10}));

    auto * l = new QVBoxLayout();
    l->addWidget(image);
    l->addWidget(text);
    w->setLayout(l);

    return w;
}

void UI_ShareDTWindow::setLocalWindows(QWidget *w)
{
    _localGroupBox.layout = new FlowLayout();
    _localGroupBox.item = new QGroupBox(QString("Localhost Group Box"));
    _localGroupBox.item->setFont(QFont({"Arial", 22}));
    _localGroupBox.item->setLayout(_localGroupBox.layout);

    auto * scrollArea = new QScrollArea();
    scrollArea->setWidget(_localGroupBox.item);
    scrollArea->setWidgetResizable( true );
    scrollArea->setGeometry(QRect(600, 1000, 1000, 900));

    _mainLayout->addWidget(scrollArea);
    
    refreshLocalBoxGroupInternal();
}

void UI_ShareDTWindow::setupMainWindow(QWidget *w)
{
    _mainLayout = new QVBoxLayout(w);
    w->setGeometry(600, 300, 1000, 900);
}

void UI_ShareDTWindow::setupUi(QWidget *w)
{
    setupMainWindow(w);
    setLocalWindows(w);
}

void UI_ShareDTWindow::refreshLocalBoxGroup(QWidget * w)
{
    QLayoutItem * child;
    while ((child = _localGroupBox.layout->itemAt(0)) != nullptr) {
        _localGroupBox.layout->removeItem(child);
        removeImageBox(child->widget());  // remove single image box(image widget and label)
        delete child;
        _localGroupBox.layout->activate();
    }

    _localGroupBox.layout->activate();

    refreshLocalBoxGroupInternal();
}

void UI_ShareDTWindow::refreshLocalBoxGroupInternal() const
{
    if (_localGroupBox.layout == nullptr) return;

    CircleWRBuf<FrameBuffer>  cwb(2);
    MonitorVectorProvider mvp;
    CapPoint cp(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    FrameBuffer * fb;
    for (auto & m : mvp.get()) {
        ExportAll ea(SP_MONITOR, m.getId());
        ItemInfo info;

        if ((fb=ea.getFrameBuffer(cwb)) == nullptr) continue;

        info.name = m.getName();
        info.argument << "-c" << "mon" <<  "-i" << std::to_string(m.getId()).c_str();
        _localGroupBox.layout->addWidget(newImageBox(fb->getWidth(),
                                                     fb->getHeight(),
                                                     fb->getData(),
                                                     info));
        //set smallest width and height
        if (cp.getX() > fb->getWidth() && cp.getY() > fb->getHeight()) {
            cp.setX((int) fb->getWidth());
            cp.setY((int) fb->getHeight());
        }
        fb->setUsed();
    }

    WindowVectorProvider wvp(-1);
    for (const auto & w : wvp.get()) {

        if (ExportAll::filterExportWinName(w.getName())) continue;

        ExportAll ea(SP_WINDOW, w.getHandler());
        ItemInfo info;

        // filter out the unnecessary window
        if ((fb=ea.getFrameBuffer(cwb)) == nullptr ||
            fb->getWidth() < cp.getX()/8 ||
            fb->getHeight() < cp.getY()/8) {
            continue;
        }

        info.name = w.getName();
        info.argument << "-c" <<  "win" <<  "-h" << std::to_string(w.getHandler()).c_str();
        _localGroupBox.layout->addWidget(newImageBox(fb->getWidth(),
                                                     fb->getHeight(),
                                                     fb->getData(),
                                                     info));
        fb->setUsed();
    }
}

/* corresponding to newImageBox() to remove box */
void UI_ShareDTWindow::removeImageBox(QWidget *w) {
    if (w == nullptr) return;

    auto *l1 = w->layout();
    QLayoutItem *c1;
    while ((c1 = l1->itemAt(0)) != nullptr) {
        l1->removeItem(c1);
        delete c1;
        l1->activate();
    }
}

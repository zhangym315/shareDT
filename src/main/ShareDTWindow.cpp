#include "ShareDTWindow.h"
#include "Buffer.h"
#include "WindowProcessor.h"
#include "Path.h"
#include "ExportAll.h"
#include "Logger.h"

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
#include <QInputDialog>
#include <QDesktopWidget>

const static int gpBoxFontSize = 15;

void UI_ShareDTWindow::newRemoteGroupBox(const String & host)
{
    if (_remoteGroupBoxes.find(host) != _remoteGroupBoxes.end()) {
        QMessageBox msgBox;
        QString message("Host already connected: ");
        message.append(host.c_str());
        msgBox.setText(message); msgBox.exec();
        return;
    }

    auto * gb= new GroupBox(host.c_str(), this);
    FlowLayout gbFL;
    gb->setLayout(&gbFL);

    auto* scrollArea = new QScrollArea();
    scrollArea->setWidget(gb);
    scrollArea->setWidgetResizable( true );

    _mainLayout->addWidget(scrollArea);
    gb->setFont(QFont(QString("Arial"), gpBoxFontSize));
    _remoteGroupBoxes[host] = gb;
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

        LOGGER.info() << "Starting display for name=\"" << _info.name << "\" command=\""
            << qPrintable(program) << " "
            << qPrintable(_info.argument.join(QChar::SpecialCharacter::Space)) << "\"";

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

#ifdef __SHAREDT_WIN__
    String n = info.name.size() < 20 ? info.name : info.name.substr(0, 10) + String(" ...");
#else
    String n = info.name.size() < 20 ? info.name : info.name.substr(0, 15) + String(" ...");
#endif
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
    _localGroupBox.item = new GroupBox(QString("Localhost"), this);
    _localGroupBox.item->setFont(QFont(QString("Arial"), gpBoxFontSize));
    _localGroupBox.item->setLayout(_localGroupBox.layout);

    auto * scrollArea = new QScrollArea();
    scrollArea->setWidget(_localGroupBox.item);
    scrollArea->setWidgetResizable( true );
    scrollArea->setGeometry(QRect(600, 1000, 1000, 900));

    _mainLayout->addWidget(scrollArea);
    _freshMainWin.start();

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

void UI_ShareDTWindow::refreshLocalBoxGroup()
{
    LOGGER.info() << "Fresh slot triggered";
    QLayoutItem * child;
    while ((child = _localGroupBox.layout->itemAt(0)) != nullptr) {
        _localGroupBox.layout->removeItem(child);
        removeImageBox(child->widget());  // remove single image box(image widget and label)
        delete child->widget();
        delete child;
        _localGroupBox.layout->activate();
    }

    _localGroupBox.layout->activate();

    refreshLocalBoxGroupInternal();
}

void UI_ShareDTWindow::refreshLocalBoxGroupInternal() const
{
    if (_localGroupBox.layout == nullptr) {
        LOGGER.error() << "Failed to refresh local box group because layout=null";
        return;
    }

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
//        LOGGER.info() << "Refreshed monitor_id=" << m.getId() << " name=\"" << info.name
//                        << "\" argument=\"" << info.argument.join(" ").toStdString() << "\"";
    }

    WindowVectorProvider wvp(-1);
    for (const auto & w : wvp.get()) {

        if (ExportAll::filterExportWinName(w.getName())) {
            LOGGER.info() << "Skipped fresh window_id=" << w.getHandler() << " name=\"" << w.getName();
            continue;
        }

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
//        LOGGER.info() << "Refreshed window_id=" << w.getHandler() << " name=\"" << info.name
//                      << "\" argument=\"" << info.argument.join(" ").toStdString() << "\"";
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

UI_ShareDTWindow::~UI_ShareDTWindow() {
    // make sure fetcher thread is shutdown
    while(!_freshMainWin.isShutDown()) {
        _freshMainWin.stop();
    }
}

void UI_ShareDTWindow::newGroupConnection() {
    bool ok;
    QString text = QInputDialog::getText(nullptr, tr("New Connections"),
                                         tr("Host Address:"), QLineEdit::Normal,
                                         "", &ok);

    newRemoteGroupBox(text.toStdString());
    LOGGER.info() << "Trying to connect to address=" << text.toStdString();
}

void UI_ShareDTWindow::startLocalCaptureServer() {
    String startServer = ShareDTHome::instance()->getArgv0Dir() + String(PATH_SEP_STR) + String("ShareDTServer");
#ifdef __SHAREDT_WIN__
    startServer.append(".exe");
#endif
    QString startServerProgram = startServer.c_str();
    QStringList args("start");

    LOGGER.info() << "Starting ShareDTServer path=" << qPrintable(startServerProgram);

    auto * newProcess = new QProcess();
    newProcess->start(startServerProgram, args);
}

void UI_ShareDTWindow::FreshMainWindow::run() {
    while(!_stopped) {
        if (_autoFresh.load())
            emit _ui->refreshSignal();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    _shutdown = true;
}


void GroupBox::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MouseButton::RightButton) {
        QMenu contextMenu(tr("Context menu"), this);

        QAction actionConnection("New Connection", this);
        connect(&actionConnection, SIGNAL(triggered()), _ui, SLOT(newGroupConnection()));
        contextMenu.addAction(&actionConnection);

        contextMenu.addSection(QString(""));

        QAction actionRefresh("Refresh Window", this);
        connect(&actionRefresh, SIGNAL(triggered()), _ui, SLOT(refreshSlot()));
        contextMenu.addAction(&actionRefresh);
        contextMenu.exec(event->globalPos());
    }

    QGroupBox::mousePressEvent(event);
}

void GroupBox::mouseReleaseEvent(QMouseEvent *event) {
    QGroupBox::mouseReleaseEvent(event);
}

ShareDTWindow::ShareDTWindow (int argc, char ** argv, QWidget *parent) :
        _ui(new Ui::ShareDTWindow)
{
#ifndef __SHAREDT_IOS__
    // set program icon, ShareDT.png should be the same directory
    std::string png = ShareDTHome::instance()->getHome() + std::string(PATH_SEP_STR) +
                std::string("bin") + std::string(PATH_SEP_STR) + std::string("ShareDT.png");
    setWindowIcon(QIcon(QPixmap(png.c_str())));
#endif

    setMenu();
    setCentralWidget(parent);
    resize(QDesktopWidget().availableGeometry(this).size() * 0.5);
    _ui->setupUi (parent);
}

void ShareDTWindow::setMenu()
{
    auto * menubar = menuBar();
    menubar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    menubar->setGeometry(QRect(0, 0, 800, 50));

    /* Edit */
    auto * menuEdit = new QMenu(menubar);
    menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
    menuEdit->setTitle(QCoreApplication::translate("ShareDTClientWin", "Edit", nullptr));

    auto * localCapture = new QAction();
    localCapture->setObjectName(QString::fromUtf8("startCapture"));
    localCapture->setText(QCoreApplication::translate("ShareDTWindow", "Start Local Capture Server", nullptr));
    menuEdit->addAction(localCapture);
    QObject::connect (localCapture, SIGNAL(triggered()), _ui, SLOT(startLocalCaptureServer()));

    auto * newConnect = new QAction();
    newConnect->setObjectName(QString::fromUtf8("new_connection"));
    newConnect->setText(QCoreApplication::translate("ShareDTWindow", "New Connection", nullptr));
    menuEdit->addAction(newConnect);
    QObject::connect (newConnect, SIGNAL(triggered()), _ui, SLOT(newGroupConnection()));
    /* Edit end*/

    /* Window */
    auto * menuWindow = new QMenu(menubar);
    menuWindow->setObjectName(QString::fromUtf8("menuWindow"));
    menuWindow->setTitle(QCoreApplication::translate("ShareDTWindow", "Window", nullptr));

    auto * freshWin = new QAction();
    freshWin->setObjectName(QString::fromUtf8("fresh_items"));
    freshWin->setText(QCoreApplication::translate("ShareDTWindow", "Refresh Items", nullptr));
    menuWindow->addAction(freshWin);
    QObject::connect (freshWin, SIGNAL(triggered()), this, SLOT(actionFreshItems()));
    /* Window end */

    /* Help */
    auto *menuHelp = new QMenu(menubar);
    menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
    menuHelp->setTitle(QCoreApplication::translate("ShareDTWindow", "Help", nullptr));

    auto * aboutWin = new QAction();
    aboutWin->setObjectName(QString::fromUtf8("about_window"));
    aboutWin->setText(QCoreApplication::translate("ShareDTWindow", "About", nullptr));
    menuHelp->addAction(aboutWin);
    /* Help  end */

    menubar->addAction(menuEdit->menuAction());
    menubar->addAction(menuWindow->menuAction());
    menubar->addAction(menuHelp->menuAction());
}

ShareDTWindow::~ShareDTWindow()
{
    delete _ui;
    LOGGER.info() << "Stopped...";
}

void ShareDTWindow::actionFreshItems()
{
    _ui->refreshLocalBoxGroup();
}

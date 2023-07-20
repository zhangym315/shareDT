#include "MainGUI.h"
#include "Buffer.h"
#include "WindowProcessor.h"
#include "Path.h"
#include "ExportAll.h"
#include "Logger.h"
#include "SubFunction.h"
#include "RemoteGetter.h"
#include "MainService.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "TimeUtil.h"
#ifdef __cplusplus
}
#endif

#include <QMessageBox>
#include <QEvent>
#include <QMouseEvent>
#include <QProcess>
#include <QInputDialog>
#include <QScreen>
#include <memory>

#ifdef __SHAREDT_WIN__
#include <Shlobj.h>
#include <windows.h>
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

const static int gpBoxFontSize = 15;

int mainGUI(struct cmdConf * conf) {
    ShareDTHome::instance()->set(conf->argv[0]);
#ifdef __SHAREDT_WIN__
    WCHAR * filepath;
    if (!SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &filepath))) {
    }
    std::wstring ws(filepath);
    std::string varRun = std::string(ws.begin(), ws.end()) + std::string(PATH_SEP_STR) +
                    std::string(SHAREDT_KEYWORD) + std::string(PATH_SEP_STR) +  std::string(VAR_RUN);
    if (!fs::exists(varRun)) fs::create_directories(varRun);
    //set log file to var/run/ShareDT.log
    Logger::instance().setLogFile((varRun+std::string(CAPTURE_LOG)).c_str());
#else
    std::string varrun = ShareDTHome::instance()->getHome() + std::string(VAR_RUN);
    if (!fs::exists(varrun)) fs::create_directories(varrun);
    //set log file to var/run/ShareDT.log
    Logger::instance().setLogFile((ShareDTHome::instance()->getHome() +
                                   std::string(VAR_RUN)+std::string(CAPTURE_LOG)).c_str());
#endif

    QApplication app(conf->argc, conf->argv);
    LOGGER.info() << "Starting " << conf->argv[0] << " ...";

    ShareDTWindow gui(conf->argc, conf->argv);
    gui.show();
    return QApplication::exec();
}

void UI_ShareDTWindow::newRemoteGroupBox(const std::string & host)
{
    /* check if already connected */
    if (_remoteGroupBoxes.find(host) != _remoteGroupBoxes.end()) {
        QMessageBox msgBox;
        QString message("Host already connected: ");
        message.append(host.c_str());
        msgBox.setText(message); msgBox.exec();
        return;
    }

    /* connection */
    SocketClient sc(host, SHAREDT_INTERNAL_PORT_START);
    if (!sc.connectWait()) {
        QMessageBox msgBox;
        msgBox.setText(QString("Cannot connect to host: ") +
                        QString::fromStdString(host));
        msgBox.exec();
        return ;
    }

    auto * gb= new GroupBox(host.c_str(), this);
    auto * gbFL = new FlowLayout();
    gb->setLayout(gbFL);

    auto* scrollArea = new QScrollArea();
    scrollArea->setWidget(gb);
    scrollArea->setWidgetResizable( true );
    scrollArea->setGeometry(QRect(600, 1000, 1000, 900));

    _mainLayout->addWidget(scrollArea);
    gb->setFont(QFont(QString("Arial"), gpBoxFontSize));
    _remoteGroupBoxes[host].reset(gb);

    std::string cmd = "ShareDT";
    cmd.append(" ");
    cmd.append(SHAREDT_SERVER_COMMAND_REMOTGET);
    sc.sendString(cmd);

    FrameBuffer fb;
    while (true) {
        RemoteGetterMsg msg{};
        if (sc.receiveBytes((unsigned char *) &msg, sizeof(msg)) <= 0) break;

        msg.convert();

        fb.reSet(msg.dataLen);
        fb.setWidthHeight(msg.w, msg.h);
        if (sc.receiveBytes(fb.getDataToWrite(), msg.dataLen) < 0 || msg.dataLen==0) break;

        LOGGER.info() << "Received frame buffer size=" << msg.dataLen
                      << " name=" << msg.name
                      << " cmdArgs=" << msg.cmdArgs;

        ItemInfo info;
        info.name = msg.name;
        info.isRemote = true;
        info.argument = QString(msg.cmdArgs).split(" ") << ::inet_ntoa(sc.getAddr().sin_addr);
        info.sadd = sc.getAddr();
        gbFL->addWidget(newImageBox(fb.getWidth(),
                                       fb.getHeight(),
                                       fb.getData(),
                                       info));
    }
}

void ImageItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::RightButton) {
    } else {
    }
    QWidget::mousePressEvent(event);
}

/*
 * Image items click, needs to start up capture server, then do a remote connect
 */
void ImageItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::RightButton) {
    } else {
        QString program = ShareDTHome::instance()->getArgv0().c_str();

        LOGGER.info() << "Starting display for name=\"" << _info.name << "\" command=\""
            << qPrintable(_info.argument.join(QChar::SpecialCharacter::Space)) << "\"";

        if (_info.isRemote) {
            SocketClient sc(_info.sadd);
            if (!sc.connect()) {
                LOGGER.error() << "Failed to connect server process.";
                return;
            }

            sc.sendString(qPrintable(_info.argument.join(QChar::SpecialCharacter::Space)));

            StartingCaptureServerMsg msg{};
            if (sc.receiveBytes((unsigned char *) &msg, sizeof(msg)) < 0) {
                LOGGER.error() << "Failed to start capture server for display, name=" << _info.name << "\" command=\""
                                << qPrintable(_info.argument.join(QChar::SpecialCharacter::Space)) << "\"";
            } else {
                LOGGER.info() << "Received info for started capture server, status=" << msg.startedStatus << " capturePort=" << msg.capturePort;
                if (msg.startedStatus == 0) {
                    _process = std::make_unique<QProcess>();
                    QStringList arg;
                    std::string addPort = std::string(::inet_ntoa(_info.sadd.sin_addr)) + std::string(":") + std::to_string(msg.capturePort-5900);
                    arg << "connect" << "-encodings" << "raw" << QString::fromStdString(addPort);
                    _process->start(program, arg);
                }
            }
        } else {
            _process = std::make_unique<QProcess>();
            _process->start(program, _info.argument);
        }
    }
    QWidget::mouseReleaseEvent(event);
}

void ImageItem::enterEvent(QEnterEvent *event)
{
    QWidget::setCursor(QCursor(Qt::PointingHandCursor));
}

QWidget * UI_ShareDTWindow::newImageBox(int width, int height, unsigned char * data,
                                        const ItemInfo & info) const
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
    std::string n = info.name.size() < 20 ? info.name : info.name.substr(0, 10) + std::string(" ...");
#else
    std::string n = info.name.size() < 20 ? info.name : info.name.substr(0, 15) + std::string(" ...");
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
        info.argument << SHAREDT_SERVER_COMMAND_DISPLAY << "-c" << "mon" <<  "-i" << std::to_string(m.getId()).c_str();
        info.isRemote = false;
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
        info.argument << SHAREDT_SERVER_COMMAND_DISPLAY << "-c" <<  "win" <<  "-h" << std::to_string(w.getHandler()).c_str();
        info.isRemote = false;
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
    std::string startServer = ShareDTHome::instance()->getArgv0Dir() + std::string(PATH_SEP_STR) + std::string("ShareDT");
#ifdef __SHAREDT_WIN__
    startServer.append(".exe");
#endif
    QString startServerProgram = startServer.c_str();
    QStringList args("start");

    LOGGER.info() << "Starting ShareDT Server path=" << qPrintable(startServerProgram);

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
        _ui(new Ui::MainGUI)
{
#ifndef __SHAREDT_IOS__
    setIcon(this);
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
    localCapture->setText(QCoreApplication::translate("MainGUI", "Start Local Capture Server", nullptr));
    menuEdit->addAction(localCapture);
    QObject::connect (localCapture, SIGNAL(triggered()), _ui, SLOT(startLocalCaptureServer()));

    auto * newConnect = new QAction();
    newConnect->setObjectName(QString::fromUtf8("new_connection"));
    newConnect->setText(QCoreApplication::translate("MainGUI", "New Connection", nullptr));
    menuEdit->addAction(newConnect);
    QObject::connect (newConnect, SIGNAL(triggered()), _ui, SLOT(newGroupConnection()));
    /* Edit end*/

    /* Window */
    auto * menuWindow = new QMenu(menubar);
    menuWindow->setObjectName(QString::fromUtf8("menuWindow"));
    menuWindow->setTitle(QCoreApplication::translate("MainGUI", "Window", nullptr));

    auto * freshWin = new QAction();
    freshWin->setObjectName(QString::fromUtf8("fresh_items"));
    freshWin->setText(QCoreApplication::translate("MainGUI", "Refresh Items", nullptr));
    menuWindow->addAction(freshWin);
    QObject::connect (freshWin, SIGNAL(triggered()), this, SLOT(actionFreshItems()));
    /* Window end */

    /* Help */
    auto *menuHelp = new QMenu(menubar);
    menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
    menuHelp->setTitle(QCoreApplication::translate("MainGUI", "Help", nullptr));

    auto * aboutWin = new QAction();
    aboutWin->setObjectName(QString::fromUtf8("about_window"));
    aboutWin->setText(QCoreApplication::translate("MainGUI", "About", nullptr));
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

void ShareDTWindow::setIcon(QWidget * q) {
    // set program icon, ShareDT.png should be the same directory
    std::string png = ShareDTHome::instance()->getHome() + std::string(PATH_SEP_STR) +
                      std::string("bin") + std::string(PATH_SEP_STR) + std::string("ShareDT.png");
    q->setWindowIcon(QIcon(QPixmap(png.c_str())));
}

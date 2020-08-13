#include "ServerMainWindow.h"
#include "ui_ServerMainWindow.h"
#include <QtGui>
#include <QtWidgets>
#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDoubleValidator>

#define ICON_EXPAND   QCoreApplication::applicationDirPath() + "/../images/expand.png"
#define ICON_CALLAPSE QCoreApplication::applicationDirPath() + "/../images/callapse.png"
#define ICON_WIDTH    120
#define ICON_HEIGHT   130

LockerButton::LockerButton(QWidget* parent)
    : QPushButton(parent)
{
    m_imageLabel = new QLabel;
    m_imageLabel->setFixedWidth(30);
    m_imageLabel->setScaledContents(true);
    m_imageLabel->setStyleSheet("QLabel{background-color:transparent;}");

    m_textLabel = new QLabel;
    m_textLabel->setStyleSheet("QLabel{background-color:transparent;}");

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_imageLabel);
    mainLayout->addWidget(m_textLabel);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    this->setLayout(mainLayout);
}

void LockerButton::SetImageLabel(const QPixmap &pixmap)
{
    m_imageLabel->setPixmap(pixmap);
}

void LockerButton::SetTextLabel(QString text)
{
    m_textLabel->setText(text);
}

QLabel* LockerButton::GetImageHandle()
{
    return m_imageLabel;
}

QLabel* LockerButton::GetTextHandle()
{
    return m_textLabel;
}

MainServerWindowWidget::MainServerWindowWidget(QWidget * parent) :
            QWidget(parent), _monSwitch(1), _winSwtich(1), _partSwitch(1)
{
    SetUpUI();
}

void MainServerWindowWidget::SetUpUI()
{
    /* monitor button */
    _monButton = new LockerButton;
    _monButton->setObjectName("monLockerButton");
    _monButton->SetTextLabel("Monitor Capture Server");
    _monButton->SetImageLabel(QPixmap(ICON_EXPAND));
    _monButton->setStyleSheet("#LockerButton{background-color:rgba(195,195,195,1)}"
                            "#LockerButton:hover{background-color:grey}"
                            "#LockerButton:pressed{background-color:rgba(127,127,127,0.4)}");
    QLabel* sizeLabel = _monButton->GetTextHandle();
    sizeLabel->setStyleSheet("QLabel{color:rgba(183,71,42,1)}");
    sizeLabel->setFont(QFont("Monitor Capture Server", 15, QFont::Black));

    /* monitor Widget */
    _monWidget = new QListWidget;
    _monWidget->resize(100,400);
    _monWidget->setViewMode(QListView::IconMode);
    _monWidget->setIconSize(QSize(ICON_WIDTH, ICON_HEIGHT));
    _monWidget->setSpacing(10);
    _monWidget->setResizeMode(QListWidget::Adjust);
    _monWidget->setMovement(QListWidget::Static);
    for(int i=0; i < 20; i++)
    {
        QListWidgetItem *imageItem = new QListWidgetItem;
        imageItem->setIcon(QIcon("/Users/yimingz/Desktop/thumbnail.jpg"));
        imageItem->setSizeHint(QSize(ICON_WIDTH, ICON_HEIGHT));
        _monWidget->addItem(imageItem);
    }

    /* windows button */
    _winButton = new LockerButton;
    _winButton->setObjectName("winLockerButton");
    _winButton->SetTextLabel("Window Capture Server");
    _winButton->SetImageLabel(QPixmap(ICON_EXPAND));
    _winButton->setStyleSheet("#LockerButton{background-color:lightgrey}"
                            "#LockerButton:hover{background-color:rgba(195,195,195,0.4)}"
                            "#LockerButton:pressed{background-color:rgba(127,127,127,0.4)}");
    _winButton->setParent(this);
    sizeLabel = _winButton->GetTextHandle();
    sizeLabel->setStyleSheet("QLabel{color:rgba(183,71,42,1)}");
    sizeLabel->setFont(QFont("size", 15, QFont::Black));

    /* window Widget */
    _winWidget = new QListWidget;
    _winWidget->resize(100,400);
    _winWidget->setViewMode(QListView::IconMode);
    _winWidget->setIconSize(QSize(ICON_WIDTH, ICON_HEIGHT));
    _winWidget->setSpacing(10);
    _winWidget->setResizeMode(QListWidget::Adjust);
    _winWidget->setMovement(QListWidget::Static);
    for(int i=0; i < 10; i++)
    {
        QListWidgetItem *imageItem = new QListWidgetItem;
        imageItem->setIcon(QIcon("/Users/yimingz/Desktop/Picture1.png"));
        imageItem->setSizeHint(QSize(ICON_WIDTH, ICON_HEIGHT));
        _winWidget->addItem(imageItem);
    }

    /* partial button */
    _partButton = new LockerButton;
    _partButton->setObjectName("partLockerButton");
    _partButton->SetTextLabel("Partial Capture Server");
    _partButton->SetImageLabel(QPixmap(ICON_EXPAND));
    _partButton->setStyleSheet("#LockerButton{background-color:lightgrey}"
                            "#LockerButton:hover{background-color:rgba(195,195,195,0.4)}"
                            "#LockerButton:pressed{background-color:rgba(127,127,127,0.4)}");
    sizeLabel = _partButton->GetTextHandle();
    sizeLabel->setStyleSheet("QLabel{color:rgba(183,71,42,1)}");
    sizeLabel->setFont(QFont("size", 15, QFont::Black));
    /* partial Widget */
    _partWidget = new QListWidget;
    _partWidget->resize(100,400);
    _partWidget->setViewMode(QListView::IconMode);
    _partWidget->setIconSize(QSize(ICON_WIDTH, ICON_HEIGHT));
    _partWidget->setSpacing(10);
    _partWidget->setResizeMode(QListWidget::Adjust);
    _partWidget->setMovement(QListWidget::Static);
    for(int i=0; i < 10; i++)
    {
        QListWidgetItem *imageItem = new QListWidgetItem;
        imageItem->setIcon(QIcon("/Users/yimingz/Desktop/thumbnail.jpg"));
        imageItem->setSizeHint(QSize(ICON_WIDTH, ICON_HEIGHT));
        _partWidget->addItem(imageItem);
    }

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->addWidget(_monButton);
    vlayout->addWidget(_monWidget);
    vlayout->addSpacing(15);
    vlayout->addWidget(_winButton);
    vlayout->addWidget(_winWidget);
    vlayout->addSpacing(15);
    vlayout->addWidget(_partButton);
    vlayout->addWidget(_partWidget);
    vlayout->addStretch();
    vlayout->setContentsMargins(0,0,0,0);
    vlayout->setSpacing(0);
    this->setLayout(vlayout);

    connect(_monButton, &LockerButton::clicked, [this](bool) {
        if (_monSwitch % 2)
        {
            _monButton->SetImageLabel(QPixmap(ICON_CALLAPSE));
            _monWidget->setVisible(false);
        }
        else
        {
            _monButton->SetImageLabel(QPixmap(ICON_EXPAND));
            _monWidget->setVisible(true);
        }
        _monSwitch++; });

    connect(_winButton, &LockerButton::clicked, [this](bool) {
        if (_winSwtich % 2)
        {
            _winButton->SetImageLabel(QPixmap(ICON_CALLAPSE));
            _winWidget->setVisible(false);
        }
        else
        {
            _winButton->SetImageLabel(QPixmap(ICON_EXPAND));
            _winWidget->setVisible(true);
        }
        _winSwtich++; });

    connect(_partButton, &LockerButton::clicked, [this](bool) {
        if (_partSwitch % 2)
        {
            _partButton->SetImageLabel(QPixmap(ICON_CALLAPSE));
            _partWidget->setVisible(false);
        }
        else
        {
            _partButton->SetImageLabel(QPixmap(ICON_EXPAND));
            _partWidget->setVisible(true);
        }
        _partSwitch++; });
}

ServerMainWindow::ServerMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ServerMainWindow)
{
    ui->setupUi(this);

    initMenu();
    initImage();
    initStartedServer();
}

ServerMainWindow::~ServerMainWindow()
{
    delete ui;
}

void ServerMainWindow::initMenu()
{
    /* 1. setup menu */
    // add main menu
    QMenu *file = menuBar()->addMenu("File");
    QMenu *view = menuBar()->addMenu("View");
    QMenu *about= menuBar()->addMenu("About");

    // 1.1 File menu
    QAction *Act_file_new = new QAction("New Capture Server", this);
    Act_file_new->setShortcuts(QKeySequence::New);  // shortcut Ctrl+N
    connect(Act_file_new, SIGNAL(triggered()), this, SLOT(newServer()));
    QAction *Act_file_close = new QAction("Close", this);
    Act_file_close->setShortcuts(QKeySequence::Close);  // shutcut Ctrl+F4
    connect(Act_file_close, SIGNAL(triggered()), this, SLOT(closeThis()));

    // add to menu list
    file->addAction(Act_file_new);
    file->addSeparator();        // separator
    file->addAction(Act_file_close);

    // set status bar
    ui->menubar->addAction(Act_file_new);

    // set tips
    Act_file_new->setStatusTip(tr("New Capture Server Process"));
    Act_file_close->setStatusTip(tr("Close"));

    // 1.2 view menu
    QMenu * list = new QMenu("List View", this);
    QAction * listDetails = new QAction("Details", this);
    connect(listDetails, SIGNAL(triggered()), this, SLOT(listDetails()));
    QAction * listIcon    = new QAction("Icon", this);
    connect(listDetails, SIGNAL(triggered()), this, SLOT(listIcons()));

    list->addAction(listDetails);
    list->addAction(listIcon);
    view->addMenu(list);

    // 1.3 about menu
    QAction * updatesThis = new QAction("Updates", this);
    QAction * helpThis    = new QAction("Help", this);
    about->addAction(updatesThis);
    about->addSeparator();        // separator
    about->addAction(helpThis);

    /* 2. setup toolbar menu */
    QString appDirPath = QCoreApplication::applicationDirPath();
    QPushButton *buttonMonitor = new QPushButton("Monitor");
    buttonMonitor->setIcon(QIcon(appDirPath + "/../images/monitor.png"));
    buttonMonitor->setIconSize(QSize(13, 13));

    QPushButton *buttonWindow = new QPushButton("Window");
    buttonWindow->setIcon(QIcon(appDirPath + "/../images/window.png"));
    buttonWindow->setIconSize(QSize(15, 15));

    QPushButton *buttonPartial = new QPushButton("Partial");
    buttonPartial->setIcon(QIcon(appDirPath + "/../images/monitor.png"));
    buttonPartial->setIconSize(QSize(15, 15));

    connect(buttonMonitor, SIGNAL(clicked()), this, SLOT(startMonitorCapture()));
    connect(buttonWindow, SIGNAL(clicked()), this, SLOT(startWindowCapture()));
    connect(buttonPartial, SIGNAL(clicked()), this, SLOT(startPartialCapture()));

    /* right alignment for the tool bar */
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->toolBar->addWidget(spacer);

    ui->toolBar->addWidget(buttonMonitor);
    ui->toolBar->addWidget(buttonWindow);
    ui->toolBar->addWidget(buttonPartial);

    /* set not movable */
    ui->toolBar->setMovable(false);
}

void ServerMainWindow::initImage()
{
    /* set white background */
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();
}

void ServerMainWindow::initStartedServer()
{
    /* set white background */
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::lightGray);

    /* main layout */
    QGridLayout *mainLayout = new QGridLayout;
    MainServerWindowWidget * lw = new MainServerWindowWidget;

    mainLayout->addWidget(lw);
    ui->MainWidget->setLayout(mainLayout);
    this->show();
}

void ServerMainWindow::startMonitorCapture()
{

}

void ServerMainWindow::startWindowCapture()
{

}

void ServerMainWindow::startPartialCapture()
{

}

void ServerMainWindow::newServer()
{

}

void ServerMainWindow::closeThis()
{
    close();
}

void ServerMainWindow::listDetails()
{

}

void ServerMainWindow::listIcons()
{

}

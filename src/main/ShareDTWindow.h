#ifndef SHAREDT_WINDOW_H
#define SHAREDT_WINDOW_H

#include "StringTools.h"
#include "Layout.h"
#include "SDThread.h"

#include <utility>
#include <vector>
#include <chrono>

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include <QScrollArea>
#include <QGroupBox>

QT_BEGIN_NAMESPACE

/*
 *  GUI Layout:
 *  =======================================
 *  | QGROUP_BOX(_localGroupBox)          |
 *  |  __                                 |
 *  | |  |(newImageBox())......           |
 *  |  --                                 |
 *  |  __    __                           |
 *  | |  |  |  | ......                   |
 *  |  --    --                           |
 *  |                                     |
 *  |                                     |
 *  | QGROUP_BOX(remoteGroupBox)          |
 *  | ......                              |
 *  |                                     |
 *  =======================================
 */
class UI_ShareDTWindow;
class GroupBox : public QGroupBox {
public:
    explicit GroupBox(const QString & s, UI_ShareDTWindow * ui) : QGroupBox(s), _ui(ui) { }
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    UI_ShareDTWindow * _ui;
};

typedef struct qgroup_item {
    GroupBox   *  item;
    FlowLayout * layout;
    String name;
} QGROUP_BOX;

typedef struct {
    String name;
    QStringList argument;
} ItemInfo;

class ImageItem : public QWidget {
    Q_OBJECT
public:
    explicit ImageItem(ItemInfo info) : _info(std::move(info)), QWidget() { }

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;

private:
    ItemInfo _info;
};

class UI_ShareDTWindow : public QObject
{
    Q_OBJECT

    class FreshMainWindow : public SDThread {
    public:
        explicit FreshMainWindow(UI_ShareDTWindow * ui) : _ui(ui) , _autoFresh(false) { }
        ~FreshMainWindow() override { _ui = nullptr; }
        void run() override;

    private:
        std::atomic<bool> _autoFresh;
        UI_ShareDTWindow * _ui;
    };

public:
    UI_ShareDTWindow() : _w_unit(140), _h_unit(110), _freshMainWin(this) {
        QObject::connect (this, SIGNAL(refreshSignal()),
                          this, SLOT(refreshSlot()));
    }
    ~UI_ShareDTWindow();
    void newRemoteGroupBox(); // TODO for remote groupbox
    QWidget * newImageBox(int w, int h, unsigned char * data, const ItemInfo & info) const;
    void setupMainWindow(QWidget * w);
    void setupUi(QWidget * w);
    void setLocalWindows(QWidget * w);

    void refreshLocalBoxGroup();

private:
    void refreshLocalBoxGroupInternal() const;
    static void removeImageBox(QWidget * w);

    int _w_unit;
    int _h_unit;

    FreshMainWindow _freshMainWin;

    QVBoxLayout * _mainLayout;
    QGROUP_BOX    _localGroupBox;
    std::vector<QGroupBox *> _remoteGroupBoxes;

signals:
    void refreshSignal();

public slots:
    void refreshSlot() { refreshLocalBoxGroup(); }
};

namespace Ui {
    class ShareDTWindow: public UI_ShareDTWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif //SHAREDT_WINDOW_H

#ifndef SHAREDT_WINDOW_H
#define SHAREDT_WINDOW_H

#include "StringTools.h"
#include "Layout.h"
#include <utility>
#include <vector>

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

typedef struct qgroup_item {
    QGroupBox *  item;
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

class UI_ShareDTWindow
{
public:
    UI_ShareDTWindow() : _w_unit(140), _h_unit(110) { }
    void newRemoteGroupBox();
    QWidget * newImageBox(int w, int h, unsigned char * data, const ItemInfo & info) const;
    void setupMainWindow(QWidget * w);
    void setupUi(QWidget * w);
    void setLocalWindows(QWidget * w);
    void setMenu(QWidget * w);

    void refreshLocalBoxGroup(QWidget * w);

private:
    void refreshLocalBoxGroupInternal() const;
    static void removeImageBox(QWidget * w);

    int _w_unit;
    int _h_unit;

    QAction * _freshWin;  /* Window -> Fresh Items */

    std::vector<QLabel *> images;
    QMenuBar * _menubar;
    QMenu * _menuEdit;
    QMenu * _menuWindow;
    QMenu * _menuHelp;

    QVBoxLayout * _mainLayout;
    QGROUP_BOX    _localGroupBox;
    std::vector<QGroupBox *> _remoteGroupBoxes;

};

namespace Ui {
    class ShareDTWindow: public UI_ShareDTWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif //SHAREDT_WINDOW_H

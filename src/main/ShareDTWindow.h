#ifndef SHAREDT_WINDOW_H
#define SHAREDT_WINDOW_H

#include "StringTools.h"
#include "Layout.h"
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

#define MAIN_WIN_W 800
#define MAIN_WIN_H 600

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

class UI_ShareDTWindow
{
private:
    int _w_unit;
    int _h_unit;
    QAction *actionnew;
    QAction * _freshWin;  /* Window -> Fresh Items */
    QAction *actionAdjust_to_original_size;
    QAction *actionShow_help;
    QWidget *imagesLayout;
    FlowLayout *horizontalLayout;
    FlowLayout *flTittle;
    std::vector<QLabel *> images;

    QMenuBar * _menubar;
    QMenu * _menuEdit;
    QMenu * _menuWindow;
    QMenu * _menuHelp;

    QVBoxLayout * _mainLayout;
    QGROUP_BOX    _localGroupBox;
    std::vector<QGroupBox *> _remoteGroupBoxes;

public:
    UI_ShareDTWindow() : _w_unit(140), _h_unit(110) { }
    void newGroupBox();
    QWidget * newImageBox(int w, int h, unsigned char * data, const String & name) const;
    void setupMainWindow(QWidget * w);
    void setupUi(QWidget * w);
    void setLocalWindows(QWidget * w);
    void setMenu(QWidget * w);
/*
    void retranslateUi(QWidget *w) const
    {
        w->setWindowTitle(QCoreApplication::translate("ShareDTWindow", "ShareDTWindow", nullptr));
        actionnew->setText(QCoreApplication::translate("ShareDTWindow", "new", nullptr));
        actionFix_to_ratio_width_height->setText(QCoreApplication::translate("ShareDTWindow", "Fixed width and height ratio", nullptr));
        actionAdjust_to_original_size->setText(QCoreApplication::translate("ShareDTWindow", "Adjust to original size", nullptr));
        actionShow_help->setText(QCoreApplication::translate("ShareDTWindow", "Show Help", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("ShareDTWindow", "Edit", nullptr));
        menuWindow->setTitle(QCoreApplication::translate("ShareDTWindow", "Window", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("ShareDTWindow", "Help", nullptr));
    } // retranslateUi
*/

public slots:
    void actionFreshItems();
};

namespace Ui {
    class ShareDTWindow: public UI_ShareDTWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif //SHAREDT_WINDOW_H

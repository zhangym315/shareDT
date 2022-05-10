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
    QAction *actionnew;
    QAction *actionFix_to_ratio_width_height;
    QAction *actionAdjust_to_original_size;
    QAction *actionShow_help;
    QWidget *imagesLayout;
    FlowLayout *horizontalLayout;
    FlowLayout *flTittle;
    std::vector<QLabel *> images;

    QMenuBar *menubar;
    QMenu *menuEdit;
    QMenu *menuWindow;
    QMenu *menuHelp;

    QVBoxLayout * _mainLayout;
    QGROUP_BOX    _localGroupBox;
    std::vector<QGroupBox *> _remoteGroupBoxes;

public:
    void newGroupBox()
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

    QWidget * newImageBox()
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

    void setupMainWindow(QWidget *ShareDTWindow)
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

    void setupUi(QWidget *ShareDTWindow)
    {
        return setupMainWindow(ShareDTWindow);
    } // setupUi

    void retranslateUi(QWidget *ShareDTWindow) const
    {
        ShareDTWindow->setWindowTitle(QCoreApplication::translate("ShareDTWindow", "ShareDTWindow", nullptr));
        actionnew->setText(QCoreApplication::translate("ShareDTWindow", "new", nullptr));
        actionFix_to_ratio_width_height->setText(QCoreApplication::translate("ShareDTWindow", "Fixed width and height ratio", nullptr));
        actionAdjust_to_original_size->setText(QCoreApplication::translate("ShareDTWindow", "Adjust to original size", nullptr));
        actionShow_help->setText(QCoreApplication::translate("ShareDTWindow", "Show Help", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("ShareDTWindow", "Edit", nullptr));
        menuWindow->setTitle(QCoreApplication::translate("ShareDTWindow", "Window", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("ShareDTWindow", "Help", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ShareDTWindow: public UI_ShareDTWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif //SHAREDT_WINDOW_H

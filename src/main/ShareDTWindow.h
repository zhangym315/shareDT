#ifndef SHAREDT_WINDOW_H
#define SHAREDT_WINDOW_H

#include "Layout.h"

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

QT_BEGIN_NAMESPACE

#define MAIN_WIN_W 800
#define MAIN_WIN_H 600

class UI_ShareDTWindow
{
public:
    QAction *actionnew;
    QAction *actionFix_to_ratio_width_height;
    QAction *actionAdjust_to_original_size;
    QAction *actionShow_help;
    QWidget *imagesLayout;
    FlowLayout *horizontalLayout;
    QLabel *imageLabel;
    QLabel *imageLabel1;
    QLabel *imageLabel2;
    QLabel *imageLabel3;
    QMenuBar *menubar;
    QMenu *menuEdit;
    QMenu *menuWindow;
    QMenu *menuHelp;

    void setupUi(QWidget *ShareDTWindow)
    {
        if (ShareDTWindow->objectName().isEmpty())
            ShareDTWindow->setObjectName(QString::fromUtf8("ShareDTWindow"));
        ShareDTWindow->resize(MAIN_WIN_W, MAIN_WIN_H);
        actionnew = new QAction(ShareDTWindow);
        actionnew->setObjectName(QString::fromUtf8("actionnew"));
        actionFix_to_ratio_width_height = new QAction(ShareDTWindow);
        actionFix_to_ratio_width_height->setObjectName(QString::fromUtf8("actionFix_to_ratio_width_height"));
        actionAdjust_to_original_size = new QAction(ShareDTWindow);
        actionAdjust_to_original_size->setObjectName(QString::fromUtf8("actionAdjust_to_original_size"));
        actionShow_help = new QAction(ShareDTWindow);
        actionShow_help->setObjectName(QString::fromUtf8("actionShow_help"));

        imagesLayout = new QWidget(ShareDTWindow);
        imagesLayout->setObjectName(QString::fromUtf8("imagesLayout"));
        imagesLayout->setGeometry(QRect(0, 0, 400, 300));
        horizontalLayout = new FlowLayout(ShareDTWindow);
        horizontalLayout->setObjectName(QString::fromUtf8("FlowLayout"));
        imageLabel = new QLabel(imagesLayout);
        imageLabel->setObjectName(QString::fromUtf8("imageLabel"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(imageLabel->sizePolicy().hasHeightForWidth());
        imageLabel->setSizePolicy(sizePolicy);
        imageLabel->setFrameShape(QFrame::Box);
        imageLabel->setFrameShadow(QFrame::Plain);
        imageLabel->setLineWidth(1);

        imageLabel1 = new QLabel(imagesLayout);
        imageLabel1->setObjectName(QString::fromUtf8("imageLabel"));
        imageLabel1->setSizePolicy(sizePolicy);
        imageLabel1->setFrameShape(QFrame::Box);
        imageLabel1->setFrameShadow(QFrame::Plain);
        imageLabel1->setLineWidth(1);

        imageLabel2 = new QLabel(imagesLayout);
        imageLabel2->setObjectName(QString::fromUtf8("imageLabel"));
        imageLabel2->setSizePolicy(sizePolicy);
        imageLabel2->setFrameShape(QFrame::Box);
        imageLabel2->setFrameShadow(QFrame::Plain);
        imageLabel2->setLineWidth(1);

        imageLabel3 = new QLabel(imagesLayout);
        imageLabel3->setObjectName(QString::fromUtf8("imageLabel"));
        imageLabel3->setSizePolicy(sizePolicy);
        imageLabel3->setFrameShape(QFrame::Box);
        imageLabel3->setFrameShadow(QFrame::Plain);
        imageLabel3->setLineWidth(3);

        horizontalLayout->addWidget(imageLabel);
        horizontalLayout->addWidget(imageLabel1);
        horizontalLayout->addWidget(imageLabel2);
        horizontalLayout->addWidget(imageLabel3);

        menubar = new QMenuBar(ShareDTWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 24));
        menuEdit = new QMenu(menubar);
        menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
        menuWindow = new QMenu(menubar);
        menuWindow->setObjectName(QString::fromUtf8("menuWindow"));
        menuHelp = new QMenu(menubar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));

        menubar->addAction(menuEdit->menuAction());
        menubar->addAction(menuWindow->menuAction());
        menubar->addAction(menuHelp->menuAction());
        menuWindow->addAction(actionFix_to_ratio_width_height);
        menuWindow->addAction(actionAdjust_to_original_size);
        menuHelp->addAction(actionShow_help);

        retranslateUi(ShareDTWindow);

        QMetaObject::connectSlotsByName(ShareDTWindow);
    } // setupUi

    void retranslateUi(QWidget *ShareDTWindow) const
    {
        ShareDTWindow->setWindowTitle(QCoreApplication::translate("ShareDTWindow", "ShareDTWindow", nullptr));
        actionnew->setText(QCoreApplication::translate("ShareDTWindow", "new", nullptr));
        actionFix_to_ratio_width_height->setText(QCoreApplication::translate("ShareDTWindow", "Fixed width and height ratio", nullptr));
        actionAdjust_to_original_size->setText(QCoreApplication::translate("ShareDTWindow", "Adjust to original size", nullptr));
        actionShow_help->setText(QCoreApplication::translate("ShareDTWindow", "Show Help", nullptr));
        imageLabel->setText(QCoreApplication::translate("ShareDTWindow", "TextLabel", nullptr));
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

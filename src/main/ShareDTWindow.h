#ifndef SHAREDT_WINDOW_H
#define SHAREDT_WINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

#define MAIN_WIN_W 800
#define MAIN_WIN_H 600

class Ui_ShareDTWindow
{
public:
    QAction *actionnew;
    QAction *actionFix_to_ratio_width_height;
    QAction *actionAdjust_to_original_size;
    QAction *actionShow_help;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *imageLabel;
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
        horizontalLayoutWidget = new QWidget(ShareDTWindow);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(0, 0, 800, 600));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        imageLabel = new QLabel(horizontalLayoutWidget);
        imageLabel->setObjectName(QString::fromUtf8("imageLabel"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(imageLabel->sizePolicy().hasHeightForWidth());
        imageLabel->setSizePolicy(sizePolicy);
        imageLabel->setFrameShape(QFrame::Box);
        imageLabel->setFrameShadow(QFrame::Plain);
        imageLabel->setLineWidth(3);

        horizontalLayout->addWidget(imageLabel);

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

    void retranslateUi(QWidget *ShareDTWindow)
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
    class ShareDTWindow: public Ui_ShareDTWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif //SHAREDT_WINDOW_H

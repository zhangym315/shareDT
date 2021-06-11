#include <QApplication>
#include <QMessageBox>

#include "ShareDTClientWin.h"

int
main(int argc, char **argv)
{
    QApplication app(argc, argv);

    ShareDTClientWin gui(argc, argv);

    if (!gui.isInited()) {
        QMessageBox msgBox;
        msgBox.setText("Failed to connect to vnc server!");
        msgBox.exec();
        return -1;
    }

    gui.show();
    gui.startVNC();

    return QApplication::exec();
}


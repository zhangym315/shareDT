#include <QApplication>

#include "ShareDTClientWin.h"

static void showUages(char **argv)
{
    std::cerr << "Usage:" << std::endl;
    std::cerr << "    " << argv[0] << " [options] <serveraddr:vncport>" << std::endl << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "    -encodings <mpeg2_422, x265_420, x265_422, x265_444, zlib, raw>" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Example: " << std::endl;
    std::cerr << "    " << argv[0] << " -encodings mpeg2_422 192.168.56.110:0" << std::endl;
    std::cerr << std::endl;
}

int
main(int argc, char **argv)
{
    QApplication app(argc, argv);
    ShareDTClientWin gui(argc, argv);

    if (!gui.isInited()) {
        std::cerr << "Failed to start connect to server" << std::endl;
        std::cerr << std::endl;
        showUages(argv);
        return -1;
    }

    gui.show();
    gui.startVNC();

    return QApplication::exec();
}


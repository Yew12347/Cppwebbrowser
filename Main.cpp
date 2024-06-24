#include <QApplication>
#include "browserwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    BrowserWindow window;
    window.resize(1024, 768);
    window.show();

    return app.exec();
}

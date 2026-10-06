#include <QApplication>
#include "ui/qt/MainWindow.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    sysmon::ui::qt::MainWindow window;
    window.show();

    return app.exec();
}
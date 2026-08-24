#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <iostream>
#include "MainWindow.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("EPFD-RAS");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("Hoang-Khiem-Triet Team");

    // Load QSS Stylesheet
    QFile styleFile("src/gui/styles.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream ts(&styleFile);
        app.setStyleSheet(ts.readAll());
        styleFile.close();
    } else {
        std::cout << "[GUI Notice] styles.qss not found in current directory; using fallback default dark styling.\n";
        app.setStyleSheet(
            "QMainWindow, QDialog, QWidget { background-color: #0d1117; color: #c9d1d9; font-family: sans-serif; }"
            "QTableWidget { background-color: #0d1117; color: #c9d1d9; gridline-color: #21262d; }"
            "QHeaderView::section { background-color: #161b22; color: #8b949e; }"
            "QPushButton { background-color: #21262d; color: #c9d1d9; border: 1px solid #30363d; border-radius: 4px; padding: 6px 12px; }"
            "QPushButton:hover { background-color: #30363d; color: white; }"
            "QTabWidget::pane { border: 1px solid #30363d; background-color: #0d1117; }"
            "QTabBar::tab { background-color: #161b22; color: #8b949e; padding: 8px 16px; border: 1px solid #30363d; }"
            "QTabBar::tab:selected { background-color: #21262d; color: #58a6ff; border-bottom: 2px solid #58a6ff; }"
            "QGroupBox { border: 1px solid #30363d; border-radius: 6px; margin-top: 10px; font-weight: bold; color: #58a6ff; }"
        );
    }

    epfd::gui::MainWindow window;
    window.show();

    return app.exec();
}

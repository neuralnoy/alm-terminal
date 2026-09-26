#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  QMainWindow mainWindow;
  mainWindow.setWindowTitle("ALM Terminal");
  mainWindow.resize(1024, 768);
  mainWindow.show();

  return app.exec();
}

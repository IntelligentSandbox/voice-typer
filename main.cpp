#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Voice Typer");
    window.resize(300, 200);

    QLabel *label = new QLabel("Hello Qt!", &window);
    label->setGeometry(10, 10, 100, 25);

    window.show();
    return app.exec();
}

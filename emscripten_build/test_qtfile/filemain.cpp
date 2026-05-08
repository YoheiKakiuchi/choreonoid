#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QDebug>

// A simple widget with a button to open a file dialog.
class Window : public QWidget {
public:
    Window(QWidget *parent = nullptr) : QWidget(parent) {
        // Create a button and set its text
        QPushButton *button = new QPushButton("Open File", this);

        // Create a layout
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(button);
        setLayout(layout);

        // Connect the button's clicked signal to our openFileDialog slot
        connect(button, &QPushButton::clicked, this, &Window::openFileDialog);

        setWindowTitle("Qt File Dialog Example");
    }

private slots:
    void openFileDialog() {
        // The core of the file dialog logic
        QString fileName = QFileDialog::getOpenFileName(
            this,                                     // Parent widget
            tr("Open File"),                          // Dialog title
            QDir::homePath(),                         // Starting directory
            tr("Text Files (*.txt);;All Files (*.*)")  // File filters
        );

        // Check if the user selected a file or cancelled the dialog
        if (!fileName.isEmpty()) {
            qDebug() << "Selected file:" << fileName;
        } else {
            qDebug() << "No file selected.";
        }
    }
};

int main(int argc, char *argv[]) {
    // Create the application object
    QApplication app(argc, argv);

    // Create and show our main window
    Window window;
    window.resize(300, 100);
    window.show();

    // Start the application's event loop
    return app.exec();
}

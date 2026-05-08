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
        QPushButton *btn_open = new QPushButton("Open File", this);
        QPushButton *btn_save = new QPushButton("Save File", this);
        // Create a layout
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(btn_open);
        layout->addWidget(btn_save);
        setLayout(layout);

        // Connect the button's clicked signal to our openFileDialog slot
        connect(btn_open, &QPushButton::clicked, this, &Window::openFileDialog);
        connect(btn_save, &QPushButton::clicked, this, &Window::saveFileDialog);

        setWindowTitle("Qt File Dialog Example");
    }

private slots:
    void openFileDialog() {
#if 0
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
#endif
        auto fileContentReady = [](const QString &fileName, const QByteArray &fileContent) {
            if (fileName.isEmpty()) {
                // No file was selected
                qDebug() << "No file selected.";
            } else {
                // Use fileName and fileContent
                qDebug() << "Selected file:" << fileName;
                qDebug() << "         size:" << fileContent.size();
                qDebug() << "      content:" << QString::fromUtf8(fileContent);
            }
        };
        QFileDialog::getOpenFileContent("Images (*.png *.xpm *.jpg);;Text files (*.txt);;All files (*)",  fileContentReady);
    }
    void saveFileDialog() {
        QByteArray imageData; // obtained from e.g. QImage::save()
        QFileDialog::saveFileContent(imageData, "myimage.png");
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

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QString>

// WebAssembly ビルド時のみ Emscripten のヘッダーを読み込む
#ifdef Q_OS_WASM
#include <emscripten.h>
#endif

class MainWindow : public QWidget {
public:
    MainWindow(QWidget *parent = nullptr) : QWidget(parent) {
        auto *layout = new QVBoxLayout(this);
        auto *label = new QLabel("Qt6 GUI Control Panel", this);
        layout->addWidget(label);

        auto *btnRed = new QPushButton("Red Cube", this);
        auto *btnGreen = new QPushButton("Green Cube", this);
        auto *btnBlue = new QPushButton("Blue Cube", this);
        auto *btnRotate = new QPushButton("Toggle Rotation", this);

        layout->addWidget(btnRed);
        layout->addWidget(btnGreen);
        layout->addWidget(btnBlue);
        layout->addWidget(btnRotate);

        // ボタンのクリックイベントと JavaScript 関数のバインド
        connect(btnRed, &QPushButton::clicked,[]() {
#ifdef Q_OS_WASM
            emscripten_run_script("window.changeCubeColor(0xff0000);");
#endif
        });
        connect(btnGreen, &QPushButton::clicked,[]() {
#ifdef Q_OS_WASM
            emscripten_run_script("window.changeCubeColor(0x00ff00);");
#endif
        });
        connect(btnBlue, &QPushButton::clicked,[]() {
#ifdef Q_OS_WASM
            emscripten_run_script("window.changeCubeColor(0x0000ff);");
#endif
        });
        connect(btnRotate, &QPushButton::clicked,[]() {
#ifdef Q_OS_WASM
            emscripten_run_script("window.toggleRotation();");
#endif
        });
        // ウィンドウサイズの調整
        setFixedSize(300, 400);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}

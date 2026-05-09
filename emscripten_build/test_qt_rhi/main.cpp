#include <QApplication>
#include <QRhiWidget>
#include <rhi/qrhi.h>
#include <QFile>
#include <memory>

class TriangleWidget : public QRhiWidget {
public:
    TriangleWidget(QWidget *parent = nullptr) : QRhiWidget(parent) {}

protected:
    // 初期化 (バッファやパイプラインの構築)
    void initialize(QRhiCommandBuffer *cb) override {
        // 既に初期化済みならスキップ
        if (m_pipeline) return;

        QRhi *rhi = this->rhi();

        // 1. 頂点データの用意 (X, Y, R, G, B)
        float vertexData[] = {
             0.0f, -0.5f,  1.0f, 0.0f, 0.0f, // 上 (赤)
            -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // 左下 (緑)
             0.5f,  0.5f,  0.0f, 0.0f, 1.0f  // 右下 (青)
        };

        // 頂点バッファの作成
        m_vbuf.reset(rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(vertexData)));
        m_vbuf->create();

        // GPUへデータをアップロード
        QRhiResourceUpdateBatch *rub = rhi->nextResourceUpdateBatch();
        rub->uploadStaticBuffer(m_vbuf.get(), vertexData);

        // 2. シェーダーの読み込み (CMakeでビルドされた .qsb ファイル)
        QShader vs = loadShader(":/color.vert.qsb");
        QShader fs = loadShader(":/color.frag.qsb");

        // 3. グラフィックスパイプラインの構築
        m_srb.reset(rhi->newShaderResourceBindings());
        m_srb->create();

        m_pipeline.reset(rhi->newGraphicsPipeline());
        m_pipeline->setShaderStages({
            { QRhiShaderStage::Vertex, vs },
            { QRhiShaderStage::Fragment, fs }
        });

        // 頂点のメモリレイアウト設定
        QRhiVertexInputLayout inputLayout;
        inputLayout.setBindings({ { 5 * sizeof(float) } }); // 1頂点あたりのバイト数
        inputLayout.setAttributes({
            // location 0: vec2 position (オフセット0)
            { 0, 0, QRhiVertexInputAttribute::Float2, 0 },
            // location 1: vec3 color (オフセット8)
            { 0, 1, QRhiVertexInputAttribute::Float3, 2 * sizeof(float) }
        });

        m_pipeline->setVertexInputLayout(inputLayout);
        m_pipeline->setShaderResourceBindings(m_srb.get());
        m_pipeline->setRenderPassDescriptor(renderTarget()->renderPassDescriptor());
        m_pipeline->create();

        // アップロードバッチをコマンドバッファに積む
        cb->resourceUpdate(rub);
    }

    // 描画ループ
    void render(QRhiCommandBuffer *cb) override {
        // 背景色をクリアして描画パスを開始
        QColor clearColor = QColor::fromRgbF(0.2, 0.2, 0.2, 1.0);
        cb->beginPass(renderTarget(), clearColor, { 1.0f, 0 });

        // パイプラインをセット
        cb->setGraphicsPipeline(m_pipeline.get());

        // ビューポートをウィジェットのピクセルサイズに合わせる
        QSize outputSize = renderTarget()->pixelSize();
        cb->setViewport(QRhiViewport(0, 0, outputSize.width(), outputSize.height()));

        // 頂点バッファをバインドして描画 (3頂点)
        QRhiCommandBuffer::VertexInput vbufBinding(m_vbuf.get(), 0);
        cb->setVertexInput(0, 1, &vbufBinding);
        cb->draw(3);

        // 描画パスの終了
        cb->endPass();
    }

private:
    QShader loadShader(const QString &name) {
        QFile f(name);
        if (f.open(QIODevice::ReadOnly))
            return QShader::fromSerialized(f.readAll());
        qWarning("Failed to load shader: %s", qPrintable(name));
        return QShader();
    }

    std::unique_ptr<QRhiBuffer> m_vbuf;
    std::unique_ptr<QRhiShaderResourceBindings> m_srb;
    std::unique_ptr<QRhiGraphicsPipeline> m_pipeline;
};

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    //
    TriangleWidget w;
    w.resize(800, 600);
    w.show();
    //
    return app.exec();
}

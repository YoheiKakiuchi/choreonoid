#include "NullSceneRenderer.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class NullSceneRenderer::Impl
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    PolymorphicSceneNodeFunctionSet renderingFunctions;
    Affine3 currentModelTransform;
    Matrix4 projectionMatrix;
    Matrix4 viewProjectionMatrix;
    Vector3 pickedPoint;
    SgNodePath pickedNodePath;
    SgMaterialPtr defaultMaterial;
    string glVersionString;
    string glVendorString;
    string glRendererString;
    LightingMode lightingMode;
    int backFaceCullingMode;
    double defaultPointSize;
    double defaultLineWidth;
    bool isTextureEnabled;
    bool isSmoothShadingEnabled;
    bool isAmbientNormalizationEnabled;
    bool isNormalVisualizationEnabled;
    double normalVisualizationLength;

    Impl()
        : lightingMode(NormalLighting),
          backFaceCullingMode(EnableBackFaceCulling),
          defaultPointSize(1.0),
          defaultLineWidth(1.0),
          isTextureEnabled(true),
          isSmoothShadingEnabled(true),
          isAmbientNormalizationEnabled(false),
          isNormalVisualizationEnabled(false),
          normalVisualizationLength(0.0)
    {
        currentModelTransform.setIdentity();
        projectionMatrix.setIdentity();
        viewProjectionMatrix.setIdentity();
        pickedPoint.setZero();
        defaultMaterial = new SgMaterial;
        glVersionString = "No OpenGL";
        glVendorString = "No OpenGL";
        glRendererString = "NullSceneRenderer";
    }
};

}


NullSceneRenderer::NullSceneRenderer(SgGroup* root)
    : GLSceneRenderer(root)
{
    impl = new Impl;
}


NullSceneRenderer::~NullSceneRenderer()
{
    delete impl;
}


PolymorphicSceneNodeFunctionSet* NullSceneRenderer::renderingFunctions()
{
    return &impl->renderingFunctions;
}


void NullSceneRenderer::renderCustomGroup(SgGroup* /* group */, const std::function<void()>& /* traverseFunction */)
{

}


void NullSceneRenderer::renderCustomTransform(SgTransform* /* transform */, const std::function<void()>& /* traverseFunction */)
{

}


void NullSceneRenderer::renderNode(SgNode* /* node */)
{

}


void NullSceneRenderer::addNodeDecoration(SgNode* /* targetNode */, NodeDecorationFunction /* func */, int /* id */)
{

}


void NullSceneRenderer::clearNodeDecorations(int /* id */)
{

}


const Affine3& NullSceneRenderer::currentModelTransform() const
{
    return impl->currentModelTransform;
}


const Matrix4& NullSceneRenderer::projectionMatrix() const
{
    return impl->projectionMatrix;
}


const Matrix4& NullSceneRenderer::viewProjectionMatrix() const
{
    return impl->viewProjectionMatrix;
}


Vector3 NullSceneRenderer::project(const Vector3& p) const
{
    return p;
}


double NullSceneRenderer::projectedPixelSizeRatio(const Vector3& /* position */) const
{
    return 1.0;
}


bool NullSceneRenderer::initializeGL(GLADloadfunc /* getProcAddress */)
{
    return true;
}


void NullSceneRenderer::flushGL()
{

}


const std::string& NullSceneRenderer::glVersionString() const
{
    return impl->glVersionString;
}


const std::string& NullSceneRenderer::glVendorString() const
{
    return impl->glVendorString;
}


const std::string& NullSceneRenderer::glRendererString() const
{
    return impl->glRendererString;
}


void NullSceneRenderer::setViewport(int x, int y, int width, int height)
{
    GLSceneRenderer::updateViewportInformation(x, y, width, height);
}


void NullSceneRenderer::updateViewportInformation()
{
    auto& vp = viewport();
    if(vp.w > 0 && vp.h > 0){
        setAspectRatio(static_cast<float>(vp.w) / static_cast<float>(vp.h));
    }
}


const Vector3& NullSceneRenderer::pickedPoint() const
{
    return impl->pickedPoint;
}


const SgNodePath& NullSceneRenderer::pickedNodePath() const
{
    return impl->pickedNodePath;
}


void NullSceneRenderer::setLightingMode(LightingMode mode)
{
    impl->lightingMode = mode;
}


GLSceneRenderer::LightingMode NullSceneRenderer::lightingMode() const
{
    return impl->lightingMode;
}


void NullSceneRenderer::setDefaultSmoothShading(bool on)
{
    impl->isSmoothShadingEnabled = on;
}


SgMaterial* NullSceneRenderer::defaultMaterial()
{
    return impl->defaultMaterial;
}


void NullSceneRenderer::enableTexture(bool on)
{
    impl->isTextureEnabled = on;
}


void NullSceneRenderer::setMaterialAmbientNormalizationEnabled(bool on)
{
    impl->isAmbientNormalizationEnabled = on;
}


void NullSceneRenderer::setDefaultPointSize(double size)
{
    impl->defaultPointSize = size;
}


void NullSceneRenderer::setDefaultLineWidth(double width)
{
    impl->defaultLineWidth = width;
}


void NullSceneRenderer::setNormalVisualizationEnabled(bool on)
{
    impl->isNormalVisualizationEnabled = on;
}


void NullSceneRenderer::setNormalVisualizationLength(double length)
{
    impl->normalVisualizationLength = length;
}


void NullSceneRenderer::requestToClearResources()
{

}


void NullSceneRenderer::enableUnusedResourceCheck(bool /* on */)
{

}


void NullSceneRenderer::setColor(const Vector3f& color)
{
    setDefaultColor(color);
}


void NullSceneRenderer::setBackFaceCullingMode(int mode)
{
    impl->backFaceCullingMode = mode;
}


int NullSceneRenderer::backFaceCullingMode() const
{
    return impl->backFaceCullingMode;
}


void NullSceneRenderer::doRender()
{
    extractPreprocessedNodes();
}


bool NullSceneRenderer::doPick(int /* x */, int /* y */)
{
    extractPreprocessedNodes();
    impl->pickedNodePath.clear();
    impl->pickedPoint.setZero();
    return false;
}
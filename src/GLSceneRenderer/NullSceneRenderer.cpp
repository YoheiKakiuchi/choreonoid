#include "NullSceneRenderer.h"

#include <iostream>

using namespace std;
using namespace cnoid;

namespace {

void traceMethod(const char* methodName)
{
    std::cout << methodName << std::endl;
}

}

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
    traceMethod("NullSceneRenderer::NullSceneRenderer");
    impl = new Impl;
}


NullSceneRenderer::~NullSceneRenderer()
{
    traceMethod("NullSceneRenderer::~NullSceneRenderer");
    delete impl;
}


PolymorphicSceneNodeFunctionSet* NullSceneRenderer::renderingFunctions()
{
    traceMethod("NullSceneRenderer::renderingFunctions");
    return &impl->renderingFunctions;
}


void NullSceneRenderer::renderCustomGroup(SgGroup* /* group */, const std::function<void()>& /* traverseFunction */)
{
    traceMethod("NullSceneRenderer::renderCustomGroup");

}


void NullSceneRenderer::renderCustomTransform(SgTransform* /* transform */, const std::function<void()>& /* traverseFunction */)
{
    traceMethod("NullSceneRenderer::renderCustomTransform");

}


void NullSceneRenderer::renderNode(SgNode* /* node */)
{
    traceMethod("NullSceneRenderer::renderNode");

}


void NullSceneRenderer::addNodeDecoration(SgNode* /* targetNode */, NodeDecorationFunction /* func */, int /* id */)
{
    traceMethod("NullSceneRenderer::addNodeDecoration");

}


void NullSceneRenderer::clearNodeDecorations(int /* id */)
{
    traceMethod("NullSceneRenderer::clearNodeDecorations");

}


const Affine3& NullSceneRenderer::currentModelTransform() const
{
    traceMethod("NullSceneRenderer::currentModelTransform");
    return impl->currentModelTransform;
}


const Matrix4& NullSceneRenderer::projectionMatrix() const
{
    traceMethod("NullSceneRenderer::projectionMatrix");
    return impl->projectionMatrix;
}


const Matrix4& NullSceneRenderer::viewProjectionMatrix() const
{
    traceMethod("NullSceneRenderer::viewProjectionMatrix");
    return impl->viewProjectionMatrix;
}


Vector3 NullSceneRenderer::project(const Vector3& p) const
{
    traceMethod("NullSceneRenderer::project");
    return p;
}


double NullSceneRenderer::projectedPixelSizeRatio(const Vector3& /* position */) const
{
    traceMethod("NullSceneRenderer::projectedPixelSizeRatio");
    return 1.0;
}


bool NullSceneRenderer::initializeGL(GLADloadfunc /* getProcAddress */)
{
    traceMethod("NullSceneRenderer::initializeGL");
    return true;
}


void NullSceneRenderer::flushGL()
{
    traceMethod("NullSceneRenderer::flushGL");

}


const std::string& NullSceneRenderer::glVersionString() const
{
    traceMethod("NullSceneRenderer::glVersionString");
    return impl->glVersionString;
}


const std::string& NullSceneRenderer::glVendorString() const
{
    traceMethod("NullSceneRenderer::glVendorString");
    return impl->glVendorString;
}


const std::string& NullSceneRenderer::glRendererString() const
{
    traceMethod("NullSceneRenderer::glRendererString");
    return impl->glRendererString;
}


void NullSceneRenderer::setViewport(int x, int y, int width, int height)
{
    traceMethod("NullSceneRenderer::setViewport");
    GLSceneRenderer::updateViewportInformation(x, y, width, height);
}


void NullSceneRenderer::updateViewportInformation()
{
    traceMethod("NullSceneRenderer::updateViewportInformation");
    auto& vp = viewport();
    if(vp.w > 0 && vp.h > 0){
        setAspectRatio(static_cast<float>(vp.w) / static_cast<float>(vp.h));
    }
}


const Vector3& NullSceneRenderer::pickedPoint() const
{
    traceMethod("NullSceneRenderer::pickedPoint");
    return impl->pickedPoint;
}


const SgNodePath& NullSceneRenderer::pickedNodePath() const
{
    traceMethod("NullSceneRenderer::pickedNodePath");
    return impl->pickedNodePath;
}


void NullSceneRenderer::setLightingMode(LightingMode mode)
{
    traceMethod("NullSceneRenderer::setLightingMode");
    impl->lightingMode = mode;
}


GLSceneRenderer::LightingMode NullSceneRenderer::lightingMode() const
{
    traceMethod("NullSceneRenderer::lightingMode");
    return impl->lightingMode;
}


void NullSceneRenderer::setDefaultSmoothShading(bool on)
{
    traceMethod("NullSceneRenderer::setDefaultSmoothShading");
    impl->isSmoothShadingEnabled = on;
}


SgMaterial* NullSceneRenderer::defaultMaterial()
{
    traceMethod("NullSceneRenderer::defaultMaterial");
    return impl->defaultMaterial;
}


void NullSceneRenderer::enableTexture(bool on)
{
    traceMethod("NullSceneRenderer::enableTexture");
    impl->isTextureEnabled = on;
}


void NullSceneRenderer::setMaterialAmbientNormalizationEnabled(bool on)
{
    traceMethod("NullSceneRenderer::setMaterialAmbientNormalizationEnabled");
    impl->isAmbientNormalizationEnabled = on;
}


void NullSceneRenderer::setDefaultPointSize(double size)
{
    traceMethod("NullSceneRenderer::setDefaultPointSize");
    impl->defaultPointSize = size;
}


void NullSceneRenderer::setDefaultLineWidth(double width)
{
    traceMethod("NullSceneRenderer::setDefaultLineWidth");
    impl->defaultLineWidth = width;
}


void NullSceneRenderer::setNormalVisualizationEnabled(bool on)
{
    traceMethod("NullSceneRenderer::setNormalVisualizationEnabled");
    impl->isNormalVisualizationEnabled = on;
}


void NullSceneRenderer::setNormalVisualizationLength(double length)
{
    traceMethod("NullSceneRenderer::setNormalVisualizationLength");
    impl->normalVisualizationLength = length;
}


void NullSceneRenderer::requestToClearResources()
{
    traceMethod("NullSceneRenderer::requestToClearResources");

}


void NullSceneRenderer::enableUnusedResourceCheck(bool /* on */)
{
    traceMethod("NullSceneRenderer::enableUnusedResourceCheck");

}


void NullSceneRenderer::setColor(const Vector3f& color)
{
    traceMethod("NullSceneRenderer::setColor");
    setDefaultColor(color);
}


void NullSceneRenderer::setBackFaceCullingMode(int mode)
{
    traceMethod("NullSceneRenderer::setBackFaceCullingMode");
    impl->backFaceCullingMode = mode;
}


int NullSceneRenderer::backFaceCullingMode() const
{
    traceMethod("NullSceneRenderer::backFaceCullingMode");
    return impl->backFaceCullingMode;
}


void NullSceneRenderer::doRender()
{
    traceMethod("NullSceneRenderer::doRender");
    extractPreprocessedNodes();
}


bool NullSceneRenderer::doPick(int /* x */, int /* y */)
{
    traceMethod("NullSceneRenderer::doPick");
    extractPreprocessedNodes();
    impl->pickedNodePath.clear();
    impl->pickedPoint.setZero();
    return false;
}
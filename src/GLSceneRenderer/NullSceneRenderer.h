#ifndef CNOID_BASE_NULL_SCENE_RENDERER_H
#define CNOID_BASE_NULL_SCENE_RENDERER_H

#include <cnoid/GLSceneRenderer>
#include <cnoid/SceneGraph>
#include <cnoid/SceneDrawables>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT NullSceneRenderer : public GLSceneRenderer
{
public:
    NullSceneRenderer(SgGroup* root = nullptr);
    virtual ~NullSceneRenderer();

    virtual PolymorphicSceneNodeFunctionSet* renderingFunctions() override;
    virtual void renderCustomGroup(SgGroup* group, const std::function<void()>& traverseFunction) override;
    virtual void renderCustomTransform(SgTransform* transform, const std::function<void()>& traverseFunction) override;
    virtual void renderNode(SgNode* node) override;
    virtual void addNodeDecoration(SgNode* targetNode, NodeDecorationFunction func, int id) override;
    virtual void clearNodeDecorations(int id) override;
    virtual const Affine3& currentModelTransform() const override;
    virtual const Matrix4& projectionMatrix() const override;
    virtual const Matrix4& viewProjectionMatrix() const override;
    virtual Vector3 project(const Vector3& p) const override;
    virtual double projectedPixelSizeRatio(const Vector3& position) const override;
    virtual bool initializeGL(GLADloadfunc getProcAddress) override;
    virtual void flushGL() override;
    virtual const std::string& glVersionString() const override;
    virtual const std::string& glVendorString() const override;
    virtual const std::string& glRendererString() const override;
    virtual void setViewport(int x, int y, int width, int height) override;
    virtual void updateViewportInformation() override;
    virtual const Vector3& pickedPoint() const override;
    virtual const SgNodePath& pickedNodePath() const override;
    virtual void setLightingMode(LightingMode mode) override;
    virtual LightingMode lightingMode() const override;
    virtual void setDefaultSmoothShading(bool on) override;
    virtual SgMaterial* defaultMaterial() override;
    virtual void enableTexture(bool on) override;
    virtual void setMaterialAmbientNormalizationEnabled(bool on) override;
    virtual void setDefaultPointSize(double size) override;
    virtual void setDefaultLineWidth(double width) override;
    virtual void setNormalVisualizationEnabled(bool on) override;
    virtual void setNormalVisualizationLength(double length) override;
    virtual void requestToClearResources() override;
    virtual void enableUnusedResourceCheck(bool on) override;
    virtual void setColor(const Vector3f& color) override;
    virtual void setBackFaceCullingMode(int mode) override;
    virtual int backFaceCullingMode() const override;

protected:
    virtual void doRender() override;
    virtual bool doPick(int x, int y) override;

private:
    class Impl;
    Impl* impl;
};

}

#endif

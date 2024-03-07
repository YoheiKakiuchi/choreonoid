#include "SceneItem.h"
#include "ItemManager.h"
#include "GeneralSceneFileImporterBase.h"
#include "RenderableItemUtil.h"
#include "Archive.h"
#include "PutPropertyFunction.h"
#include <cnoid/CloneMap>
#include <cnoid/SceneLoader>
#include <cnoid/EigenArchive>
#include <cnoid/SceneEffects>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace {

static unique_ptr<RenderableItemUtil> renderableItemUtil;

class GeneralSceneFileImporter : public GeneralSceneFileImporterBase
{
public:
    GeneralSceneFileImporter()
    {
        setCaption(_("Scene"));
        setFileTypeCaption(_("Scene / Mesh"));

        addFormatAlias("AVAILABLE-SCENE-FILE");
        addFormatAlias("VRML-FILE");
        addFormatAlias("STL-FILE");
    }
    
    virtual Item* createItem() override
    {
        return new SceneItem;
    }
    
    virtual bool load(Item* item, const std::string& filename) override
    {
        SgNode* scene = loadScene(filename);
        if(!scene){
            return false;
        }
        auto sceneItem = static_cast<SceneItem*>(item);
        auto topNode = sceneItem->topNode();
        topNode->clearChildren();
        topNode->addChild(scene);
        if(sceneItem->isLightweightRenderingEnabled()){
            sceneItem->setLightweightRenderingEnabled(true);
        }
        return true;
    }
};

}


namespace cnoid {

class SceneItem::Location : public LocationProxy
{
public:
    SceneItem* item;

    Location(SceneItem* item);
    virtual std::string getName() const override;
    virtual Isometry3 getLocation() const override;
    virtual bool setLocation(const Isometry3& T) override;
    virtual SignalProxy<void()> sigLocationChanged() override;
};

}


void SceneItem::initializeClass(ExtensionManager* ext)
{
    static bool initialized = false;
    if(!initialized){
        auto im = &ext->itemManager();
        im->registerClass<SceneItem>(N_("SceneItem"));
        registerSceneItemFileIoSet(im);
        initialized = true;
    }
}


SceneItem::SceneItem()
{
    setAttributes(FileImmutable | Reloadable);
    topNode_ = new SgPosTransform;
    isLightweightRenderingEnabled_ = false;
}


SceneItem::SceneItem(const SceneItem& org, CloneMap* cloneMap)
    : Item(org)
{
    // shallow copy
    auto orgTopNode = org.topNode();
    topNode_ = new SgPosTransform(*orgTopNode);
    if(cloneMap){
        cloneMap->setClone(orgTopNode, topNode_);
    }
    isLightweightRenderingEnabled_ = org.isLightweightRenderingEnabled_;
}


SceneItem::~SceneItem()
{
    
}


Item* SceneItem::doCloneItem(CloneMap* cloneMap) const
{
    return new SceneItem(*this, cloneMap);
}


bool SceneItem::setName(const std::string& name)
{
    topNode_->setName(name);
    return Item::setName(name);
}


void SceneItem::setTranslation(const Vector3& translation)
{
    topNode_->setTranslation(translation);
    topNode_->notifyUpdate();
    notifyUpdate();
}


void SceneItem::setTranslation(const Vector3f& translation)
{
    topNode_->setTranslation(translation);
    topNode_->notifyUpdate();
    notifyUpdate();
}


void SceneItem::setRotation(const AngleAxis& rotation)
{
    topNode_->setRotation(rotation);
    topNode_->notifyUpdate();
    notifyUpdate();
}


void SceneItem::setRotation(const AngleAxisf& rotation)
{
    topNode_->setRotation(rotation);
    topNode_->notifyUpdate();
    notifyUpdate();
}


void SceneItem::setLightweightRenderingEnabled(bool on)
{
    if(on){
        if(!topNode_->findNodeOfType<SgLightweightRenderingGroup>(1)){
            auto lightweight = new SgLightweightRenderingGroup;
            topNode_->moveChildrenTo(lightweight);
            topNode_->addChild(lightweight, true);
        }
    } else {
        auto lightweight = topNode_->findNodeOfType<SgLightweightRenderingGroup>(1);
        if(lightweight){
            lightweight->moveChildrenTo(topNode_);
            topNode_->removeChild(lightweight, true);
        }
    }
    isLightweightRenderingEnabled_ = on;
}


void SceneItem::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("File"), fileName());

    putProperty(_("Translation"), str(Vector3(topNode_->translation())),
                [&](const std::string& value){
                    Vector3 p;
                    if(toVector3(value, p)){
                        topNode_->setTranslation(p);
                        topNode_->notifyUpdate();
                        return true;
                    }
                    return false;
                });

    Vector3 rpy(TO_DEGREE * rpyFromRot(topNode_->rotation()));
    
    putProperty(_("Rotation"), str(rpy),
                [&](const std::string& value){
                    Vector3 rpy;
                    if(toVector3(value, rpy)){
                        topNode_->setRotation(rotFromRpy(TO_RADIAN * rpy));
                        topNode_->notifyUpdate();
                        return true;
                    }
                    return false;
                });
    
    putProperty(_("Lightweight rendering"), isLightweightRenderingEnabled_,
                [&](bool on){ setLightweightRenderingEnabled(on); return true; });
}


bool SceneItem::store(Archive& archive)
{
    if(archive.writeFileInformation(this)){
        write(archive, "translation", topNode_->translation());

        writeDegreeAngleAxis(archive, "rotation", AngleAxis(topNode_->rotation()));
        // The following element is used to distinguish the value type from the old one using radian.
        // The old format is deprecated, and writing the following element should be omitted in the future.
        archive.write("angle_unit", "degree");
        
        archive.write("lightweight_rendering", isLightweightRenderingEnabled_);
    }
    return true;
}


bool SceneItem::restore(const Archive& archive)
{
    if(archive.loadFileTo(this)){
        Vector3 translation;
        if(read(archive, "translation", translation)){
            topNode_->setTranslation(translation);
        }
        AngleAxis rot;
        string unit;
        bool hasRot = false;
        if(archive.read("angle_unit", unit) && unit == "degree"){
            hasRot = readDegreeAngleAxis(archive, "rotation", rot);
        } else { // for the backward compatibility
            hasRot = readRadianAngleAxis(archive, "rotation", rot);
        }
        if(hasRot){
            topNode_->setRotation(rot);
        }
        if(archive.get("lightweight_rendering", false)){
            setLightweightRenderingEnabled(true);
        }
        return true;
    }
    return false;
}


static RenderableItemUtil* getOrCreateRenderableItemUtil(SceneItem* item)
{
    if(!renderableItemUtil){
        renderableItemUtil = make_unique<RenderableItemUtil>();
    }
    renderableItemUtil->setItem(item);
    return renderableItemUtil.get();
}


void SceneItem::getDependentFiles(std::vector<std::string>& out_files)
{
    auto& fp = filePath();
    if(!fp.empty()){
        out_files.push_back(fp);
        auto util = getOrCreateRenderableItemUtil(this);
        util->getSceneFilesForArchiving(out_files);
    }
}


void SceneItem::relocateDependentFiles
(std::function<std::string(const std::string& path)> getRelocatedFilePath)
{
    auto util = getOrCreateRenderableItemUtil(this);
    util->initializeSceneObjectUrlRelocation();
    util->relocateSceneObjectUris(getRelocatedFilePath);
}


SgNode* SceneItem::getScene()
{
    return topNode_;
}


LocationProxyPtr SceneItem::getLocationProxy()
{
    if(!location){
        location = new Location(this);
    }
    return location;
}


SceneItem::Location::Location(SceneItem* item)
    : LocationProxy(GlobalLocation),
      item(item)
{

}


std::string SceneItem::Location::getName() const
{
    return fmt::format(_("Offset position of {0}"), item->displayName());
}


Isometry3 SceneItem::Location::getLocation() const
{
    return item->topNode_->position();
}


bool SceneItem::Location::setLocation(const Isometry3& T)
{
    item->topNode_->setPosition(T);
    item->topNode_->notifyUpdate();
    item->notifyUpdate();
    return true;
}


SignalProxy<void()> SceneItem::Location::sigLocationChanged()
{
    return item->sigUpdated();
}

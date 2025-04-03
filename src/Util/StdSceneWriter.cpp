#include "StdSceneWriter.h"
#include "YAMLWriter.h"
#include "ObjSceneWriter.h"
#include "SceneGraph.h"
#include "SceneDrawables.h"
#include "SceneLights.h"
#include "SceneCameras.h"
#include "SceneEffects.h"
#include "SceneGraphOptimizer.h"
#include "PolymorphicSceneNodeFunctionSet.h"
#include "EigenArchive.h"
#include "FilePathVariableProcessor.h"
#include "UriSchemeProcessor.h"
#include "FileUtil.h"
#include "CloneMap.h"
#include "NullOut.h"
#include "UTF8.h"
#include "Format.h"
#include <cnoid/stdx/filesystem>
#include <unordered_map>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = cnoid::stdx::filesystem;

namespace cnoid {

class StdSceneWriter::Impl
{
public:
    StdSceneWriter* self;

    PolymorphicSceneNodeFunctionSet writeFunctions;
    MappingPtr currentArchive;
    SgNodePtr nodeToIntegrate;
    bool isDegreeMode;
    bool isTopGroupNodeSkippingEnabled;
    bool isTransformIntegrationEnabled;
    bool isAppearanceEnabled;
    bool isReplacingExistingModelFile;
    bool isMeshEnabled;
    SgMaterialPtr defaultMaterial;
    unique_ptr<UriSchemeProcessor> uriSchemeProcessor;
    FilePathVariableProcessorPtr filePathVariableProcessor;
    unique_ptr<YAMLWriter> yamlWriter;
    unique_ptr<StdSceneWriter> subSceneWriter;
    unique_ptr<ObjSceneWriter> objSceneWriter;
    int numSkippedNode;
    string mainSceneName;
    string outputBaseDirectory;
    filesystem::path outputBaseDirPath;
    filesystem::path originalBaseDirPath;
    typedef unordered_map<SgObjectPtr, ValueNodePtr> SceneToYamlNodeMap;
    SceneToYamlNodeMap sceneToYamlNodeMap;

    int extModelFileMode;
    bool isOriginalSceneExtModelFileUriRewritingEnabled;
    unordered_map<SgObjectPtr, string> uriRewritingMap;
    set<string> extModelFiles;

    ostream* os_;
    ostream& os() { return *os_; }

    Impl(StdSceneWriter* self);
    void copyConfigurations(const Impl* org);
    YAMLWriter* getOrCreateYamlWriter();
    StdSceneWriter* getOrCreateSubSceneWriter();
    ObjSceneWriter* getOrCreateObjSceneWriter();
    void setOutputBaseDirectory(const std::string& directory);
    void ensureUriSchemeProcessor();
    bool writeScene(const std::string& filename, SgNode* node, const std::vector<SgNode*>* pnodes);
    void rewriteOriginalSceneExtModelFileUris();
    pair<MappingPtr, bool> findOrCreateMapping(SgObject* object);
    pair<ListingPtr, bool> findOrCreateListing(SgObject* object);
    ValueNodePtr writeSceneNode(SgNode* node);
    void makeLinkToOriginalModelFile(Mapping* archive, SgObject* sceneObject);
    void copyModelFilesAndLinkToCopiedFile(Mapping* archive, SgObject* sceneObject);
    string copyModelFiles(SgObject* sceneObject);
    bool replaceOriginalModelFile(
        Mapping* archive, SgNode* node, bool isAppearanceEnabled, SgObject* objectOfUri);
    void processUnknownNode(Mapping* archive, SgNode* node);
    void writeObjectHeader(Mapping* archive, const char* typeName, SgObject* object);
    void writeGroup(Mapping* archive, SgGroup* group);
    void writePosTransform(Mapping* archive, SgPosTransform* transform);
    void writeScaleTransform(Mapping* archive, SgScaleTransform* transform);
    void writeShape(Mapping* archive, SgShape* shape);
    MappingPtr writeGeometry(SgShape* shape);
    void writeMeshAttributes(Mapping* archive, SgMesh* mesh);
    bool writeMesh(Mapping* archive, SgMesh* mesh);
    void writeVertices(Mapping* archive, SgVertexArray* srcVertices);
    void writePrimitiveAttributes(Mapping* archive, SgMesh* mesh);
    void writeBox(Mapping* archive, SgMesh* mesh);
    void writeSphere(Mapping* archive, SgMesh* mesh);
    void writeCylinder(Mapping* archive, SgMesh* mesh);
    void writeCone(Mapping* archive, SgMesh* mesh);
    void writeCapsule(Mapping* archive, SgMesh* mesh);
    MappingPtr writeAppearance(SgShape* shape);
    MappingPtr writeMaterial(SgMaterial* material);
    MappingPtr writeTexture(SgTexture* texture);
    MappingPtr writeTextureTransform(SgTextureTransform * transform);
    void writeLineSet(Mapping* archive, SgLineSet* lineSet);
    void writeLight(Mapping* archive, SgLight* light);
    void writeDirectionalLight(Mapping* archive, SgDirectionalLight* light);
    void writePointLight(Mapping* archive, SgPointLight* light);
    void writeSpotLight(Mapping* archive, SgSpotLight* light);
    void writeCamera(Mapping* archive, SgCamera* camera);
    void writePerspectiveCamera(Mapping* archive, SgPerspectiveCamera* camera);
    void writeOrthographicCamera(Mapping* archive, SgOrthographicCamera* camera);
    void writeFog(Mapping* archive, SgFog* fog);
};

}

StdSceneWriter::StdSceneWriter()
{
    impl = new Impl(this);
}


StdSceneWriter::Impl::Impl(StdSceneWriter* self)
    : self(self)
{
    writeFunctions.setFunction<SgNode>(
        [&](SgNode* node){ processUnknownNode(currentArchive, node); });
    writeFunctions.setFunction<SgGroup>(
        [&](SgGroup* group){ writeGroup(currentArchive, group); });
    writeFunctions.setFunction<SgPosTransform>(
        [&](SgPosTransform* transform){ writePosTransform(currentArchive, transform); });
    writeFunctions.setFunction<SgScaleTransform>(
        [&](SgScaleTransform* transform){ writeScaleTransform(currentArchive, transform); });
    writeFunctions.setFunction<SgShape>(
        [&](SgShape* shape){ writeShape(currentArchive, shape); });
    writeFunctions.setFunction<SgLineSet>(
        [&](SgLineSet* lineSet){ writeLineSet(currentArchive, lineSet); });
    writeFunctions.setFunction<SgDirectionalLight>(
        [&](SgDirectionalLight* light){ writeDirectionalLight(currentArchive, light); });
    writeFunctions.setFunction<SgPointLight>(
        [&](SgPointLight* light){ writePointLight(currentArchive, light); });
    writeFunctions.setFunction<SgSpotLight>(
        [&](SgSpotLight* light){ writeSpotLight(currentArchive, light); });
    writeFunctions.setFunction<SgPerspectiveCamera>(
        [&](SgPerspectiveCamera* camera){ writePerspectiveCamera(currentArchive, camera); });
    writeFunctions.setFunction<SgOrthographicCamera>(
        [&](SgOrthographicCamera* camera){ writeOrthographicCamera(currentArchive, camera); });
    writeFunctions.setFunction<SgFog>(
        [&](SgFog* fog){ writeFog(currentArchive, fog); });

    writeFunctions.updateDispatchTable();

    isDegreeMode = true;
    isTopGroupNodeSkippingEnabled = false;
    isTransformIntegrationEnabled = false;
    isAppearanceEnabled = true;
    isReplacingExistingModelFile = false;
    isMeshEnabled = true;
    isOriginalSceneExtModelFileUriRewritingEnabled = false;
    extModelFileMode = EmbedModels;

    os_ = &nullout();    
}


StdSceneWriter::StdSceneWriter(const StdSceneWriter& org)
    : StdSceneWriter()
{
    impl->copyConfigurations(org.impl);
}


void StdSceneWriter::Impl::copyConfigurations(const Impl* org)
{
    isDegreeMode = org->isDegreeMode;
    isTopGroupNodeSkippingEnabled = org->isTopGroupNodeSkippingEnabled;
    isTransformIntegrationEnabled = org->isTransformIntegrationEnabled;
    isAppearanceEnabled = org->isAppearanceEnabled;
    isMeshEnabled = org->isMeshEnabled;
    extModelFileMode = org->extModelFileMode;
    outputBaseDirectory = org->outputBaseDirectory;
    outputBaseDirPath = org->outputBaseDirPath;
    originalBaseDirPath = org->originalBaseDirPath;
    os_ = org->os_;
    filePathVariableProcessor = org->filePathVariableProcessor;
    if(org->yamlWriter){
        getOrCreateYamlWriter()->setIndentWidth(org->yamlWriter->indentWidth());
    }
}


StdSceneWriter::~StdSceneWriter()
{
    delete impl;
}


void StdSceneWriter::setMessageSink(std::ostream& os)
{
    AbstractSceneWriter::setMessageSink(os);
    
    impl->os_ = &os;
    if(impl->objSceneWriter){
        impl->objSceneWriter->setMessageSink(os);
    }
}


YAMLWriter* StdSceneWriter::Impl::getOrCreateYamlWriter()
{
    if(!yamlWriter){
        yamlWriter.reset(new YAMLWriter);
        yamlWriter->setKeyOrderPreservationMode(true);
    }
    return yamlWriter.get();
}


StdSceneWriter* StdSceneWriter::Impl::getOrCreateSubSceneWriter()
{
    if(!subSceneWriter){
        subSceneWriter.reset(new StdSceneWriter(*self));
        subSceneWriter->setExtModelFileMode(EmbedModels);
        subSceneWriter->impl->isReplacingExistingModelFile = true;
    }
    return subSceneWriter.get();
}


ObjSceneWriter* StdSceneWriter::Impl::getOrCreateObjSceneWriter()
{
    if(!objSceneWriter){
        objSceneWriter.reset(new ObjSceneWriter);
        objSceneWriter->setMessageSink(os());
    }
    return objSceneWriter.get();
}


void StdSceneWriter::setMainSceneName(const std::string& name)
{
    impl->mainSceneName = name;
}


void StdSceneWriter::setOutputBaseDirectory(const std::string& directory)
{
    impl->setOutputBaseDirectory(directory);
}


void StdSceneWriter::setBaseDirectory(const std::string& directory)
{
    impl->setOutputBaseDirectory(directory);
}


void StdSceneWriter::Impl::setOutputBaseDirectory(const std::string& directory)
{
    outputBaseDirectory = directory;
    outputBaseDirPath = fromUTF8(directory);
    if(filePathVariableProcessor){
        filePathVariableProcessor->setBaseDirectory(directory);
    }
}


void StdSceneWriter::Impl::ensureUriSchemeProcessor()
{
    if(!uriSchemeProcessor){
        uriSchemeProcessor = make_unique<UriSchemeProcessor>();
        if(!filePathVariableProcessor){
            filePathVariableProcessor = new FilePathVariableProcessor;
            filePathVariableProcessor->setBaseDirPath(outputBaseDirPath);
        }
        uriSchemeProcessor->setFilePathVariableProcessor(filePathVariableProcessor);
    }
}


void StdSceneWriter::setFilePathVariableProcessor(FilePathVariableProcessor* fpvp)
{
    impl->filePathVariableProcessor = fpvp;
    impl->outputBaseDirectory = fpvp->baseDirectory();
    impl->outputBaseDirPath = fpvp->baseDirPath();
    if(impl->uriSchemeProcessor){
        impl->uriSchemeProcessor->setFilePathVariableProcessor(fpvp);
    }
}


void StdSceneWriter::setIndentWidth(int n)
{
    impl->getOrCreateYamlWriter()->setIndentWidth(n);
}


void StdSceneWriter::setExtModelFileMode(int mode)
{
    impl->extModelFileMode = mode;
}


int StdSceneWriter::extModelFileMode() const
{
    return impl->extModelFileMode;
}


void StdSceneWriter::setOriginalSceneExtModelFileUriRewritingEnabled(bool on)
{
    impl->isOriginalSceneExtModelFileUriRewritingEnabled = on;
}


bool StdSceneWriter::isOriginalSceneExtModelFileUriRewritingEnabled() const
{
    return impl->isOriginalSceneExtModelFileUriRewritingEnabled;
}


void StdSceneWriter::setOriginalBaseDirectory(const std::string& directory)
{
    impl->originalBaseDirPath = fromUTF8(directory);
}


void StdSceneWriter::setTopGroupNodeSkippingEnabled(bool on)
{
    impl->isTopGroupNodeSkippingEnabled = on;
}


bool StdSceneWriter::isTopGroupNodeSkippingEnabled() const
{
    return impl->isTopGroupNodeSkippingEnabled;
}


void StdSceneWriter::setTransformIntegrationEnabled(bool on)
{
    impl->isTransformIntegrationEnabled = on;
}


bool StdSceneWriter::isTransformIntegrationEnabled() const
{
    return impl->isTransformIntegrationEnabled;
}


void StdSceneWriter::setAppearanceEnabled(bool on)
{
    impl->isAppearanceEnabled = on;
}


bool StdSceneWriter::isAppearanceEnabled() const
{
    return impl->isAppearanceEnabled;
}


void StdSceneWriter::setMeshEnabled(bool on)
{
    impl->isMeshEnabled = on;
}


bool StdSceneWriter::isMeshEnabled() const
{
    return impl->isMeshEnabled;
}


void StdSceneWriter::clear()
{
    impl->mainSceneName.clear();
    impl->sceneToYamlNodeMap.clear();
    impl->uriRewritingMap.clear();
    impl->extModelFiles.clear();
    clearImageFileInformation();
}


ref_ptr<ValueNode> StdSceneWriter::writeScene(SgNode* node)
{
    ValueNodePtr archive;

    if(!impl->isTopGroupNodeSkippingEnabled || (typeid(*node) != typeid(SgGroup))){
        archive = impl->writeSceneNode(node);

    } else {
        auto group = node->toGroupNode();
        ListingPtr nodes = new Listing;
        for(auto& child : *group){
            if(auto childArchive = impl->writeSceneNode(child)){
                nodes->append(childArchive);
            }
        }
        if(nodes->size() >= 2){
            archive = nodes;
        } else if(nodes->size() == 1){
            archive = nodes->at(0);
        }
    }

    return archive;
}


bool StdSceneWriter::writeScene(const std::string& filename, SgNode* node)
{
    return impl->writeScene(filename, node, nullptr);
}


bool StdSceneWriter::writeScene(const std::string& filename, const std::vector<SgNode*>& nodes)
{
    return impl->writeScene(filename, nullptr, &nodes);
}


bool StdSceneWriter::Impl::writeScene
(const std::string& filename, SgNode* node, const std::vector<SgNode*>* pnodes)
{
    getOrCreateYamlWriter();

    if(!yamlWriter->openFile(filename)){
        return false;
    }

    SgGroupPtr group = new SgGroup;
    if(node){
        group->addChild(node);
    } else if(pnodes){
        for(auto& node : *pnodes){
            group->addChild(node);
        }
    }

    if(isTransformIntegrationEnabled &&
       extModelFileMode != LinkToOriginalModelFiles &&
       extModelFileMode != CopyModelFiles){
        SceneGraphOptimizer optimizer;
        CloneMap cloneMap;
        SgObject::setNonNodeCloning(cloneMap, false);
        group = cloneMap.getClone(group);
        SgObject::setNonNodeCloning(cloneMap, true);
        optimizer.setUriNodePreservingMode(extModelFileMode != EmbedModels);
        optimizer.simplifyTransformPathsWithTransformedMeshes(group, cloneMap);
    }
       
    MappingPtr header = new Mapping;
    header->write("format", "choreonoid_scene");
    header->write("format_version", "1.0");
    header->write("angle_unit", "degree");
    
    setOutputBaseDirectory(toUTF8(filesystem::path(fromUTF8(filename)).parent_path().generic_string()));

    numSkippedNode = 0;

    ListingPtr nodeList = new Listing;
    for(auto& node : *group){
        nodeList->append(writeSceneNode(node));
    }
    header->insert("scene", nodeList);

    if(numSkippedNode == 1){
        os() << _("Warning: There is an unsupported node.") << endl;
    } else if(numSkippedNode >= 2){
        os() << formatR(_("Warning: {0} unsupported nodes were skipped to output."), numSkippedNode) << endl;
    }
    
    yamlWriter->putNode(header);
    yamlWriter->closeFile();

    extModelFiles.clear();

    sceneToYamlNodeMap.clear();    
    
    return true;
}


void StdSceneWriter::rewriteOriginalSceneExtModelFileUris()
{
    impl->rewriteOriginalSceneExtModelFileUris();
}


void StdSceneWriter::Impl::rewriteOriginalSceneExtModelFileUris()
{
    for(auto& kv : uriRewritingMap){
        auto& object = kv.first;
        auto& uri = kv.second;
        object->setUriWithFilePathAndBaseDirectory(uri, outputBaseDirectory);
    }
}


pair<MappingPtr, bool> StdSceneWriter::Impl::findOrCreateMapping(SgObject* object)
{
    bool found = false;
    MappingPtr mapping = new Mapping;
    auto inserted = sceneToYamlNodeMap.insert(SceneToYamlNodeMap::value_type(object, mapping));
    if(!inserted.second){
        mapping = dynamic_pointer_cast<Mapping>(inserted.first->second);
        found = true;
    }
    return make_pair(mapping, found);
}


pair<ListingPtr, bool> StdSceneWriter::Impl::findOrCreateListing(SgObject* object)
{
    bool found = false;
    ListingPtr listing = new Listing;
    auto inserted = sceneToYamlNodeMap.insert(SceneToYamlNodeMap::value_type(object, listing));
    if(!inserted.second){
        listing = dynamic_pointer_cast<Listing>(inserted.first->second);
        found = true;
    }
    return make_pair(listing, found);
}


ValueNodePtr StdSceneWriter::Impl::writeSceneNode(SgNode* node)
{
    MappingPtr archive;
    bool found;
    std::tie(archive, found) = findOrCreateMapping(node);
    if(found){
        return archive;
    }

    if(node->hasUri() && (extModelFileMode != EmbedModels)){
        writeObjectHeader(archive, "Resource", node);
        if(extModelFileMode == LinkToOriginalModelFiles){
            makeLinkToOriginalModelFile(archive, node);
        } else if(extModelFileMode == CopyModelFiles){
            copyModelFilesAndLinkToCopiedFile(archive, node);
        } else {
            if(!replaceOriginalModelFile(archive, node, true, node)){
                // TODO: write the models as embeded models when the replacement fails
                archive.reset();
            }
        }
        return archive;
    }

    currentArchive = archive;
    writeFunctions.dispatch(node);
    currentArchive.reset();

    if(nodeToIntegrate){
        node = nodeToIntegrate;
        nodeToIntegrate.reset();
    }

    if(node->isGroupNode()){
        auto group = node->toGroupNode();
        ListingPtr elements = new Listing;
        for(auto& child : *group){
            if(auto childArchive = writeSceneNode(child)){
                elements->append(childArchive);
            }
        }
        if(!elements->empty()){
            archive->insert("elements", elements);
        }
    }

    if(archive->empty()){
        archive.reset();
    }

    return archive;
}


void StdSceneWriter::Impl::makeLinkToOriginalModelFile(Mapping* archive, SgObject* sceneObject)
{
    ensureUriSchemeProcessor();

    string uri = sceneObject->uri();
    if(!uriSchemeProcessor->detectScheme(uri)){
        uri = sceneObject->absoluteUri();
        auto filePath = uriSchemeProcessor->getParameterizedFilePath(uri);
        if(!filePath.empty()){
            uri = filePath;
        }
    }
        
    archive->write("uri", uri, DOUBLE_QUOTED);
    if(auto metadata = sceneObject->uriMetadata()){
        archive->insert("metadata", metadata);
    }
}


void StdSceneWriter::Impl::copyModelFilesAndLinkToCopiedFile(Mapping* archive, SgObject* sceneObject)
{
    ensureUriSchemeProcessor();
    
    string relativeFilePathToCopiedFile = copyModelFiles(sceneObject);
    
    if(relativeFilePathToCopiedFile.empty()){
        makeLinkToOriginalModelFile(archive, sceneObject);
        os() << formatR(_("Warning: Model file \"{0}\" cannot be copied."), sceneObject->uri()) << endl;
    } else {
        archive->write("uri", relativeFilePathToCopiedFile, DOUBLE_QUOTED);
        if(auto metadata = sceneObject->uriMetadata()){
            archive->insert("metadata", metadata);
        }
    }
}


string StdSceneWriter::Impl::copyModelFiles(SgObject* sceneObject)
{
    string relativeFilePathToCopiedFile;

    if(sceneObject->hasUri()){
        stdx::error_code ec;
        string srcFile = sceneObject->localFileAbsolutePath();
        filesystem::path srcFilePath(fromUTF8(srcFile));
        if(!srcFilePath.empty()){
            if(!filesystem::exists(srcFilePath, ec)){
                os() << formatR(_("Warning: File \"{0}\" is not found."), srcFile) << endl;
            } else {
                filesystem::path relativeFilePath;
                if(!originalBaseDirPath.empty()){
                    findPathInDirectory(originalBaseDirPath, srcFilePath, relativeFilePath);
                }
                if(relativeFilePath.empty()){
                    filesystem::path uriPath = fromUTF8(uriSchemeProcessor->getFilePath(sceneObject->uri()));
                    uriPath = uriPath.lexically_normal();
                    if(uriPath.has_root_path()){
                        uriPath = uriPath.relative_path();
                    }
                    // Remove ".." elements
                    for(auto& element : uriPath){
                        if(element != ".."){
                            relativeFilePath /= element;
                        }
                    }
                }
                if(!relativeFilePath.empty()){
                    filesystem::path destFilePath = outputBaseDirPath / relativeFilePath;
                    if(!filesystem::equivalent(srcFilePath, destFilePath, ec)){
                        filesystem::path destDirPath = destFilePath.parent_path();
                        filesystem::create_directories(destDirPath, ec);
                        if(!ec){
#if __cplusplus > 201402L
                            filesystem::copy_file(
                                srcFilePath, destFilePath, filesystem::copy_options::update_existing, ec);
                            //os() << format("Copy \"{0}\" to \"{1}\".",
                            //               toUTF8(srcFilePath.string()), toUTF8(destFilePath.string())) << endl;
#else
                            bool doCopy = true;
                            if(filesystem::exists(destFilePath, ec)){
                                auto srcTime = filesystem::last_write_time(srcFilePath, ec); 
                                auto destTime = filesystem::last_write_time(destFilePath, ec);
                                if(destTime >= srcTime){
                                    doCopy = false;
                                }
                            }
                            if(doCopy){
                                filesystem::copy_file(
                                    srcFilePath, destFilePath, filesystem::copy_option::overwrite_if_exists, ec);
                            }
#endif
                        }
                    }
                    if(!ec){
                        relativeFilePathToCopiedFile = toUTF8(relativeFilePath.generic_string());
                        if(isOriginalSceneExtModelFileUriRewritingEnabled){
                            uriRewritingMap[sceneObject] = relativeFilePathToCopiedFile;
                        }
                    } else {
                        os() << formatR(_("Warning: File \"{0}\" cannot be copied: {1}"),
                                        sceneObject->localFileAbsolutePath(), ec.message()) << endl;
                    }
                }
            }
        }
    }

    int n = sceneObject->numChildObjects();
    for(int i=0; i < n; ++i){
        copyModelFiles(sceneObject->childObject(i));
    }

    return relativeFilePathToCopiedFile;
}


/*
  TODO: Check if the scene can be written as the target model file format.
  If it cannot, give up the writing.
*/
bool StdSceneWriter::Impl::replaceOriginalModelFile
(Mapping* archive, SgNode* node, bool isAppearanceEnabled, SgObject* objectOfUri)
{
    ensureUriSchemeProcessor();

    filesystem::path path;
    uriSchemeProcessor->detectScheme(objectOfUri->uri());
    path = fromUTF8(uriSchemeProcessor->path());
    path = path.stem();
    if(!mainSceneName.empty()){
        path = filesystem::path(fromUTF8(mainSceneName)) / path;
    }
        
    if(objectOfUri->hasUriFragment()){
        path += "-";
        path += objectOfUri->uriFragment();
    }
    if(extModelFileMode == ReplaceWithObjModelFiles){
        path += ".obj";
    } else {
        path += ".scen";
    }
    auto fullPath = filesystem::lexically_normal(outputBaseDirPath / path);
    auto filename = fullPath.string();

    bool replaced = false;
    stdx::error_code ec;
    
    auto emplaced = extModelFiles.emplace(filename);
    if(!emplaced.second){ // Already replaced
        replaced = true;

    } else { // New reference
        if(objectOfUri->hasAbsoluteUri()){
            auto& absUri = objectOfUri->absoluteUri();
            if(absUri.find("file://") == 0){
                filesystem::path orgFilePath(absUri.substr(7));
                if(filesystem::equivalent(fullPath, orgFilePath, ec)){
                    os() << formatR(_("Model file \"{0}\" cannot be replaced with the same format file in the same directory"),
                                    filename) << endl;
                    return false;
                }
            }
        }
        filesystem::create_directories(fullPath.parent_path(), ec);
        if(!ec){
            /*
              TODO: If isOriginalSceneExtModelFileUriRewritingEnabled is true,
              a similar option should be applied for the following sub writers.
            */
            if(extModelFileMode == ReplaceWithObjModelFiles){
                auto writer = getOrCreateObjSceneWriter();
                writer->setMaterialEnabled(isAppearanceEnabled);
                replaced = writer->writeScene(filename, node);
            } else {
                auto writer = getOrCreateSubSceneWriter();
                writer->setAppearanceEnabled(isAppearanceEnabled);
                replaced = writer->writeScene(filename, node);
            }
        }
    }

    if(replaced){
        string uri = toUTF8(path.generic_string());
        archive->write("uri", uri, DOUBLE_QUOTED);

        if(isOriginalSceneExtModelFileUriRewritingEnabled){
            uriRewritingMap[objectOfUri] = uri;
        }
    } else {
        if(ec){
            os() << formatR(_("Warning: Failed to replace model file \"{0}\" with \"{1}\". {2}"),
                            objectOfUri->uri(), filename, ec.message()) << endl;
        } else {
            os() << formatR(_("Warning: Failed to replace model file \"{0}\" with \"{1}\"."),
                            objectOfUri->uri(), filename) << endl;
        }
    }
    
    return replaced;
}


void StdSceneWriter::Impl::processUnknownNode(Mapping* archive, SgNode* node)
{
    ++numSkippedNode;
}


void StdSceneWriter::Impl::writeObjectHeader(Mapping* archive, const char* typeName, SgObject* object)
{
    if(typeName){
        auto typeNode = new ScalarNode(typeName);
        typeNode->setAsHeaderInMapping();
        archive->insert("type", typeNode);
    }
    const auto& name = object->name();
    if(!name.empty() && (!object->hasUriObjectName() || name != object->uriObjectName())){
        archive->write("name", object->name());
    }
    if(object->hasAttribute(SgObject::MetaScene)){
        archive->write("is_meta_scene", true);
    }
}


void StdSceneWriter::Impl::writeGroup(Mapping* archive, SgGroup* group)
{
    writeObjectHeader(archive, "Group", group);
}


void StdSceneWriter::Impl::writePosTransform(Mapping* archive, SgPosTransform* transform)
{
    archive->setFloatingNumberFormat("%.12g");
    writeObjectHeader(archive, "Transform", transform);
    AngleAxis aa(transform->rotation());
    if(aa.angle() != 0.0){
        writeDegreeAngleAxis(archive, "rotation", aa);
    }
    Vector3 p(transform->translation());
    if(!p.isZero()){
        write(archive, "translation", p);
    }

    if(isTransformIntegrationEnabled){
        if(transform->numChildren() == 1){
            if(auto scaleNode = dynamic_cast<SgScaleTransform*>(transform->child(0))){
                write(archive, "scale", scaleNode->scale());
                nodeToIntegrate = scaleNode;
            }
        }
    }
}


void StdSceneWriter::Impl::writeScaleTransform(Mapping* archive, SgScaleTransform* transform)
{
    writeObjectHeader(archive, "Transform", transform);
    write(archive, "scale", transform->scale());
}


void StdSceneWriter::Impl::writeShape(Mapping* archive, SgShape* shape)
{
    if(auto mesh = shape->mesh()){
        if(mesh->hasUri() && !shape->material() && !shape->texture()){
            // Write the mesh as a resouce node
            if(auto geometry = writeGeometry(shape)){
                archive->insert(geometry);
                return;
            }
        }
    }
    
    writeObjectHeader(archive, "Shape", shape);

    if(isAppearanceEnabled){
        if(auto appearance = writeAppearance(shape)){
            archive->insert("appearance", appearance);
        }
    }
    if(auto geometry = writeGeometry(shape)){
        archive->insert("geometry", geometry);
    }
}


MappingPtr StdSceneWriter::Impl::writeGeometry(SgShape* shape)
{
    auto mesh = shape->mesh();
    if(!mesh){
        return nullptr;
    }

    bool isResource = mesh->hasUri() && (extModelFileMode != EmbedModels);

    MappingPtr archive;
    if(isResource){
        // Geometry resource nodes should not be shared to make scene files more readable
        // by avoiding aliases and anchors for URIs.
        archive = new Mapping;
    } else {
        bool found;
        std::tie(archive, found) = findOrCreateMapping(mesh);
        if(found){
            return archive;
        }
    }
    
    if(isResource){
        writeObjectHeader(archive, "Resource", mesh);
        if(extModelFileMode == LinkToOriginalModelFiles){
            makeLinkToOriginalModelFile(archive, mesh);
        } else if(extModelFileMode == CopyModelFiles){
            copyModelFilesAndLinkToCopiedFile(archive, mesh);
        } else {
            if(!replaceOriginalModelFile(archive, shape, false, mesh)){
                // TODO: write the models as embeded models when the replacement fails
                archive.reset();
            }
        }
        if(archive){
            writeMeshAttributes(archive, mesh);
        }
    } else {
        switch(mesh->primitiveType()){
        case SgMesh::MeshType:
            if(!isMeshEnabled || !writeMesh(archive, mesh)){
                archive.reset();
            }
            break;
        case SgMesh::BoxType:
            writeBox(archive, mesh);
            break;
        case SgMesh::SphereType:
            writeSphere(archive, mesh);
            break;
        case SgMesh::CylinderType:
            writeCylinder(archive, mesh);
            break;
        case SgMesh::ConeType:
            writeCone(archive, mesh);
            break;
        case SgMesh::CapsuleType:
            writeCapsule(archive, mesh);
            break;
        default:
            archive.reset();
            break;
        }
    }

    return archive;
}


void StdSceneWriter::Impl::writeMeshAttributes(Mapping* archive, SgMesh* mesh)
{
    if(mesh->creaseAngle() > 0.0f){
        archive->write("crease_angle", degree(mesh->creaseAngle()));
    }
    if(mesh->isSolid()){
        archive->write("solid", true);
    }
}


bool StdSceneWriter::Impl::writeMesh(Mapping* archive, SgMesh* mesh)
{
    bool isValid = false;
    int numTriangles = mesh->numTriangles();

    if(mesh->hasVertices() && numTriangles > 0){

        archive->write("type", "TriangleMesh");

        writeMeshAttributes(archive, mesh);
        writeVertices(archive, mesh->vertices());
        
        Listing& indexList = *archive->createFlowStyleListing("faces");
        const int numTriScalars = numTriangles * 3;
        indexList.reserve(numTriScalars);
        for(int i=0; i < numTriangles; ++i){
            auto triangle = mesh->triangle(i);
            indexList.append(triangle[0], 15, numTriScalars);
            indexList.append(triangle[1], 15, numTriScalars);
            indexList.append(triangle[2], 15, numTriScalars);
        }

        if(mesh->hasNormals() && mesh->creaseAngle() == 0.0f){
            const auto srcNormals = mesh->normals();
            ListingPtr normals;
            bool found;
            tie(normals, found) = findOrCreateListing(srcNormals);
            if(!found){
                normals->setFlowStyle();
                const int scalarElementSize = srcNormals->size() * 3;
                normals->reserve(scalarElementSize);
                for(auto& n : *srcNormals){
                    normals->append(n.x(), 12, scalarElementSize);
                    normals->append(n.y(), 12, scalarElementSize);
                    normals->append(n.z(), 12, scalarElementSize);
                }
            }
            archive->insert("normals", normals);

            if(mesh->hasNormalIndices()){
                const auto& srcNormalIndices = mesh->normalIndices();
                const int n = srcNormalIndices.size();
                Listing& indexList = *archive->createFlowStyleListing("normal_indices");
                indexList.reserve(n);
                for(auto& index : srcNormalIndices){
                    indexList.append(index, 15, n);
                }
            }
        }

        if(isAppearanceEnabled && mesh->hasTexCoords()){
            const auto srcTexCoords = mesh->texCoords();
            ListingPtr texCoords;
            bool found;
            tie(texCoords, found) = findOrCreateListing(srcTexCoords);
            if(!found){
                texCoords->setFlowStyle();
                const int scalarElementSize = srcTexCoords->size() * 2;
                texCoords->reserve(scalarElementSize);
                for(auto& t : *srcTexCoords){
                    texCoords->append(t.x(), 12, scalarElementSize);
                    texCoords->append(t.y(), 12, scalarElementSize);
                }
            }
            archive->insert("tex_coords", texCoords);
            
            if(mesh->hasTexCoordIndices()){
                const auto& srcTexCoordIndices = mesh->texCoordIndices();
                const int n = srcTexCoordIndices.size();
                Listing& indexList = *archive->createFlowStyleListing("tex_coord_indices");
                indexList.reserve(n);
                for(auto& index : srcTexCoordIndices){
                    indexList.append(index, 15, n);
                }
            }
        }
        
        isValid = true;
    }
    
    return isValid;
}


void StdSceneWriter::Impl::writeVertices(Mapping* archive, SgVertexArray* srcVertices)
{
    if(srcVertices->empty()){
        return;
    }
    ListingPtr vertices;
    bool found;
    tie(vertices, found) = findOrCreateListing(srcVertices);
    if(!found){
        vertices->setFlowStyle();
        const int scalarElementSize = srcVertices->size() * 3;
        vertices->reserve(scalarElementSize);
        for(auto& v : *srcVertices){
            vertices->append(v.x(), 12, scalarElementSize);
            vertices->append(v.y(), 12, scalarElementSize);
            vertices->append(v.z(), 12, scalarElementSize);
        }
    }
    archive->insert("vertices", vertices);
}


void StdSceneWriter::Impl::writePrimitiveAttributes(Mapping* archive, SgMesh* mesh)
{
    if(mesh->isSolid()){
        archive->write("solid", true);
    }
    if(mesh->divisionNumber() >= 1){
        archive->write("division_number", mesh->divisionNumber());
    }
    if(mesh->extraDivisionNumber() >= 1){
        archive->write("extra_division_number", mesh->extraDivisionNumber());
        int mode = mesh->extraDivisionMode();
        if(mode != SgMesh::ExtraDivisionPreferred){
            string symbol;
            if(mode == SgMesh::ExtraDivisionX){
                symbol = "x";
            } else if(mode == SgMesh::ExtraDivisionY){
                symbol = "y";
            } else if(mode == SgMesh::ExtraDivisionZ){
                symbol = "z";
            }
            archive->write("extra_division_mode", symbol);
        }
    }
}


void StdSceneWriter::Impl::writeBox(Mapping* archive, SgMesh* mesh)
{
    const auto& box = mesh->primitive<SgMesh::Box>();
    archive->write("type", "Box");
    write(archive, "size", box.size);
    writePrimitiveAttributes(archive, mesh);
}


void StdSceneWriter::Impl::writeSphere(Mapping* archive, SgMesh* mesh)
{
    const auto& sphere = mesh->primitive<SgMesh::Sphere>();
    archive->write("type", "Sphere");
    archive->write("radius", sphere.radius);
    writePrimitiveAttributes(archive, mesh);
}


void StdSceneWriter::Impl::writeCylinder(Mapping* archive, SgMesh* mesh)
{
    const auto& cylinder = mesh->primitive<SgMesh::Cylinder>();
    archive->write("type", "Cylinder");
    archive->write("radius", cylinder.radius);
    archive->write("height", cylinder.height);
    if(!cylinder.top){
        archive->write("top", false);
    }
    if(!cylinder.bottom){
        archive->write("bottom", false);
    }
    writePrimitiveAttributes(archive, mesh);
}


void StdSceneWriter::Impl::writeCone(Mapping* archive, SgMesh* mesh)
{
    const auto& cone = mesh->primitive<SgMesh::Cone>();
    archive->write("type", "Cone");
    archive->write("radius", cone.radius);
    archive->write("height", cone.height);
    if(!cone.bottom){
        archive->write("bottom", false);
    }
    writePrimitiveAttributes(archive, mesh);
}


void StdSceneWriter::Impl::writeCapsule(Mapping* archive, SgMesh* mesh)
{
    const auto& capsule = mesh->primitive<SgMesh::Capsule>();
    archive->write("type", "Capsule");
    archive->write("radius", capsule.radius);
    archive->write("height", capsule.height);
    writePrimitiveAttributes(archive, mesh);
}


MappingPtr StdSceneWriter::Impl::writeAppearance(SgShape* shape)
{
    MappingPtr archive = new Mapping;

    if(auto materialInfo = writeMaterial(shape->material())){
        archive->insert("material", materialInfo);
    }
    auto texture = shape->texture();
    auto textureInfo = writeTexture(shape->texture());
    if(textureInfo){
        archive->insert("texture", textureInfo);
        if(auto transformInfo = writeTextureTransform(texture->textureTransform())){
            archive->insert("texture_transform", transformInfo);
        }
    }
    if(archive->empty()){
        archive = nullptr;
    }
    
    return archive;
}


MappingPtr StdSceneWriter::Impl::writeMaterial(SgMaterial* material)
{
    if(!material){
        return nullptr;
    }
    if(!defaultMaterial){
        defaultMaterial = new SgMaterial;
    }

    MappingPtr archive;
    bool found;
    std::tie(archive, found) = findOrCreateMapping(material);
    if(found){
        return archive;
    }

    if(material->ambientIntensity() != defaultMaterial->ambientIntensity()){
        archive->write("ambient", material->ambientIntensity());
    }
    if(material->diffuseColor() != defaultMaterial->diffuseColor()){
        write(archive, "diffuse", material->diffuseColor());
    }
    if(material->emissiveColor() != defaultMaterial->emissiveColor()){
        write(archive, "emissive", material->emissiveColor());
    }
    if(material->specularColor() != defaultMaterial->specularColor()){
        write(archive, "specular", material->specularColor());
    }
    if(material->specularExponent() != defaultMaterial->specularExponent()){
        archive->write("specular_exponent", material->specularExponent());
    }
    if(material->transparency() != defaultMaterial->transparency()){
        archive->write("transparency", material->transparency());
    }

    if(archive->empty()){
        archive = nullptr;
    }
    
    return archive;
}


MappingPtr StdSceneWriter::Impl::writeTexture(SgTexture* texture)
{
    if(!texture){
        return nullptr;
    }

    MappingPtr archive;
    bool found;
    std::tie(archive, found) = findOrCreateMapping(texture);
    if(found){
        return archive;
    }
    
    bool isValid = false;

    if(auto image = texture->image()){
        //SgImageResource *r = dynamic_cast<SgImageResource *>(image);
        //if(!!r) {
        //} else
        if(image->hasUri()){
            filesystem::path imageDirPath = outputBaseDirPath;
            filesystem::path mainSceneNamePath;
            if(!mainSceneName.empty()){
                mainSceneNamePath = filesystem::path(fromUTF8(mainSceneName));
                imageDirPath /= mainSceneNamePath;
            }
            string copiedFile;
            if(self->findOrCopyImageFile(image, toUTF8(imageDirPath.string()), copiedFile)){
                filesystem::path path(fromUTF8(copiedFile));
                if(path.is_relative() && !mainSceneName.empty()){
                    path = mainSceneNamePath / path;
                }
                archive->write("uri", toUTF8(path.generic_string()), DOUBLE_QUOTED);
                if(texture->repeatS() == texture->repeatT()){
                    archive->write("repeat", texture->repeatS());
                } else {
                    auto& repeat = *archive->createFlowStyleListing("repeat");
                    repeat.append(texture->repeatS());
                    repeat.append(texture->repeatT());
                }
                isValid = true;
            }
        }
    }
    if(!isValid){
        archive.reset();
    }
    return archive;
}


MappingPtr StdSceneWriter::Impl::writeTextureTransform(SgTextureTransform * transform)
{
    if(!transform){
        return nullptr;
    }

    MappingPtr archive;
    bool found;
    std::tie(archive, found) = findOrCreateMapping(transform);
    if(found){
        return archive;
    }

    if(!transform->center().isZero()){
        write(archive, "center", transform->center());
    }
    if(!transform->scale().isOnes()){
        write(archive, "scale", transform->scale());
    }
    if(!transform->translation().isZero()){
        write(archive, "translation", transform->translation());
    }
    if(transform->rotation() != 0.0){
        archive->write("rotation", transform->rotation());
    }
    
    return archive;
}


void StdSceneWriter::Impl::writeLineSet(Mapping* archive, SgLineSet* lineSet)
{
    writeObjectHeader(archive, "LineSet", lineSet);

    if(isAppearanceEnabled){
        if(auto materialInfo = writeMaterial(lineSet->material())){
            archive->insert("material", materialInfo);
        }
    }

    if(lineSet->lineWidth() > 0.0f){
        archive->write("line_width", lineSet->lineWidth());
    }

    writeVertices(archive, lineSet->vertices());

    const int numLines = lineSet->numLines();
    Listing& lines = *archive->createFlowStyleListing("lines");
    const int numLineIndices = numLines * 2;
    lines.reserve(numLineIndices);
    for(int i=0; i < numLines; ++i){
        auto line = lineSet->line(i);
        lines.append(line[0], 16, numLineIndices);
        lines.append(line[1], 16, numLineIndices);
    }
}


void StdSceneWriter::Impl::writeLight(Mapping* archive, SgLight* light)
{
    write(archive, "color", light->color());
    archive->write("intensity", light->intensity());
    archive->write("ambient", light->ambientIntensity());
}
    

void StdSceneWriter::Impl::writeDirectionalLight(Mapping* archive, SgDirectionalLight* light)
{
    writeObjectHeader(archive, "DirectionalLight", light);
    writeLight(archive, light);
    write(archive, "direction", light->direction());
}


void StdSceneWriter::Impl::writePointLight(Mapping* archive, SgPointLight* light)
{
    writeObjectHeader(archive, "PointLight", light);
    writeLight(archive, light);
    write(archive, "attenuation",
          Vector3(light->constantAttenuation(),
                  light->linearAttenuation(),
                  light->quadraticAttenuation()));
}


void StdSceneWriter::Impl::writeSpotLight(Mapping* archive, SgSpotLight* light)
{
    writeObjectHeader(archive, "SpotLight", light);
    writePointLight(archive, light);
    write(archive, "direction", light->direction());
    archive->write("beam_width", degree(light->beamWidth()));
    archive->write("cut_off_angle", degree(light->cutOffAngle()));
    archive->write("cut_off_exponent", light->cutOffExponent());
}


void StdSceneWriter::Impl::writeCamera(Mapping* archive, SgCamera* camera)
{
    archive->write("near_clip_distance", camera->nearClipDistance());
    archive->write("far_clip_distance", camera->farClipDistance());
}


void StdSceneWriter::Impl::writePerspectiveCamera(Mapping* archive, SgPerspectiveCamera* camera)
{
    writeObjectHeader(archive, "PerspectiveCamera", camera);
    writeCamera(archive, camera);
    archive->write("field_of_view", camera->fieldOfView());
}


void StdSceneWriter::Impl::writeOrthographicCamera(Mapping* archive, SgOrthographicCamera* camera)
{
    writeObjectHeader(archive, "OrthographicCamera", camera);
    writeCamera(archive, camera);
    archive->write("height", camera->height());
}


void StdSceneWriter::Impl::writeFog(Mapping* archive, SgFog* fog)
{
    writeObjectHeader(archive, "Fog", fog);
    write(archive, "color", fog->color());
    archive->write("visibility_range", fog->visibilityRange());
}

#ifndef CNOID_ASSIMP_SCENE_WRITER_H
#define CNOID_ASSIMP_SCENE_WRITER_H

#include <cnoid/AbstractSceneWriter>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT AssimpSceneWriter : AbstractSceneWriter
{
public:
    AssimpSceneWriter();
    AssimpSceneWriter(const AssimpSceneWriter& org);
    ~AssimpSceneWriter();

    AssimpSceneWriter(AssimpSceneWriter&&) = delete;
    AssimpSceneWriter& operator=(const AssimpSceneWriter&) = delete;
    AssimpSceneWriter& operator=(AssimpSceneWriter&&) = delete;

    //void setMessageSink(std::ostream& os) override;
    virtual bool writeScene(const std::string& filename, SgNode* node) override;

    void setOutputType(const std::string& _type);
    const std::string &getOutputType();
protected:
    //bool findOrCopyImageFile(SgImage* image, const std::string& outputBaseDir);
    //std::ostream& os(){ return *os_; }

private:
    class Impl;
    Impl* impl;
};

};

#endif

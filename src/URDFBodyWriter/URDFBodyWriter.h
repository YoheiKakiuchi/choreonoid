#ifndef CNOID_URDF_BODY_WRITER_URDF_BODY_WRITER_H
#define CNOID_URDF_BODY_WRITER_URDF_BODY_WRITER_H

#include <cnoid/Body>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT URDFBodyWriter
{
public:
    URDFBodyWriter();
    ~URDFBodyWriter();
    void setMessageSink(std::ostream& os);

    bool writeBody(Body* body, const std::string& filename);

private:
    class Impl;
    Impl* impl;
};

};  // namespace cnoid

#endif

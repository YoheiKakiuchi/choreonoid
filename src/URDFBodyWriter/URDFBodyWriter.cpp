#include "URDFBodyWriter.h"

using namespace cnoid;
//using fmt::format;
//namespace filesystem = cnoid::stdx::filesystem;

namespace cnoid {

class URDFBodyWriter::Impl
{
};

};  // namespace cnoid

URDFBodyWriter::URDFBodyWriter()
{
    impl = new Impl();
}
URDFBodyWriter::~URDFBodyWriter()
{
    delete impl;
}

void URDFBodyWriter::setMessageSink(std::ostream& os)
{
}

bool URDFBodyWriter::writeBody(Body* body, const std::string& filename)
{
    return true;
}

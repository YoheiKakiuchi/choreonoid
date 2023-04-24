#include "URDFBodyWriter.h"

#include <cnoid/NullOut>

#include <pugixml.hpp>

#include <set>

using namespace cnoid;
//using fmt::format;
//namespace filesystem = cnoid::stdx::filesystem;

namespace cnoid {

class URDFBodyWriter::Impl
{
public:
    Impl();

    bool writeBody(Body *body, const std::string& fname);

    std::ostream* os_;
    std::ostream& os() { return *os_; }

    bool use_xacro;
    bool use_geometry;
    bool use_offset;

    std::string robot_name;
protected:
    void addXacroScripts(pugi::xml_node &_node);
    void addTransMission(pugi::xml_node &_node, const std::string &_joint_name, bool useXacro);

    void addGeometry(pugi::xml_node &_node, const std::string &_filee_name, bool isVisual);
    void addMassParam(pugi::xml_node &_node, const cnoid::Link *_lk);

    //void addLink
    //void addJoint
};

};  // namespace cnoid
URDFBodyWriter::Impl::Impl()
{
    os_ = &nullout();
    use_xacro = true;
    use_geometry = true;
    use_offset = true;
}
void URDFBodyWriter::Impl::addXacroScripts(pugi::xml_node &_node)
{
}
void URDFBodyWriter::Impl::addGeometry(pugi::xml_node &_node, const std::string &_filee_name, bool isVisual)
{
}
void URDFBodyWriter::Impl::addMassParam(pugi::xml_node &_node, const cnoid::Link *_lk)
{
}
void URDFBodyWriter::Impl::addTransMission(pugi::xml_node &_node, const std::string &_joint_name, bool useXacro)
{
}
bool URDFBodyWriter::Impl::writeBody(Body* body, const std::string& fname)
{
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("robot");

    root.append_attribute("name") = "robot_name";

    if (use_xacro) {
        addXacroScripts(root);
    }

    doc.save_file(fname.c_str());
    return true;
}
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
    impl->os_ = &os;
}
bool URDFBodyWriter::writeBody(Body* body, const std::string& filename)
{
    return impl->writeBody(body, filename);
}

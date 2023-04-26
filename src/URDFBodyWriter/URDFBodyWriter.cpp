#include "URDFBodyWriter.h"

#include <cnoid/NullOut>
#include <cnoid/MeshExtractor>
#include <cnoid/EigenUtil>

#include <pugixml.hpp>

#include <sstream>
#include <iomanip>
// #include <set>
using namespace cnoid;
//using fmt::format;
//namespace filesystem = cnoid::stdx::filesystem;

#define _SET_PRECISION(strm,prec_)                                      \
    int prc;                                                            \
    prc = strm.precision();                                             \
    strm << std::scientific << std::setprecision(prec_);                \

#define _UNSET_PRECISION(strm)                                          \
    strm << std::fixed << std::setprecision(prc);                       \

//// inline functions for printing
inline void _print_vector(Vector3 &_vec, std::ostream &ostrm)
{
    _SET_PRECISION(ostrm,12);
    ostrm << "\"";
    ostrm << _vec.x() << " ";
    ostrm << _vec.y() << " ";
    ostrm << _vec.z() << "\"";
    _UNSET_PRECISION(ostrm);
}

namespace cnoid {

class URDFBodyWriter::Impl
{
public:
    Impl();

    bool writeBody(Body* _body, const std::string& _fname);

    std::ostream* os_;
    std::ostream& os() { return *os_; }

    bool use_xacro;
    bool use_geometry;
    bool use_offset;

    std::string robot_name;
protected:
    void addXacroScripts(pugi::xml_node &_node);
    void addTransMission(pugi::xml_node &_node, const std::string &_joint_name, bool useXacro);

    void addGeometry(pugi::xml_node &_node, const cnoid::Link *_lk, bool isVisual);
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
void URDFBodyWriter::Impl::addGeometry(pugi::xml_node &_node, const cnoid::Link *_lk, bool isVisual)
{
}
void URDFBodyWriter::Impl::addMassParam(pugi::xml_node &_node, const cnoid::Link *_lk)
{
}
void URDFBodyWriter::Impl::addTransMission(pugi::xml_node &_node, const std::string &_joint_name, bool useXacro)
{
}
bool URDFBodyWriter::Impl::writeBody(Body* _body, const std::string& _fname)
{
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("robot");

    std::string robot_name = _body->modelName();
    root.append_attribute("name") = robot_name.c_str();

    if (use_xacro) {
        addXacroScripts(root);
    }

    for(int i = 0; i < _body->numLinks(); i++) {
        cnoid::Link *cur_lk = _body->link(i);
        pugi::xml_node link_node =  root.append_child("link");
        link_node.append_attribute("name") = cur_lk->name().c_str();
        if (use_geometry) {
            pugi::xml_node vis_node = link_node.append_child("visual");
            addGeometry(vis_node, cur_lk, true);
            pugi::xml_node col_node = link_node.append_child("collision");
            addGeometry(col_node, cur_lk, false);
        }

        addMassParam(link_node, cur_lk);

        if(cur_lk->isRoot()) {
            continue;
        }

        std::string _jtype = "fixed";
        switch(cur_lk->jointType()) {
        case cnoid::Link::RevoluteJoint:
            _jtype = "revolute";
            if(cur_lk->q_upper() > 1e16) {
                _jtype = "continuous";
            }
            break;
        case cnoid::Link::PrismaticJoint:
            _jtype = "prismatic";
            break;
        case cnoid::Link::FreeJoint:
            _jtype = "floating";
            break;
            // _jtype = "planer"
        }

        pugi::xml_node joint_node =  root.append_child("joint");
        joint_node.append_attribute("name") = cur_lk->jointName().c_str();
        joint_node.append_attribute("type") = _jtype.c_str();

        cnoid::Link *plk_ = cur_lk->parent();
        if(!!plk_) {
            //pugi::xml_node pt_node = joint_node.append_child("parent");
            //pt_node.append_attribute("link") = plk_->name();
            joint_node.append_child("parent").append_attribute("link") = plk_->name().c_str();
        }
        //pugi::xml_node clk_node = joint_node.append_child("child");
        //pt_node.append_attribute("link") = plk_->name();
        joint_node.append_child("child").append_attribute("link") = cur_lk->name().c_str();

        if(use_offset) {
            Isometry3::TranslationPart ltrans(cur_lk->translation());
            Isometry3::TranslationPart ptrans(plk_->translation());
            Matrix3 lrot(cur_lk->rotation());
            Matrix3 prot(plk_->rotation());

            Matrix3 prot_inv = prot.transpose();
            Matrix3 trot = prot_inv * lrot;
            Vector3 _xyz = prot_inv * (ltrans - ptrans);
            Vector3 _rpy = cnoid::rpyFromRot(trot);

            pugi::xml_node o_node = joint_node.append_child("origin");
            //ostrm << "<origin xyz=";
            //_print_vector(_xyz, ostrm);
            //ostrm << " rpy=";
            //_print_vector(_rpy, ostrm);
            //ostrm << "/>" << std::endl;
        }

        if(!cur_lk->isFixedJoint()) {
            if(use_offset) {
                Vector3 ax_ = cur_lk->a();
                pugi::xml_node ax_node = joint_node.append_child("axis");
                //_indent(4, ostrm);
                //ostrm << "<axis xyz=";
                //_print_vector(ax_, ostrm);
                //ostrm << "/>" << std::endl;
            }

            pugi::xml_node lm_node = joint_node.append_child("limit");
            // _indent(4, ostrm);
            // ostrm << "<limit lower=";
            // _print_float(lk->q_lower(), ostrm);
            // ostrm << " upper=";
            // _print_float(lk->q_upper(), ostrm);
            // ostrm << " velocity=";
            // _print_float(lk->dq_upper(), ostrm);
            // ostrm << " effort=";
            // _print_float(lk->u_upper(), ostrm);
            // ostrm << "/>" << std::endl;
        }
    }
    doc.save_file(_fname.c_str());
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

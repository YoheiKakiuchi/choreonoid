#include "URDFBodyWriter.h"

#include <cnoid/EigenUtil>
#include <cnoid/NullOut>
#include <cnoid/SceneDrawables>
#include <cnoid/MeshExtractor>
#include <cnoid/AssimpSceneWriter>

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
    std::string mesh_file_prefix;
protected:
    void addXacroScripts(pugi::xml_node &_node);
    void addTransmission(pugi::xml_node &_node, const std::string &_joint_name);

    void addGeometry(pugi::xml_node &_node, const cnoid::Link *_lk, bool isVisual);
    void addMassParam(pugi::xml_node &_node, const cnoid::Link *_lk);
    // void addMaterial(pugi::xml_node &_node, xxxx);
    void extractPrimitiveGeometry(MeshExtractor &_me, pugi::xml_node &_node, bool isVisual);
};

};  // namespace cnoid
URDFBodyWriter::Impl::Impl()
{
    os_ = &nullout();
    use_xacro = true;
    use_geometry = true;
    use_offset = true;
}
void URDFBodyWriter::Impl::addXacroScripts(pugi::xml_node &_root)
{
    _root.append_attribute("xmlns:xi"        ) = "http://www.w3.org/2001/Xinclude";
    _root.append_attribute("xmlns:gazebo"    ) = "http://playerstage.sourceforge.net/gazebo/xmlschema/#gz";
    _root.append_attribute("xmlns:model"     ) = "http://playerstage.sourceforge.net/gazebo/xmlschema/#model";
    _root.append_attribute("xmlns:sensor"    ) = "http://playerstage.sourceforge.net/gazebo/xmlschema/#sensor";
    _root.append_attribute("xmlns:body"      ) = "http://playerstage.sourceforge.net/gazebo/xmlschema/#body";
    _root.append_attribute("xmlns:geom"      ) = "http://playerstage.sourceforge.net/gazebo/xmlschema/#geom";
    _root.append_attribute("xmlns:joint"     ) = "http://playerstage.sourceforge.net/gazebo/xmlschema/#joint";
    _root.append_attribute("xmlns:interface" ) = "http://playerstage.sourceforge.net/gazebo/xmlschema/#interface";
    _root.append_attribute("xmlns:rendering" ) = "http://playerstage.sourceforge.net/gazebo/xmlschema/#rendering";
    _root.append_attribute("xmlns:renderable") = "http://playerstage.sourceforge.net/gazebo/xmlschema/#renderable";
    _root.append_attribute("xmlns:controller") = "http://playerstage.sourceforge.net/gazebo/xmlschema/#controller";
    _root.append_attribute("xmlns:physics"   ) = "http://playerstage.sourceforge.net/gazebo/xmlschema/#physics";
    _root.append_attribute("xmlns:xacro"     ) = "http://www.ros.org/wiki/xacro";

    {
    pugi::xml_node xac = _root.append_child("xacro:macro");
    xac.append_attribute("name") = "gz_trans_pos";
    xac.append_attribute("params") = "joint_name";
    pugi::xml_node trans = xac.append_child("transmission");
    trans.append_attribute("name") = "${joint_name}_trans";
    trans.append_child("type")
         .append_child(pugi::node_pcdata)
         .set_value("transmission_interface/SimpleTransmission");
    pugi::xml_node t_joint = trans.append_child("joint");
    t_joint.append_attribute("name") = "${joint_name}";
    t_joint.append_child("hardwareInterface")
           .append_child(pugi::node_pcdata)
           .set_value("hardware_interface/PositionJointInterface</hardwareInterface");
    pugi::xml_node t_act   = trans.append_child("actuator");
    t_act.append_attribute("name") = "${joint_name}_motor";
    t_act.append_child("hardwareInterface")
         .append_child(pugi::node_pcdata)
         .set_value("hardware_interface/PositionJointInterface</hardwareInterface");
    }

    {
    pugi::xml_node xac = _root.append_child("xacro:macro");
    xac.append_attribute("name") = "gz_trans_vel";
    xac.append_attribute("params") = "joint_name";
    pugi::xml_node trans = xac.append_child("transmission");
    trans.append_attribute("name") = "${joint_name}_trans";
    trans.append_child("type")
         .append_child(pugi::node_pcdata)
         .set_value("transmission_interface/SimpleTransmission");
    pugi::xml_node t_joint = trans.append_child("joint");
    t_joint.append_attribute("name") = "${joint_name}";
    t_joint.append_child("hardwareInterface")
           .append_child(pugi::node_pcdata)
           .set_value("hardware_interface/VelocityJointInterface</hardwareInterface");
    pugi::xml_node t_act   = trans.append_child("actuator");
    t_act.append_attribute("name") = "${joint_name}_motor";
    t_act.append_child("hardwareInterface")
         .append_child(pugi::node_pcdata)
         .set_value("hardware_interface/VelocityJointInterface</hardwareInterface");
    }

    {
    pugi::xml_node xac = _root.append_child("xacro:macro");
    xac.append_attribute("name") = "gz_trans_eff";
    xac.append_attribute("params") = "joint_name";
    pugi::xml_node trans = xac.append_child("transmission");
    trans.append_attribute("name") = "${joint_name}_trans";
    trans.append_child("type")
         .append_child(pugi::node_pcdata)
         .set_value("transmission_interface/SimpleTransmission");
    pugi::xml_node t_joint = trans.append_child("joint");
    t_joint.append_attribute("name") = "${joint_name}";
    t_joint.append_child("hardwareInterface")
           .append_child(pugi::node_pcdata)
           .set_value("hardware_interface/EffortJointInterface</hardwareInterface");
    pugi::xml_node t_act   = trans.append_child("actuator");
    t_act.append_attribute("name") = "${joint_name}_motor";
    t_act.append_child("hardwareInterface")
         .append_child(pugi::node_pcdata)
         .set_value("hardware_interface/EffortJointInterface</hardwareInterface");
    }
}
void URDFBodyWriter::Impl::addGeometry(pugi::xml_node &_node, const cnoid::Link *_lk, bool isVisual)
{
    if(isVisual) {
        MeshExtractor me_;
        me_.extract(_lk->visualShape(), [this, &me_, &_node](){ extractPrimitiveGeometry(me_, _node, true); });

        AssimpSceneWriter asw;
        bool res_ =  asw.writeScene("/tmp/hoge.stl", _lk->visualShape());
        if (res_) {

        }
    } else {
        MeshExtractor me_;
        me_.extract(_lk->collisionShape(), [this, &me_, &_node](){ extractPrimitiveGeometry(me_, _node, false); });

        AssimpSceneWriter asw;
        bool res_ =  asw.writeScene("/tmp/hoge.stl", _lk->collisionShape());
        if (res_) {

        }
    }
}
void URDFBodyWriter::Impl::extractPrimitiveGeometry(MeshExtractor &_me, pugi::xml_node &_node, bool isVisual)
{
    SgMesh* mesh = _me.currentMesh();
    if(mesh->primitiveType() != SgMesh::BOX &&
       mesh->primitiveType() != SgMesh::CYLINDER &&
       mesh->primitiveType() != SgMesh::SPHERE ) {
        return;
    }
    SgShape* shape = _me.currentShape();
    const Affine3& T = _me.currentTransform();
    const Isometry3 &Ti = _me.currentTransformWithoutScaling();
#if 0
        //origin
    {
        Vector3 trs_ = Ti.translation();
        Matrix3 mat_ = Ti.linear();
        Vector3 rpy_ = cnoid::rpyFromRot(mat_);
        strm << "<origin xyz="; _print_vector(trs_, strm);
        strm << " rpy="; _print_vector(rpy_, strm);
        strm << "/>";
    }
    switch(mesh->primitiveType()) {
    case SgMesh::BOX:
    {
        SgMesh::Box bx = mesh->primitive<SgMesh::Box>();
        //bx.size
        strm << "<geometry><box size=";
        _print_vector(bx.size, strm);
        strm << "/></geometry>" << std::endl;
        // material
    }
    break;
    case SgMesh::CYLINDER:
    {
        SgMesh::Cylinder cyl = mesh->primitive<SgMesh::Cylinder>();
        //cyl.radius;
        //cyl.height
        //strm << "<origin />";
        strm << "<geometry><cylinder radius=";
        _print_float(cyl.radius, strm);
        strm << " height=";
        _print_float(cyl.height, strm);
        strm << "/></geometry>" << std::endl;
        // material
    }
    break;
    case SgMesh::SPHERE:
    {
        SgMesh::Sphere sph = mesh->primitive<SgMesh::Sphere>();
        //sph.radius
        //strm << "<origin />";
        strm << "<geometry><shpere radius=";
        _print_float(sph.radius, strm);
        strm << "/></geometry>" << std::endl;
        // material
    }
    break;
    }
#endif
}
void URDFBodyWriter::Impl::addMassParam(pugi::xml_node &_node, const cnoid::Link *_lk)
{
    Vector3 com_ = _lk->centerOfMass();
    Matrix3 Iner_ = _lk->I();
    pugi::xml_node i_node = _node.append_child("inertial");

    i_node.append_child("mass").append_attribute("value") = _lk->mass();

    std::ostringstream oss;
    _print_vector(com_, oss);
    i_node.append_child("origin").append_attribute("xyz") = oss.str().c_str();

    pugi::xml_node in_node = i_node.append_child("inertia");
    in_node.append_attribute("ixx") = Iner_(0,0);
    in_node.append_attribute("ixy") = Iner_(0,1);
    in_node.append_attribute("ixz") = Iner_(0,2);
    in_node.append_attribute("iyy") = Iner_(1,1);
    in_node.append_attribute("iyz") = Iner_(1,2);
    in_node.append_attribute("izz") = Iner_(2,2);
}
void URDFBodyWriter::Impl::addTransmission(pugi::xml_node &_root, const std::string &_joint_name)
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
            Vector3 xyz_ = prot_inv * (ltrans - ptrans);
            Vector3 rpy_ = cnoid::rpyFromRot(trot);

            pugi::xml_node o_node = joint_node.append_child("origin");
            {
                std::ostringstream oss;
                _print_vector(xyz_, oss);
                o_node.append_attribute("xyz") = oss.str().c_str();
            }
            {
                std::ostringstream oss;
                _print_vector(rpy_, oss);
                o_node.append_attribute("rpy") = oss.str().c_str();
            }
        }

        if(!cur_lk->isFixedJoint()) {
            if(use_offset) {
                Vector3 ax_ = cur_lk->a();
                pugi::xml_node ax_node = joint_node.append_child("axis");
                std::ostringstream oss;
                _print_vector(ax_, oss);
                ax_node.append_attribute("xyz") = oss.str().c_str();
            }

            pugi::xml_node lm_node = joint_node.append_child("limit");
            lm_node.append_attribute("lower") = cur_lk->q_lower();
            lm_node.append_attribute("upper") = cur_lk->q_upper();
            lm_node.append_attribute("velocity") = cur_lk->dq_upper();
            lm_node.append_attribute("effort")   = cur_lk->u_upper();

            addTransmission(root, cur_lk->jointName());
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

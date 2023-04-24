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
bool URDFBodyWriter::Impl::writeBody(Body* _body, const std::string& _fname)
{
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("robot");
    
    std::string robot_name = _body->modelName();
    root.append_attribute("name") = robot_name;

    if (use_xacro) {
        addXacroScripts(root);
    }

    for(int i = 0; i < _body->numLinks(); i++) {
        cnoid::Link *cur_lk = _body->link(i);
	pugi::xml_node link_node =  root.append_child("link");
	link_node.append_attribute("name") = cur_lk->name();
	if (use_geometry) {
	   // extract-mesh
	   pugi::xml_node vis_node = link_node.append_child("visual");
	   // extract-geom
	   pugi::xml_node col_node = link_node.append_child("collision");
	}

	addMassParam(link_node, cur_lk);

	if(lk->isRoot()) {
	  continue;
	}

	std::string _jtype = "fixed";
        switch(lk->jointType()) {
        case cnoid::Link::RevoluteJoint:
            _jtype = "revolute";
            if(lk->q_upper() > 1e16) {
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
	joint_node.append_attribute("name") = cur_lk->jointName();
	joint_node.append_attribute("type") = _jtype;

	cnoid::Link *plk_ = cur_lk->parent();
        if(!!plk_) {
  	  //pugi::xml_node pt_node = joint_node.append_child("parent");
	  //pt_node.append_attribute("link") = plk_->name();
	  joint_node.append_child("parent").append_attribute("link") = plk_->name();
        }
	//pugi::xml_node clk_node = joint_node.append_child("child");
	//pt_node.append_attribute("link") = plk_->name();
	joint_node.append_child("child").append_attribute("link") = cur_lk->name();

	if(use_offset) {
            Isometry3::TranslationPart ltrans(lk->translation());
            Isometry3::TranslationPart ptrans(plk->translation());
            Matrix3 lrot(lk->rotation());
            Matrix3 prot(plk->rotation());

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

	if(!lk->isFixedJoint()) {
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

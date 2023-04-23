#include <pugixml.hpp>

#include <iostream>

int main(int argc, char **argv)
{
    pugi::xml_document doc;

    pugi::xml_node root = doc.append_child("robot");

    {
        pugi::xml_node nd = root.append_child("desc");
        nd.append_attribute("name") = "at_name";
        nd.append_attribute("val(int)") = 1;
        nd.append_attribute("val(float)") = 1.234567890123456789;
    }

    doc.save_file("/tmp/hoge.urdf");

    return 0;
}

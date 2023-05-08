/*!
  @author Yohei Kakiuchi
*/

#include <cnoid/PyUtil>
#include <cnoid/PyEigenTypes>

#include <cnoid/URDFBodyLoader>
#include <cnoid/URDFBodyWriter>

using namespace cnoid;
namespace py = pybind11;

PYBIND11_MODULE(URDFPlugin, m)
{
    m.doc() = "Choreonoid URDF Utility module";

    py::module::import("cnoid.Body");

    py::class_<URDFBodyWriter>(m, "URDFBodyWriter")
        .def(py::init<>())
        .def("writeBody", &URDFBodyWriter::writeBody)
        //.def("setVerbose", &AssimpSceneWriter::setVerbose)
        .def("setUseXacro", &URDFBodyWriter::setUseXacro)
        .def("setAddGeometry", &URDFBodyWriter::setAddGeometry)
        .def("setAddOffset", &URDFBodyWriter::setAddOffset)
        .def("setRobotName", &URDFBodyWriter::setRobotName)
        .def("setMeshFilePrefix", &URDFBodyWriter::setMeshFilePrefix)
        .def("setMeshURLPrefix", &URDFBodyWriter::setMeshURLPrefix)
        //.def("setMessageSinkStdErr", &AssimpSceneWriter::setMessageSinkStdErr)
        ;

}

#include "PyAssimp.h"

#include "../AssimpSceneLoader.h"

using namespace cnoid;
namespace py = pybind11;

PYBIND11_MODULE(Assimp, m)
{
  m.doc() = "AssimpSceneLoader module";

  py::module::import("cnoid.Util");

  py::class_<AssimpSceneLoader>(m, "AssimpSceneLoader")
    .def(py::init<>())
    .def("load", &AssimpSceneLoader::load)
    ;
}

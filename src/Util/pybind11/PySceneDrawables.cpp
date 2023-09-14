#include "PyUtil.h"
#include "../SceneDrawables.h"
#include "../CloneMap.h"

using namespace cnoid;
namespace py = pybind11;

namespace cnoid {

void exportPySceneDrawables(py::module& m)
{
    py::class_<SgMaterial, SgMaterialPtr, SgObject>(m, "SgMaterial")
        .def(py::init<>())
        .def_property("ambientIntensity", &SgMaterial::ambientIntensity, &SgMaterial::setAmbientIntensity)
        .def("setAmbientIntensity", &SgMaterial::setAmbientIntensity)
        .def_property("diffuseColor",
                      &SgMaterial::diffuseColor,
                      [](SgMaterial& self, const Vector3f c){ self.setDiffuseColor(c); })
        .def("setDiffuseColor", [](SgMaterial& self, const Vector3f c){ self.setDiffuseColor(c); })
        .def_property("emissiveColor",
                      &SgMaterial::emissiveColor,
                      [](SgMaterial& self, const Vector3f c){ self.setEmissiveColor(c); })
        .def("setEmissiveColor", [](SgMaterial& self, const Vector3f c){ self.setEmissiveColor(c); })
        .def_property("specularExponent", &SgMaterial::specularExponent, &SgMaterial::specularExponent)
        .def("setSpecularExponent", &SgMaterial::setSpecularExponent)
        .def_property("specularColor",
                      &SgMaterial::specularColor,
                      [](SgMaterial& self, const Vector3f c){ self.setSpecularColor(c); })
        .def("setSpecularColor", [](SgMaterial& self, const Vector3f c){ self.setSpecularColor(c); })
        .def_property("transparency", &SgMaterial::transparency, &SgMaterial::setTransparency)
        .def("setTransparency", &SgMaterial::setTransparency)
        ;

    py::class_<SgMeshBase, SgMeshBasePtr, SgObject>(m, "SgMeshBase")
        .def("boundingBox", [](SgMeshBase &self) { BoundingBox ret = self.boundingBox(); return ret; })
        .def("updateBoundingBox", &SgMeshBase::updateBoundingBox)
        .def("hasVertices", &SgMeshBase::hasVertices)
        .def("vertices", [](SgMeshBase &self) {
             SgVertexArray *va_ = self.vertices();
             Eigen::Matrix<float, -1, -1, Eigen::RowMajor> rm(va_->size(), 3);
             int cntr = 0;
             for(auto it = va_->begin(); it != va_->end(); it++, cntr++) rm.row(cntr) = *it;
             return rm; })
        .def("hasNormals", &SgMeshBase::hasNormals)
        .def("normals", [](SgMeshBase &self) {
             SgNormalArray *nm_ = self.normals();
             Eigen::Matrix<float, -1, -1, Eigen::RowMajor> rm(nm_->size(), 3);
             int cntr = 0;
             for(auto it = nm_->begin(); it != nm_->end(); it++, cntr++) rm.row(cntr) = *it;
             return rm; })
        .def("hasColors", &SgMeshBase::hasColors)
        .def("colors", [](SgMeshBase &self) {
             SgColorArray *col_ = self.colors();
             Eigen::Matrix<float, -1, -1, Eigen::RowMajor> rm(col_->size(), 3);
             int cntr = 0;
             for(auto it = col_->begin(); it != col_->end(); it++, cntr++) rm.row(cntr) = *it;
             return rm; })
        .def("hasTexCoords", &SgMeshBase::hasTexCoords)
        .def("texCoords", [](SgMeshBase &self) {
             SgTexCoordArray *tc_ = self.texCoords();
             Eigen::Matrix<float, -1, -1, Eigen::RowMajor> rm(tc_->size(), 2);
             int cntr = 0;
             for(auto it = tc_->begin(); it != tc_->end(); it++, cntr++) rm.row(cntr) = *it;
             return rm; })
        .def("hasFaceVertexIndices", &SgMeshBase::hasFaceVertexIndices)
        .def("faceVertexIndices", (SgIndexArray&(SgMeshBase::*)())&SgMeshBase::faceVertexIndices)
        //.def("faceVertexIndices", [](SgMeshBase &self) { SgIndexArray idx_ = self.faceVertexIndices(); return idx_; })
        .def("hasNormalIndices", &SgMeshBase::hasNormalIndices)
        .def("normalIndices", (SgIndexArray&(SgMeshBase::*)())&SgMeshBase::normalIndices)
        //.def("normalIndices", [](SgMeshBase &self) { SgIndexArray idx_ = self.normalIndices(); return idx_; })
        .def("hasColorIndices", &SgMeshBase::hasColorIndices)
        .def("colorIndices", (SgIndexArray&(SgMeshBase::*)())&SgMeshBase::colorIndices)
        //.def("colorIndices", [](SgMeshBase &self) { SgIndexArray idx_ = self.colorIndices(); return idx_; })
        .def("hasTexCoordIndices", &SgMeshBase::hasTexCoordIndices)
        .def("texCoordIndices", (SgIndexArray&(SgMeshBase::*)())&SgMeshBase::texCoordIndices)
        //.def("texCoordIndices", [](SgMeshBase &self) { SgIndexArray idx_ = self.texCoordIndices(); return idx_; })
        .def("creaseAngle", &SgMeshBase::creaseAngle)
        .def("isSolid", &SgMeshBase::isSolid)
        ;

    py::class_<SgMesh, SgMeshPtr, SgMeshBase> sgMesh(m, "SgMesh");

    py::enum_<SgMesh::PrimitiveType>(sgMesh, "PrimitiveType")
        .value("MeshType", SgMesh::MeshType)
        .value("BoxType", SgMesh::BoxType)
        .value("SphereType", SgMesh::SphereType)
        .value("CylinderType", SgMesh::CylinderType)
        .value("ConeType", SgMesh::ConeType)
        .value("CapsuleType", SgMesh::CapsuleType)
        .export_values();

    py::enum_<SgMesh::ExtraDivisionMode>(sgMesh, "ExtraDivisionMode")
        .value("ExtraDivisionPreferred", SgMesh::ExtraDivisionPreferred)
        .value("ExtraDivisionX", SgMesh::ExtraDivisionX)
        .value("ExtraDivisionY", SgMesh::ExtraDivisionY)
        .value("ExtraDivisionZ", SgMesh::ExtraDivisionZ)
        .value("ExtraDivisionAll", SgMesh::ExtraDivisionAll)
        .export_values();

    sgMesh
        .def(py::init<>())
        .def("updateBoundingBox", &SgMesh::updateBoundingBox)
        .def("triangleVertices", (SgIndexArray&(SgMesh::*)())&SgMesh::triangleVertices)
        //.def("triangleVertices", [](SgMesh &self) { SgIndexArray idx_ = self.triangleVertices(); return idx_; })
        .def("hasTriangles", &SgMesh::hasTriangles)
        .def("numTriangles", &SgMesh::numTriangles)
        .def("triangle", [](SgMesh &self, int index) { Array3i ret = self.triangle(index); return ret; })
        .def_property("divisionNumber", &SgMesh::divisionNumber, &SgMesh::setDivisionNumber)
        .def_property("extraDivisionNumber", &SgMesh::extraDivisionNumber, &SgMesh::setExtraDivisionNumber)
        .def_property("extraDivisionMode", &SgMesh::extraDivisionMode, &SgMesh::setExtraDivisionMode)
        .def_property_readonly("primitiveType", &SgMesh::primitiveType)
        .def_property_readonly("primitive", [](SgMesh  &self) {
            int tp = self.primitiveType();
            if (tp == 1) {
                SgMesh::Box *pri = new SgMesh::Box(self.primitive<SgMesh::Box>());
                return py::cast(pri);
            } else if (tp == 2) {
                SgMesh::Sphere *pri = new SgMesh::Sphere(self.primitive<SgMesh::Sphere>());
                return py::cast(pri);
            } else if (tp == 3) {
                SgMesh::Cylinder *pri = new SgMesh::Cylinder(self.primitive<SgMesh::Cylinder>());
                return py::cast(pri);
            } else if (tp == 4) {
                SgMesh::Cone *pri = new SgMesh::Cone(self.primitive<SgMesh::Cone>());
                return py::cast(pri);
            } else if (tp == 5) {
                SgMesh::Capsule *pri = new SgMesh::Capsule(self.primitive<SgMesh::Capsule>());
                return py::cast(pri);
            }
            return py::cast(nullptr); })
        ;


    py::class_<SgMesh::Box> (m, "SgMeshBox")
    .def_property_readonly("volume", [](SgMesh::Box &self) { return self.size[0] * self.size[1] * self.size[2]; })
    .def_property_readonly("COM", [](SgMesh::Box &self) { return Vector3::Zero(); })
    .def_property_readonly("inertia", [](SgMesh::Box &self) { Matrix3 ret = Matrix3::Zero();
            ret(0, 0) = (self.size[1] * self.size[1] + self.size[2] * self.size[2] ) / 12.0;
            ret(1, 1) = (self.size[0] * self.size[0] + self.size[2] * self.size[2] ) / 12.0;
            ret(2, 2) = (self.size[0] * self.size[0] + self.size[1] * self.size[2] ) / 12.0;
            return ret; })
    .def_property_readonly("size", [](SgMesh::Box &self) { return self.size; })
    ;
    py::class_<SgMesh::Sphere> (m, "SgMeshSphere")
    .def_property_readonly("volume", [](SgMesh::Sphere &self) { return 4.0 * M_PI * self.radius * self.radius * self.radius / 3.0; })
    .def_property_readonly("COM", [](SgMesh::Sphere &self) { return Vector3::Zero(); })
    .def_property_readonly("inertia", [](SgMesh::Sphere &self) { Matrix3 ret = Matrix3::Zero();
            ret(0, 0) = ret(1, 1) = ret(2, 2) = 2.0 * (self.radius * self.radius) / 5.0;
            return ret; })
    .def_property_readonly("radius", [](SgMesh::Sphere &self) { return self.radius; })
    ;
    // geometry circle on xz-plane / height: y-direction
    py::class_<SgMesh::Cylinder> (m, "SgMeshCylinder")
    .def_property_readonly("volume", [](SgMesh::Cylinder &self) { return M_PI * self.radius * self.radius * self.height; })
    .def_property_readonly("COM", [](SgMesh::Cylinder &self) { return Vector3::Zero(); })
    .def_property_readonly("inertia", [](SgMesh::Cylinder &self) { Matrix3 ret = Matrix3::Zero();
            ret(1, 1) = (self.radius * self.radius) / 2.0;
            ret(0, 0) = ret(2, 2) = ((self.radius * self.radius) / 4.0) + ((self.height * self.height) / 12.0);
            return ret; })
    .def_property_readonly("radius", [](SgMesh::Cylinder &self) { return self.radius; })
    .def_property_readonly("height", [](SgMesh::Cylinder &self) { return self.height; })
    ;
    // geometry circle on xz-plane (y = -height/2) / height: y-direction
    py::class_<SgMesh::Cone> (m, "SgMeshCone")
    .def_property_readonly("volume", [](SgMesh::Cone &self) { return M_PI * self.radius * self.radius * self.height / 3.0; })
    .def_property_readonly("COM", [](SgMesh::Cone &self) { Vector3 ret = Vector3::Zero(); ret[1] = - self.height/2.0; return ret; })
    .def_property_readonly("inertia", [](SgMesh::Cone &self) { Matrix3 ret = Matrix3::Zero();
            ret(1, 1) = 0.3 * (self.radius * self.radius);
            ret(0, 0) = ret(2, 2) = 0.15 * (self.radius * self.radius) + 3.0 / 80.0 * (self.height * self.height);
            return ret; })
    .def_property_readonly("radius", [](SgMesh::Cone &self) { return self.radius; })
    .def_property_readonly("height", [](SgMesh::Cone &self) { return self.height; })
    ;
    // geometry circle on xz-plane / height: y-direction
    py::class_<SgMesh::Capsule> (m, "SgMeshCapsule")
    .def_property_readonly("volume", [](SgMesh::Capsule &self) {
        // return (4.0 * M_PI * self.radius * self.radius * self.radius / 3.0) + (M_PI * self.radius * self.radius * self.height);
        return M_PI * self.radius * self.radius * ((4.0 * self.radius / 3.0) + self.height); })
    .def_property_readonly("COM", [](SgMesh::Capsule &self) { return Vector3::Zero(); })
    .def_property_readonly("inertia", [](SgMesh::Capsule &self) { Matrix3 ret = Matrix3::Zero();
            double ratio_cyl = self.height / ((4.0 * self.radius / 3.0) + self.height);
            double ratio_sph = 1 - ratio_cyl;
            double sph_i = ratio_sph * 2.0 * (self.radius * self.radius) / 5.0;
            ret(1, 1) = (ratio_cyl * (self.radius * self.radius) / 2.0) + sph_i;
            ret(0, 0) = ret(2, 2) = ratio_cyl * (((self.radius * self.radius) / 4.0) + ((self.height * self.height) / 12.0))
                                    + sph_i + ratio_sph * self.height * self.height / 4.0;
            return ret; })
    .def_property_readonly("radius", [](SgMesh::Capsule &self) { return self.radius; })
    .def_property_readonly("height", [](SgMesh::Capsule &self) { return self.height; })
    ;

    py::class_<SgShape, SgShapePtr, SgNode>(m, "SgShape")
        .def(py::init<>())
        .def_property("mesh", (SgMesh* (SgShape::*)()) &SgShape::mesh, &SgShape::setMesh)
        .def("setMesh", &SgShape::setMesh)
        .def("getOrCreateMesh", &SgShape::getOrCreateMesh)
        .def_property("material", (SgMaterial* (SgShape::*)()) &SgShape::material, &SgShape::setMaterial)
        .def("setMaterial", &SgShape::setMaterial)
        .def("getOrCreateMaterial", &SgShape::getOrCreateMaterial)
        ;

    ////
    ///py::class_<SgPlot, SgPlotPtr, SgNode>(m, "SgPlot")

}

}
#if 0
  CLASS_DECL - ['cnoid'] SgPlot
   CXX_METHOD - numChildObjects
   CXX_METHOD - childObject
   CXX_METHOD - boundingBox
   CXX_METHOD - untransformedBoundingBox
   CXX_METHOD - updateBoundingBox
   CXX_METHOD - clear

   CXX_METHOD - hasVertices
   CXX_METHOD - setVertices
   CXX_METHOD - vertices p_ro

   CXX_METHOD - setMaterial
   CXX_METHOD - material p_ro

   CXX_METHOD - hasColors
   CXX_METHOD - setColors
   CXX_METHOD - colors p_ro
   CXX_METHOD - colorIndices p?

   CXX_METHOD - hasNormals
   CXX_METHOD - setNormals
   CXX_METHOD - normals p_ro
   CXX_METHOD - normalIndices p?

  hasXXIndices
  indices-> IndicesSize p_ro
  indices-> replaceXXIndices(int pos, int idx)
---
   CXX_METHOD - getOrCreateVertices
   CXX_METHOD - getOrCreateMaterial
   CXX_METHOD - getOrCreateColors
   CXX_METHOD - getOrCreateNormals

  CLASS_DECL - ['cnoid'] SgPointSet
   CXX_METHOD - setPointSize ??
   CXX_METHOD - pointSize

  CLASS_DECL - ['cnoid'] SgLineSet
   CXX_METHOD - lineVertexIndices
   CXX_METHOD - numLines
   CXX_METHOD - setNumLines
   CXX_METHOD - reserveNumLines
   CXX_METHOD - clearLines

   CXX_METHOD - line
   CXX_METHOD - setLine
   CXX_METHOD - addLine
   CXX_METHOD - addLine

   CXX_METHOD - resizeColorIndicesForNumLines

   CXX_METHOD - setLineColor

   CXX_METHOD - setLineWidth
   CXX_METHOD - lineWidth


  CLASS_DECL - ['cnoid'] SgText
   CXX_METHOD - boundingBox
   CXX_METHOD - untransformedBoundingBox
   CXX_METHOD - text
   CXX_METHOD - setText
   CXX_METHOD - textHeight
   CXX_METHOD - setTextHeight
   CXX_METHOD - color

  CLASS_DECL - ['cnoid'] SgPolygonMesh
   CXX_METHOD - updateBoundingBox
   CXX_METHOD - polygonVertices

  CLASS_DECL - ['cnoid'] SgMesh
   CXX_METHOD - updateBoundingBox
   CXX_METHOD - triangleVertices
   CXX_METHOD - triangleVertices
   CXX_METHOD - hasTriangles
   CXX_METHOD - numTriangles
   CXX_METHOD - setNumTriangles
   CXX_METHOD - reserveNumTriangles
   CXX_METHOD - clearTriangles
   CXX_METHOD - triangle
   CXX_METHOD - triangle
   CXX_METHOD - setTriangle
   CXX_METHOD - newTriangle
   CXX_METHOD - addTriangle
   CXX_METHOD - addTriangles
   CXX_METHOD - addTriangle
   CXX_METHOD - primitiveType
   CXX_METHOD - setPrimitive
   CXX_METHOD - divisionNumber
   CXX_METHOD - setDivisionNumber
   CXX_METHOD - extraDivisionNumber
   CXX_METHOD - setExtraDivisionNumber
   CXX_METHOD - extraDivisionMode
   CXX_METHOD - setExtraDivisionMode
   CXX_METHOD - transform
   CXX_METHOD - transform
   CXX_METHOD - translate
   CXX_METHOD - rotate
   CXX_METHOD - doClone

  CLASS_DECL - ['cnoid'] SgMaterial
   CXX_METHOD - ambientIntensity
   CXX_METHOD - setAmbientIntensity
   CXX_METHOD - diffuseColor
   CXX_METHOD - emissiveColor
   CXX_METHOD - specularColor
   CXX_METHOD - specularExponent
   CXX_METHOD - setSpecularExponent
   CXX_METHOD - shininess
   CXX_METHOD - setShininess
   CXX_METHOD - transparency
   CXX_METHOD - setTransparency
   CXX_METHOD - doClone

  CLASS_DECL - ['cnoid'] SgShape
   CXX_METHOD - numChildObjects
   CXX_METHOD - childObject
   CXX_METHOD - boundingBox
   CXX_METHOD - untransformedBoundingBox
   CXX_METHOD - mesh
   CXX_METHOD - mesh
   CXX_METHOD - setMesh
   CXX_METHOD - getOrCreateMesh
   CXX_METHOD - material
   CXX_METHOD - material
   CXX_METHOD - setMaterial
   CXX_METHOD - getOrCreateMaterial
   CXX_METHOD - texture
   CXX_METHOD - texture
   CXX_METHOD - setTexture
   CXX_METHOD - getOrCreateTexture
   CXX_METHOD - doClone

  CLASS_DECL - ['cnoid'] SgMeshBase
   CXX_METHOD - numChildObjects
   CXX_METHOD - childObject
   CXX_METHOD - boundingBox
   CXX_METHOD - updateBoundingBox
   CXX_METHOD - setBoundingBox
   CXX_METHOD - setBoundingBox
   CXX_METHOD - hasVertices
   CXX_METHOD - vertices
   CXX_METHOD - vertices
   CXX_METHOD - setVertices
   CXX_METHOD - getOrCreateVertices
   CXX_METHOD - hasNormals
   CXX_METHOD - normals
   CXX_METHOD - normals
   CXX_METHOD - setNormals
   CXX_METHOD - getOrCreateNormals
   CXX_METHOD - hasColors
   CXX_METHOD - colors
   CXX_METHOD - colors
   CXX_METHOD - setColors
   CXX_METHOD - getOrCreateColors
   CXX_METHOD - hasTexCoords
   CXX_METHOD - texCoords
   CXX_METHOD - texCoords
   CXX_METHOD - setTexCoords
   CXX_METHOD - getOrCreateTexCoords
   CXX_METHOD - hasFaceVertexIndices
   CXX_METHOD - faceVertexIndices
   CXX_METHOD - faceVertexIndices
   CXX_METHOD - hasNormalIndices
   CXX_METHOD - normalIndices
   CXX_METHOD - normalIndices
   CXX_METHOD - hasColorIndices
   CXX_METHOD - colorIndices
   CXX_METHOD - colorIndices
   CXX_METHOD - hasTexCoordIndices
   CXX_METHOD - texCoordIndices
   CXX_METHOD - texCoordIndices
   CXX_METHOD - creaseAngle
   CXX_METHOD - setCreaseAngle
   CXX_METHOD - isSolid
   CXX_METHOD - setSolid

#endif

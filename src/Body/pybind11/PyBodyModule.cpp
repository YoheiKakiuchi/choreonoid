#include "../Body.h"
#include "../BodyLoader.h"
#include "../BodyMotion.h"
#include "../BodyStateSeq.h"
#include "../InverseKinematics.h"
#include "../JointPath.h"
#include "../LeggedBodyHelper.h"
#include "../StdBodyWriter.h"
#include <cnoid/PyUtil>
#include <pybind11/operators.h>
#include <iostream>

using namespace std;
using namespace cnoid;
namespace py = pybind11;

namespace {

using Matrix4RM = Eigen::Matrix<double, 4, 4, Eigen::RowMajor>;

}

namespace cnoid {

void exportPyBody(py::module& m);
void exportPyLink(py::module& m);
void exportPyDeviceTypes(py::module& m);
void exportPyMaterial(py::module& m);

}

PYBIND11_MODULE(Body, m)
{
    m.doc() = "Choreonoid Body module";

    py::module::import("cnoid.Util");

    exportPyBody(m);
    exportPyLink(m);
    exportPyDeviceTypes(m);
    exportPyMaterial(m);

    py::class_<AbstractBodyLoader>(m, "AbstractBodyLoader")
        .def("setVerbose", &AbstractBodyLoader::setVerbose)
        .def("setShapeLoadingEnabled", &AbstractBodyLoader::setShapeLoadingEnabled)
        .def("setDefaultDivisionNumber", &AbstractBodyLoader::setDefaultDivisionNumber)
        .def("setDefaultCreaseAngle", &AbstractBodyLoader::setDefaultCreaseAngle)
        .def("load", &AbstractBodyLoader::load)
        .def("setMessageSinkStdErr", [](AbstractBodyLoader &self) {
            self.setMessageSink(std::cerr);
        })
        ;

    py::class_<BodyLoader, AbstractBodyLoader>(m, "BodyLoader")
        .def(py::init<>())
        .def("load", (Body*(BodyLoader::*)(const string&))&BodyLoader::load)
        .def("lastActualBodyLoader", &BodyLoader::lastActualBodyLoader)
        ;

    py::class_<JointPath, shared_ptr<JointPath>>(m, "JointPath")
        .def(py::init<>())
        .def_static("getCustomPath",
                    [](Link* baseLink, Link* endLink){ return JointPath::getCustomPath(baseLink, endLink); })
        .def_property_readonly("empty", &JointPath::empty)
        .def_property_readonly("size", &JointPath::size)
        .def_property_readonly("numJoints", &JointPath::numJoints)
        .def("joint", &JointPath::joint)
        .def_property_readonly("baseLink", &JointPath::baseLink)
        .def_property_readonly("endLink", &JointPath::endLink)
        .def("isJointDownward", &JointPath::isJointDownward)
        .def("calcForwardKinematics", &JointPath::calcForwardKinematics,
             py::arg("calcVelocity") = false, py::arg("calcAcceleration") = false)
        .def("indexOf", &JointPath::indexOf)
        .def("isCustomIkDisabled", &JointPath::isCustomIkDisabled)
        .def("setCustomIkDisabled", &JointPath::setCustomIkDisabled)
        .def("isBestEffortIkMode", &JointPath::isBestEffortIkMode)
        .def("setBestEffortIkMode", &JointPath::setBestEffortIkMode)
        .def("setNumericalIkMaxIkError", &JointPath::setNumericalIkMaxIkError)
        .def("setNumericalIkDeltaScale", &JointPath::setNumericalIkDeltaScale)
        .def("setNumericalIkMaxIterations", &JointPath::setNumericalIkMaxIterations)
        .def("setNumericalIkDampingConstant", &JointPath::setNumericalIkDampingConstant)
        .def_property_readonly("numericalIkDefaultDeltaScale", &JointPath::numericalIkDefaultDeltaScale)
        .def_property_readonly("numericalIkDefaultMaxIterations", &JointPath::numericalIkDefaultMaxIterations)
        .def_property_readonly("numericalIkDefaultMaxIkError", &JointPath::numericalIkDefaultMaxIkError)
        .def_property_readonly("numericalIkDefaultDampingConstant", &JointPath::numericalIkDefaultDampingConstant)
        .def("customizeTarget", &JointPath::customizeTarget)
        .def("calcInverseKinematics", (bool(JointPath::*)())&JointPath::calcInverseKinematics)
        .def("setBaseLinkGoal",
             [](JointPath& self, Eigen::Ref<Matrix4RM> T) -> JointPath& { return self.setBaseLinkGoal(Isometry3(T)); })
        .def("calcInverseKinematics",
             [](JointPath& self, Eigen::Ref<const Matrix4RM> T){ return self.calcInverseKinematics(Isometry3(T)); })
        .def("calcRemainingPartForwardKinematicsForInverseKinematics",
             &JointPath::calcRemainingPartForwardKinematicsForInverseKinematics)
        .def_property_readonly("numIterations", &JointPath::numIterations)
        .def("hasCustomIK", &JointPath::hasCustomIK)
        .def_property("name", &JointPath::name, &JointPath::setName)
        .def("setName", &JointPath::setName)

        // deprecated
        .def_static("getCustomPath",
                    [](Body*, Link* baseLink, Link* endLink){ return JointPath::getCustomPath(baseLink, endLink); })
        .def("getNumJoints", &JointPath::numJoints)
        .def("getJoint", &JointPath::joint)
        .def("getBaseLink", &JointPath::baseLink)
        .def("getEndLink", &JointPath::endLink)
        .def("getIndexOf", &JointPath::indexOf)
        .def("getNumIterations", &JointPath::numIterations)
        ;

    // deprecated
    m.def("getCustomJointPath",
          [](Body*, Link* baseLink, Link* endLink){ return JointPath::getCustomPath(baseLink, endLink); });

    py::class_<BodyMotion, shared_ptr<BodyMotion>> bodyMotion(m, "BodyMotion");
    bodyMotion
        .def("cloneSeq", &BodyMotion::cloneSeq)
        .def_property("frameRate", &BodyMotion::frameRate, &BodyMotion::setFrameRate)
        .def_property_readonly("timeStep", &BodyMotion::timeStep)
        .def_property("offsetTime", &BodyMotion::offsetTime, &BodyMotion::setOffsetTime)
        .def_property("numFrames", &BodyMotion::numFrames, &BodyMotion::setNumFrames)
        .def_property_readonly("stateSeq", [](BodyMotion& self){ return self.stateSeq(); })
        .def("getFrame", [](BodyMotion& self, int f){ return self.frame(f); })
        .def("load", [](BodyMotion& self, const std::string& filename){ return self.load(filename); })
        .def("save", [](BodyMotion& self, const std::string& filename){ return self.save(filename); })
        
        // AbstractSeq members
        .def("getFrameRate",&BodyMotion::frameRate)
        .def("setFrameRate", &BodyMotion::setFrameRate)
        .def("getOffsetTimeFrame", &BodyMotion::getOffsetTimeFrame)
        .def("getNumFrames", &BodyMotion::numFrames)
        .def("setNumFrames", &BodyMotion::setNumFrames)

        // AbstractMultiSeq members
        .def("setNumParts", &BodyMotion::setNumJoints)
        .def("getNumParts", &BodyMotion::numJoints)

        // deprecated
        .def_property_readonly("positionSeq", [](BodyMotion& self){ return self.stateSeq(); })
        .def_property("numJoints", &BodyMotion::numJoints, &BodyMotion::setNumJoints)
        .def("getNumJoints", &BodyMotion::numJoints)
        .def("setNumJoints", &BodyMotion::setNumJoints)
        .def_property_readonly("numLinks", &BodyMotion::numLinks)
        .def("getNumLinks", &BodyMotion::numLinks)
        .def_property_readonly("jointPosSeq", [](BodyMotion& self){ return self.jointPosSeq(); })
        .def("getJointPosSeq", [](BodyMotion& self){ return self.jointPosSeq(); })
        .def_property_readonly("linkPosSeq", [](BodyMotion& self){ return self.linkPosSeq(); })
        .def("getLinkPosSeq", [](BodyMotion& self){ return self.linkPosSeq(); })
        .def("frame", [](BodyMotion& self, int f){ return self.frame(f); })
        ;

    py::class_<BodyMotion::Frame>(m, "Frame")
        .def("frame", &BodyMotion::Frame::frame)
        .def(py::self << Body())
        .def(Body() >> py::self)

        // deprecated
        .def("getFrame", &BodyMotion::Frame::frame)
        ;

    py::class_<BodyStateSeq, shared_ptr<BodyStateSeq>>(m, "BodyStateSeq")
        .def_property_readonly("numLinkPositionsHint", &BodyStateSeq::numLinkPositionsHint)
        .def("setNumLinkPositionsHint", &BodyStateSeq::setNumLinkPositionsHint)
        .def_property_readonly("numJointDisplacementsHint", &BodyStateSeq::numJointDisplacementsHint)
        .def("setNumJointDisplacementsHint", &BodyStateSeq::setNumJointDisplacementsHint)
        ;

    py::class_<LeggedBodyHelper>(m, "LeggedBodyHelper")
        .def(py::init<>())
        .def(py::init<Body*>())
        .def_property_readonly("numFeet", &LeggedBodyHelper::numFeet)
        .def("footLink", &LeggedBodyHelper::footLink)
        ;

    py::class_<StdBodyWriter>(m, "StdBodyWriter")
    .def(py::init<>())
    .def("writeBody", &StdBodyWriter::writeBody)
    .def("setMessageSinkStdErr", [](StdBodyWriter &self) {
            self.setMessageSink(std::cerr);
        })
    .def("setExtModelFileMode", &StdBodyWriter::setExtModelFileMode)
    .def("extModelFileMode", &StdBodyWriter::extModelFileMode)
    .def("setOriginalShapeExtModelFileUriRewritingEnabled", &StdBodyWriter::setOriginalShapeExtModelFileUriRewritingEnabled)
    .def("isOriginalShapeExtModelFileUriRewritingEnabled", &StdBodyWriter::isOriginalShapeExtModelFileUriRewritingEnabled)
    .def("setOriginalBaseDirectory", &StdBodyWriter::setOriginalBaseDirectory)
    .def("setTransformIntegrationEnabled", &StdBodyWriter::setTransformIntegrationEnabled)
    .def("isTransformIntegrationEnabled", &StdBodyWriter::isTransformIntegrationEnabled)
    ;
}

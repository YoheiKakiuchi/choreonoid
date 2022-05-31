from cnoid.Base import *
from cnoid.BodyPlugin import *
from cnoid.PythonSimScriptPlugin import *
import cnoid.Body

import numpy as np

def getItemTreeView():
    if callable(ItemTreeView.instance):
        return ItemTreeView.instance()
    else:
        return ItemTreeView.instance


def getRootItem():
    if callable(RootItem.instance):
        return RootItem.instance()
    else:
        return RootItem.instance


def getWorld():
    rI = getRootItem()
    ret = rI.findItem("World")
    if ret == None:
        ret = WorldItem()
        rI.addChildItem(ret)
        getItemTreeView().checkItem(ret)
    return ret


def cnoidPosition(R = None, trans = None):
  ret = np.identity(4)
  if not (R is None):
    ret[:3, :3] = R
  if not (trans is None):
    ret[:3, 3] = trans
  return ret


def cnoidRotation(cPosition):
  return cPosition[:3, :3]


def cnoidTranslation(cPosition):
  return cPosition[:3, 3]


# fl='/home/leus/src/rtmros_choreonoid/jvrc_models/JAXON_JVRC/JAXON_JVRCmain.wrl'
def loadRobot(fname, name = None):
    bodyItem = BodyItem()
    bodyItem.load(fname)
    if name:
        bodyItem.setName(name)
    if callable(bodyItem.body):
        robot = bodyItem.body()
    else:
        robot = bodyItem.body
    robot.calcForwardKinematics()
    bodyItem.storeInitialState()##
    world = getWorld()
    if callable(world.childItem):
        world.insertChildItem(bodyItem, world.childItem())
    else:
        world.insertChildItem(bodyItem, world.childItem)
    getItemTreeView().checkItem(bodyItem)
    return robot


def findRobot(name):
    ret = getRootItem().findItem(name)
    ## ret (bodyItem)
    if callable(ret.body):
        return ret.body()
    else:
        return ret.body

robot = loadRobot('/home/leus/src/rtmros_choreonoid/jvrc_models/JAXON_JVRC/JAXON_JVRCmain.wrl', 'MODEL_JAXON')


for idx in range(robot.getNumJoints()):
    l = robot.getLink(idx)
    print('%d: %s'%(idx, l.getName()))


j0 = robot.getLink('LARM_JOINT0')
j1 = robot.getLink('LARM_JOINT1')
rt = robot.getLink('CHEST_JOINT2')
ed = robot.getLink('LARM_JOINT7')
joint_path = cnoid.Body.getCustomJointPath(robot, robot.getLink('CHEST_JOINT2'), robot.getLink('LARM_JOINT7'))
r_item  = getRootItem().findItem('MODEL_JAXON')


for idx in range(joint_path.numJoints()):
    j = joint_path.getJoint(idx)
    q = (j.q_upper + j.q_lower)*0.5
    print(q)
    j.q = q


## update View in choreonoid
robot.calcForwardKinematics()
r_item.notifyKinematicStateChange()
mview = MessageView.getInstance()


import time
for i in range(100):
    joint_path.calcInverseKinematics(ed.getPosition()[:3, 3] + np.array([0, -0.002, 0.000]), ed.getPosition()[:3,:3])
    robot.calcForwardKinematics()
    r_item.notifyKinematicStateChange()
    mview.flush()
    time.sleep(0.05)

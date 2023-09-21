## cnoid.Util
import cnoid.Util as cutil
## cnoid.Body
import cnoid.Body as cbody
from cnoid.Body import Body
from cnoid.Body import Link
from cnoid.Body import Device

## IRSL (not base)
from cnoid.IRSLCoords import coordinates
import cnoid.IRSLCoords as IC
from irsl_choreonoid.draw_coords import GeneralDrawInterfaceWrapped as DrawInterface
## from irsl_choreonoid.draw_coords import DrawCoordsListWrapped as DrawCoords
import irsl_choreonoid.make_shapes as mkshapes
import irsl_choreonoid.cnoid_util as iu
import irsl_choreonoid.robot_util as ru
## from irsl_choreonoid.robot_util import RobotModelWrapped as RobotModel
## etc
import numpy as np
from numpy import array as npa
from numpy.linalg import norm
import math
from math import pi as PI
##
if iu.isInChoreonoid():
    ## in base
    import irsl_choreonoid.cnoid_base as ib
    import cnoid.Base as cbase
    import cnoid.BodyPlugin as BodyPlugin

class MassParam(object):
    def __init__(self, shape, coords):
        ## shape is primitive or mesh(wo primitive)
        self.volume = 0.0
        self.mass  = 0.0
        self.COM   = npa([0, 0, 0])
        self.unit_inertia = npa([[0, 0, 0],[0, 0, 0],[0, 0, 0]])
        self.density = None
        self.coords = coords
        self.shape  = shape

        if hasattr(shape, 'volume'):
            self.volume = shape.volume
        if hasattr(shape, 'COM'):
            self.COM = shape.COM
        if hasattr(shape, 'inertia'):
            self.unit_inertia = shape.inertia

    def setDensity(self, density=1.0):
        if self.volume != 0.0:
            self.mass = density * self.volume
            self.density = density

    def setMass(self, mass):
        self.mass = mass
        if self.volume != 0.0:
            self.density = self.volume/mass

    @property
    def Inertia(self):
        return self.mass * self.unit_inertia

def __removeNode(gnode, target):
    if gnode.isGroupNode():
        for idx in range(gnode.numChildren):
            ch = gnode.getChild(idx)
            if ch is target:
                gnode.remoteChildAt(idx)
                return True
            if __revemoNode(ch, target):
                return True
    return False

def __searchSceneGraph(gnode, name, currentCoords=None):
    res = []
    if gnode.name==name:
        res += [(gnode, currentCoords)]
    if gnode.isGroupNode():
        if isinstance(gnode, cutil.SgPosTransform):
            if currentCoords is None:
                currentCoords = coordinates(gnode.T)
            else:
                currentCoords = currentCoords.copy().transform(coordinates(gnode.T))
        for idx in range(gnode.numChildren):
            ch = gnode.getChild(idx)
            res += __searchSceneGraph(ch, name, currentCoords)
    return res

def __traverseSceneGraph(gnode, currentCoords=None, use_primitive=True, excludes=[]):
    ## extract shape and coords
    ## print('call {} / {}'.format(gnode, currentCoords))
    res = []
    if gnode.name in excludes:
        return res
    if gnode.isGroupNode():
        if isinstance(gnode, cutil.SgPosTransform):
            if currentCoords is None:
                currentCoords = coordinates(gnode.T)
            else:
                currentCoords = currentCoords.copy().transform(coordinates(gnode.T))
        for idx in range(gnode.numChildren):
            ch = gnode.getChild(idx)
            res += __traverseSceneGraph(ch, currentCoords, use_primitive=use_primitive)
    elif isinstance(gnode, cutil.SgShape):
        if use_primitive:
            m = gnode.mesh
            if m is not None:
                pri = m.primitive
                if pri is None:
                    pri = m
            res = [ MassParam(pri, currentCoords) ]
        else:
            res = [(gnode, currentCoords)]
    return res

def __mergeResults(result, newcoords=coordinates(), mass=None, density=None):
    if mass is None and density is None:
        raise Exception('set weight or density')
    ##
    total_volume = 0.0
    for masspara in result:
        total_volume += masspara.volume
    ##
    total_mass = 0.0
    for masspara in result:
        if mass is not None:
            masspara.setMass(mass * masspara.volume/total_volume)
        else:
            masspara.setDensity(density)
        total_mass += masspara.mass
    ##
    newCOM_w = npa([0., 0., 0.])
    for masspara in result:
        newCOM_w += masspara.mass * masspara.coords.transformVector(masspara.COM)
    newCOM_w /= total_mass
    ##
    newIner_w = npa([[0., 0., 0.],[0., 0., 0.],[0., 0., 0.]])
    for masspara in result:
        trans = newCOM_w - masspara.coords.transformVector(masspara.COM)
        i_trans = masspara.mass * npa([[trans[0] * trans[0], 0, 0], [0, trans[1] * trans[1], 0], [0, 0, trans[2] * trans[2]]])
        rot = masspara.coords.rot
        i_rot = np.transpose(rot).dot(masspara.Inertia).dot(rot)
        newIner_w += (i_trans + i_rot)

    res = {}
    res['mass'] = total_mass
    res['volume'] = total_volume
    res['COM'] = newCOM_w
    res['inertia'] = newIner_w

    return res

## add inertia on shape
class RobotBuilder(object):
    def __init__(self, robot=None, gui=True): ## item
        self.bodyItem = None
        self.__di = None
        if gui and iu.isInChoreonoid():
            self.__di=DrawInterface()
            if isinstance(robot, BodyPlugin.BodyItem):
                self.bodyItem = robot
            elif type(robot) is str:
                self.bodyItem = ib.loadRobotItem(robot, world=False, addItem=False)
            else:
                self.bodyItem=BodyPlugin.BodyItem()
            self.bodyItem.setName('BodyBuilder')
            cbase.RootItem.instance.addChildItem(self.bodyItem)
            cbase.ItemTreeView.instance.checkItem(self.bodyItem)
        if self.bodyItem is not None:
            self.body = self.bodyItem.body
        else:
            if isinstance(robot, Body):
                self.body = robot
            elif type(robot) is str:
                self.robot = iu.loadRobot(robot)
            else:
                self.body = Body()
        self.created_links = []
### start: GUI wrapper
    @property
    def draw(self):
        return self.__di

    def hideRobot(self):
        if self.__di is not None:
            cbase.ItemTreeView.instance.checkItem(self.bodyItem, False)

    def drawRobot(self):
        if self.__di is not None:
            cbase.ItemTreeView.instance.checkItem(self.bodyItem)
            self.notifyUpdate()

    def clear(self):
        if self.__di is not None:
            self.__di.clear()

    def addShape(self, shape):
        if self.__di is not None:
            self.__di.addObject(shape)

    def createLinkFromDrawn(self, mass=None, density=1.0, name=None, parentLnk=None, root=False, clear=True, **kwargs):
        if self.__di is None:
            return
        if name is None:
            name='LINK_{}'.format(len(self.created_links))

        groot=self.__di.target
        res = __searchSceneGraph(groot, 'joint_root')
        if len(res) == 0:
            jtype='fixed'
            jaxis=cutil.UnitZ
            cds_w_j = coordinates(self__di.target.T)
        else:
            if len(res) > 1:
                ##warn
                pass
            res=res[0]
            ###
            jax=res[0].getChild(0)
            jtype=jax.name
            jtype=jtype.split(':')[1]
            jaxis=coordinates(jax.T).y_axis
            ###
            cds = coordinates(jroot.T)
            if res[1] is None:
                cds_w_j = cds
            else:
                cds_w_j = coordinates(res[1]).transform(cds)
            __removeChild(groot, res[0])

        if jtype=='fixed':
            jtype=Link.JointType.FixedJoint
        elif jtype=='free':
            jtype=Link.JointType.FreeJoint
        elif jtype=='revolute':
            jtype=Link.JointType.RevoluteJoint
        elif jtype=='prismatic':
            jtype=Link.JointType.PrismaticJoint
        else:
            jtype=Link.JointType.FixedJoint
        cds_offset = cds_w_j.inverseTransformation()
        ##link.visual <= shapes(org:joint_root)
        groot.setPosition(cds_offset.cnoidPosition)
        res = __traverseSceneGraph(groot, exclude=['joint_root', 'COM_root', 'inertia_root', 'joint_axis'])
        if mass is not None:
            info=__mergeResults(res, mass=mass)
        else:
            info=__mergeResults(res, density=density)
        ##link.axis (joint_axis)
        lk=self.createLink(name=name, mass=info['mass'], COM=info['COM'], inertia=info['inertia'],
                           shape=groot, JointType=jtype, JointAxis=jaxis)
        ##link.T <= joint_root
        lk.setPosition(cds_w_j.cnoidPosition)
        if parentLink is not None:
            self.appendLink(parentLink, lk)
            if clear:
                self.clear()
        elif root:
            self.setRootLink(lk)
            if clear:
                self.clear()
        return lk

    def viewInfo(self, **kwargs):
        res = []
        for lk in self.body.links:
            vis=self.createVisualizedLinkShape(lk, **kwargs)
            res.append(vis)
            self.addShape(vis)
        return res
### end: GUI wrapper
    def notifyUpdate(self):
        self.body.updateLinkTree()
        self.body.calcForwardKinematics()
        if self.bodyItem is not None:
            self.bodyItem.notifyModelUpdate(sum([int(i) for i in (BodyPlugin.BodyItem.ModelUpdateFlag.LinkSetUpdate, BodyPlugin.BodyItem.ModelUpdateFlag.LinkSpecUpdate,
                                                                  BodyPlugin.BodyItem.ModelUpdateFlag.DeviceSetUpdate, BodyPlugin.BodyItem.ModelUpdateFlag.DeviceSpecUpdate,
                                                                  BodyPlugin.BodyItem.ModelUpdateFlag.ShapeUpdate)]))
            self.bodyItem.notifyKinematicStateUpdate()

    def createJointShape(self, jointType=Link.JointType.FreeJoint, wrapped=True, coords=None, add=True, scale=0.3, **kwargs):
        kwargs['scale']=scale
        tp='fixed'
        if jointType == Link.JointType.FreeJoint:
            sh=self.__freeJointShape(**kwargs)
            tp='free'
        elif jointType == Link.JointType.FixedJoint:
            sh=self.__fixedJointShape(**kwargs)
        elif jointType == Link.JointType.RevoluteJoint:
            sh=self.__revoluteJointShape(**kwargs)
            tp='revolute'
        elif jt == Link.JointType.PrismaticJoint:
            sh=self.__prismaticJointShape(**kwargs)
            tp='prismatic'
        else:
            pass
        sh.setName('joint_axis:{}'.format(tp)) ## just rotate
        trs=cutil.SgPosTransform()
        trs.setName('joint_root')
        trs.addChild(sh)
        if coords is not None:
            trs.setPosition(coords.cnoidPosition)
        if wrapped:
            trs = mkshapes.coordsWrapper(trs)
        if add:
            self.addShape(trs)
        return trs
#OffsetPosition
#JointType
#JointId
#JointName
#JointAxis
#InitialJointAngle
#InitialJointDisplacement
#JointRange
#JointVelocityRange
#JointEffortRange
#EquivalentRotorInertia
    def createLink(self, name='', mass=0.1, COM=None, density=None, inertia=None, shape=None, visual=None, collision=None, **kwargs):
        lk = self.createLinkBase(name=name, mass=mass, COM=COM, density=density, inertia=inertia, shape=shape, visual=visual, collision=collision)
        for k, v in kwargs.items():
            exec('lk.set{}(v)'.format(k))
        return lk

    def createLinkBase(self, name='', mass=0.1, density=None, COM=None, inertia=None, shape=None, visual=None, collision=None, **kwargs):
        baselk = self.body.createLink()
        baselk.setName(name)
        baselk.setMass(mass)
        if COM is None:
            baselk.setCenterOfMass(npa([0., 0., 0.]))
        else:
            baselk.setCenterOfMass(npa(COM))
        if inertia is None:
            baselk.setInertia(npa([[1., 0., 0.], [0., 1., 0.], [0., 0., 1.]]))
        else:
            baselk.setInertia(npa(inertia))
        if shape is not None:
            baselk.addShapeNode(shape)
        if visual is not None:
            baselk.addVisualShapeNode(visual)
        if collision is not None:
            baselk.addCollisionShapeNode(collision)
        self.created_links.append(baselk)
        return baselk

    def createRootLink(self, **kwargs):
        lk=self.createLinkBase(**kwargs)
        lk.setJointType(Link.JointType.FreeJoint)
        self.body.setRootLink(lk)
        self.notifyUpdate()
        return lk

    def updateMassParameterAsUniformDensity(self, alink, density=None, shape='visual'):
        if shape == 'visual':
            sh=alink.visual
        else:
            sh=alink.collision
        res = __traverseSceneGraph(sh)
        if density is not None:
            info=__mergeResults(res, density=density)
            alink.setMass(info['mass'])
        else:
            info=__mergeResults(res, mass=alink.mass)
        alink.setCenterOfMass(info['COM'])
        alink.setInertia(info['inertia'])

    def setRootLink(self, link):
        self.body.setRootLink(link)
        self.notifyUpdate()

    def appendLink(self, link0, link1=None, offset=None):
        if link1 is None:
            link1 = link0
            link0 = self.body.rootLink
        if offset is None:
            p_cds = link0.getCoords()
            c_cds = link1.getCoords()
            offset=p_cds.transformation(c_cds)
        child.setOffsetPosition(offset.cnoidPosition)
        link0.appendChild(link1)
        self.notifyUpdate()

    def removeLink(self, link):
        ## not implemented yet
        self.notifyUpdate()

    def resetRobot(self):
        ## not implemented yet
        if self.bodyItem is not None:
            pass

    ### start: link visualization
    def __addShape(self, alink, shape):
        sh=alink.shape
        if type(sh) is cutil.SgGroup:
            sh.addChild(shape)
        else:
            cur=cutil.SgGroup()
            cur.setName('visual_root')
            if sh is not None:
                cur.addChild(sh)
            cur.addChild(shape)
            alink.setShape(cur)

    def addCOMShape(self, alink, scale=None, color=[1,1,0], transparent=0.7):
        mass=alink.mass
        COM=alink.centerOfMass
        ## equivalant radius of sphere of iron with the same mass
        rr = math.pow((mass/8000 * 3)/(4*PI), 1.0/3)
        sh=mkshapes.makeSphere(rr, color=color, transparent=transparent)
        r2=rr*2
        ls=cutil.SgLineSet()
        ls.lineWidth=4
        ls.setVertices(npa([[-r2,0,0],[r2,0,0],[0,-r2,0],[0,r2,0],[0,0,-r2],[0,0,r2]], dtype='float32'))
        ls.addLine(0,1)
        ls.addLine(2,3)
        ls.addLine(4,5)
        ls.setColors(npa([color], dtype='float32'))
        ls.resizeColorIndicesForNumLines(ls.numLines)
        ls.setLineColor(0, 0)
        ls.setLineColor(1, 0)
        ls.setLineColor(2, 0)
        sh.target.addChild(ls)
        sh.translate(COM)
        sh.target.setName('COM_root')
        self.__addShape(alink, sh.target)

    def addInertiaShape(self, alink, color=[1,0,1], transparent=0.5, useBox=False):
        inertia, mat = np.linalg.eig(alink.I)
        mass=alink.mass
        rx=mat[:,0]
        ry=mat[:,1]
        rz=mat[:,2]
        if math.fabs(np.dot(rx, ry)) >  0.001:
            ry = np.cross(rz, rx)
        elif math.fabs(np.dot(rx, rz)) >  0.001:
            rz = np.cross(rx, ry)
        rot=np.column_stack([rx, ry, rz])
        if useBox:
            sh=mkshapes.makeBox(1.0, 1.0, 1.0, color=color, transparent=transparent, wrapped=False)
            vscale=npa([  math.pow(6.0/mass * ( -inertia[0] + inertia[1] + inertia[2] ), 0.5),
                          math.pow(6.0/mass * (  inertia[0] - inertia[1] + inertia[2] ), 0.5),
                          math.pow(6.0/mass * (  inertia[0] + inertia[1] - inertia[2] ), 0.5) ])
        else:
            vscale=npa([  math.pow(2.5/mass * ( -inertia[0] + inertia[1] + inertia[2] ), 0.5),
                          math.pow(2.5/mass * (  inertia[0] - inertia[1] + inertia[2] ), 0.5),
                          math.pow(2.5/mass * (  inertia[0] + inertia[1] - inertia[2] ), 0.5) ])
            sh=mkshapes.makeSphere(1.0, color=color, transparent=transparent, wrapped=False)
        scl = cutil.SgScaleTransform(vscale)
        trs = cutil.SgPosTransform()
        scl.addChild(sh)
        trs.addChild(scl)
        cds=coordinates(alink.centerOfMass, rot)
        trs.setPosition(cds.cnoidPosition)
        trs.setName('inertia_root')
        self.__addShape(alink, trs)

    def addJointShape(self, alink, **kwargs):
        jt=alink.jointType
        if jt == Link.JointType.FreeJoint:
            sh=self.__freeJointShape(**kwargs)
        elif jt == Link.JointType.FixedJoint:
            sh=self.__fixedJointShape(**kwargs)
        elif jt == Link.JointType.RevoluteJoint:
            sh=self.__revoluteJointShape(**kwargs)
            ### align y-axis to alink.axis
            v1 = alink.jointAxis / norm(alink.jointAxis)
            v0 = np.cross(v1, cutil.UnitY)
            if norm(v0) < 0.5:
                v0 = np.cross(v1, cutil.UnitZ)
            v0 /= norm(v0)
            v2 = np.cross(v0, v1)
            rot=np.column_stack([v0, v1, v2])
            sh.setPosition(coordinates(rot).cnoidPosition)
        elif jt == Link.JointType.PrismaticJoint:
            sh=self.__prismaticJointShape(**kwargs)
            ### align y-axis to alink.axis
            v1 = alink.jointAxis / norm(alink.jointAxis)
            v0 = np.cross(v1, cutil.UnitY)
            if norm(v0) < 0.5:
                v0 = np.cross(v1, cutil.UnitZ)
            v0 /= norm(v0)
            v2 = np.cross(v0, v1)
            rot=np.column_stack([v0, v1, v2])
            sh.setPosition(coordinates(rot).cnoidPosition)
        else:
            print('JointType: {} is not implemtented yet'.format(jt))
            return
        sh.setName('joint_axis') ## just rotate
        self.__addShape(alink, sh)

    def __revoluteJointShape(self, scale=None, color=[0,1,1], transparent=0.6):
        ## +y-axis ## TODO : rotate direction
        RR = 0.5
        rr = 0.05
        r0 = 0.2
        r1 = 0.3
        l0 = 0.5
        l1 = 0.3
        bd0 = mkshapes.makeTorus(RR-rr, rr, color=color, transparent=transparent)
        bd1 = mkshapes.makeCylinder(r0, l0, color=color, transparent=transparent)
        bd1.translate(npa([0,l0/2,0]))
        bd2 = mkshapes.makeCone(r1, l1, color=color, transparent=transparent)
        bd2.translate(npa([0,l0+l1/2,0]))
        ##
        res=cutil.SgPosTransform()
        if scale is not None:
            current = cutil.SgScaleTransform(scale)
            res.addChild(current)
        else:
            current = res
        current.addChild(bd0.target)
        current.addChild(bd1.target)
        current.addChild(bd2.target)
        return res

    def __prismaticJointShape(self, scale=None, color=[0,1,1], transparent=0.6):
        w0= 0.4
        l0= 0.7
        w1= 0.3
        l1= 0.4
        bd0 = mkshapes.makeBox(w0, l0, w0, color=color, transparent=transparent)
        bd1 = mkshapes.makeBox(w1, l1, w1, color=color, transparent=transparent)
        bd0.translate(npa([0, -l0/2, 0]))
        bd1.translate(npa([0,  l1/2, 0]))
        bd2 = mkshapes.makeCone(w1, w1, color=color, transparent=transparent, DivisionNumber=4)
        bd2.rotate(PI/2, cutil.UnitY)
        bd2.translate(npa([0, l1+w1/2, 0]))
        res=cutil.SgPosTransform()
        if scale is not None:
            current = cutil.SgScaleTransform(scale)
            res.addChild(current)
        else:
            current = res
        current.addChild(bd0.target)
        current.addChild(bd1.target)
        current.addChild(bd2.target)
        return res

    def __freeJointShape(self, scale=None, color=None, transparent=0.6):
        R0=0.15
        L0=0.5
        rr=0.25
        ll=0.3
        if color is None:
            col = [0, 1, 0]
        else:
            col = color
        bd0 = mkshapes.makeCylinder(R0, L0, color=col, transparent=transparent)
        bd0.translate(npa([0,L0/2,0]))
        a0 = mkshapes.makeCone(rr, ll, color=col, transparent=transparent)
        a0.translate(npa([0,L0+ll/2,0]))
        ##
        if color is None:
            col = [0, 0, 1]
        else:
            col = color
        bd1 = mkshapes.makeCylinder(R0, L0, color=col, transparent=transparent)
        bd1.rotate(PI/2, cutil.UnitX)
        bd1.translate(npa([0,L0/2,0]))
        a1 = mkshapes.makeCone(rr, ll, color=col, transparent=transparent)
        a1.rotate(PI/2, cutil.UnitX)
        a1.translate(npa([0,L0+ll/2,0]))
        ##
        if color is None:
            col = [1, 0, 0]
        else:
            col = color
        bd2 = mkshapes.makeCylinder(R0, L0, color=col, transparent=transparent)
        bd2.rotate(-PI/2, cutil.UnitZ)
        bd2.translate(npa([0,L0/2,0]))
        a2 = mkshapes.makeCone(rr, ll, color=col, transparent=transparent)
        a2.rotate(-PI/2, cutil.UnitZ)
        a2.translate(npa([0,L0+ll/2,0]))
        ##
        res=cutil.SgPosTransform()
        if scale is not None:
            current = cutil.SgScaleTransform(scale)
            res.addChild(current)
        else:
            current = res
        current.addChild(bd0.target)
        current.addChild(bd1.target)
        current.addChild(bd2.target)
        current.addChild(a0.target)
        current.addChild(a1.target)
        current.addChild(a2.target)
        return res

    def __fixedJointShape(self, scale=None, color=None, transparent=0.6):
        RR=0.2
        L0=0.7
        if color is None:
            col = [0, 1, 0]
        else:
            col = color
        bd0 = mkshapes.makeBox(RR, L0, RR, color=col, transparent=transparent)
        bd0.translate(npa([0, L0/2, 0]))
        if color is None:
            col = [0, 0, 1]
        else:
            col = color
        bd1 = mkshapes.makeBox(RR, RR, L0, color=col, transparent=transparent)
        bd1.translate(npa([0, 0, L0/2]))
        if color is None:
            col = [1, 0, 0]
        else:
            col = color
        bd2 = mkshapes.makeBox(L0, RR, RR, color=col, transparent=transparent)
        bd2.translate(npa([L0/2, 0, 0]))
        ##
        res=cutil.SgPosTransform()
        if scale is not None:
            current = cutil.SgScaleTransform(scale)
            res.addChild(current)
        else:
            current = res
        current.addChild(bd0.target)
        current.addChild(bd1.target)
        current.addChild(bd2.target)
        return res

    def addDeviceShape(self, alink):
        ## TODO / not implemented yet ##
        pass

    def createVisualizedLinkShape(self, alink, wrapped=True, addCOM=True, addInertia=True, addJoint=True, noLinkVisual=False, useCollision=False, useInertiaBox=False):
        ## SgTransform(linkPos) / SgTransform(shapeBase)
        cloned_lk = self.body.createLink(alink)
        if addCOM:
            self.addCOMShape(cloned_lk)
        if addInertia:
            self.addInertiaShape(cloned_lk, useBox=useInertiaBox)
        if addJoint:
            self.addJointShape(cloned_lk, scale=0.1)
        linkorg = cutil.SgPosTransform()
        linkorg.setName('linkPos')
        sh = cloned_lk.shape
        if sh is not None:
            linkorg.addChild(sh)
        if wrapped:
            res = mkshapes.coordsWrapper(linkorg)
            res.newcoords(cloned_lk.getCoords())
        else:
            res = linkorg
            linkorg.setPosition(cloned_lk.T)
        return res
    ### end: link visualization
    def writeBody(self, fname, modelName=None, **kwargs):
        bw = cbody.StdBodyWriter()
        bw.setMessageSinkStdErr()
        return bw.writeBody(self.body, fname)
    ### start: mkshapes wrapper
    def loadScene(self, fname, wrapped=True, add=True, **kwargs):
        res = mkshapes.loadScene(fname, wrapped, **kwargs)
        if add:
            self.addShape(res)
        return res
    def loadMesh(self, fname, wrapped=True, add=True, **kwargs):
        res = mkshapes.loadMesh(fname, wrapped, **kwargs)
        if add:
            self.addShape(res)
        return res
    def makeBox(self, x, y = None, z = None, wrapped=True, add=True, **kwargs):
        res = mkshapes.makeBox(x,y,z,wrapped,**kwargs)
        if add:
            self.addShape(res)
        return res
    def makeCylinder(self, radius, height, wrapped=True, add=True, **kwargs):
        res = mkshapes.makeCylinder(radius, height, wrapped, **kwargs)
        if add:
            self.addShape(res)
        return res
    def makeSphere(self, radius, wrapped=True, add=True, **kwargs):
        res = mkshapes.makeSphere(radius, wrapped, **kwargs)
        if add:
            self.addShape(res)
        return res
    def makeCone(self, radius, height, wrapped=True, add=True, **kwargs):
        res = mkshapes.makeCone(radius, height, wrapped, **kwargs)
        if add:
            self.addShape(res)
        return res
    def makeCapsule(self, radius, height, wrapped=True, add=True, **kwargs):
        res = mkshapes.makeCapsule(radius, height, wrapped, **kwargs)
        if add:
            self.addShape(res)
        return res
    def makeTorus(self, radius, corssSectionRadius, beginAngle = None, endAngle = None, wrapped=True, add=True, **kwargs):
        res = mkshapes.makeTorus(radius, corssSectionRadius, beginAngle, endAngle, wrapped, **kwargs)
        if add:
            self.addShape(res)
        return res
    def makeExtrusion(self, crossSection, spine, wrapped=True, add=True, **kwargs):
        ## crossSection, spine, orientation=None, scale=None, creaseAngle=None, beginCap=None, endCap=None
        param=mkshapes.makeExtrusionParam(crossSection, spine, **kwargs)
        res = mkshapes.makeExtrusion(param, wrapped, **kwargs)
        if add:
            self.addShape(res)
        return res
    def makeElevationGrid(self, xDimension, zDimension, xSpacing, zSpacing, height, wrapped=True, add=True, **kwargs):
        ## xDimension, zDimension, xSpacing, zSpacing, height, ccw=None, creaseAngle=None
        param=mkshapes.makeElevationParam(xDimension, zDimension, xSpacing, zSpacing, height, **kwargs)
        res = mkshapes.makeElevationGrid(param, wrapped, **kwargs)
        if add:
            self.addShape(res)
        return res
    def make3DAxis(self, coords=None, add=True, **kwargs):
        res = mkshapes.make3DAxis(coords, **kwargs)
        if add:
            self.addShape(res)
        return res
    def make3DAxisBox(self, coords=None, add=True, **kwargs):
        res = mkshapes.make3DAxisBox(coords, **kwargs)
        if add:
            self.addShape(res)
        return res
    def makeCoords(self, coords=None, add=True, **kwargs):
        res = mkshapes.makeCoords(coords, **kwargs)
        if add:
            self.addShape(res)
        return res
    def makeCross(self, coords=None, add=True, **kwargs):
        res = mkshapes.makeCross(coords, **kwargs)
        if add:
            self.addShape(res)
        return res
    ### end: mkshapes wrapper

def writeSingleLinkBody(fname, **kwargs):
    bc = BodyCreator()
    bc.createRootLink(**kwargs)
    bc.writeBody(fname)

#exec(open('/home/irsl/sandbox/choreonoid_ws/install/share/irsl_choreonoid/sample/irsl_import.py').read())

## add inertia on shape
class BodyCreator(object):
    def __init__(self): ## item
        self.body = Body()
        self.links = []

    def notifyUpdate(self):
        ## TODO
        pass

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
        self.links.append(baselk)
        return baselk

    def calcMassParamAsUniformDensity(self, alink, shape='visual'):
        pass

    def createRootLink(self, **kwargs):
        lk=self.createLinkBase(**kwargs)
        lk.setJointType(Link.JointType.FreeJoint)
        self.body.setRootLink(lk)
        self.body.updateLinkTree()
        return lk

    def writeBody(self, fname, modelName=None):
        bw = cbody.StdBodyWriter()
        bw.setMessageSinkStdErr()
        return bw.writeBody(self.body, fname)

    def appendLink(self, parent, child, offset=None):
        p_cds = parent.getCoords()
        c_cds = child.getCoords()
        parent.appendChild(child)
        child.setOffsetPosition(offset.cnoidPosition)
        self.body.updateLinkTree()

    def addCOMShape(self, alink, scale=None, color=[1,0,1], transparent=0.6):
        ##cube
        ##cross
        pass

    def addInertiaShape(self, alink, color=[1,0,1], transparent=0.6):
        inertia, mat = numpy.linalg.eig(alink.I)
        rx=mat[:,0]
        ry=mat[:,1]
        rz=mat[:,2]
        if math.fabs(numpy.dot(rx, ry)) >  0.001:
            ry = numpy.cross(rz, rx)
        elif math.fabs(numpy.dot(rx, rz)) >  0.001:
            rz = numpy.cross(rx, ry)
        ###
        ##5*inertia[0] = bb + cc
        ##5*inertia

    def addJointShape(self, alink, **kwargs):
        pass
    def addDeviceShape(self, alink):
        ## TODO ##
        pass

    def __revoluteJointShape(self, scale=None, color=[1,0,1], transparent=0.6):
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
        if sale is not None:
            current = cutil.SgScale(scale)
            res.addChild(current)
        else:
            current = res
        current.addChild(bd0.target)
        current.addChild(bd1.target)
        current.addChild(bd2.target)
        return res

    def __prismaticJointShape(self, scale=None, color=[1,0,1], transparent=0.6):
        #l0=
        #l1=
        pass

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
        if sale is not None:
            current = cutil.SgScale(scale)
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
        RR=0.3
        L0=0.6
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
        if sale is not None:
            current = cutil.SgScale(scale)
            res.addChild(current)
        else:
            current = res
        current.addChild(bd0.target)
        current.addChild(bd1.target)
        current.addChild(bd2.target)
        return res

    def linkVisual(self, alink, wrapped=True):
        ## SgTransform(linkPos) / SgTransform(shapeBase)
        cloned_lk = self.body.createLink(alink)
        linkorg = SgPosTransform()
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
        # cloned_lk.visual
        # cloned_lk.shape
        # cloned_lk.collision
        # bd = Body()
        # bd.setRootLink(alink)
        # bd.updateLinkTree()
        ## alink.T
        ## bd.T = alink.T
        ## RootItem
        ## BodyItem

def writeSingleLinkBody(fname, **kwargs):
    bc = BodyCreator()
    bc.createRootLink(**kwargs)
    bc.writeBody(fname)

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

def __traverseSceneGraph(gnode, currentCoords=None, use_primitive=True):
    ## extract shape and coords
    res = []
    ## print('call {} / {}'.format(gnode, currentCoords))
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

## add equ
## add COM shape
## add Joint shape
## add Inertia shape
## add Device shape

## merge inertia
#### param
# new_cds
# mass0, com0, iner0, cds0
# mass1, com1, iner1, cds1
# massN, comN, inerN, cdsN
####
# total_mass = sigma[x = 0 -> N] { massx }
# total_com  = 1/total_mass * sigma [x = 0 -> N] { massx * comx(new_cds) }
#    where comx(new_cds) := transformation(cdsx->new_cds) * comx
# iner_total = sigma [ x = 0 -> N ] { inerx(new_cds) }
#    Tx = transformation(cdsx->new_cds)
#    XX where inerx(new_com) :=  Diagonal(Tx.trans^2 * massx,,) + Tx.rot * inerx * transpose(Tx.rot)
####
## equivalant volume
## setInertiaMatrix() uniformDensity
##
## bd = Body()
## rootlk = bd.createLink()
## 
## rootlk.setName('Root')
## rootlk.setMass(0.1)
## rootlk.setCenterOfMass(npa([0, 0, 0]))
## rootlk.setInertia(npa([[1, 0, 0], [0, 1, 0], [0, 0, 1]]))
## 
## cyl = mkshapes.makeCylinder(0.2, 0.6, color = [0, 0, 1], wrapped=False)
## rootlk.addShapeNode(cyl)
## # addVisualShapeNode
## # addCollisionShapeNode
## 
## rootlk.setJointType(Link.JointType.FreeJoint)
## bd.setRootLink(rootlk)
## bd.updateLinkTree()
## bd.setName('HOGE')
## lk0 = bd.createLink()
## lk0.setName('L0')
## lk0.setJointType(Link.JointType.RevoluteJoint)
## lk0.setJointName('J0')
## lk0.setJointId(0)
## lk0.setJointAxis(npa([1.0, 0, 0]))
## rootlk.appendChild(lk0)
## bd.updateLinkTree()
## # rootlk.setPosition()
## # rootlk.setOffsetPosition()
## ## lk.setJointId
## ## lk.setJointName
## ## lk.setJointAxis
## ## lk.setInitialJointAngle
## ## lk.setInitialJointDisplacement
## ## lk.setJointRange
## ## lk.setJointVelocityRange
## ## lk.setJointEffortRange
## ## lk.setEquivalentRotorInertia
#
#
#Position
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
#
#
## 
## bw = cbody.StdBodyWriter()
## bw.setMessageSinkStdErr()
## 
## bw.writeBody(bd, '/tmp/hoge.body')
##

##
# exec(open('/home/irsl/sandbox/choreonoid_ws/install/share/irsl_choreonoid/sample/irsl_import.py').read())
# bx0 = mkshapes.makeBox(0.3, 0.2, 0.5, color=[1,0,0])
# bx1 = mkshapes.makeBox(0.2, 0.2, 0.5, color=[0,1,0])
# bx2 = mkshapes.makeBox(0.5, 0.2, 0.2, color=[0,0,1])
# di = DrawInterface()
# di.addObject(bx0)
# di.addObject(bx1)
# di.addObject(bx2)
# #root = cutil.SgPosTransform()
# #root.addChild(bx0.target)
# #root.addChild(bx1.target)
# #root.addChild(bx2.target)
# exec(open('make_body.py').read())
# res = __traverseSceneGraph(di.SgPosTransform)
#

##
#exec(open('/home/irsl/sandbox/choreonoid_ws/install/share/irsl_choreonoid/sample/irsl_import.py').read())
#exec(open('make_body.py').read())
#bc = BodyCreator()
#rl = bc.createRootLink(name='Root')
#l0 = bc.createLink(name='l0', JointName='j0', JointId=0, JointType=Link.JointType.RevoluteJoint, JointAxis=npa([1.0, 0, 0]), OffsetPosition=coordinates(npa([1.0, 0, 0])).cnoidPosition)
#bc.appendLink(rl, l0)
#bc.writeBody('/tmp/test.body')

# mkshapes.coordsWrapper
# sh = mkshapes.makeSphere(0.5, color=[0.7,0.4,0.4], transparent=transparent, wrapped=False)
# tmp = sh.getChild(0)
# scl = cutil.SgScaleTransform(npa([1.0, 2.0, 0.5]))
# sh.clearChildren()
# scl.addChild(tmp)
# sh.addChild(scl)
# ww = mkshapes.coordsWrapper(sh)

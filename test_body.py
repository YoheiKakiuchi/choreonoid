exec(open('/home/irsl/sandbox/choreonoid_ws/install/share/irsl_choreonoid/sample/irsl_import.py').read())
# bd = Body()
# rootlk = bd.createLink()
# rootlk.setName('Root')
cyl = mkshapes.makeCylinder(0.2, 0.6, color = [0, 0, 1], wrapped=False)
box = mkshapes.makeBox(0.1, 0.2, 0.4, color = [1, 0, 0], wrapped=False)
#rootlk.addShapeNode(cyl)
#bd.setRootLink(rootlk)
#bd.updateLinkTree()

bi = BodyPlugin.BodyItem()
bi.setName('MainBody')
bi.body.rootLink.addShapeNode(cyl)
bi.body.updateLinkTree()
bi.body.calcForwardKinematics()
cds = coordinates(npa([0, 0, 1]))
bi.body.rootLink.T = cds.cnoidPosition
bi.body.updateLinkTree()
bi.body.calcForwardKinematics()
### add to item
cbase.RootItem.instance.addChildItem(bi)
cbase.ItemTreeView.instance.checkItem(bi)
#bi.notifyKinematicStateChange()
bi.notifyKinematicStateUpdate()

lk = bi.body.createLink()
lk.setName('lk0')
lk.addShapeNode(box)
lk.setJointType(Link.JointType.FixedJoint)
lk.setOffsetPosition(coordinates(npa([0, 1, 1])).cnoidPosition)
bi.body.rootLink.appendChild(lk)
bi.body.updateLinkTree()
bi.body.calcForwardKinematics()
bi.notifyModelUpdate(sum([int(i) for i in (BodyPlugin.BodyItem.ModelUpdateFlag.LinkSetUpdate, BodyPlugin.BodyItem.ModelUpdateFlag.LinkSpecUpdate,
                                           BodyPlugin.BodyItem.ModelUpdateFlag.DeviceSetUpdate, BodyPlugin.BodyItem.ModelUpdateFlag.DeviceSpecUpdate,
                                           BodyPlugin.BodyItem.ModelUpdateFlag.ShapeUpdate)]))
bi.notifyKinematicStateUpdate()

di=DrawInterface()

import cnoid.Util

def castValueNode(vn):
    if vn.isMapping():
        return mappingToDict(vn.toMapping())
    elif vn.isListing():
        return listingToList(vn.toListing())
    elif vn.isScalar():
        instr = vn.toString()
        try:
            res = int(instr)
        except Exception as e:
            try:
                res = float(instr)
            except Exception as e:
                lowerstr = instr.lower()
                if lowerstr == 'true':
                    return True
                elif lowerstr == 'false':
                    return False
                else:
                    return instr
        return res

def mappingToDict(mapping):
    res = {}
    for key in mapping.keys():
        res[key] = castValueNode(mapping[key])
    return res

def listingToList(listing):
    res = []
    for idx in range(len(listing)):
        res.append(castValueNode(listing[idx]))
    return res

def dictToMapping(indict):
    res = cnoid.Util.Mapping()
    for k, v in indict.items():
        if type(v) is dict:
            res.insert(k, dictToMapping(v))
        elif type(v) is list or type(v) is tuple:
            res.insert(k, listToListing(v))
        else:
            res.write(k, v)
    return res

def listToListing(inlist):
    res = cnoid.Util.Listing()
    for v in inlist:
        if type(v) is dict:
            res.append(dictToMapping(v))
        elif type(v) is list or type(v) is tuple:
            res.append(listToListing(v))
        elif type(v) is bool:
            res.append(str(v).lower())
        else:
            res.append(v)
    return res

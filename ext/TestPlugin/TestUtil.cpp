#include "TestUtil.h"
#include <iostream>

using namespace cnoid;

TestUtil::TestUtil()
{
    std::cout << "Util" << std::endl;

    SgPosTransform nd;
    nd.setName("SgNode:TestUtil");
    std::cout << "SgNode(name) : " << nd.name() << std::endl;
    std::cout << "SgNode(cls)  : " << nd.className() << std::endl;
    s_int = 44;
    s_double = 16.0;
}

bool TestUtil::util0(int &i)
{
    i = s_int;
    return true;
}

bool TestUtil::util1(double &d)
{
    d = s_double;
    return false;
}

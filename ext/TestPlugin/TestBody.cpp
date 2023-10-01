#include "TestBody.h"
#include <iostream>
using namespace cnoid;

//namespace {
//}

//namespace cnoid {

TestBody::TestBody()
{
    std::cout << "Body" << std::endl;

    Body bd("TestBody");
    std::cout << "Body Name : " << bd.name() << std::endl;

    s_int = 31;
    s_double = 42.0;
}

bool TestBody::test0(int &i)
{
    i = s_int;
    return true;
}

bool TestBody::test1(double &b)
{
    b = s_double;
    return false;
}

//}

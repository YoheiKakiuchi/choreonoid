#include "TestBody.h"

using namespace cnoid;

namespace {

}

namespace cnoid {

TestBody::TestBody()
{
    s_int = 31;
    s_double = 42.0;
}

TestBody::test0(int &i)
{
    i = s_int;
}

TestBody::test1(double &b)
{
    b = s_double;
}

}

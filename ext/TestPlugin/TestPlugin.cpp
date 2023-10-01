#include "TestPlugin.h"
#include "TestBody.h"
#include "TestUtil.h"
#include <iostream>
using namespace cnoid;

// namespace cnoid {

TestPlugin::TestPlugin()
    : Plugin("Test")
{
    setActivationPriority(0);
}

bool TestPlugin::initialize()
{
    std::cout << "TestPlugin::initialize" << std::endl;

    TestBody tb;
    int i;
    double d;
    {
        bool res = tb.test0(i);
        std::cout << "test : " << res << " : " << i << std::endl;
    }
    {
        bool res = tb.test1(d);
        std::cout << "test : " << res << " : " << d << std::endl;
    }
    TestUtil tu;
    {
        bool res = tu.util0(i);
        std::cout << "util : " << res << " : " << i << std::endl;
    }
    {
        bool res = tu.util1(d);
        std::cout << "util : " << res << " : " << d << std::endl;
    }
    return true;
}


bool TestPlugin::finalize()
{
    return true;
}


const char* TestPlugin::description() const
{
    static std::string text = "Test Plugin Version";
    return text.c_str();
}

//}

CNOID_IMPLEMENT_PLUGIN_ENTRY(TestPlugin);

#include "TestPlugin.h"

using namespace cnoid;

TestPlugin::TestPlugin()
    : Plugin("Test")
{
    addOldName("SimpleController");
    setActivationPriority(0);

    instance_ = this;
}


bool TestPlugin::initialize()
{
    setUTF8ToModuleTextDomain("Test");
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


CNOID_IMPLEMENT_PLUGIN_ENTRY(TestPlugin);

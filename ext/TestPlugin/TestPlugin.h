#ifndef CNOID_TEST_PLUGIN_TEST_PLUGIN_H
#define CNOID_TEST_PLUGIN_TEST_PLUGIN_H

#include <cnoid/Plugin>

namespace cnoid {

class TestPlugin : public Plugin
{
public:
    TestPlugin();
    virtual bool initialize() override;
    virtual bool finalize() override;
    virtual const char* description() const override;
};

}

#endif

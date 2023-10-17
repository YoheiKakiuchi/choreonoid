/*
  This file is part of Choreonoid, an extensible graphical robotics application suite.
  Copyright (c) 2019-2020 Choreonoid Inc.
  Copyright (c) 2007-2019 National Institute of Advanced Industrial Science and Technology (AIST)
  Released under the MIT license. See 'LICENSE' for more information.
*/

#include <cnoid/App>
#include <cnoid/UTF8>
#include <cstdlib>

using namespace cnoid;

int execute(cnoid::App& app);

#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#include <iostream>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#if 0
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
        AllocConsole(); // allocate new console if launched from Explorer
    }
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    std::cout << "### choreonoid with stream ###" << std::endl;
#endif
    cnoid::App app(hInstance, hPrevInstance, lpCmdLine, nCmdShow, "Choreonoid", "Choreonoid");
    return execute(app);
}
#endif

int main(int argc, char *argv[])
{
    cnoid::App app(argc, argv, "Choreonoid", "Choreonoid");
    return execute(app);
}

int execute(cnoid::App& app)
{
    if(auto pluginPath = getenv("CNOID_PLUGIN_PATH")){
        app.addPluginPath(toUTF8(pluginPath));
    }
    app.setBuiltinProject(":/Base/project/layout.cnoid");
    return app.exec();
}

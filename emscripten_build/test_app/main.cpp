#include <cnoid/App>
#include <QtResource>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(Base);
    Q_INIT_RESOURCE(GLSceneRenderer);

    cnoid::App app(argc, argv, "Choreonoid", "Choreonoid");
    app.setBuiltinProject(":/Base/project/layout.cnoid");
    return app.exec();
}

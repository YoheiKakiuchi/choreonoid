#include <cnoid/SceneGraph>

#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT TestUtil : public SgObject
{
public:
    TestUtil();

    bool util0(int &i);
    bool util1(double &b);

private:
    int s_int;
    double s_double;
};

}

#include <cnoid/Body>

#include "exportdecl_plugin.h"

namespace cnoid {

class CNOID_EXPORT TestBody : public Body
{
public:
    TestBody();

    bool test0(int &i);
    bool test1(double &b);

private:
    int s_int;
    double s_double;
};

}

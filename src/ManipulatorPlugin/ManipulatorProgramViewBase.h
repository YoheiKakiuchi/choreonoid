#ifndef CNOID_MANIPULATOR_PLUGIN_MANIPULATOR_PROGRAM_VIEW_BASE_H
#define CNOID_MANIPULATOR_PLUGIN_MANIPULATOR_PROGRAM_VIEW_BASE_H

#include <cnoid/View>
#include <cnoid/Signal>
#include "exportdecl.h"

namespace cnoid {

class Archive;
class ManipulatorProgramItemBase;
class ManipulatorStatement;
    
class CNOID_EXPORT ManipulatorProgramViewBase : public View
{
public:
    ManipulatorProgramViewBase();
    virtual ~ManipulatorProgramViewBase();

    virtual void onDeactivated() override;

    ManipulatorProgramItemBase* currentProgramItem();

    ManipulatorStatement* currentStatement();
    SignalProxy<void(ManipulatorStatement* statement)> sigCurrentStatementChanged();

    virtual bool storeState(Archive& archive) override;
    virtual bool restoreState(const Archive& archive) override;
            
    class Impl;

protected:
    void addStatementButton(QWidget* button, int row);

    enum InsertionType { BeforeTargetPosition, AfterTargetPosition };
    bool insertStatement(
        ManipulatorStatement* statement, int insertionType = AfterTargetPosition);

    virtual bool onCurrentProgramItemChanged(ManipulatorProgramItemBase* item) = 0;
    virtual void onCurrentStatementChanged(ManipulatorStatement* statement);

    /**
       This function is called when the current statemet is changed or clicked.
    */
    virtual void onCurrentStatementActivated(ManipulatorStatement* statement);

private:
    Impl* impl;
};

}

#endif

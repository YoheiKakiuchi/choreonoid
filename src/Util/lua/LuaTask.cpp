/*!
  @author Shin'ichiro Nakaoka
*/

#include "../Task.h"
#include "../AbstractTaskSequencer.h"
#include "../ValueTree.h"
#include "LuaUtil.h"

using namespace std;
using namespace std::placeholders;
using namespace cnoid;

namespace {

class LuaTaskFunc
{
    sol::function func;
public:
    LuaTaskFunc(sol::function f) : func(f) { }
    void operator()(TaskProc* proc){
        try {
            func(proc);
        } catch(const sol::error& ex) {
            // put ex.what() by using func.lua_state() and the print function
        }
    }
};


class TaskWrap : public Task
{
public:
    sol::function onMenuRequest_;
    sol::function onActivated_;
    sol::function onDeactivated_;
    sol::function storeState_;
    sol::function restoreState_;
    
    TaskWrap(sol::this_state s) {
        onMenuRequest_ = makeLuaFunction(s, [](TaskWrap* self, TaskMenu& menu) { self->Task::onMenuRequest(menu); });
        onActivated_ = makeLuaFunction(s, [](TaskWrap* self, AbstractTaskSequencer* s) { self->Task::onActivated(s); });
        onDeactivated_ = makeLuaFunction(s, [](TaskWrap* self, AbstractTaskSequencer* s) { self->Task::onDeactivated(s); });
        storeState_ = makeLuaFunction(s, [](TaskWrap* self, AbstractTaskSequencer* s, Mapping& a) { self->Task::storeState(s, a); });
        restoreState_ = makeLuaFunction(s, [](TaskWrap* self, AbstractTaskSequencer* s, Mapping& a) { self->Task::restoreState(s, a); });
        
    }
    //TaskWrap(const std::string& name, const std::string& caption) : Task(name, caption) { };
    //TaskWrap(const Task& org, bool doDeepCopy = true) : Task(org, doDeepCopy) { };

    virtual void onMenuRequest(TaskMenu& menu) override {
        onMenuRequest_(this, std::ref(menu));
    }
    
    virtual void onActivated(AbstractTaskSequencer* sequencer){
        onActivated_(this, sequencer);
    }
    virtual void onDeactivated(AbstractTaskSequencer* sequencer){
        onDeactivated_(this, sequencer);
    }
    /*
    virtual void storeState(AbstractTaskSequencer* sequencer, Mapping& archive){
        storeState_(this, sequencer, archive);
    }
    virtual void restoreState(AbstractTaskSequencer* sequencer, Mapping& archive){
        restoreState_(this, sequencer, archive);
    }
    */
};

typedef ref_ptr<TaskWrap> TaskWrapPtr;


bool waitForSignal(sol::object self, sol::object signalProxy, bool isBooleanSignal, double timeout, sol::this_state s)
{
    signalProxy.push();
    sol::stack::get_field(s, "connect");
    sol::function connect(s);

    sol::function notifyCommandFinish;
    if(isBooleanSignal){
        notifyCommandFinish = makeLuaFunction(s, [](TaskProc* self, bool isCompleted){ self->notifyCommandFinish(isCompleted); });
    } else {
        notifyCommandFinish = makeLuaFunction(s, [](TaskProc* self){ self->notifyCommandFinish(true); });
    }

    sol::object connection = connect(signalProxy, notifyCommandFinish);

    sol::function waitForCommandToFinish =
        makeLuaFunction(s, [](TaskProc* self, Connection c, double timeout){ return self->waitForCommandToFinish(c, timeout); });
    
    bool result = waitForCommandToFinish(self, connection, timeout);
    
    return result;
}

}

namespace cnoid {

void exportLuaTaskTypes(sol::table& module)
{
    module.new_usertype<TaskProc>(
        "TaskProc",
        "new", sol::no_constructor,
        "currentPhaseIndex", &TaskProc::currentPhaseIndex,
        "isAutoMode", &TaskProc::isAutoMode,
        "breakSequence", &TaskProc::breakSequence,
        "setNextCommand", &TaskProc::setNextCommand,
        "setNextPhase", &TaskProc::setNextPhase,
        "setCommandLinkAutomatic", &TaskProc::setCommandLinkAutomatic,
        "executeCommand", &TaskProc::executeCommand,
        "wait", &TaskProc::wait,
        "waitForCommandToFinish", sol::overload(
            [](TaskProc* self){ return self->waitForCommandToFinish(); },
            [](TaskProc* self, double t){ return self->waitForCommandToFinish(t); },
            [](TaskProc* self, Connection c, bool t){ return self->waitForCommandToFinish(c, t); }),
        "notifyCommandFinish", &TaskProc:: notifyCommandFinish,
        "waitForSignal", sol::overload(
            [](sol::object self, sol::object signalProxy, sol::this_state s){
                return waitForSignal(self, signalProxy, false, 0.0, s); },
            [](sol::object self, sol::object signalProxy, double timeout, sol::this_state s){
                return waitForSignal(self, signalProxy, false, timeout, s); }),
        "waitForBooleanSignal", sol::overload(
            [](sol::object self, sol::object signalProxy, sol::this_state s){
                return waitForSignal(self, signalProxy, true, 0.0, s); },
            [](sol::object self, sol::object signalProxy, double timeout, sol::this_state s){
                return waitForSignal(self, signalProxy, true, timeout, s); })
        );

    module.new_usertype<TaskToggleState>(
        "TaskToggleState",
        "isChecked", &TaskToggleState::isChecked,
        "setChecked", &TaskToggleState::setChecked,
        "sigToggled", &TaskToggleState::sigToggled
        );

    module.new_usertype<TaskCommand>(
        "TaskCommand",
        "new", sol::factories([](const char* caption) -> TaskCommandPtr { return new TaskCommand(caption); }),
        "caption", &TaskCommand::caption,
        "setCaption", [](TaskCommand* self, const char* caption) -> TaskCommandPtr { return self->setCaption(caption); },
        "description", &TaskCommand::description,
        "setDescription", [](TaskCommand* self, const char* description) -> TaskCommandPtr { return self->setDescription(description); },
        "function", &TaskCommand::function,
        "setFunction", [](TaskCommand* self, sol::function func) -> TaskCommandPtr { return self->setFunction(LuaTaskFunc(func)); },
        "setDefault", sol::overload(
            [](TaskCommand* self) -> TaskCommandPtr { return self->setDefault(); },
            [](TaskCommand* self, bool on) -> TaskCommandPtr { return self->setDefault(on); }),
        "isDefault", &TaskCommand::isDefault,
        "setCheckable", sol::overload(
            [](TaskCommand* self) -> TaskCommandPtr { return self->setCheckable(); },
            [](TaskCommand* self, bool on) -> TaskCommandPtr { return self->setCheckable(on); }),
        "setToggleState", [](TaskCommand* self, TaskToggleState* state) -> TaskCommandPtr { return self->setToggleState(state); },
        "toggleState", [](TaskCommand* self) -> TaskToggleStatePtr { return self->toggleState(); },
        "setChecked", [](TaskCommand* self, bool on) -> TaskCommandPtr { return self->setChecked(on); },
        "isChecked", &TaskCommand::isChecked,
        "nextPhaseIndex", &TaskCommand::nextPhaseIndex,
        "setPhaseLink", [](TaskCommand* self, int phaseIndex) -> TaskCommandPtr { return self->setPhaseLink(phaseIndex); },
        "setPhaseLinkStep", [](TaskCommand* self, int phaseIndexStep) -> TaskCommandPtr { return self->setPhaseLinkStep(phaseIndexStep); },
        "linkToNextPhase", [](TaskCommand* self) -> TaskCommandPtr { return self->linkToNextPhase(); },
        "nextCommandIndex", &TaskCommand::nextCommandIndex,
        "setCommandLink", [](TaskCommand* self, int commandIndex) -> TaskCommandPtr { return self->setCommandLink(commandIndex); },
        "setCommandLinkStep", [](TaskCommand* self, int commandIndexStep) -> TaskCommandPtr { return self->setCommandLinkStep(commandIndexStep); },
        "linkToNextCommand", [](TaskCommand* self) -> TaskCommandPtr { return self->linkToNextCommand(); },
        "isCommandLinkAutomatic", &TaskCommand::isCommandLinkAutomatic,
        "setCommandLinkAutomatic", sol::overload(
            [](TaskCommand* self) -> TaskCommandPtr { return self->setCommandLinkAutomatic(); },
            [](TaskCommand* self, bool on) -> TaskCommandPtr { return self->setCommandLinkAutomatic(on); }),
        "setLevel", [](TaskCommand* self, int level) -> TaskCommandPtr { return self->setLevel(level); },
        "level", &TaskCommand::level
        );

    module.new_usertype<TaskPhase>(
        "TaskPhase",
        "new", sol::factories(
            [](const char* caption) -> TaskPhasePtr { return new TaskPhase(caption); },
            [](TaskPhase* org) -> TaskPhasePtr { return new TaskPhase(*org); },
            [](TaskPhase* org, bool doDeepCopy) -> TaskPhasePtr { return new TaskPhase(*org, doDeepCopy); }),
        "clone", sol::overload(
            [](TaskPhase* self) -> TaskPhasePtr { return self->clone(); },
            [](TaskPhase* self, bool doDeepCopy) -> TaskPhasePtr { return self->clone(doDeepCopy); }),
        "caption", &TaskPhase::caption,
        "setCaption", &TaskPhase::setCaption,
        "isSkipped", &TaskPhase::isSkipped,
        "setSkipped", &TaskPhase::setSkipped,
        "setPreCommand", [](TaskPhase* self, sol::function func) { self->setPreCommand(LuaTaskFunc(func)); },
        "preCommand", &TaskPhase::preCommand,
        "addCommand", sol::overload(
            [](TaskPhase* self) -> TaskCommandPtr { return self->addCommand(); },
            [](TaskPhase* self, const char* caption) -> TaskCommandPtr { return self->addCommand(caption); }),
        "addToggleCommand", sol::overload(
            [](TaskPhase* self) -> TaskCommandPtr { return self->addToggleCommand(); },
            [](TaskPhase* self, const char* caption) -> TaskCommandPtr { return self->addToggleCommand(caption); }),
        "numCommands", &TaskPhase::numCommands,
        "command", [](TaskPhase* self, int index) -> TaskCommandPtr { return self->command(index); },
        "lastCommandIndex", &TaskPhase::lastCommandIndex,
        "lastCommand", [](TaskPhase* self) -> TaskCommandPtr { return self->lastCommand(); },
        "commandLevel", &TaskPhase::commandLevel,
        "maxCommandLevel", &TaskPhase::maxCommandLevel
        );

    module.new_usertype<TaskPhaseProxy>(
        "TaskPhaseProxy",
        "new", sol::no_constructor,
        "setCommandLevel", &TaskPhaseProxy::setCommandLevel,
        "commandLevel", &TaskPhaseProxy::commandLevel,
        "addCommand", sol::overload(
            [](TaskPhaseProxy* self) -> TaskCommandPtr { return self->addCommand(); },
            [](TaskPhaseProxy* self, const char* caption) -> TaskCommandPtr { return self->addCommand(caption); }),
        "addToggleCommand", sol::overload(
            [](TaskPhaseProxy* self) -> TaskCommandPtr { return self->addToggleCommand(); },
            [](TaskPhaseProxy* self, const char* caption) -> TaskCommandPtr { return self->addToggleCommand(caption); })
        );
    
    module.new_usertype<TaskMenu>(
        "TaskMenu",
        "new", sol::no_constructor,
        "addMenuItem", [](TaskMenu& self, const char* caption, sol::function func) { self.addMenuItem(caption, func); },
        "addCheckMenuItem", [](TaskMenu& self, const char* caption, bool isChecked, sol::function func) {
            self.addCheckMenuItem(caption, isChecked, func); },
        "addMenuSeparator", [](TaskMenu& self) { self.addMenuSeparator(); }
        );

    module.new_usertype<Task>(
        "TaskBase",
        "new", sol::no_constructor
        );

    module.new_usertype<TaskWrap>(
        "Task",
        sol::base_classes, sol::bases<Task>(),
        "new", sol::factories([](sol::this_state s) -> TaskWrapPtr { return new TaskWrap(s); }),

        "name", [](sol::object self) { return native<Task>(self)->name(); },
        "setName", [](sol::object self, const char* name) { native<Task>(self)->setName(name); },
        "caption", [](sol::object self) { return native<Task>(self)->caption(); },
        "setCaption", [](sol::object self, const char* caption) { native<Task>(self)->setCaption(caption); },
        "numPhases", [](sol::object self) { return native<Task>(self)->numPhases(); },
        "phase", [](sol::object self, int index) -> TaskPhasePtr { return native<Task>(self)->phase(index); },
        "addPhase", sol::overload(
            [](sol::object self, TaskPhase* phase) -> TaskPhasePtr { return native<Task>(self)->addPhase(phase); },
            [](sol::object self, const char* caption) -> TaskPhasePtr { return native<Task>(self)->addPhase(caption); }),
        "lastPhase", [](sol::object self) -> TaskPhasePtr { return native<Task>(self)->lastPhase(); },
        "setPreCommand", [](sol::object self, sol::function func) { native<Task>(self)->setPreCommand(LuaTaskFunc(func)); },
        "addCommand", sol::overload(
            [](sol::object self) -> TaskCommandPtr { return native<Task>(self)->addCommand(); },
            [](sol::object self, const char* caption) -> TaskCommandPtr { return native<Task>(self)->addCommand(caption); }),
        "addToggleCommand", sol::overload(
            [](sol::object self) -> TaskCommandPtr { return native<Task>(self)->addToggleCommand(); },
            [](sol::object self, const char* caption) -> TaskCommandPtr { return native<Task>(self)->addToggleCommand(caption); }),
        "lastCommand", [](sol::object self) -> TaskCommandPtr { return native<Task>(self)->lastCommand(); },
        "lastCommandIndex", [](sol::object self) { return native<Task>(self)->lastCommandIndex(); },
        "funcToSetCommandLink", [](sol::object self, int commandIndex) { return native<Task>(self)->funcToSetCommandLink(commandIndex); },
        "commandLevel", [](sol::object self, int level) { return native<Task>(self)->commandLevel(level); },
        "maxCommandLevel", [](sol::object self) { return native<Task>(self)->maxCommandLevel(); },

        "get_onMenuRequest", [](sol::object self) { return native<TaskWrap>(self)->onMenuRequest_; },
        "set_onMenuRequest", [](sol::object self, sol::function f) { native<TaskWrap>(self)->onMenuRequest_ = f; },
        "get_onActivated", [](sol::object self) { return native<TaskWrap>(self)->onActivated_; },
        "set_onActivated", [](sol::object self, sol::function f) { native<TaskWrap>(self)->onActivated_ = f; },
        "get_onDeactivated", [](sol::object self) { return native<TaskWrap>(self)->onDeactivated_; },
        "set_onDeactivated", [](sol::object self, sol::function f) { native<TaskWrap>(self)->onDeactivated_ = f; },
        "get_storeState", [](sol::object self) { return native<TaskWrap>(self)->storeState_; },
        "set_storeState", [](sol::object self, sol::function f) { native<TaskWrap>(self)->storeState_ = f; },
        "get_restoreState", [](sol::object self) { return native<TaskWrap>(self)->restoreState_; },
        "set_restoreState", [](sol::object self, sol::function f) { native<TaskWrap>(self)->restoreState_ = f; }
        );

    module.new_usertype<AbstractTaskSequencer>(
        "AbstractTaskSequencer",
        "new", sol::no_constructor,
        "activate", sol::overload(
            [](AbstractTaskSequencer* self){ self->activate(true); },
            [](AbstractTaskSequencer* self, bool on){ self->activate(on); }),
        "isActive", &AbstractTaskSequencer::isActive,
        "addTask", [](AbstractTaskSequencer* self, sol::object task) { return self->addTask(native<Task>(task)); },
        "updateTask", [](AbstractTaskSequencer* self, sol::object task){ return self->updateTask(native<Task>(task)); },
        "removeTask", [](AbstractTaskSequencer* self, sol::object task) { return self->removeTask(native<Task>(task)); },
        "sigTaskAdded", &AbstractTaskSequencer::sigTaskAdded,
        "sigTaskRemoved", &AbstractTaskSequencer::sigTaskRemoved,
        "clearTasks", &AbstractTaskSequencer::clearTasks,
        "numTasks", &AbstractTaskSequencer::numTasks,
        "task", [](AbstractTaskSequencer* self, int index) -> TaskPtr { return self->task(index); },
        "currentTaskIndex", &AbstractTaskSequencer::currentTaskIndex,
        "setCurrentTask", &AbstractTaskSequencer::setCurrentTask,
        "sigCurrentTaskChanged", &AbstractTaskSequencer::sigCurrentTaskChanged,
        "currentPhaseIndex", &AbstractTaskSequencer::currentPhaseIndex,
        "setCurrentPhase", &AbstractTaskSequencer::setCurrentPhase,
        "sigCurrentPhaseChanged", &AbstractTaskSequencer::sigCurrentPhaseChanged,
        "currentCommandIndex", &AbstractTaskSequencer::currentCommandIndex,
        "executeCommand", &AbstractTaskSequencer::executeCommand,
        "sigCurrentCommandChanged", &AbstractTaskSequencer::sigCurrentCommandChanged,
        "isBusy", &AbstractTaskSequencer::isBusy,
        "sigBusyStateChanged", &AbstractTaskSequencer::sigBusyStateChanged,
        "isAutoMode", &AbstractTaskSequencer::isAutoMode,
        "setAutoMode", &AbstractTaskSequencer::setAutoMode,
        "sigAutoModeToggled", &AbstractTaskSequencer::sigAutoModeToggled
        );
}

}

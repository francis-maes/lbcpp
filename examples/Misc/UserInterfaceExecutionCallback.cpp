/*-----------------------------------------.---------------------------------.
| Filename: UserInterfaceExecutionCallback.cpp  | User Interface Callback    |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 15:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
using namespace lbcpp;

class MySubWorkUnit : public WorkUnit
{
public:
  MySubWorkUnit(const String& name)
    : WorkUnit(name) {}
 
  virtual bool run(ExecutionContext& context)
  {
    context.informationCallback(T("Before"));
    context.statusCallback(T("SubWorking..."));
    for (size_t i = 0; i < 100; ++i)
    {
      Thread::sleep(5);
      context.progressCallback(i + 1.0, 100.0, T("%"));
    }
    context.informationCallback(T("After"));
    return true;
  }
};

class MyWorkUnit : public WorkUnit
{
public:
  MyWorkUnit() : WorkUnit(T("My Work Unit !")) {}
 
  virtual bool run(ExecutionContext& context)
  {
    context.statusCallback(T("Working..."));

//    context.errorCallback(T("My Error"));
    Thread::sleep(100);
    context.warningCallback(T("My Warning"));
    Thread::sleep(500);
    context.informationCallback(T("My Information"));

    for (size_t i = 0; i < 10; ++i)
    {
      Thread::sleep(200);
      context.progressCallback(i + 1.0, 10.0, T("epochs"));
    }

    CompositeWorkUnitPtr subWorkUnits(new CompositeWorkUnit(T("My 8 Sub Work Units"), 8));
    for (size_t i = 0; i < subWorkUnits->getNumWorkUnits(); ++i)
      subWorkUnits->setWorkUnit(i, new MySubWorkUnit(T("SubWU ") + String((int)i)));
    subWorkUnits->setPushChildrenIntoStackFlag(true);
    context.run(subWorkUnits);

    context.informationCallback(T("Finished."));
    return true;
  }
};

ExecutionContextPtr createExecutionContext()
{
  ExecutionContextPtr res = multiThreadedExecutionContext(8);
  res->appendCallback(userInterfaceExecutionCallback());
  res->appendCallback(consoleExecutionCallback());
  return res;
}

int main(int argc, char* argv[])
{
  lbcpp::initialize(argv[0]);
  ExecutionContextPtr context = createExecutionContext();
  context->declareType(new DefaultClass(T("MyWorkUnit"), T("WorkUnit")));
  context->declareType(new DefaultClass(T("MySubWorkUnit"), T("WorkUnit")));
  int exitCode = WorkUnit::main(*context, new MyWorkUnit(), argc, argv);
  userInterfaceManager().waitUntilAllWindowsAreClosed();
  context->clearCallbacks();
  lbcpp::deinitialize();
  return exitCode;
}

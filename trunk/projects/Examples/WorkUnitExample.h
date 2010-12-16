/*-----------------------------------------.---------------------------------.
| Filename: WorkUnitExample.h              | Illustrates Work Units          |
| Author  : Francis Maes                   |                                 |
| Started : 16/12/2010 14:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXAMPLES_WORK_UNIT_H_
# define LBCPP_EXAMPLES_WORK_UNIT_H_

# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class SubWorkUnitExample : public WorkUnit
{
public:
  SubWorkUnitExample(const String& name = T("SubWorkUnitExample"))
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

class WorkUnitExample : public WorkUnit
{
public:
  WorkUnitExample() : WorkUnit(T("My Work Unit !")) {}
 
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
      subWorkUnits->setWorkUnit(i, new SubWorkUnitExample(T("SubWU ") + String((int)i)));
    subWorkUnits->setPushChildrenIntoStackFlag(true);
    context.run(subWorkUnits);

    context.informationCallback(T("Finished."));
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_WORK_UNIT_H_

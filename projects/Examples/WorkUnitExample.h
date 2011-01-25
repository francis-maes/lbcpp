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
  virtual bool run(ExecutionContext& context)
  {
    context.informationCallback(T("Before"));
    for (size_t i = 0; i < 100; ++i)
    {
      Thread::sleep(5);
      context.progressCallback(new ProgressionState(i + 1.0, 100.0, T("%")));
    }
    context.informationCallback(T("After"));
    context.resultCallback(T("aStringResult"), T("Hello"));
    return true;
  }
};

class WorkUnitExample : public WorkUnit
{
public:
  virtual String toString() const
    {return T("My Work Unit !");}
 
  virtual bool run(ExecutionContext& context)
  {
//    context.errorCallback(T("My Error"));
    Thread::sleep(100);
    context.warningCallback(T("My Warning"));
    Thread::sleep(500);
    context.informationCallback(T("My Information"));

    for (size_t i = 0; i < 10; ++i)
    {
      Thread::sleep(200);
      context.progressCallback(new ProgressionState(i + 1.0, 10.0, T("epochs")));
    }

    CompositeWorkUnitPtr subWorkUnits(new CompositeWorkUnit(T("My 8 Sub Work Units"), 8));
    for (size_t i = 0; i < subWorkUnits->getNumWorkUnits(); ++i)
      subWorkUnits->setWorkUnit(i, new SubWorkUnitExample());
    subWorkUnits->setPushChildrenIntoStackFlag(true);
    context.run(subWorkUnits);

    context.informationCallback(T("Finished."));
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_WORK_UNIT_H_

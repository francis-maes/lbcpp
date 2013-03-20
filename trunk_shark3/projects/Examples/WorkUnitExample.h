/*-----------------------------------------.---------------------------------.
| Filename: WorkUnitExample.h              | Illustrates Work Units          |
| Author  : Francis Maes                   |                                 |
| Started : 16/12/2010 14:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXAMPLES_WORK_UNIT_H_
# define EXAMPLES_WORK_UNIT_H_

# include <oil/Execution/WorkUnit.h>

namespace lbcpp
{

class SubWorkUnitExample : public WorkUnit
{
public:
  virtual ObjectPtr run(ExecutionContext& context)
  {
    context.informationCallback(JUCE_T("Before"));
    for (size_t i = 0; i < 100; ++i)
    {
      Thread::sleep(5);
      context.progressCallback(new ProgressionState(i + 1.0, 100.0, JUCE_T("%")));
    }
    context.informationCallback(JUCE_T("After"));
    return new String("Hello");
  }
};

class WorkUnitExample : public WorkUnit
{
public:
  virtual string toString() const
    {return JUCE_T("My Work Unit !");}
 
  virtual ObjectPtr run(ExecutionContext& context)
  {
//    context.errorCallback(JUCE_T("My Error"));
    Thread::sleep(100);
    context.warningCallback(JUCE_T("My Warning"));
    Thread::sleep(500);
    context.informationCallback(JUCE_T("My Information"));

    for (size_t i = 0; i < 10; ++i)
    {
      Thread::sleep(200);
      context.progressCallback(new ProgressionState(i + 1.0, 10.0, JUCE_T("epochs")));
    }

    CompositeWorkUnitPtr subWorkUnits(new CompositeWorkUnit(JUCE_T("My 8 Sub Work Units"), 8));
    for (size_t i = 0; i < subWorkUnits->getNumWorkUnits(); ++i)
      subWorkUnits->setWorkUnit(i, new SubWorkUnitExample());
    subWorkUnits->setPushChildrenIntoStackFlag(true);
    context.run(subWorkUnits);

    context.resultCallback(JUCE_T("toto"), JUCE_T("tata"));//TMP
    context.informationCallback(JUCE_T("Finished."));
    return new Double(0.85);  // return fictive score
  }
};

}; /* namespace lbcpp */

#endif // !EXAMPLES_WORK_UNIT_H_

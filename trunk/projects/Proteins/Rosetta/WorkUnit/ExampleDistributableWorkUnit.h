/*-----------------------------------------.---------------------------------.
| Filename: ExampleDistributableWorkUnit.h | Example Distributable WorkUnit  |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Jan 17, 2012  4:15:30 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_WORKUNIT_EXAMPLEDISTRIBUTABLEWORKUNIT_H_
# define LBCPP_PROTEINS_ROSETTA_WORKUNIT_EXAMPLEDISTRIBUTABLEWORKUNIT_H_

# include "DistributableWorkUnit.h"

namespace lbcpp
{

class TestDumbWorkUnit : public WorkUnit
{
public:
  TestDumbWorkUnit() {}
  TestDumbWorkUnit(size_t j) : j(j) {}

  virtual Variable run(ExecutionContext& context)
  {
    context.enterScope(T("DumbWorkUnit ") + String((int)j));

    juce::RelativeTime t1 = juce::RelativeTime::milliseconds(juce::Time::currentTimeMillis());

    RandomGeneratorPtr random = new RandomGenerator((juce::uint32)j);
    DenseDoubleVectorPtr values = new DenseDoubleVector(100, -1);

    for (size_t i = 0; i < 100; ++i)
    {
      values->setValue(i, (double)random->sampleInt(0, 100));
      if ((i + 1) % 10 == 0)
        context.progressCallback(new ProgressionState(i + 1, 100, T("It")));
      juce::Thread::sleep(context.getRandomGenerator()->sampleInt(10, 30));
    }

    juce::RelativeTime t2 = juce::RelativeTime::milliseconds(juce::Time::currentTimeMillis());
    juce::RelativeTime t3 = t2 - t1;
    double elapsed = t3.inSeconds();

    context.leaveScope(Variable(elapsed));
    return Variable(values);
  }

protected:
  friend class TestDumbWorkUnitClass;

  size_t j;
};

class TestDistribWorkUnit : public DistributableWorkUnit
{
public:
  TestDistribWorkUnit() {}
  TestDistribWorkUnit(size_t num) : num(num) {}

  virtual void initializeWorkUnits(ExecutionContext& context)
  {
    for (size_t i = 0; i < num; i++)
      workUnits->addWorkUnit(new TestDumbWorkUnit(i));
  }

  virtual Variable singleResultCallback(ExecutionContext& context, Variable& result)
  {
    size_t numElements = result.getObjectAndCast<DenseDoubleVector> ()->getNumValues();

    for (size_t i = 0; i < numElements; i++)
    {
      context.enterScope(T("Result"));
      context.resultCallback(T("Step"), Variable((int)i));
      context.resultCallback(T("Value"), Variable(result.getObjectAndCast<DenseDoubleVector> ()->getValue(i)));
      context.leaveScope();
    }

    return (result);
  }

  virtual Variable multipleResultCallback(ExecutionContext& context, VariableVector& results)
  {
    size_t numElements = results.getElement(0).getObjectAndCast<DenseDoubleVector> ()->getNumValues();
    size_t numResults = results.getNumElements();

    DenseDoubleVectorPtr mean = new DenseDoubleVector(numElements, -1);

    for (size_t j = 0; j < numResults; j++)
      for (size_t i = 0; i < numElements; i++)
        mean->setValue(i, mean->getValue(i) + results.getElement(j).getObjectAndCast<DenseDoubleVector> ()->getValue(i) / (double)numResults);

    for (size_t i = 0; i < numElements; i++)
    {
      context.enterScope(T("Result"));
      context.resultCallback(T("Step"), Variable((int)i));
      context.resultCallback(T("Value"), Variable(mean->getValue(i)));
      context.leaveScope();
    }

    return Variable(mean);
  }

protected:
  friend class TestDistribWorkUnitClass;

  size_t num;
};

extern DistributableWorkUnitPtr testDistribWorkUnit();

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_WORKUNIT_EXAMPLEDISTRIBUTABLEWORKUNIT_H_


/*-----------------------------------------.---------------------------------.
| Filename: DistributableWorkUnit.h        | Distributable WorkUnit          |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 21, 2011  3:02:51 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_WORKUNIT_DISTRIBUTABLEWORKUNIT_H_
# define LBCPP_PROTEINS_ROSETTA_WORKUNIT_DISTRIBUTABLEWORKUNIT_H_

# include <lbcpp/Execution/WorkUnit.h>

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

    for (size_t i = 0; i < 100; ++i)
    {
      if ((i + 1) % 10 == 0)
        context.progressCallback(new ProgressionState(i + 1, 100, T("It")));
      juce::Thread::sleep(context.getRandomGenerator()->sampleInt(100, 300));
    }

    juce::RelativeTime t2 = juce::RelativeTime::milliseconds(juce::Time::currentTimeMillis());
    juce::RelativeTime t3 = t2 - t1;
    double elapsed = t3.inSeconds();

    context.leaveScope(Variable(elapsed));
    return Variable(elapsed);
  }

protected:
  friend class TestDumbWorkUnitClass;

  size_t j;
};

class DistributableWorkUnit : public WorkUnit
{
public:
  DistributableWorkUnit() : name(T("DistributableWorkUnit")) {}
  DistributableWorkUnit(String name) : name(name) {}

  /**
   * Each subworkunit should open a scope for itself to avoid undesired effects
   * in traces.
   *
   * workUnits = new CompositeWorkUnit(T("blabla"));
   * workUnits->addWorkUnit(new WorkUnit());
   *
   */
  virtual void initializeWorkUnits(ExecutionContext& context)=0;

  /**
   * Manage the VariableVector output by run.
   * Should enter its own scope.
   */
  virtual Variable resultsCallback(ExecutionContext& context, VariableVector& results)
    {return Variable();}

  virtual CompositeWorkUnitPtr getWorkUnits()
    {return workUnits;}

  virtual Variable run(ExecutionContext& context)
  {
    // initialization
    context.enterScope(T("Initializing distributable work unit::local"));
    initializeWorkUnits(context);
    context.leaveScope();

    // computing time
    context.enterScope(name);
    Variable result = context.run(workUnits, true);
    context.leaveScope();

    // results callback
    context.enterScope(T("Managing results"));
    Variable gatheredResult = resultsCallback(context, *result.getObjectAndCast<VariableVector> ());
    context.leaveScope();

    context.informationCallback(T("Local distributable work unit done."));
    return gatheredResult;
  }

  String getName()
    {return name;}

protected:
  friend class DistributableWorkUnitClass;

  String name;
  CompositeWorkUnitPtr workUnits;
};

typedef ReferenceCountedObjectPtr<DistributableWorkUnit> DistributableWorkUnitPtr;
extern ClassPtr distributableWorkUnitClass;

class TestDistribWorkUnit : public DistributableWorkUnit
{
public:
  TestDistribWorkUnit() : DistributableWorkUnit() {}

  virtual void initializeWorkUnits(ExecutionContext& context)
  {
    workUnits = new CompositeWorkUnit(name);
    for (size_t i = 0; i < 10; i++)
      workUnits->addWorkUnit(new TestDumbWorkUnit(i));
  }

  virtual Variable resultsCallback(ExecutionContext& context, VariableVector& results)
  {
    context.enterScope(T("Manage results"));
    double j = 0;

    for (size_t i = 0; i < results.getNumElements(); i++)
      j += results.getElement(i).getDouble();
    j /= (double)results.getNumElements();

    context.informationCallback(T("Average is ") + String(j));
    context.leaveScope(Variable(j));

    return Variable();
  }

protected:
  friend class TestDistribWorkUnitClass;
};

extern DistributableWorkUnitPtr testDistribWorkUnit();

class DistributeToClusterWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ExecutionContextPtr remoteContext = distributedExecutionContext(context, remoteHostName,
        remoteHostPort, distributable->getName(), localHostLogin, clusterLogin,
        fixedResourceEstimator(memory, time, 1));

    // initialization
    context.enterScope(T("Initializing distributable work unit::distributed"));
    distributable->initializeWorkUnits(context);
    context.leaveScope();
    CompositeWorkUnitPtr units = distributable->getWorkUnits();
    if (units->getNumWorkUnits() == 0)
    {
      context.informationCallback(T("No work units to treat."));
      return Variable();
    }

    // computing time
    context.enterScope(distributable->getName());
    context.informationCallback(T("Treating ") + String((int)units->getNumWorkUnits())
        + T(" work units."));
    //Variable result = remoteContext->run(units);
    context.leaveScope();

    // managing results
    context.enterScope(T("Managing results"));
//    Variable gatheredResult = distributable->resultsCallback(context, *result.getObjectAndCast<
//        VariableVector> ());
    context.leaveScope();

    context.informationCallback(T("Distribution to cluster done."));

    //return gatheredResult;
    return Variable();
  }

protected:
  friend class DistributeToClusterWorkUnitClass;

  String clusterLogin; // nic3
  size_t memory; // in MB
  size_t time; // in hours
  DistributableWorkUnitPtr distributable;

  String localHostLogin; // me
  String remoteHostName; // monster
  size_t remoteHostPort; // 1664
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_WORKUNIT_DISTRIBUTABLEWORKUNIT_H_

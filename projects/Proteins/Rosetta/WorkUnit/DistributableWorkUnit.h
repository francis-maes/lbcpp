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

class DistributableExecutionContextCallback : public ExecutionContextCallback
{
public:
  DistributableExecutionContextCallback(ExecutionContext& context, String projectName)
    : context(context), projectName(projectName) {}

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    savedResult = result;
    File f(context.getProjectDirectory().getFullPathName() + T("/") + projectName + T("/")
        + workUnit->toString() + T(".xml"));
    result.saveToFile(context, f);
  }

  static Variable gatherResults(VariableVector& callbacks)
  {
    VariableVectorPtr results = variableVector(callbacks.getNumElements());

    for (size_t i = 0; i < callbacks.getNumElements(); i++)
    {
      results->setElement(i, callbacks.getElement(i).getObjectAndCast<
          DistributableExecutionContextCallback> ()->savedResult);
    }

    return results;
  }

protected:
  ExecutionContext& context;
  String projectName;

  Variable savedResult;
};

typedef ReferenceCountedObjectPtr<DistributableExecutionContextCallback> DistributableExecutionContextCallbackPtr;

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
  virtual Variable singleResultCallback(ExecutionContext& context, Variable& result)
    {return Variable(result);}

  /**
   * Manage the VariableVector output by run.
   * Should enter its own scope.
   */
  virtual Variable multipleResultCallback(ExecutionContext& context, VariableVector& results)
    {return Variable();}

  virtual CompositeWorkUnitPtr getWorkUnits()
    {return workUnits;}

  virtual Variable run(ExecutionContext& context)
  {
    // initialization
    context.enterScope(T("Initializing distributable work unit::local"));
    initializeWorkUnits(context);
    context.leaveScope();
    if (workUnits->getNumWorkUnits() == 0)
    {
      context.informationCallback(T("No work units to treat."));
      return Variable();
    }

    // computing time
    context.enterScope(name);
    context.informationCallback(T("Treating ") + String((int)workUnits->getNumWorkUnits())
        + T(" work units."));
    Variable result = context.run(workUnits, true);
    context.leaveScope();

    // show results
    context.enterScope(T("Results"));
    for (size_t i = 0; i < workUnits->getNumWorkUnits(); i++)
    {
      context.enterScope(workUnits->getWorkUnit(i));
      context.leaveScope(singleResultCallback(context,
          result.getObjectAndCast<VariableVector> ()->getElement(i)));
    }
    context.leaveScope();

    // managing results
    context.enterScope(T("Managing results"));
    Variable gatheredResult = multipleResultCallback(context, *result.getObjectAndCast<
        VariableVector> ());
    context.leaveScope(gatheredResult);

    context.informationCallback(T("local"), T("Distributable work unit done."));

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

class DistributeToClusterWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (distributable->getName().isEmpty())
    {
      context.errorCallback(T("Name of distributable should not be empty."));
      return Variable();
    }

    ExecutionContextPtr remoteContext = distributedExecutionContext(context, remoteHostName,
        remoteHostPort, distributable->getName(), localHostLogin, clusterLogin,
        fixedResourceEstimator(1, memory, time), true);

    File distributableWorkUnitDirectory = context.getFile(distributable->toString());
    if (!distributableWorkUnitDirectory.exists())
      distributableWorkUnitDirectory.createDirectory();

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

    // check for already solved workunits and execute unsolved ones
    for (size_t i = 0; i < units->getNumWorkUnits(); i++)
    {
      File wuResult(distributableWorkUnitDirectory.getFullPathName() + T("/") + units->getWorkUnit(
          i)->toString() + T(".xml"));
      if (!wuResult.exists())
        remoteContext->pushWorkUnit(units->getWorkUnit(i),
            new DistributableExecutionContextCallback(context, distributable->toString()));
    }
    remoteContext->waitUntilAllWorkUnitsAreDone();

    // reload results
    Variable result = variableVector(units->getNumWorkUnits());
    for (size_t i = 0; i < units->getNumWorkUnits(); i++)
    {
      File wuResult(distributableWorkUnitDirectory.getFullPathName() + T("/") + units->getWorkUnit(
          i)->toString() + T(".xml"));
      Variable singleSolution = Variable::createFromFile(context, wuResult);
      result.getObjectAndCast<VariableVector> ()->setElement(i, singleSolution);
    }

    context.leaveScope();

    // show results
    context.enterScope(T("Results"));
    for (size_t i = 0; i < distributable->getWorkUnits()->getNumWorkUnits(); i++)
    {
      context.enterScope(distributable->getWorkUnits()->getWorkUnit(i));
      context.leaveScope(distributable->singleResultCallback(context, result.getObjectAndCast<
          VariableVector> ()->getElement(i)));
    }
    context.leaveScope();

    // managing results
    context.enterScope(T("Managing results"));
    Variable gatheredResult = distributable->multipleResultCallback(context,
        *result.getObjectAndCast<VariableVector> ());
    context.leaveScope(gatheredResult);

    context.informationCallback(T("cluster"), T("Distributable work unit done."));

    return gatheredResult;
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

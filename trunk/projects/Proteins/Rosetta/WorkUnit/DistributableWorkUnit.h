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
# include <lbcpp/Execution/ExecutionTrace.h>

namespace lbcpp
{

class DistributableExecutionContextCallback : public ExecutionContextCallback
{
public:
  DistributableExecutionContextCallback(ExecutionContext& context, String projectName)
    : context(context), projectName(projectName) {}

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result, const ExecutionTracePtr& trace)
  {
    savedResult = result;
    File fR(context.getProjectDirectory().getFullPathName() + T("/") + projectName + T("/") + workUnit->toString() + T(".xml"));
    savedResult.saveToFile(context, fR);

    savedTrace = trace;
    File fT(context.getProjectDirectory().getFullPathName() + T("/") + projectName + T("/") + workUnit->toString() + T(".trace"));
    savedTrace->saveToFile(context, fT);
  }

  static Variable gatherResults(VariableVector& callbacks)
  {
    VariableVectorPtr results = variableVector(callbacks.getNumElements());

    for (size_t i = 0; i < callbacks.getNumElements(); i++)
      results->setElement(i, callbacks.getElement(i).getObjectAndCast<DistributableExecutionContextCallback> ()->savedResult);

    return results;
  }

protected:
  ExecutionContext& context;
  String projectName;

  Variable savedResult;
  ExecutionTracePtr savedTrace;
};

typedef ReferenceCountedObjectPtr<DistributableExecutionContextCallback> DistributableExecutionContextCallbackPtr;

class DistributableWorkUnit : public WorkUnit
{
public:
  DistributableWorkUnit() {workUnits = new CompositeWorkUnit(T("DistributableWorkUnit"));}

  /**
   * Each subworkunit should open a scope for itself to avoid undesired effects
   * in traces.
   *
   * workUnits = new CompositeWorkUnit(T("blabla"));
   * workUnits->addWorkUnit(new WorkUnit());
   *
   */
  virtual void initializeWorkUnits(ExecutionContext& context) = 0;

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
    context.enterScope(T("DistributableWorkUnit::local : Initialization"));
    initializeWorkUnits(context);
    context.leaveScope();
    if (workUnits->getNumWorkUnits() == 0)
    {
      context.informationCallback(T("No work units to treat."));
      return Variable();
    }

    // computing time
    context.enterScope(T("DistributableWorkUnit::local : Computing"));
    context.informationCallback(T("Treating ") + String((int)workUnits->getNumWorkUnits()) + T(" work units."));
    Variable result = variableVector(workUnits->getNumWorkUnits());
    for (size_t i = 0; i < workUnits->getNumWorkUnits(); ++i)
      result.getObjectAndCast<VariableVector> ()->setElement(i, context.run(workUnits->getWorkUnit(i)));
    context.leaveScope();

    // show results
    context.enterScope(T("DistributableWorkUnit::local : singleResultCallback"));
    for (size_t i = 0; i < workUnits->getNumWorkUnits(); i++)
    {
      context.enterScope(workUnits->getWorkUnit(i));
      context.leaveScope(singleResultCallback(context, result.getObjectAndCast<VariableVector> ()->getElement(i)));
    }
    context.leaveScope();

    // managing results
    context.enterScope(T("DistributableWorkUnit::local : multipleResultCallback"));
    Variable gatheredResult = multipleResultCallback(context, *result.getObjectAndCast<VariableVector> ());
    context.leaveScope(gatheredResult);

    context.informationCallback(T("DistributableWorkUnit::local : Done"));

    return gatheredResult;
  }

protected:
  friend class DistributableWorkUnitClass;

  CompositeWorkUnitPtr workUnits;
};

typedef ReferenceCountedObjectPtr<DistributableWorkUnit> DistributableWorkUnitPtr;
extern ClassPtr distributableWorkUnitClass;

class DistributeToClusterWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ExecutionContextPtr remoteContext = distributedExecutionContext(context, remoteHostName, remoteHostPort, T("DistributableDirectory"), localHostLogin, clusterLogin, fixedResourceEstimator(1,
        memory, time), true);

    File distributableWorkUnitDirectory = context.getFile(distributable->toString());
    if (!distributableWorkUnitDirectory.exists())
      distributableWorkUnitDirectory.createDirectory();

    // initialization
    context.enterScope(T("DistributableWorkUnit::cluster : Initialization"));
    distributable->initializeWorkUnits(context);
    context.leaveScope();
    CompositeWorkUnitPtr units = distributable->getWorkUnits();
    size_t numWorkUnits = units->getNumWorkUnits();

    // computing time
    context.enterScope(T("DistributableWorkUnit::cluster : Computing"));
    context.informationCallback(T("Treating ") + String((int)units->getNumWorkUnits()) + T(" work units."));

    // check for already solved workunits and execute unsolved ones
    size_t numExecuted = 0;
    for (size_t i = 0; i < units->getNumWorkUnits(); i++)
    {
      File wuResult(distributableWorkUnitDirectory.getFullPathName() + T("/") + units->getWorkUnit(i)->toString() + T(".xml"));
      File wuTrace(distributableWorkUnitDirectory.getFullPathName() + T("/") + units->getWorkUnit(i)->toString() + T(".trace"));
      if (!wuResult.exists() || !wuTrace.exists())
      {
        if (numExecuted >= 20)
        {
          remoteContext->waitUntilAllWorkUnitsAreDone(time * 60 * 60 * 400);
          numExecuted = 0;
        }
        remoteContext->pushWorkUnit(units->getWorkUnit(i), new DistributableExecutionContextCallback(context, distributable->toString()));
        ++numExecuted;
      }
    }
    context.informationCallback(T("DistributableWorkUnit::cluster : all workunits sent"));
    remoteContext->waitUntilAllWorkUnitsAreDone();

    // reload results
    Variable result = variableVector(units->getNumWorkUnits());
    Variable traces = variableVector(units->getNumWorkUnits());
    for (size_t i = 0; i < units->getNumWorkUnits(); i++)
    {
      // reload results
      File wuResult(distributableWorkUnitDirectory.getFullPathName() + T("/") + units->getWorkUnit(i)->toString() + T(".xml"));
      Variable singleSolution = Variable::createFromFile(context, wuResult);
      result.getObjectAndCast<VariableVector> ()->setElement(i, singleSolution);

      // reload traces
      File wuTrace(distributableWorkUnitDirectory.getFullPathName() + T("/") + units->getWorkUnit(i)->toString() + T(".trace"));
      Variable singleTrace;
      if (wuTrace.exists())
        singleTrace = Variable::createFromFile(context, wuTrace);
      else
        singleTrace = ExecutionTracePtr();
      traces.getObjectAndCast<VariableVector> ()->setElement(i, singleTrace);
    }
    context.leaveScope();

    // show results
    context.enterScope(T("DistributableWorkUnit::cluster : singleResultCallback"));
    for (size_t i = 0; i < distributable->getWorkUnits()->getNumWorkUnits(); i++)
    {
      context.enterScope(distributable->getWorkUnits()->getWorkUnit(i));
      context.enterScope(T("Trace"));
      context.leaveScope(traces.getObjectAndCast<VariableVector> ()->getElement(i));
      context.enterScope(T("Result callback"));
      context.leaveScope(distributable->singleResultCallback(context, result.getObjectAndCast<VariableVector> ()->getElement(i)));
      context.leaveScope();
    }
    context.leaveScope();

    // managing results
    context.enterScope(T("DistributableWorkUnit::cluster : multipleResultCallback"));
    Variable gatheredResult = distributable->multipleResultCallback(context, *result.getObjectAndCast<VariableVector> ());
    context.leaveScope(gatheredResult);

    context.informationCallback(T("DistributableWorkUnit::cluster : Done"));

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

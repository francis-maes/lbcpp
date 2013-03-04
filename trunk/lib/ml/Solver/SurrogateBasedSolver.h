/*-----------------------------------------.---------------------------------.
| Filename: SurrogateBasedSolver.h         | Surrogate Based Solver          |
| Author  : Francis Maes                   |                                 |
| Started : 27/11/2012 14:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SOLVER_SURROGATE_BASED_H_
# define ML_SOLVER_SURROGATE_BASED_H_

# include <ml/Solver.h>
# include <ml/Sampler.h>
# include <ml/ExpressionDomain.h>
# include <ml/RandomVariable.h>
# include <ml/VariableEncoder.h>
# include <ml/SelectionCriterion.h>

namespace lbcpp
{

/** Class for surrogate-based optimization.
 *  In this class, the initial sampler should return an OVector with all \f$N\f$ initial samples
 *  as a result of its sample() function. The initial sample will be retrieved upon calling
 *  startSolver(), and will be added to the surrogate data in the first \f$N\f$ calls to iterateSolver.
 **/
class SurrogateBasedSolver : public IterativeSolver
{
public:
  SurrogateBasedSolver(SamplerPtr initialVectorSampler, SolverPtr surrogateLearner, SolverPtr surrogateSolver,
                       VariableEncoderPtr variableEncoder, SelectionCriterionPtr selectionCriterion, size_t numIterations)
    : IterativeSolver(numIterations), initialVectorSampler(initialVectorSampler), surrogateLearner(surrogateLearner),
      surrogateSolver(surrogateSolver), variableEncoder(variableEncoder), selectionCriterion(selectionCriterion) {}
  SurrogateBasedSolver() {}

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    IterativeSolver::startSolver(context, problem, callback, startingSolution);
    initialVectorSampler->initialize(context, new VectorDomain(problem->getDomain()));
    
    std::pair<ProblemPtr, TablePtr> p = createSurrogateLearningProblem(context, problem);
    surrogateLearningProblem = p.first;
    surrogateData = p.second;

    // initialize the surrogate data
    initialSamples = initialVectorSampler->sample(context).staticCast<OVector>();
  }

  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    ObjectPtr object;
    
    if (iter < initialSamples->getNumElements())
      object = initialSamples->getAndCast<Object>(iter);
    else
    {
      // learn surrogate
      if (verbosity >= verbosityDetailed)
        context.enterScope("Learn surrogate");
      ExpressionPtr surrogateModel = learnSurrogateModel(context, surrogateLearningProblem);
      if (verbosity >= verbosityDetailed)
      {
        context.resultCallback("surrogateModel", surrogateModel);
        context.leaveScope();
      }
      
      // optimize surrogate
      if (verbosity >= verbosityDetailed)
        context.enterScope("Optimize surrogate");
      object = optimizeSurrogate(context, surrogateModel);
      if (verbosity >= verbosityDetailed)
      {
        context.resultCallback("object", object);
        context.leaveScope();
      }
    }
    // evaluate point and add to training data
    FitnessPtr fitness = evaluate(context, object);
    
    currentBest = (!currentBest || fitness->dominates(currentBest) ? fitness : currentBest);
    
    if (verbosity >= verbosityDetailed)
      context.resultCallback("fitness", fitness);
    addSurrogateData(context, object, fitness, surrogateData);      
    return true;
  }

protected:
  friend class SurrogateBasedSolverClass;

  SamplerPtr initialVectorSampler;
  SolverPtr surrogateLearner;
  SolverPtr surrogateSolver;
  VariableEncoderPtr variableEncoder;
  SelectionCriterionPtr selectionCriterion;
  
  OVectorPtr initialSamples;
  
  FitnessPtr currentBest;
  
  ProblemPtr surrogateLearningProblem;

  TablePtr surrogateData;
  
  struct SurrogateBasedSelectionObjective : public Objective
  {
    SurrogateBasedSelectionObjective(VariableEncoderPtr encoder, ExpressionPtr model, SelectionCriterionPtr selectionCriterion)
      : encoder(encoder), model(model), selectionCriterion(selectionCriterion) {}
    
    virtual void getObjectiveRange(double& worst, double& best) const
      {return selectionCriterion->getObjectiveRange(worst, best);}
    
    virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
    {
      std::vector<ObjectPtr> row;
      encoder->encodeIntoVariables(context, object, row);
      return selectionCriterion->evaluate(context, model->compute(context, &row[0]));
    }
    
    VariableEncoderPtr encoder;
    ExpressionPtr model;
    SelectionCriterionPtr selectionCriterion;
    
  };
  
  ClassPtr getSurrogateSupervisionClass() const
    {return surrogateLearningProblem->getDomain().staticCast<ExpressionDomain>()->getSupervision()->getType();}

  std::pair<ProblemPtr, TablePtr> createSurrogateLearningProblem(ExecutionContext& context, ProblemPtr problem)
  {
    ExpressionDomainPtr surrogateDomain = createSurrogateDomain(context, problem);
    TablePtr data = surrogateDomain->createTable(0);
    ProblemPtr res = new Problem();
    res->setDomain(surrogateDomain);
    if (problem->getNumObjectives() == 1)
      res->addObjective(mseRegressionObjective(data, surrogateDomain->getSupervision()));
    else
      res->addObjective(mseMultiRegressionObjective(data, surrogateDomain->getSupervision()));
    return std::make_pair(res, data);
  }
  
  ExpressionDomainPtr createSurrogateDomain(ExecutionContext& context, ProblemPtr problem)
  {
    ExpressionDomainPtr res = new ExpressionDomain();
    
    DomainPtr domain = problem->getDomain();
    variableEncoder->createEncodingVariables(context, domain, res);
      
    if (problem->getNumObjectives() == 1)
      res->createSupervision(doubleClass, "y");
    else
    {
      DefaultEnumerationPtr objectivesEnumeration = new DefaultEnumeration("objectives");
      for (size_t i = 0; i < problem->getNumObjectives(); ++i)
        objectivesEnumeration->addElement(context, problem->getObjective(i)->toShortString());
      ClassPtr dvClass = denseDoubleVectorClass(objectivesEnumeration, doubleClass);
      res->createSupervision(dvClass, "y");
    }
    return res;
  }

  void addSurrogateData(ExecutionContext& context, ObjectPtr object, FitnessPtr fitness, TablePtr data)
  {
    std::vector<ObjectPtr> row;
    variableEncoder->encodeIntoVariables(context, object, row);
      
    if (fitness->getNumValues() == 1)
      row.push_back(new Double(fitness->getValue(0)));
    else
    {
      ClassPtr dvClass = getSurrogateSupervisionClass();
      DenseDoubleVectorPtr fitnessVector(new DenseDoubleVector(dvClass));
      for (size_t i = 0; i < fitness->getNumValues(); ++i)
        fitnessVector->setValue(i, fitness->getValue(i));
      row.push_back(fitnessVector);
    }
    surrogateLearningProblem->getObjective(0).staticCast<LearningObjective>()->getIndices()->append(data->getNumRows());
    data->addRow(row);
  }

  ExpressionPtr learnSurrogateModel(ExecutionContext& context, ProblemPtr surrogateLearningProblem)
  {
    ExpressionPtr res;
    surrogateLearner->solve(context, surrogateLearningProblem, storeBestSolutionSolverCallback(*(ObjectPtr* )&res));
    return res;
  }
   
  ProblemPtr createSurrogateOptimizationProblem(ExpressionPtr surrogateModel)
  {
    ProblemPtr res = new Problem();
    res->setDomain(problem->getDomain());
    selectionCriterion->initialize(problem);
    res->addObjective(new SurrogateBasedSelectionObjective(variableEncoder, surrogateModel, selectionCriterion));
    
    for (size_t i = 0; i < problem->getNumObjectives(); ++i)
      res->addValidationObjective(problem->getObjective(i));
    
    return res;
  }

  ObjectPtr optimizeSurrogate(ExecutionContext& context, ExpressionPtr surrogateModel)
  {
    ProblemPtr surrogateProblem = createSurrogateOptimizationProblem(surrogateModel);
    ObjectPtr res;
    FitnessPtr bestFitness;
    surrogateSolver->solve(context, surrogateProblem, storeBestSolverCallback(res, bestFitness));
    if (verbosity >= verbosityDetailed)
      context.resultCallback("surrogateObjectiveValue", bestFitness);
    return res;
  }
  
};
  
typedef ReferenceCountedObjectPtr<SurrogateBasedSolver> SurrogateBasedSolverPtr;

}; /* namespace lbcpp */

#endif // !ML_SOLVER_SURROGATE_BASED_H_

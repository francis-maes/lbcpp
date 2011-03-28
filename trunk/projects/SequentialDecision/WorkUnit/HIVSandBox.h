/*-----------------------------------------.---------------------------------.
| Filename: HIVSandBox.h                   | HIV Sand Box                    |
| Author  : Francis Maes                   |                                 |
| Started : 28/03/2011 13:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_HIV_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_HIV_SAND_BOX_H_

# include "../Core/SearchTree.h"
# include "../Core/SearchPolicy.h"
# include "../Core/SearchHeuristic.h"
# include "../Problem/DamienDecisionProblem.h"
# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class HIVSearchFeatures : public CompositeFunction
{
public:
  HIVSearchFeatures(double discount = 0.9) : discount(discount) {}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t node = builder.addInput(searchTreeNodeClass, T("node"));

    builder.startSelection();

      builder.addFunction(greedySearchHeuristic(), node, T("maxReward"));
      builder.addFunction(greedySearchHeuristic(discount), node, T("maxDiscountedReward"));
      builder.addFunction(maxReturnSearchHeuristic(), node, T("maxReturn"));
      builder.addFunction(optimisticPlanningSearchHeuristic(discount), node, T("optimistic"));
      builder.addFunction(minDepthSearchHeuristic(), node, T("minDepth"));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }

protected:
  double discount;
};

class HIVSearchHeuristic : public LearnableSearchHeuristic
{
public:
  HIVSearchHeuristic(DenseDoubleVectorPtr parameters)
    : parameters(parameters) {}

  virtual FunctionPtr createPerceptionFunction() const
    {return new HIVSearchFeatures();}

  virtual FunctionPtr createScoringFunction() const
  {
    NumericalLearnableFunctionPtr function = linearLearnableFunction();
    function->setParameters(parameters);
    return function;
  }

protected:
  DenseDoubleVectorPtr parameters;
};

// DenseDoubleVector -> Double
class EvaluateHIVSearchHeuristic : public SimpleUnaryFunction
{
public:
  EvaluateHIVSearchHeuristic(DecisionProblemPtr problem, size_t samplesCount, size_t maxSearchNodes)
    : SimpleUnaryFunction(TypePtr(), doubleType), problem(problem), samplesCount(samplesCount), maxSearchNodes(maxSearchNodes)
  {
    FunctionPtr tmp = new HIVSearchFeatures();
    tmp->initialize(defaultExecutionContext(), searchTreeNodeClass);
    inputType = denseDoubleVectorClass(DenseDoubleVector::getElementsEnumeration(tmp->getOutputType()));
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const DenseDoubleVectorPtr& parameters = input.getObjectAndCast<DenseDoubleVector>();
    FunctionPtr heuristic = new HIVSearchHeuristic(parameters);
    PolicyPtr searchPolicy = new BestFirstSearchPolicy(heuristic);

    double res = 0.0;
    for (size_t i = 0; i < samplesCount; ++i)
    {
      DecisionProblemStatePtr initialState = problem->sampleInitialState(RandomGenerator::getInstance());
      SearchTreePtr searchTree = new SearchTree(problem, initialState, maxSearchNodes);
      searchTree->doSearchEpisode(context, searchPolicy, maxSearchNodes);
      res += searchTree->getBestReturn();
    }      
    return res / samplesCount;
  }

protected:
  DecisionProblemPtr problem;
  size_t samplesCount;
  size_t maxSearchNodes;
};

extern ClassPtr independentDoubleVectorDistributionClass(TypePtr elementsEnumeration);

class IndependentDoubleVectorDistribution : public MultiVariateDistribution
{
public:
  IndependentDoubleVectorDistribution(EnumerationPtr elementsEnumeration)
    : MultiVariateDistribution(independentDoubleVectorDistributionClass(elementsEnumeration)),
      elementsEnumeration(elementsEnumeration), elementsType(denseDoubleVectorClass(elementsEnumeration))
  {
    subDistributions.resize(elementsEnumeration->getNumElements());
  }
  IndependentDoubleVectorDistribution() {}

  virtual TypePtr getElementsType() const
    {return elementsType;}

  virtual double computeEntropy() const
    {jassert(false); return 0.0;}

  virtual double computeProbability(const Variable& value) const
    {jassert(false); return 0.0;}

  virtual Variable sample(RandomGeneratorPtr random) const
  {
    DenseDoubleVectorPtr res(new DenseDoubleVector(elementsType, subDistributions.size()));
    for (size_t i = 0; i < subDistributions.size(); ++i)
      res->setValue(i, subDistributions[i]->sample(random).getDouble());
    return res;
  }

  virtual Variable sampleBest(RandomGeneratorPtr random) const
    {jassert(false); return Variable();}
  
  virtual DistributionBuilderPtr createBuilder() const
    {jassert(false); return DistributionBuilderPtr();}

  void setSubDistribution(size_t index, const DistributionPtr& distribution)
    {subDistributions[index] = distribution;}

protected:
  EnumerationPtr elementsEnumeration;
  TypePtr elementsType;
  std::vector<DistributionPtr> subDistributions;
};

typedef ReferenceCountedObjectPtr<IndependentDoubleVectorDistribution> IndependentDoubleVectorDistributionPtr;

class HIVSandBox : public WorkUnit
{
public:
  // default values
  HIVSandBox()
  {
  }

  virtual Variable run(ExecutionContext& context)
  {
    DecisionProblemPtr problem = hivDecisionProblem(1.0);
    if (!problem)
    {
      context.errorCallback(T("No decision problem"));
      return false;
    }

    FunctionPtr functionToOptimize = new EvaluateHIVSearchHeuristic(problem, 1, 100);
    EnumerationPtr featuresEnumeration = DoubleVector::getElementsEnumeration(functionToOptimize->getRequiredInputType(0, 1));
    ClassPtr parametersClass = denseDoubleVectorClass(featuresEnumeration, doubleType);

    IndependentDoubleVectorDistributionPtr initialDistribution = new IndependentDoubleVectorDistribution(featuresEnumeration);
    for (size_t i = 0; i < featuresEnumeration->getNumElements(); ++i)
      initialDistribution->setSubDistribution(i, new GaussianDistribution(0.0, 1.0));
    
    RandomGeneratorPtr random = RandomGenerator::getInstance();


    for (size_t i = 1; i < 20; ++i)
    {
      context.enterScope(T("iter ") + String((int)i));
      size_t maxNodes = (int)pow(2.0, (double)i);
      context.resultCallback(T("log2(maxNodes)"), i);
      context.resultCallback(T("maxNodes"), maxNodes);
      SearchTreePtr searchTree = new SearchTree(problem, problem->sampleInitialState(random), maxNodes);
      searchTree->doSearchEpisode(context, bestFirstSearchPolicy(new MaxReturnSearchHeuristic()), maxNodes);
      context.resultCallback(T("bestReturn"), searchTree->getBestReturn());
      context.leaveScope(searchTree->getBestReturn());
    }

    return true;

    for (size_t i = 0; i < 10; ++i)
    {
      context.enterScope(T("test ") + String((int)i));

      DenseDoubleVectorPtr parameters = initialDistribution->sample(random).getObjectAndCast<DenseDoubleVector>();
      context.resultCallback(T("parameters L0"), parameters->l0norm());
      context.resultCallback(T("parameters L2"), parameters->l2norm());
      double score = functionToOptimize->compute(context, parameters).getDouble();
      context.resultCallback(T("score"), score);

      context.leaveScope(score);
    }
    return true;
/*

    DecisionProblemStatePtr state = problem->sampleInitialState(RandomGenerator::getInstance());
    Variable action = state->getAvailableActions()->getElement(0);
    for (size_t i = 0; i < 50; ++i)
    {
      double reward;
      state->performTransition(action, reward);

      context.enterScope(T("Step ") + String((int)i + 1));
      context.resultCallback(T("step"), i + 1);
      context.resultCallback(T("log10(reward)"), log10(reward));
      for (size_t j = 0; j < 6; ++j)
        context.resultCallback(T("log10(state") + String((int)j + 1) + T(")"), log10(state.staticCast<DamienState>()->getStateDimension(j)));
      context.leaveScope(reward);
    }
    return true;*/
  }

private:
  friend class HIVSandBoxClass;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

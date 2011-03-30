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
  HIVSearchHeuristic(DenseDoubleVectorPtr parameters, double discount)
    : parameters(parameters), discount(discount) {}

  virtual FunctionPtr createPerceptionFunction() const
    {return new HIVSearchFeatures(discount);}

  virtual FunctionPtr createScoringFunction() const
  {
    NumericalLearnableFunctionPtr function = linearLearnableFunction();
    function->setParameters(parameters);
    return function;
  }

protected:
  DenseDoubleVectorPtr parameters;
  double discount;
};

// DenseDoubleVector -> Double
class EvaluateHIVSearchHeuristic : public SimpleUnaryFunction
{
public:
  EvaluateHIVSearchHeuristic(DecisionProblemPtr problem, size_t samplesCount, size_t maxSearchNodes, double discount)
    : SimpleUnaryFunction(TypePtr(), doubleType), problem(problem), samplesCount(samplesCount), maxSearchNodes(maxSearchNodes), discount(discount)
  {
    FunctionPtr tmp = new HIVSearchFeatures();
    tmp->initialize(defaultExecutionContext(), searchTreeNodeClass);
    inputType = denseDoubleVectorClass(DenseDoubleVector::getElementsEnumeration(tmp->getOutputType()));
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const DenseDoubleVectorPtr& parameters = input.getObjectAndCast<DenseDoubleVector>();
    FunctionPtr heuristic = new HIVSearchHeuristic(parameters, discount);
    PolicyPtr searchPolicy = new BestFirstSearchPolicy(heuristic);

    double res = 0.0;
    for (size_t i = 0; i < samplesCount; ++i)
    {
      DecisionProblemStatePtr initialState = problem->sampleInitialState(RandomGenerator::getInstance());
      SearchTreePtr searchTree = new SearchTree(problem, initialState, maxSearchNodes);
      searchTree->doSearchEpisode(context, searchPolicy, maxSearchNodes);
      res += searchTree->getBestReturn();
    }      
    return -res / samplesCount;
  }

protected:
  DecisionProblemPtr problem;
  size_t samplesCount;
  size_t maxSearchNodes;
  double discount;
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
    // /!\ This distribution only outputs normalized double vectors
    DenseDoubleVectorPtr res(new DenseDoubleVector(elementsType, subDistributions.size()));
    double sumOfSquares = 0.0;
    for (size_t i = 0; i < subDistributions.size(); ++i)
    {
      double value = subDistributions[i]->sample(random).getDouble();
      sumOfSquares += value * value;
      res->setValue(i, value);
    }
    if (sumOfSquares)
      res->multiplyByScalar(1.0 / sqrt(sumOfSquares)); // normalize
    return res;
  }

  virtual Variable sampleBest(RandomGeneratorPtr random) const
    {jassert(false); return Variable();}
  
  virtual DistributionBuilderPtr createBuilder() const;

  void setSubDistribution(size_t index, const DistributionPtr& distribution)
    {subDistributions[index] = distribution;}

  size_t getNumSubDistributions() const
    {return subDistributions.size();}

protected:
  friend class IndependentDoubleVectorDistributionClass;

  EnumerationPtr elementsEnumeration;
  TypePtr elementsType;
  std::vector<DistributionPtr> subDistributions;
};

typedef ReferenceCountedObjectPtr<IndependentDoubleVectorDistribution> IndependentDoubleVectorDistributionPtr;

class IndependentDoubleVectorDistributionBuilder : public DistributionBuilder
{
public:
  IndependentDoubleVectorDistributionBuilder(EnumerationPtr elementsEnumeration, const std::vector<DistributionBuilderPtr>& subBuilders)
    : elementsEnumeration(elementsEnumeration), subBuilders(subBuilders) {}
  IndependentDoubleVectorDistributionBuilder() {}

  virtual TypePtr getInputType() const
    {return denseDoubleVectorClass();}

  virtual void clear()
  {
    for (size_t i = 0; i < subBuilders.size(); ++i)
      subBuilders[i]->clear();
  }

  virtual void addElement(const Variable& element, double weight = 1.0)
  {
    const DenseDoubleVectorPtr& doubleVector = element.getObjectAndCast<DenseDoubleVector>();
    for (size_t i = 0; i < subBuilders.size(); ++i)
      subBuilders[i]->addElement(doubleVector->getValue(i), weight);
  }

  virtual void addDistribution(const DistributionPtr& distribution, double weight = 1.0)
    {jassert(false);}

  virtual DistributionPtr build(ExecutionContext& context) const
  {
    IndependentDoubleVectorDistributionPtr res = new IndependentDoubleVectorDistribution(elementsEnumeration);
    jassert(res->getNumSubDistributions() == subBuilders.size());
    for (size_t i = 0; i < subBuilders.size(); ++i)
      res->setSubDistribution(i, subBuilders[i]->build(context));
    return res;
  }

private:
  EnumerationPtr elementsEnumeration;
  std::vector<DistributionBuilderPtr> subBuilders;
};

inline DistributionBuilderPtr IndependentDoubleVectorDistribution::createBuilder() const
{
  std::vector<DistributionBuilderPtr> subBuilders(subDistributions.size());
  for (size_t i = 0; i < subBuilders.size(); ++i)
    subBuilders[i] = subDistributions[i]->createBuilder();
  return new IndependentDoubleVectorDistributionBuilder(elementsEnumeration, subBuilders);
}


class HIVSandBox : public WorkUnit
{
public:
  // default values
  HIVSandBox() : discount(0.98), maxSearchNodes(64), samplesCount(1),
                  iterations(100), populationSize(100), numBests(10)
  {
  }

  virtual Variable run(ExecutionContext& context)
  {
    if (numBests >= populationSize)
    {
      context.errorCallback(T("Invalid parameters: numBests is greater than populationSize"));
      return false;
    }

    DecisionProblemPtr problem = hivDecisionProblem(discount);
    if (!problem)
    {
      context.errorCallback(T("No decision problem"));
      return false;
    }

    FunctionPtr functionToOptimize = new EvaluateHIVSearchHeuristic(problem, samplesCount, maxSearchNodes, discount);
    EnumerationPtr featuresEnumeration = DoubleVector::getElementsEnumeration(functionToOptimize->getRequiredInputType(0, 1));
    context.informationCallback(String((int)featuresEnumeration->getNumElements()) + T(" parameters"));
    ClassPtr parametersClass = denseDoubleVectorClass(featuresEnumeration, doubleType);

    IndependentDoubleVectorDistributionPtr initialDistribution = new IndependentDoubleVectorDistribution(featuresEnumeration);
    for (size_t i = 0; i < featuresEnumeration->getNumElements(); ++i)
      initialDistribution->setSubDistribution(i, new GaussianDistribution(0.0, 1.0));
    
    RandomGeneratorPtr random = RandomGenerator::getInstance();

    double bestIndividualScore = evaluateEachFeature(context, functionToOptimize, featuresEnumeration);

    return performEDA(context, functionToOptimize, initialDistribution);
  }

private:
  friend class HIVSandBoxClass;

  double discount;
  size_t maxSearchNodes;
  size_t samplesCount;

  size_t iterations;
  size_t populationSize;
  size_t numBests;

  double performEDA(ExecutionContext& context, const FunctionPtr& functionToOptimize, const DistributionPtr& initialDistribution) const
  {
    double bestScore = DBL_MAX;
    DistributionPtr distribution = initialDistribution;
    context.enterScope(T("Optimizing"));
    for (size_t i = 0; i < iterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i + 1));
      double score = performEDAIteration(context, functionToOptimize, distribution);
      context.resultCallback(T("distribution"), distribution);
      context.leaveScope(score);
      if (score < bestScore)
        bestScore = score;
      context.progressCallback(new ProgressionState(i + 1, iterations, T("Iterations")));
    }
    context.leaveScope(bestScore);
    return bestScore;
  }

  double performEDAIteration(ExecutionContext& context, const FunctionPtr& functionToMinimize, DistributionPtr& distribution) const
  {
    jassert(numBests < populationSize);

    RandomGeneratorPtr random = RandomGenerator::getInstance();
    
    // generate evaluation work units
    CompositeWorkUnitPtr workUnit(new CompositeWorkUnit(T("Evaluating ") + String((int)populationSize) + T(" parameters"), populationSize));
    std::vector<Variable> inputs(populationSize);
    std::vector<Variable> scores(populationSize);
    for (size_t i = 0; i < populationSize; ++i)
    {
      inputs[i] = distribution->sample(random);
      workUnit->setWorkUnit(i, new FunctionWorkUnit(functionToMinimize, inputs[i], String::empty, &scores[i]));
    }
    workUnit->setProgressionUnit(T("parameters"));
    workUnit->setPushChildrenIntoStackFlag(false);

    // run work units
    context.run(workUnit);

    // sort by scores
    std::multimap<double, size_t> sortedScores;
    for (size_t i = 0; i < populationSize; ++i)
      sortedScores.insert(std::make_pair(scores[i].getDouble(), i));
    jassert(sortedScores.size() == populationSize);

    // build new distribution
    std::multimap<double, size_t>::const_iterator it = sortedScores.begin();
    DistributionBuilderPtr builder = distribution->createBuilder();
    for (size_t i = 0; i < numBests; ++i, ++it)
    {
      size_t index = it->second;
      context.informationCallback(T("Best ") + String((int)i + 1) + T(": ") + inputs[index].toShortString() + T(" (") + scores[index].toShortString() + T(")"));
      builder->addElement(inputs[index]);
    }
    distribution = builder->build(context);

    // return best score
    return sortedScores.begin()->first;
  }

  double evaluateEachFeature(ExecutionContext& context, const FunctionPtr& functionToMinimize, EnumerationPtr featuresEnumeration) const
  {
    context.enterScope(T("Evaluating features"));
    size_t n = featuresEnumeration->getNumElements();
    CompositeWorkUnitPtr workUnit(new CompositeWorkUnit(T("Evaluating ") + String((int)n) + T(" features"), n));
    std::vector<Variable> scores(n);
    for (size_t i = 0; i < n; ++i)
    {
      DenseDoubleVectorPtr doubleVector = new DenseDoubleVector(featuresEnumeration, doubleType, n);
      doubleVector->setValue(i, 1.0);
      workUnit->setWorkUnit(i, new FunctionWorkUnit(functionToMinimize, doubleVector, String::empty, &scores[i]));
    }
    workUnit->setProgressionUnit(T("features"));
    workUnit->setPushChildrenIntoStackFlag(false);
    context.run(workUnit);

    double bestScore = DBL_MAX;
    for (size_t i = 0; i < n; ++i)
    {
      double score = scores[i].getDouble();
      if (score < bestScore)
        bestScore = score;
      context.resultCallback(featuresEnumeration->getElementName(i), score);
    }
    context.leaveScope(bestScore);
    return bestScore;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_

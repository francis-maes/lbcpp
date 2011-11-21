/*-----------------------------------------.---------------------------------.
| Filename: LuapeGradientBoostingLearner.h | Luape Gradient Boosting Learner |
| Author  : Francis Maes                   |  base class                     |
| Started : 21/11/2011 15:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_GRAPH_LEARNER_GRADIENT_BOOSTING_H_
# define LBCPP_LUAPE_GRAPH_LEARNER_GRADIENT_BOOSTING_H_

# include "LuapeGraphLearner.h"

namespace lbcpp
{

class LuapeGradientBoostingLearner : public LuapeGraphLearner
{
public:
  LuapeGradientBoostingLearner(double learningRate, size_t maxDepth)
    : learningRate(learningRate), maxDepth(maxDepth)
  {
  }  
  LuapeGradientBoostingLearner() : learningRate(0.0), maxDepth(maxDepth) {}

  /*
  ** Gradient boosting
  */
  virtual LuapeNodePtr doWeakLearning(ExecutionContext& context, const DenseDoubleVectorPtr& predictions) const = 0;
  virtual bool addWeakLearnerToGraph(ExecutionContext& context, const DenseDoubleVectorPtr& predictions, LuapeNodePtr completion)
  {
    // 1- compute optimal weight
    LuapeNodeCachePtr weakLearnerCache = completion->getCache();
    BooleanVectorPtr weakPredictions = weakLearnerCache->getSamples(true).staticCast<BooleanVector>();
    double optimalWeight = problem->getObjective()->optimizeWeightOfWeakLearner(context, predictions, weakPredictions);
    context.informationCallback(T("Optimal weight: ") + String(optimalWeight));
    
    // 2- add weak learner and associated weight to graph 
    graph->pushMissingNodes(context, completion);
    if (completion->getType() == booleanType)
    {
      // booleans are yielded directory
      graph->pushNode(context, new LuapeYieldNode(completion));
    }
    else
    {
      jassert(completion->getType()->isConvertibleToDouble());
      
      jassert(false); // todo: create stump
    }
    function->getVotes()->append(optimalWeight * learningRate);
    if ((graph->getNumNodes() % 100) == 0)
      context.informationCallback(T("Graph: ") + graph->toShortString());
    return true;
  }

  /*
  ** LuapeGraphLearner
  */
  virtual bool doLearningIteration(ExecutionContext& context)
  {
    LuapeObjectivePtr objective = problem->getObjective();

    double time = juce::Time::getMillisecondCounterHiRes();

    // 1- compute graph outputs and compute loss derivatives
    context.enterScope(T("Computing predictions"));
    DenseDoubleVectorPtr predictions = function->computeSamplePredictions(context, true);
    context.leaveScope();
    context.enterScope(T("Computing pseudo-residuals"));
    double lossValue;
    objective->computeLoss(predictions, &lossValue, &pseudoResiduals);
    context.leaveScope();
    context.resultCallback(T("loss"), lossValue);

    context.resultCallback(T("predictions"), predictions);
    context.resultCallback(T("pseudoResiduals"), pseudoResiduals);

    double newTime = juce::Time::getMillisecondCounterHiRes();
    context.resultCallback(T("compute predictions and residuals time"), (newTime - time) / 1000.0);
    time = newTime;

    // 2- find best weak learner
    LuapeNodePtr weakLearner = doWeakLearning(context, predictions);
    if (!weakLearner)
      return false;

    newTime = juce::Time::getMillisecondCounterHiRes();
    context.resultCallback(T("learn weak time"), (newTime - time) / 1000.0);
    time = newTime;

    // 3- add weak learner to graph
    if (!addWeakLearnerToGraph(context, predictions, weakLearner))
      return false;

    newTime = juce::Time::getMillisecondCounterHiRes();
    context.resultCallback(T("add weak to graph time"), (newTime - time) / 1000.0);

    // ok
    return true;
  }
  
  virtual double computeCompletionReward(ExecutionContext& context, const LuapeNodePtr& completion) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();

    completion->updateCache(context, true);

    ContainerPtr samples = completion->getCache()->getTrainingSamples();
    DenseDoubleVectorPtr scalars = samples.dynamicCast<DenseDoubleVector>();
    if (scalars)
    {
      jassert(false); // FIXME: evaluate quality of decision stump
      return 0.0;
    }
    else
    {
      std::vector<bool>::const_iterator it = samples.staticCast<BooleanVector>()->getElements().begin();

      ScalarVariableMeanAndVariance trainPositive;
      ScalarVariableMeanAndVariance validationPositive;
      ScalarVariableMeanAndVariance trainNegative;
      ScalarVariableMeanAndVariance validationNegative;

      for (size_t i = 0; i < pseudoResiduals->getNumValues(); ++i)
      {
        bool isPositive = *it++;

        double value = pseudoResiduals->getValue(i);
        double weight = 1.0; // fabs(value)

        //if (random->sampleBool(p))
          (isPositive ? trainPositive : trainNegative).push(value, weight);
        //else
          (isPositive ? validationPositive : validationNegative).push(value, weight);
      }
      
      double meanSquareError = 0.0;
      if (validationPositive.getCount())
        meanSquareError += validationPositive.getCount() * (trainPositive.getSquaresMean() + validationPositive.getSquaresMean() 
                                                              - 2 * trainPositive.getMean() * validationPositive.getMean());
      if (validationNegative.getCount())
        meanSquareError += validationNegative.getCount() * validationNegative.getSquaresMean(); // when negative, we always predict 0
  //      meanSquareError += validationNegative.getCount() * (trainNegative.getSquaresMean() + validationNegative.getSquaresMean() 
  //                                                            - 2 * trainNegative.getMean() * validationNegative.getMean());

      if (validationPositive.getCount() || validationNegative.getCount())
        meanSquareError /= validationPositive.getCount() + validationNegative.getCount();
      
      return 1.0 - sqrt(meanSquareError) / 2.0; // ... bring into [0,1]
    }
  }

protected:
  double learningRate;
  size_t maxDepth;

  DenseDoubleVectorPtr pseudoResiduals;
};

typedef ReferenceCountedObjectPtr<LuapeGradientBoostingLearner> LuapeGradientBoostingLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_LEARNER_GRADIENT_BOOSTING_H_

/*-----------------------------------------.---------------------------------.
| Filename: LuapeGradientBoostingLearner.h | Luape Gradient Boosting Learner |
| Author  : Francis Maes                   |  base class                     |
| Started : 21/11/2011 15:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_GRAPH_LEARNER_GRADIENT_BOOSTING_H_
# define LBCPP_LUAPE_GRAPH_LEARNER_GRADIENT_BOOSTING_H_

# include "LuapeLearner.h"

namespace lbcpp
{

class LuapeGradientBoostingLearner : public LuapeBoostingLearner
{
public:
  LuapeGradientBoostingLearner(LuapeWeakLearnerPtr weakLearner, double learningRate)
    : LuapeBoostingLearner(weakLearner), learningRate(learningRate)
  {
  }  
  LuapeGradientBoostingLearner() : learningRate(0.0) {}

  virtual bool doLearningIteration(ExecutionContext& context)
  {
    // TODO: update predictions incrementally 

    // 1- compute predictions and pseudo residuals 
    computePredictionsAndPseudoResiduals(context);

    // 2- find best weak learner
    BooleanVectorPtr weakPredictions;
    LuapeNodePtr weakNode = doWeakLearningAndAddToGraph(context, weakPredictions);
    if (!weakNode)
      return false;

    // 3- add weak learner to graph
    {
      TimedScope _(context, "optimize weight");
      double optimalWeight = problem->getObjective()->optimizeWeightOfWeakLearner(context, predictions, weakPredictions);
      function->getVotes()->append(optimalWeight * learningRate);
    }
    return true;
  }

  virtual double computeWeakObjective(ExecutionContext& context, const LuapeNodePtr& completion) const
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

  virtual double computeBestStumpThreshold(ExecutionContext& context, const LuapeNodePtr& numberNode) const
  {
    jassert(false); // todo: implement
    return 0.0;
  }

protected:
  double learningRate;

  DenseDoubleVectorPtr predictions;
  DenseDoubleVectorPtr pseudoResiduals;

  void computePredictionsAndPseudoResiduals(ExecutionContext& context)
  {
    TimedScope _(context, "compute predictions and residuals");

    context.enterScope(T("Computing predictions"));
    predictions = function->computeSamplePredictions(context, true);
    context.leaveScope();
    context.enterScope(T("Computing pseudo-residuals"));
    double lossValue;
    problem->getObjective()->computeLoss(predictions, &lossValue, &pseudoResiduals);
    context.leaveScope();
    context.resultCallback(T("loss"), lossValue);

    context.resultCallback(T("predictions"), predictions);
    context.resultCallback(T("pseudoResiduals"), pseudoResiduals);
  }
};

typedef ReferenceCountedObjectPtr<LuapeGradientBoostingLearner> LuapeGradientBoostingLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_LEARNER_GRADIENT_BOOSTING_H_

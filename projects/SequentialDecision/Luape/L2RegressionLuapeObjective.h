/*-----------------------------------------.---------------------------------.
| Filename: L2RegressionLuapeObjective.h   | Luape L2-Regression Objective   |
| Author  : Francis Maes                   |                                 |
| Started : 19/11/2011 13:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_OBJECTIVE_L2_REGRESSION_H_
# define LBCPP_LUAPE_OBJECTIVE_L2_REGRESSION_H_

# include "LuapeProblem.h"

namespace lbcpp
{

class L2RegressionLuapeObjective : public LuapeObjective
{
public:
 virtual bool initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function)
  {
    this->problem = problem;
    this->function = function;
    this->graph = function->getGraph();
    return true;
  }

  virtual void setExamples(bool isTrainingData, const std::vector<ObjectPtr>& data)
  {
    graph->clearSamples(isTrainingData, !isTrainingData);
    LuapeNodeCachePtr inputNodeCache = graph->getNode(0)->getCache();
    inputNodeCache->resizeSamples(isTrainingData, data.size());
    DenseDoubleVectorPtr supervisions = new DenseDoubleVector(data.size(), 0.0);
    for (size_t i = 0; i < data.size(); ++i)
    {
      const PairPtr& example = data[i].staticCast<Pair>();
      inputNodeCache->setSample(isTrainingData, i, example->getFirst());
      supervisions->setValue(i, example->getSecond().getDouble());
    }

    if (isTrainingData)
      trainingSupervisions = supervisions;
    else
      validationSupervisions = supervisions;
  }

  virtual void computeLoss(const DenseDoubleVectorPtr& predictions, double* lossValue, DenseDoubleVectorPtr* lossGradient) const
  {
    if (lossValue)
      *lossValue = 0.0;
    if (lossGradient)
      *lossGradient = new DenseDoubleVector(predictions->getNumValues(), 0.0);
  
    size_t n = trainingSupervisions->getNumValues();
    jassert(n == predictions->getNumValues());
    size_t index = 0;
    for (size_t i = 0; i < n; ++i)
    {
      double predicted = predictions->getValue(i);
      double correct = trainingSupervisions->getValue(i);

      if (lossValue)
        *lossValue += (predicted - correct) * (predicted - correct);
      if (lossGradient)
        (*lossGradient)->setValue(i, correct - predicted);
    }
    if (lossValue)
      *lossValue /= n;
    if (lossGradient)
      (*lossGradient)->multiplyByScalar(-1.0);
  }

protected:
  LuapeProblemPtr problem;
  LuapeInferencePtr function;
  LuapeGraphPtr graph;

  DenseDoubleVectorPtr trainingSupervisions;
  DenseDoubleVectorPtr validationSupervisions;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_OBJECTIVE_L2_REGRESSION_H_

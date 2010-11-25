/*-----------------------------------------.---------------------------------.
| Filename: RankingInference.h             | Ranking Inference classes       |
| Author  : Francis Maes                   |                                 |
| Started : 25/10/2010 15:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_INFERENCE_RANKING_H_
# define LBCPP_NUMERICAL_LEARNING_INFERENCE_RANKING_H_

# include <lbcpp/Inference/DecoratorInference.h>
# include <lbcpp/Inference/ParallelInference.h>
# include <lbcpp/Inference/SequentialInference.h>
# include <lbcpp/NumericalLearning/LossFunctions.h>
# include "LinearInference.h"

namespace lbcpp
{

class LossBasedRankingInference : public SharedParallelInference
{
public:
  LossBasedRankingInference(const String& name, InferencePtr scoreInference)
    : SharedParallelInference(name, scoreInference) {}
  LossBasedRankingInference() {}

  virtual TypePtr getInputType() const
    {return containerClass(getSubInference()->getInputType());} // input vector

  virtual TypePtr getSupervisionType() const
    {return rankingLossFunctionClass;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return containerClass(doubleType);} // score vector

  virtual ParallelInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    const ContainerPtr& inputContainer = input.getObjectAndCast<Container>(context);
    jassert(inputContainer);
    size_t n = inputContainer->getNumElements();

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve(n);
    for (size_t i = 0; i < n; ++i)
      res->addSubInference(subInference, inputContainer->getElement(i), Variable());
    return res;
  }

  virtual Variable finalizeInference(ExecutionContext& context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    size_t n = state->getNumSubInferences();
    VectorPtr res = vector(doubleType, n);
    bool atLeastOneResult = false;
    for (size_t i = 0; i < n; ++i)
    {
      Variable result = state->getSubOutput(i);
      if (result.exists())
      {
        res->setElement(i, result.getDouble());
        atLeastOneResult = true;
      }
    }
    return atLeastOneResult ? res : Variable::missingValue(res->getClass());
  }
};

class RankingInference : public NumericalSupervisedInference
{
public:
  RankingInference(const String& name, InferencePtr scoreInference)
    : NumericalSupervisedInference(name, new LossBasedRankingInference(name, scoreInference)) {}
  RankingInference() {}
 
  virtual TypePtr getSupervisionType() const
    {return containerClass(doubleType);}

  virtual void setName(const String& name)
    {StaticDecoratorInference::setName(name); decorated->setName(name + T(" scores"));}

  virtual DecoratorInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);
    RankingLossFunctionPtr lossFunction;
    if (supervision.exists())
    {
      const ContainerPtr& costContainer = supervision.getObjectAndCast<Container>(context);
      std::vector<double> costs(costContainer->getNumElements());
      for (size_t i = 0; i < costs.size(); ++i)
        costs[i] = costContainer->getElement(i).getDouble();
      lossFunction = createRankingLoss(costs);
    }
    res->setSubInference(decorated, input, lossFunction);
    return res;
  }

protected:
  virtual RankingLossFunctionPtr createRankingLoss(const std::vector<double>& costs) const = 0;
};

class AdditiveRankingInference : public RankingInference
{
public:
  AdditiveRankingInference(const String& name, InferencePtr scoreInference, BinaryClassificationLossFunctionPtr baseLoss)
    : RankingInference(name, scoreInference), baseLoss(baseLoss) {}
  AdditiveRankingInference() {}

protected:
  friend class AdditiveRankingInferenceClass;

  BinaryClassificationLossFunctionPtr baseLoss;
};

class MostViolatedPairRankingInference : public AdditiveRankingInference
{
public:
  MostViolatedPairRankingInference(const String& name, InferencePtr scoreInference, BinaryClassificationLossFunctionPtr baseLoss)
    : AdditiveRankingInference(name, scoreInference, baseLoss) {}
  MostViolatedPairRankingInference() {}

  virtual RankingLossFunctionPtr createRankingLoss(const std::vector<double>& costs) const
    {return mostViolatedPairRankingLossFunction(baseLoss, costs);}
};

class BestAgainstAllRankingInference : public AdditiveRankingInference
{
public:
  BestAgainstAllRankingInference(const String& name, InferencePtr scoreInference, BinaryClassificationLossFunctionPtr baseLoss)
    : AdditiveRankingInference(name, scoreInference, baseLoss) {}
  BestAgainstAllRankingInference() {}

  virtual RankingLossFunctionPtr createRankingLoss(const std::vector<double>& costs) const
    {return bestAgainstAllRankingLossFunction(baseLoss, costs);}
};

class AllPairsRankingInference : public AdditiveRankingInference
{
public:
  AllPairsRankingInference(const String& name, InferencePtr scoreInference, BinaryClassificationLossFunctionPtr baseLoss)
    : AdditiveRankingInference(name, scoreInference, baseLoss) {}
  AllPairsRankingInference() {}

  virtual RankingLossFunctionPtr createRankingLoss(const std::vector<double>& costs) const
    {return allPairsRankingLossFunction(baseLoss, costs);}
};

class AllPairsRankingLinearSVMInference : public AllPairsRankingInference
{
public:
  AllPairsRankingLinearSVMInference(const String& name, PerceptionPtr perception)
    : AllPairsRankingInference(name, linearInference(name, perception), hingeLossFunction(true)) {}
  AllPairsRankingLinearSVMInference() {}

  virtual RankingLossFunctionPtr createRankingLoss(const std::vector<double>& costs) const
    {return allPairsRankingLossFunction(baseLoss, costs);}
};

class BinaryClassificationRankingLinearSVMInference : public AdditiveRankingInference
{
public:
  BinaryClassificationRankingLinearSVMInference(const String& name, PerceptionPtr perception, bool optimizeMcc)
    : AdditiveRankingInference(name, linearInference(name, perception), hingeLossFunction(true)), optimizeMcc(optimizeMcc) {}
  BinaryClassificationRankingLinearSVMInference() : optimizeMcc(false) {}

  virtual RankingLossFunctionPtr createRankingLoss(const std::vector<double>& costs) const
    {return optimizeMcc ? mccRankingLossFunction(baseLoss, costs) : f1ScoreRankingLossFunction(baseLoss, costs);}

protected:
  friend class BinaryClassificationRankingLinearSVMInferenceClass;

  bool optimizeMcc;
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_INFERENCE_MULTI_CLASS_H_

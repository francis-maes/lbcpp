/*-----------------------------------------.---------------------------------.
| Filename: LuapeProblem.h                 | Lua Program Evolution Problem   |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2011 18:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_PROBLEM_H_
# define LBCPP_LUAPE_PROBLEM_H_

# include "LuapeGraph.h"
# include "LuapeInference.h"
# include <lbcpp/Learning/LossFunction.h>

namespace lbcpp
{

class LuapeProblem;
typedef ReferenceCountedObjectPtr<LuapeProblem> LuapeProblemPtr;

class LuapeObjective : public Object
{
public:
  virtual bool initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function) = 0;
  virtual void setExamples(bool isTrainingData, const std::vector<ObjectPtr>& data) = 0;
  
  // specific to gradient boosting
  virtual void computeLoss(const DenseDoubleVectorPtr& predictions, double* lossValue, DenseDoubleVectorPtr* lossGradient) const = 0;

  virtual double optimizeWeightOfWeakLearner(ExecutionContext& context, const DenseDoubleVectorPtr& predictions, const BooleanVectorPtr& weakPredictions) const
  {
    context.enterScope(T("Optimize weight"));

    double bestLoss = DBL_MAX;
    double bestWeight = 0.0;

    for (double K = -2.5; K <= 2.5; K += 0.02)
    {
      context.enterScope(T("K = ") + String(K));
      context.resultCallback(T("K"), K);

      DenseDoubleVectorPtr newPredictions = predictions->cloneAndCast<DenseDoubleVector>();
      for (size_t i = 0; i < weakPredictions->getNumElements(); ++i)
        if (weakPredictions->get(i))
          newPredictions->incrementValue(i, K);
      double lossValue;
      computeLoss(newPredictions, &lossValue, NULL);

      if (lossValue < bestLoss)
      {
        bestLoss = lossValue;
        bestWeight = K;
      }

      context.resultCallback(T("loss"), lossValue);
      context.leaveScope(lossValue);
    }

    context.leaveScope(bestLoss);
    return bestWeight;
  }
};

typedef ReferenceCountedObjectPtr<LuapeObjective> LuapeObjectivePtr;

extern LuapeObjectivePtr l2RegressionLuapeObjective();
extern LuapeObjectivePtr rankingLuapeObjective(RankingLossFunctionPtr rankingLoss);

class LuapeProblem : public Object
{
public:
  LuapeProblem(LuapeObjectivePtr objective = LuapeObjectivePtr())
    : objective(objective), failed(false) {}

  void addInput(const TypePtr& type, const String& name)
    {inputs.push_back(new VariableSignature(type, name));}

  size_t getNumInputs() const
    {return inputs.size();}

  const VariableSignaturePtr& getInput(size_t index) const
    {jassert(index < inputs.size()); return inputs[index];}

  void addFunction(const LuapeFunctionPtr& function)
    {functions.push_back(function);}

  size_t getNumFunctions() const
    {return functions.size();}

  const LuapeFunctionPtr& getFunction(size_t index) const
    {jassert(index < functions.size()); return functions[index];}

  LuapeGraphPtr createInitialGraph(ExecutionContext& context) const
  {
    LuapeGraphPtr res = new LuapeGraph();
    for (size_t i = 0; i < inputs.size(); ++i)
      res->pushNode(context, new LuapeInputNode(inputs[i]->getType(), inputs[i]->getName(), i));
    return res;
  }

  static int input(LuaState& state);
  static int function(LuaState& state);
  //static int objective(LuaState& state);
  
  const LuapeObjectivePtr& getObjective() const
    {return objective;}

protected:
  friend class LuapeProblemClass;

  LuapeObjectivePtr objective;
  std::vector<VariableSignaturePtr> inputs;
  std::vector<LuapeFunctionPtr> functions;
  bool failed;
};

extern ClassPtr luapeProblemClass;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_PROBLEM_H_

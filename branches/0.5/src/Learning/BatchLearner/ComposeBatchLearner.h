/*-----------------------------------------.---------------------------------.
| Filename: ComposeBatchLearner.h          | Compose Batch Learner           |
| Author  : Julien Becker                  |                                 |
| Started : 06/10/2011 10:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_BATCH_LEARNER_COMPOSE_H_
# define LBCPP_CORE_BATCH_LEARNER_COMPOSE_H_

# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Core/DynamicObject.h>
# include "../../Core/Function/ComposeFunction.h"

namespace lbcpp
{

class ComposeBatchLearner : public BatchLearner
{
public:
  virtual TypePtr getRequiredFunctionType() const
    {return composeFunctionClass;}

  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    ComposeFunctionPtr composeFunction = function.staticCast<Function>();
    if (composeFunction->f->hasBatchLearner() && !composeFunction->f->train(context, trainingData, validationData))
    {
      context.errorCallback(T("ComposeBatchLearner::train"), T("Error while training f !"));
      return false;
    }

    if (!composeFunction->g->hasBatchLearner())
      return true;

    std::vector<ObjectPtr> trainingDataForG;
    applyFunctionOnData(context, composeFunction->f, trainingData, trainingDataForG);
    std::vector<ObjectPtr> validationDataForG;
    applyFunctionOnData(context, composeFunction->f, validationData, validationDataForG);

    if (!composeFunction->g->train(context, trainingDataForG, validationDataForG))
    {
      context.errorCallback(T("ComposeBatchLearner::train"), T("Error while training g !"));
      return false;
    }

    return true;
  }

protected:
  friend class ComposeBatchLearnerClass;

  void applyFunctionOnData(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& data, std::vector<ObjectPtr>& results) const
  {
    ClassPtr inputsClass = f->getInputsClass();
    results.resize(data.size());
    for (size_t i = 0; i < data.size(); ++i)
    {
      ObjectPtr res = Object::create(inputsClass);
      res->setVariable(0, f->compute(context, data[i]->getVariable(0)));
      results[i] = res;
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_BATCH_LEARNER_COMPOSE_H_

/*-----------------------------------------.---------------------------------.
| Filename: ContainerElementsEvaluator.h   | Container Evaluator             |
| Author  : Francis Maes                   |                                 |
| Started : 01/03/2011 16:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SUPERVISED_EVALUATOR_CONTAINER_H_
# define LBCPP_FUNCTION_SUPERVISED_EVALUATOR_CONTAINER_H_

# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Core/Container.h>

namespace lbcpp
{

class ContainerSupervisedEvaluator : public SupervisedEvaluator
{
public:
  ContainerSupervisedEvaluator(SupervisedEvaluatorPtr elementEvaluator)
    : elementEvaluator(elementEvaluator) {}
  ContainerSupervisedEvaluator() {}

  virtual TypePtr getRequiredPredictionType() const
    {return elementEvaluator->getRequiredPredictionType();}

  virtual TypePtr getRequiredSupervisionType() const
    {return elementEvaluator->getRequiredSupervisionType();}

  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context) const
    {return elementEvaluator->createEmptyScoreObject(context);}
  
  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const
  {
    ContainerPtr supervisionContainer = example->getVariable(1).dynamicCast<Container>();
    ContainerPtr predictedContainer = output.dynamicCast<Container>();
    size_t n = predictedContainer->getNumElements();
    jassert(supervisionContainer->getNumElements() == n);
    for (size_t i = 0; i < n; ++i)
    {
      Variable supervision = supervisionContainer->getElement(i);
      Variable predicted = predictedContainer->getElement(i);
      if (!supervision.exists())
        continue;
      if (!predicted.exists())
      {
        context.errorCallback(T("ContainerSupervisedEvaluator::updateScoreObject"), T("Missing prediction"));
        return false;
      }
      addPrediction(context, predicted, supervision, scores);
    }
    return true;
  }

  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const
    {elementEvaluator->addPrediction(context, prediction, supervision, result);}

  virtual void finalizeScoreObject(const ScoreObjectPtr& scores) const
    {elementEvaluator->finalizeScoreObject(scores);}

protected:
  friend class ContainerSupervisedEvaluatorClass;

  SupervisedEvaluatorPtr elementEvaluator;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SUPERVISED_EVALUATOR_CONTAINER_H_

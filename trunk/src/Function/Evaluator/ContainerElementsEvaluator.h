/*-----------------------------------------.---------------------------------.
| Filename: ContainerElementsEvaluator.h   | Container Evaluator             |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 16:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_EVALUATOR_CONTAINER_ELEMENTS_H_
# define LBCPP_FUNCTION_EVALUATOR_CONTAINER_ELEMENTS_H_

# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Core/Container.h>

namespace lbcpp
{

class ContainerElementsEvaluator : public Evaluator
{
public:
  ContainerElementsEvaluator(EvaluatorPtr elementEvaluator)
    : elementEvaluator(elementEvaluator) {}
  ContainerElementsEvaluator() {}

  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context) const
    {return elementEvaluator->createEmptyScoreObject(context);}
  
  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const
  {
    ContainerPtr examplesContainer = example.dynamicCast<Container>();
    ContainerPtr outputContainer = output.dynamicCast<Container>();
    size_t n = outputContainer->getNumElements();
    jassert(examplesContainer->getNumElements() == n);
    for (size_t i = 0; i < n; ++i)
      if (!elementEvaluator->updateScoreObject(context, scores, examplesContainer->getElement(i).getObject(), outputContainer->getElement(i).getObject()))
        return false;
    return true;
  }

protected:
  friend class ContainerElementsEvaluatorClass;

  EvaluatorPtr elementEvaluator;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_CONTAINER_ELEMENTS_H_

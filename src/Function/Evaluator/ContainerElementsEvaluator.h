/*-----------------------------------------.---------------------------------.
| Filename: ContainerElementsEvaluator.h   | Container Evaluator             |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 16:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_EVALUATOR_CONTAINER_ELEMENTS_H_
# define LBCPP_FUNCTION_EVALUATOR_CONTAINER_ELEMENTS_H_

# include <lbcpp/Function/OldEvaluator.h>
# include <lbcpp/Core/Container.h>

namespace lbcpp
{

class ContainerElementsEvaluator : public OldEvaluator
{
public:
  ContainerElementsEvaluator(const String& name, OldEvaluatorPtr elementEvaluator)
    : OldEvaluator(name), elementEvaluator(elementEvaluator) {}
  ContainerElementsEvaluator() {}

  virtual String toString() const
    {return elementEvaluator->toString();}

  virtual double getDefaultScore() const
    {return elementEvaluator->getDefaultScore();}

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
    {elementEvaluator->getScores(res);}

  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject)
  {
    if (!predictedObject.exists() || !correctObject.exists())
      return;

    ContainerPtr predicted = predictedObject.dynamicCast<Container>();
    ContainerPtr correct = correctObject.dynamicCast<Container>();
    size_t n = predicted->getNumElements();
    jassert(correct->getNumElements() == n);
    for (size_t i = 0; i < n; ++i)
      elementEvaluator->addPrediction(context, predicted->getElement(i), correct->getElement(i));
  }

protected:
  friend class ContainerElementsEvaluatorClass;

  OldEvaluatorPtr elementEvaluator;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_CONTAINER_ELEMENTS_H_

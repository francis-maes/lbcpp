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
# include <lbcpp/Data/Container.h>

namespace lbcpp
{

class ContainerElementsEvaluator : public Evaluator
{
public:
  ContainerElementsEvaluator(const String& name, EvaluatorPtr elementEvaluator)
    : Evaluator(name), elementEvaluator(elementEvaluator) {}
  ContainerElementsEvaluator() {}

  virtual String toString() const
    {return elementEvaluator->toString();}

  virtual double getDefaultScore() const
    {return elementEvaluator->getDefaultScore();}

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
    {elementEvaluator->getScores(res);}

  virtual void addPrediction(const Variable& predictedObject, const Variable& correctObject)
  {
    if (!predictedObject || !correctObject)
      return;

    ContainerPtr predicted = predictedObject.dynamicCast<Container>();
    ContainerPtr correct = correctObject.dynamicCast<Container>();
    size_t n = predicted->getNumElements();
    jassert(correct->getNumElements() == n);
    for (size_t i = 0; i < n; ++i)
      elementEvaluator->addPrediction(predicted->getElement(i), correct->getElement(i));
  }

protected:
  friend class ContainerElementsEvaluatorClass;

  EvaluatorPtr elementEvaluator;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_CONTAINER_ELEMENTS_H_

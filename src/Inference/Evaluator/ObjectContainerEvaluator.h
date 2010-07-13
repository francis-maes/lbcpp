/*-----------------------------------------.---------------------------------.
| Filename: ObjectContainerEvaluator.h     | Object Container Evaluator      |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 16:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EVALUATOR_OBJECT_CONTAINER_H_
# define LBCPP_EVALUATOR_OBJECT_CONTAINER_H_

# include <lbcpp/Inference/Evaluator.h>
# include <lbcpp/Data/Container.h> // new
# include <lbcpp/Object/ObjectContainer.h> // old

namespace lbcpp
{

 // new 
class ContainerElementsEvaluator : public Evaluator
{
public:
  ContainerElementsEvaluator(const String& name, EvaluatorPtr elementEvaluator)
    : Evaluator(name), elementEvaluator(elementEvaluator) {}

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
    size_t n = predicted->size();
    jassert(correct->size() == n);
    for (size_t i = 0; i < n; ++i)
      elementEvaluator->addPrediction(predicted->getVariable(i), correct->getVariable(i));
  }

protected:
  EvaluatorPtr elementEvaluator;
};

// old
class ObjectContainerEvaluator : public Evaluator
{
public:
  ObjectContainerEvaluator(const String& name, EvaluatorPtr objectEvaluator)
    : Evaluator(name), objectEvaluator(objectEvaluator) {}

  virtual String toString() const
    {return objectEvaluator->toString();}

  virtual double getDefaultScore() const
    {return objectEvaluator->getDefaultScore();}

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
    {objectEvaluator->getScores(res);}

  virtual void addPrediction(const Variable& predictedObject, const Variable& correctObject)
  {
    ObjectContainerPtr predicted = predictedObject.dynamicCast<ObjectContainer>();
    ObjectContainerPtr correct = correctObject.dynamicCast<ObjectContainer>();
    if (!predicted || !correct)
      return;
    size_t n = predicted->size();
    jassert(correct->size() == n);
    for (size_t i = 0; i < n; ++i)
      objectEvaluator->addPrediction(predicted->get(i), correct->get(i));
  }

protected:
  EvaluatorPtr objectEvaluator;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EVALUATOR_OBJECT_CONTAINER_H_

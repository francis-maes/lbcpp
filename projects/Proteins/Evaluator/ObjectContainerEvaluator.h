/*-----------------------------------------.---------------------------------.
| Filename: ObjectContainerEvaluator.h     | Object Container Evaluator      |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 16:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EVALUATOR_OBJECT_CONTAINER_H_
# define LBCPP_EVALUATOR_OBJECT_CONTAINER_H_

# include "Evaluator.h"

namespace lbcpp
{

class ObjectContainerEvaluator : public Evaluator
{
public:
  ObjectContainerEvaluator(const String& name, EvaluatorPtr objectEvaluator)
    : Evaluator(name), objectEvaluator(objectEvaluator) {}

  virtual String toString() const
    {return objectEvaluator->toString();}

  virtual double getDefaultScore() const
    {return objectEvaluator->getDefaultScore();}

  virtual void addPrediction(ObjectPtr predictedObject, ObjectPtr correctObject)
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

/*-----------------------------------------.---------------------------------.
| Filename: Evaluator.h                    | Base class for evaluators       |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 15:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EVALUATOR_H_
# define LBCPP_EVALUATOR_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class Evaluator : public NameableObject
{
public:
  Evaluator(const String& name) : NameableObject(name) {}
  Evaluator() {}

  virtual void addPrediction(ObjectPtr predicted, ObjectPtr correct) = 0;
  virtual double getDefaultScore() const = 0;
};

typedef ReferenceCountedObjectPtr<Evaluator> EvaluatorPtr;

extern EvaluatorPtr classificationAccuracyEvaluator(const String& name);
extern EvaluatorPtr binaryClassificationConfusionEvaluator(const String& name);
extern EvaluatorPtr regressionErrorEvaluator(const String& name);

extern EvaluatorPtr objectContainerEvaluator(const String& name, EvaluatorPtr objectEvaluator);
extern EvaluatorPtr scoreVectorSequenceRegressionErrorEvaluator(const String& name);

inline EvaluatorPtr sequenceLabelingAccuracyEvaluator(const String& name)
  {return objectContainerEvaluator(name, classificationAccuracyEvaluator(name));}

inline EvaluatorPtr binarySequenceLabelingConfusionEvaluator(const String& name)
  {return objectContainerEvaluator(name, binaryClassificationConfusionEvaluator(name));}


}; /* namespace lbcpp */

#endif // !LBCPP_EVALUATOR_H_

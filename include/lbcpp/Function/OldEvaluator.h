/*-----------------------------------------.---------------------------------.
| Filename: Evaluator.h                    | Base class for evaluators       |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 15:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OLD_EVALUATOR_H_
# define LBCPP_OLD_EVALUATOR_H_

# include <lbcpp/Core/Variable.h>
# include <lbcpp/Data/RandomVariable.h>
# include "predeclarations.h"
# include "Evaluator.h"

namespace lbcpp
{

class OldEvaluator : public NameableObject
{
public:
  OldEvaluator(const String& name) : NameableObject(name) {}
  OldEvaluator() {}

  virtual void addPrediction(ExecutionContext& context, const Variable& predicted, const Variable& correct) = 0;

  virtual void getScores(std::vector< std::pair<String, double> >& res) const = 0;
  virtual double getDefaultScore() const = 0;

  lbcpp_UseDebuggingNewOperator
};
#if 0
class OldRegressionErrorEvaluator : public OldEvaluator
{
public:
  OldRegressionErrorEvaluator(const String& name);
  OldRegressionErrorEvaluator() {}

  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject);
  virtual void addDelta(double delta);

  virtual String toString() const;

  virtual void getScores(std::vector< std::pair<String, double> >& res) const;

  virtual double getDefaultScore() const
    {return -getRMSE();}

  double getRMSE() const;

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  lbcpp_UseDebuggingNewOperator

protected:
  ScalarVariableMeanPtr absoluteError;
  ScalarVariableMeanPtr squaredError;
};

typedef ReferenceCountedObjectPtr<OldRegressionErrorEvaluator> OldRegressionErrorEvaluatorPtr;

// Classification
extern OldEvaluatorPtr oldClassificationAccuracyEvaluator(const String& name = T("accuracy"));
extern OldEvaluatorPtr oldBinaryClassificationConfusionEvaluator(const String& name);
extern OldEvaluatorPtr oldROCAnalysisEvaluator(const String& name);

// Multi-label Classification
extern OldEvaluatorPtr oldMultiLabelClassificationEvaluator(const String& name = T("multi-label"));

// Regression
extern OldEvaluatorPtr oldRegressionErrorEvaluator(const String& name);
extern OldRegressionErrorEvaluatorPtr oldDihedralRegressionErrorEvaluator(const String& name);

// Structured
extern OldEvaluatorPtr containerElementsEvaluator(const String& name, OldEvaluatorPtr elementEvaluator);
inline OldEvaluatorPtr sequenceLabelingAccuracyEvaluator(const String& name)
  {return containerElementsEvaluator(name, oldClassificationAccuracyEvaluator(name));}
inline OldEvaluatorPtr binarySequenceLabelingConfusionEvaluator(const String& name)
  {return containerElementsEvaluator(name, oldBinaryClassificationConfusionEvaluator(name));}
#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_OLD_EVALUATOR_H_

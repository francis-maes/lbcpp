/*-----------------------------------------.---------------------------------.
| Filename: Evaluator.h                    | Base class for evaluators       |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 15:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EVALUATOR_H_
# define LBCPP_EVALUATOR_H_

# include <lbcpp/Object/Object.h>
# include <lbcpp/Utilities/RandomVariable.h>

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

class RegressionErrorEvaluator : public Evaluator
{
public:
  RegressionErrorEvaluator(const String& name);
  RegressionErrorEvaluator() {}

  virtual void addPrediction(ObjectPtr predictedObject, ObjectPtr correctObject);
  virtual void addDelta(double delta);

  virtual String toString() const;

  virtual double getDefaultScore() const
    {return -getRMSE();}

  double getRMSE() const;

protected:
  ScalarVariableMeanPtr absoluteError;
  ScalarVariableMeanPtr squaredError;
};

typedef ReferenceCountedObjectPtr<RegressionErrorEvaluator> RegressionErrorEvaluatorPtr;

// Classification
extern EvaluatorPtr classificationAccuracyEvaluator(const String& name);
extern EvaluatorPtr binaryClassificationConfusionEvaluator(const String& name);
extern EvaluatorPtr rocAnalysisEvaluator(const String& name);

// Regression
extern EvaluatorPtr regressionErrorEvaluator(const String& name);
extern EvaluatorPtr dihedralRegressionErrorEvaluator(const String& name);

// Structured
extern EvaluatorPtr objectContainerEvaluator(const String& name, EvaluatorPtr objectEvaluator);

inline EvaluatorPtr sequenceLabelingAccuracyEvaluator(const String& name)
  {return objectContainerEvaluator(name, classificationAccuracyEvaluator(name));}

inline EvaluatorPtr binarySequenceLabelingConfusionEvaluator(const String& name)
  {return objectContainerEvaluator(name, binaryClassificationConfusionEvaluator(name));}


class BinaryClassificationConfusionMatrix : public Object
{
public:
  BinaryClassificationConfusionMatrix();

  virtual String toString() const;

  void clear();
  void addPrediction(bool predicted, bool correct);

  double computeMatthewsCorrelation() const;
  void computePrecisionRecallAndF1(double& precision, double& recall, double& f1score) const;

  size_t getSampleCount() const
    {return totalCount;}

  size_t getTruePositives() const
    {return truePositive;}

  size_t getFalsePositives() const
    {return falsePositive;}

  size_t getFalseNegatives() const
    {return falseNegative;}

  size_t getTrueNegatives() const
    {return trueNegative;}

private:
 // correct: positive   negative
  size_t truePositive, falsePositive; // predicted as positive
  size_t falseNegative, trueNegative; // predicted as negative

  size_t totalCount;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EVALUATOR_H_

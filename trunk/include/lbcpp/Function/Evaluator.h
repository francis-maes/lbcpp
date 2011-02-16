/*-----------------------------------------.---------------------------------.
| Filename: Evaluator.h                    | Base class for evaluators       |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 15:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EVALUATOR_H_
# define LBCPP_EVALUATOR_H_

# include <lbcpp/Core/Variable.h>
# include <lbcpp/Data/RandomVariable.h>

namespace lbcpp
{

class Evaluator : public NameableObject
{
public:
  Evaluator(const String& name) : NameableObject(name) {}
  Evaluator() {}

  virtual void addPrediction(ExecutionContext& context, const Variable& predicted, const Variable& correct) = 0;

  virtual void getScores(std::vector< std::pair<String, double> >& res) const = 0;
  virtual double getDefaultScore() const = 0;

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<Evaluator> EvaluatorPtr;

class RegressionErrorEvaluator : public Evaluator
{
public:
  RegressionErrorEvaluator(const String& name);
  RegressionErrorEvaluator() {}

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

typedef ReferenceCountedObjectPtr<RegressionErrorEvaluator> RegressionErrorEvaluatorPtr;

// Classification
extern EvaluatorPtr classificationAccuracyEvaluator(const String& name = T("accuracy"));
extern EvaluatorPtr binaryClassificationConfusionEvaluator(const String& name);
extern EvaluatorPtr rocAnalysisEvaluator(const String& name);

// Multi-label Classification
extern EvaluatorPtr multiLabelClassificationEvaluator(const String& name = T("multi-label"));

// Regression
extern EvaluatorPtr regressionErrorEvaluator(const String& name);
extern RegressionErrorEvaluatorPtr dihedralRegressionErrorEvaluator(const String& name);

// Structured
extern EvaluatorPtr containerElementsEvaluator(const String& name, EvaluatorPtr elementEvaluator);
inline EvaluatorPtr sequenceLabelingAccuracyEvaluator(const String& name)
  {return containerElementsEvaluator(name, classificationAccuracyEvaluator(name));}
inline EvaluatorPtr binarySequenceLabelingConfusionEvaluator(const String& name)
  {return containerElementsEvaluator(name, binaryClassificationConfusionEvaluator(name));}

// Save To Directory
extern EvaluatorPtr saveToDirectoryEvaluator(const File& directory, const String& extension = T(".xml"));

class BinaryClassificationConfusionMatrix : public Object
{
public:
  BinaryClassificationConfusionMatrix(const BinaryClassificationConfusionMatrix& otherMatrix);
  BinaryClassificationConfusionMatrix();

  virtual String toString() const;

  static bool convertToBoolean(ExecutionContext& context, const Variable& variable, bool& res);

  void clear();
  void set(size_t truePositive, size_t falsePositive, size_t falseNegative, size_t trueNegative);
  void addPrediction(bool predicted, bool correct, size_t count = 1);
  void removePrediction(bool predicted, bool correct, size_t count = 1);

  void addPredictionIfExists(ExecutionContext& context, const Variable& predicted, const Variable& correct, size_t count = 1);

  double computeAccuracy() const;
  double computeF1Score() const;
  double computePrecision() const;
  double computeRecall() const;
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

  size_t getCount(bool predicted, bool correct) const;

  size_t getPositives() const
    {return truePositive + falseNegative;}

  size_t getNegatives() const
    {return trueNegative + falsePositive;}

  bool operator ==(const BinaryClassificationConfusionMatrix& other) const;
  bool operator !=(const BinaryClassificationConfusionMatrix& other) const
    {return !(*this == other);}

private:
 // correct: positive   negative
  size_t truePositive, falsePositive; // predicted as positive
  size_t falseNegative, trueNegative; // predicted as negative

  size_t totalCount;
};

class ROCAnalyse : public Object
{
public:
  ROCAnalyse() : numPositives(0), numNegatives(0) {}

  typedef double (BinaryClassificationConfusionMatrix::*ScoreFunction)() const;

  void addPrediction(ExecutionContext& context, double predictedScore, bool isPositive); 
  void getScores(std::vector< std::pair<String, double> >& res) const;
  double findBestThreshold(ScoreFunction measure, double& bestScore, double margin = 1.0) const;

  size_t getSampleCount() const
    {ScopedLock _(lock); return predictedScores.size();}

  size_t getNumPositives() const
    {ScopedLock _(lock); return numPositives;}

  size_t getNumNegatives() const
    {ScopedLock _(lock); return numNegatives;}

  void clear()
    {ScopedLock _(lock); predictedScores.clear(); numPositives = numNegatives = 0;}

private:
  typedef std::map<double, std::pair<size_t, size_t> > ScoresMap;

  CriticalSection lock;
  ScoresMap predictedScores;
  size_t numPositives, numNegatives;

  double getBestThreshold(ScoresMap::const_iterator lastLower, double margin = 1.0) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EVALUATOR_H_

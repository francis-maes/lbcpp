/*-----------------------------------------.---------------------------------.
| Filename: Evaluator.h                    | Base class for evaluators       |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 15:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EVALUATOR_H_
# define LBCPP_EVALUATOR_H_

# include <lbcpp/Data/Variable.h>
# include <lbcpp/Data/RandomVariable.h>

namespace lbcpp
{

class Evaluator : public NameableObject
{
public:
  Evaluator(const String& name) : NameableObject(name) {}
  Evaluator() {}

  virtual void addPrediction(const Variable& predicted, const Variable& correct) = 0;

  virtual void getScores(std::vector< std::pair<String, double> >& res) const = 0;
  virtual double getDefaultScore() const = 0;
};

typedef ReferenceCountedObjectPtr<Evaluator> EvaluatorPtr;

class RegressionErrorEvaluator : public Evaluator
{
public:
  RegressionErrorEvaluator(const String& name);
  RegressionErrorEvaluator() {}

  virtual void addPrediction(const Variable& predictedObject, const Variable& correctObject);
  virtual void addDelta(double delta);

  virtual String toString() const;

  virtual void getScores(std::vector< std::pair<String, double> >& res) const;

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
  BinaryClassificationConfusionMatrix();

  virtual String toString() const;

  void clear();
  void addPrediction(bool predicted, bool correct);

  double computeMatthewsCorrelation() const;
  double computeAccuracy() const;
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

class ROCAnalyse : public Object
{
public:
  ROCAnalyse() : numPositives(0), numNegatives(0) {}

  void addPrediction(double predictedScore, bool isPositive);
  double findThresholdMaximisingF1(double& bestF1Score, double& precision, double& recall) const;
  double findThresholdMaximisingRecallGivenPrecision(double minimumPrecision, double& recall) const;
  double findThresholdMaximisingMCC(double& bestMCC) const;

  void getScores(std::vector< std::pair<String, double> >& res) const;

  size_t getSampleCount() const
    {return predictedScores.size();}

  size_t getNumPositives() const
    {return numPositives;}

  size_t getNumNegatives() const
    {return numNegatives;}

  void clear()
    {predictedScores.clear(); numPositives = numNegatives = 0;}

private:
  std::map<double, std::pair<size_t, size_t> > predictedScores;
  size_t numPositives, numNegatives;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EVALUATOR_H_

/*-----------------------------------------.---------------------------------.
| Filename: Evaluator.h                    | Base class for evaluators       |
| Author  : Julien Becker                  |                                 |
| Started : 22/02/2011 10:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_EVALUATOR_H_
# define LBCPP_FUNCTION_EVALUATOR_H_

# include "predeclarations.h"
# include <lbcpp/Core/Function.h>

namespace lbcpp
{

class ScoreObject : public Object
{
public:
  virtual double getScoreToMinimize() const = 0;
  virtual void getScores(std::vector< std::pair<String, double> >& res) const = 0;
};

typedef ReferenceCountedObjectPtr<ScoreObject> ScoreObjectPtr;

extern ClassPtr scoreObjectClass;

class Evaluator : public Function
{
public:
  Evaluator()
    {numInputs = 2;}

  virtual TypePtr getRequiredPredictedElementsType() const = 0;
  
  virtual TypePtr getRequiredSupervisionElementsType() const = 0;
  
  /* Function */
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return containerClass(index == 0 ? getRequiredPredictedElementsType() : getRequiredSupervisionElementsType());}
  
  virtual String getOutputPostFix() const
    {return T("Evaluated");}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return scoreObjectClass;}

protected:
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

  virtual ScoreObjectPtr createEmptyScoreObject() const = 0;

  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject, ScoreObjectPtr& result) const = 0;
};

// Classification
extern EvaluatorPtr classificationAccuracyEvaluator();

// Multi-label Classification
extern EvaluatorPtr multiLabelClassificationEvaluator();

// Regression
extern EvaluatorPtr regressionErrorEvaluator();
extern EvaluatorPtr dihedralRegressionErrorEvaluator();

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

typedef ReferenceCountedObjectPtr<BinaryClassificationConfusionMatrix> BinaryClassificationConfusionMatrixPtr;

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

};

#endif // !LBCPP_FUNCTION_EVALUATOR_H_

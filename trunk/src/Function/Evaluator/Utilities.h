/*-----------------------------------------.---------------------------------.
| Filename: Utilities.h                    | Utilities for evaluators        |
| Author  : Julien Becker                  |                                 |
| Started : 23/02/2011 11:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_EVALUATOR_UTILITIES_H_
# define LBCPP_FUNCTION_EVALUATOR_UTILITIES_H_

# include <lbcpp/Core/Object.h>
# include <lbcpp/Function/Evaluator.h>
# include <algorithm>

namespace lbcpp
{

class BinaryClassificationConfusionMatrix : public ScoreObject
{
public:
  BinaryClassificationConfusionMatrix(const BinaryClassificationConfusionMatrix& otherMatrix);
  BinaryClassificationConfusionMatrix(BinaryClassificationScore scoreToOptimize = binaryClassificationAccuracyScore);

  // ScoreObject
  virtual double getScoreToMinimize() const
  {
    switch (scoreToOptimize) {
      case binaryClassificationAccuracyScore:
        return 1.0 - computeAccuracy();
      case binaryClassificationF1Score:
        return 1.0 - computeF1Score();
      case binaryClassificationMCCScore:
        return 1.0 - computeMatthewsCorrelation();
      case binaryClassificationSensitivityAndSpecificityScore:
        return 1.0 - computeSensitivityAndSpecificity();
      default:
        jassertfalse;
    }
    return DBL_MAX;
  }

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
  double computeSensitivity() const;
  double computeSpecificity() const;
  double computeSensitivityAndSpecificity() const;
  
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
  
  /* Object */
  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);

  bool operator ==(const BinaryClassificationConfusionMatrix& other) const;
  bool operator !=(const BinaryClassificationConfusionMatrix& other) const
    {return !(*this == other);}

protected:
  friend class BinaryClassificationConfusionMatrixClass;

  BinaryClassificationScore scoreToOptimize;

 // correct: positive   negative
  size_t truePositive, falsePositive; // predicted as positive
  size_t falseNegative, trueNegative; // predicted as negative

  size_t totalCount;
};

typedef ReferenceCountedObjectPtr<BinaryClassificationConfusionMatrix> BinaryClassificationConfusionMatrixPtr;

class ROCScoreObjectElement : public Object
{
public:
  ROCScoreObjectElement(const BinaryClassificationConfusionMatrixPtr& confusionMatrix)
  {
    accuracy = confusionMatrix->computeAccuracy();
    f1Score = confusionMatrix->computeF1Score();
    precision = confusionMatrix->computePrecision();
    recallOrSensitivity = confusionMatrix->computeRecall();
    specificity = confusionMatrix->computeSpecificity();
    falsePositiveRate = 1 - specificity;
    matthewsCorrelation = confusionMatrix->computeMatthewsCorrelation();
  }

  ROCScoreObjectElement() {}

protected:
  friend class ROCScoreObjectElementClass;

  double accuracy;
  double f1Score;
  double precision;
  double recallOrSensitivity;
  double specificity;
  double falsePositiveRate;
  double matthewsCorrelation;
};

extern ClassPtr rocScoreObjectElementClass;

class ROCScoreObject : public ScoreObject
{
public:
  ROCScoreObject(BinaryClassificationScore scoreToOptimize = binaryClassificationAccuracyScore)
    : scoreToOptimize(scoreToOptimize), bestThreshold(0.0), bestThresholdScore(0.0), numPositives(0), numNegatives(0) {}

  virtual double getScoreToMinimize() const
    {return 1.0 - bestThresholdScore;}
  
  typedef double (BinaryClassificationConfusionMatrix::*ScoreFunction)() const;

  void addPrediction(ExecutionContext& context, double predictedScore, bool isPositive); 
  void finalize(bool saveConfusionMatrices);
  double findBestThreshold(BinaryClassificationScore scoreToOptimize, double& bestScore) const;
  
  BinaryClassificationConfusionMatrixPtr findBestSensitivitySpecificityTradeOff() const;

  size_t getSampleCount() const
    {ScopedLock _(lock); return numPositives + numNegatives;}

  size_t getNumPositives() const
    {ScopedLock _(lock); return numPositives;}

  size_t getNumNegatives() const
    {ScopedLock _(lock); return numNegatives;}

  void clear()
    {ScopedLock _(lock); predictedScores.clear(); numPositives = numNegatives = 0;}

  virtual String toString() const
  {
    if (!getSampleCount())
      return String::empty;
    return T("tuned score: ") + String(bestThresholdScore * 100, 2) + T("% threshold = ") + String(bestThreshold);
  }
  
  ContainerPtr createROCCurveElements() const
  {
    const size_t n = confusionMatrices.size();
    ContainerPtr res = vector(rocScoreObjectElementClass, n);
    for (size_t i = 0; i < n; ++i)
      res->setElement(i, new ROCScoreObjectElement(confusionMatrices[i]));
    return res;
  }

protected:
  friend class ROCScoreObjectClass;
  
  BinaryClassificationScore scoreToOptimize;
  double bestThreshold;
  double bestThresholdScore;

  BinaryClassificationConfusionMatrixPtr bestConfusionMatrix;
  std::vector< BinaryClassificationConfusionMatrixPtr > confusionMatrices;

private:
  typedef std::map<double, std::pair<size_t, size_t> > ScoresMap;

  CriticalSection lock;
  ScoresMap predictedScores;
  size_t numPositives, numNegatives;

  double getBestThreshold(ScoresMap::const_iterator lastLower, double margin = 1.0) const;
  double findBestThreshold(ScoreFunction measure, double& bestScore, double margin = 1.0) const;
};

typedef ReferenceCountedObjectPtr<ROCScoreObject> ROCScoreObjectPtr;

};

#endif // !LBCPP_FUNCTION_EVALUATOR_UTILITIES_H_

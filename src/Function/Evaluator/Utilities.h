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
      default:
        jassertfalse;
    }
    return DBL_MAX;
  }
  
  void finalize();
  
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

protected:
  friend class BinaryClassificationConfusionMatrixClass;

  BinaryClassificationScore scoreToOptimize;

  double precision;
  double recall;
  double f1score;
  double matthewsCorrelation;
  double accuracy;

 // correct: positive   negative
  size_t truePositive, falsePositive; // predicted as positive
  size_t falseNegative, trueNegative; // predicted as negative

  size_t totalCount;
};

typedef ReferenceCountedObjectPtr<BinaryClassificationConfusionMatrix> BinaryClassificationConfusionMatrixPtr;

class ROCScoreObjectElement : public Object
{
public:
  ROCScoreObjectElement(double falsePositiveRate, double truePositiveRate)
    : falsePositiveRate(falsePositiveRate), truePositiveRate(truePositiveRate) {}
  ROCScoreObjectElement() {}

protected:
  friend class ROCScoreObjectElementClass;

  double falsePositiveRate;
  double truePositiveRate;
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
  void finalize();
  double findBestThreshold(BinaryClassificationScore scoreToOptimize, double& bestScore) const;

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
    std::vector<double> sortedPredictedScores;
    sortedPredictedScores.reserve(predictedScores.size());
    for (ScoresMap::const_iterator it = predictedScores.begin(); it != predictedScores.end(); ++it)
      sortedPredictedScores.push_back(it->first);
    sort(sortedPredictedScores.begin(), sortedPredictedScores.end());

    ContainerPtr res = vector(rocScoreObjectElementClass, sortedPredictedScores.size() + 2);
    res->setElement(0, new ROCScoreObjectElement(0, 0));
    size_t numFalseNegative = 0;
    size_t numTruePositive = 0;
    for (size_t i = 0; i < sortedPredictedScores.size(); ++i)
    {
      std::pair<size_t, size_t> n = predictedScores.find(sortedPredictedScores[i])->second;
      numFalseNegative += n.first;
      numTruePositive += n.second;

      res->setElement(i + 1, new ROCScoreObjectElement((double)numFalseNegative / (double)numNegatives,
                                                       (double)numTruePositive / (double)numPositives));
    }
    res->setElement(sortedPredictedScores.size() + 1, new ROCScoreObjectElement(1, 1));

    return res;
  }

protected:
  friend class ROCScoreObjectClass;
  
  BinaryClassificationScore scoreToOptimize;
  double bestThreshold;
  double bestThresholdScore;
  std::vector< std::pair<double, double> > precision;
  std::vector< std::pair<double, double> > recall;

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

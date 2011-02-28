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
  BinaryClassificationConfusionMatrix();

  // ScoreObject
  virtual double getScoreToMinimize() const
    {return -computeF1Score();}
  
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

class ROCAnalyse : public ScoreObject
{
public:
  ROCAnalyse() : bestF1(0.0), numPositives(0), numNegatives(0) {}

  virtual double getScoreToMinimize() const
    {return -bestF1;}
  
  typedef double (BinaryClassificationConfusionMatrix::*ScoreFunction)() const;

  void addPrediction(ExecutionContext& context, double predictedScore, bool isPositive); 
  void finalize();
  double findBestThreshold(ScoreFunction measure, double& bestScore, double margin = 1.0) const;
  
  double findBestThreshold(BinaryClassificationScore scoreToOptimize, double& bestScore) const;

  size_t getSampleCount() const
    {ScopedLock _(lock); return predictedScores.size();}

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
    
    double bestF1;
    double bestThreshold = findBestThreshold(&BinaryClassificationConfusionMatrix::computeF1Score, bestF1);
    return T("tuned F1: ") + String(bestF1 * 100, 2) + T("% threshold = ") + String(bestThreshold);
  }

protected:
  friend class ROCAnalyseClass;
  
  double bestF1;
  std::vector< std::pair<double, double> > precision;
  std::vector< std::pair<double, double> > recall;

private:
  typedef std::map<double, std::pair<size_t, size_t> > ScoresMap;

  CriticalSection lock;
  ScoresMap predictedScores;
  size_t numPositives, numNegatives;

  double getBestThreshold(ScoresMap::const_iterator lastLower, double margin = 1.0) const;
};

};

#endif // !LBCPP_FUNCTION_EVALUATOR_UTILITIES_H_

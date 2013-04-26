/*-----------------------------------------.---------------------------------.
| Filename: Utilities.h                    | Utilities for evaluators        |
| Author  : Julien Becker                  |                                 |
| Started : 23/02/2013 11:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_EVALUATOR_UTILITIES_H_
# define LBCPP_FUNCTION_EVALUATOR_UTILITIES_H_

# include <lbcpp/Core/Object.h>
# include <lbcpp/Function/Evaluator.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Lua/Lua.h>

namespace lbcpp
{

class BinaryClassificationConfusionMatrix : public ScoreObject
{
public:
  BinaryClassificationConfusionMatrix(BinaryClassificationScore scoreToMinimize = binaryClassificationAccuracyScore,
                                      double threshold = 0.5f);

  /* ScoreObject */
  virtual double getScoreToMinimize() const;
  virtual String toString() const;

  void addPredictionIfExists(const Variable& predicted, const Variable& correct);
  void addPrediction(bool predicted, bool correct);

  double computeAccuracy() const;
  double computeF1Score() const;
  double computePrecision() const;
  double computeRecall() const;
  double computeSpecificity() const;
  double computeMatthewsCorrelation() const;
  double computeSensitivityAndSpecificity() const;
  double computeSensitivity() const
    {return computeRecall();}

  double getThreshold() const
    {return threshold;}

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

  size_t getPositives() const
    {return truePositive + falseNegative;}

  size_t getNegatives() const
    {return trueNegative + falsePositive;}

  /* Object */
  void saveToXml(XmlExporter& exporter) const;
  bool loadFromXml(XmlImporter& importer);
  
  /*
   ** Lua
   */
  static int accuracy(LuaState& state);
  static int f1(LuaState& state);
  static int precision(LuaState& state);
  static int recall(LuaState& state);
  static int specificity(LuaState& state);
  static int mcc(LuaState& state);

protected:
  friend class BinaryClassificationConfusionMatrixClass;

  BinaryClassificationScore scoreToMinimize;
  double threshold;

  // correct: positive   negative
  size_t truePositive, falsePositive; // predicted as positive
  size_t falseNegative, trueNegative; // predicted as negative

  size_t totalCount;
};
  

typedef ReferenceCountedObjectPtr<BinaryClassificationConfusionMatrix> BinaryClassificationConfusionMatrixPtr;
extern ClassPtr binaryClassificationConfusionMatrixClass;

/* User interface element */
class BinaryClassificationCurveElement : public Object
{
public:
  BinaryClassificationCurveElement(const BinaryClassificationConfusionMatrixPtr& confusionMatrix)
  {
    threshold = confusionMatrix->getThreshold();
    accuracy = confusionMatrix->computeAccuracy();
    f1Score = confusionMatrix->computeF1Score();
    precision = confusionMatrix->computePrecision();
    recallOrSensitivity = confusionMatrix->computeRecall();
    specificity = confusionMatrix->computeSpecificity();
    falsePositiveRate = 1 - specificity;
    matthewsCorrelation = confusionMatrix->computeMatthewsCorrelation();
  }

protected:
  friend class BinaryClassificationCurveElementClass;

  BinaryClassificationCurveElement() {}

  double threshold;
  double accuracy;
  double f1Score;
  double precision;
  double recallOrSensitivity;
  double specificity;
  double falsePositiveRate;
  double matthewsCorrelation;
};

extern ClassPtr binaryClassificationCurveElementClass;
typedef ReferenceCountedObjectPtr<BinaryClassificationCurveElement> BinaryClassificationCurveElementPtr;

class BinaryClassificationCurveScoreObject : public ScoreObject
{
public:
  BinaryClassificationCurveScoreObject(BinaryClassificationScore scoreToMinimize = binaryClassificationAccuracyScore);

  virtual double getScoreToMinimize() const;
  void addPrediction(const Variable& predicted, const Variable& correct);
  void finalize(bool saveConfusionMatrices);

  size_t getSampleCount() const
    {return predictions.size();}

  BinaryClassificationConfusionMatrixPtr getBestConfusionMatrix() const
    {return bestConfusionMatrix;}

  ContainerPtr createBinaryClassificationCurveElements() const;
  void getAllThresholds(std::vector<double>& result) const;

  double getAreaUnderCurve() const
    {return areaUnderCurve;}

protected:
  friend class BinaryClassificationCurveScoreObjectClass;

  BinaryClassificationScore scoreToMinimize;
  std::vector<BinaryClassificationConfusionMatrixPtr> confusionMatrices;
  BinaryClassificationConfusionMatrixPtr bestConfusionMatrix;
  double areaUnderCurve;
  double accuracyAt5Fpr;

  BinaryClassificationConfusionMatrixPtr createBinaryConfusionMatrix(double threshold) const;
  void computeAreaUnderCurve();
  void computeAccuracyAt5Fpr();

private:
  std::vector< std::pair<Variable, Variable> > predictions;
  std::map<double, bool> thresholds;
};

typedef ReferenceCountedObjectPtr<BinaryClassificationCurveScoreObject> BinaryClassificationCurveScoreObjectPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_UTILITIES_H_

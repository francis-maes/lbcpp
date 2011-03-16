/*-----------------------------------------.---------------------------------.
| Filename: ClassificationAccuracyEvalua..h| Classification Accuracy         |
| Author  : Julien Becker                  |   Evaluator                     |
| Started : 22/02/2011 11:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_EVALUATOR_CLASSIFICATION_ACCURACY_H_
# define LBCPP_FUNCTION_EVALUATOR_CLASSIFICATION_ACCURACY_H_

# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/Data/RandomVariable.h>

namespace lbcpp
{

class ClassificationScoreObject : public ScoreObject
{
public:
  ClassificationScoreObject()
    : accuracy(0.0), accuracyVector(new ScalarVariableMean()) {}
  
  virtual double getScoreToMinimize() const 
    {return 1.0 - accuracy;}

  void push(bool isCorrect)
    {accuracyVector->push(isCorrect);}
  
  virtual String toString() const
  {
    double count = accuracyVector->getCount();
    if (!count)
      return String::empty;
    return getName() + T(": ") + String(accuracy * 100.0, 2) + T("% (") + String((int)count) + T(" examples)");
  }
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    ScoreObject::clone(context, target);
    target.staticCast<ClassificationScoreObject>()->accuracyVector = accuracyVector->cloneAndCast<ScalarVariableMean>(context);
  }
  
  void finalize()
    {accuracy = accuracyVector->getMean();}
  
protected:
  friend class ClassificationScoreObjectClass;
  
  double accuracy;
  ScalarVariableMeanPtr accuracyVector;
};

typedef ReferenceCountedObjectPtr<ClassificationScoreObject> ClassificationScoreObjectPtr;

class ClassificationEvaluator : public SupervisedEvaluator
{
public:
  virtual TypePtr getRequiredPredictionType() const
    {return doubleVectorClass(enumValueType, probabilityType);}
  
  virtual TypePtr getRequiredSupervisionType() const
    {return enumValueType;}
  
protected:
  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return new ClassificationScoreObject();}
  
  virtual void finalizeScoreObject(const ScoreObjectPtr& score) const
    {score.staticCast<ClassificationScoreObject>()->finalize();}

  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject, const ScoreObjectPtr& result) const
  {
    ClassificationScoreObjectPtr score = result.staticCast<ClassificationScoreObject>();

    int correctLabel = getLabel(correctObject);
    if (correctLabel >= 0)
      score->push(correctLabel == getLabel(predictedObject));
  }

protected:
  friend class ClassificationAccuracyEvaluatorClass;

  int getLabel(const Variable& value) const
  {
    if (value.isEnumeration())
      return value.getInteger();
    DoubleVectorPtr scores = value.dynamicCast<DoubleVector>();
    if (scores)
      return scores->getIndexOfMaximumValue();

    jassert(false);
    return -1;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_EVALUATOR_CLASSIFICATION_ACCURACY_H_

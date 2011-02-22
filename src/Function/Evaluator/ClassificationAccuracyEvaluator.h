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

class ClassificationAccuracyScoreObject : public ScoreObject
{
public:
  ClassificationAccuracyScoreObject()
    : accuracy(new ScalarVariableMean()) {}
  
  virtual double getScoreToMinimize() const 
    {return -accuracy->getMean();}

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
    {res.push_back(std::make_pair(T("Accuracy"), accuracy->getMean()));}
  
  void push(bool isCorrect)
    {accuracy->push(isCorrect);}
  
  virtual String toString() const
  {
    double count = accuracy->getCount();
    if (!count)
      return String::empty;
    return getName() + T(": ") + String(accuracy->getMean() * 100.0, 2) + T("% (") + String((int)count) + T(" examples)");
  }
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    ScoreObject::clone(context, target);
    target.staticCast<ClassificationAccuracyScoreObject>()->accuracy = accuracy->cloneAndCast<ScalarVariableMean>(context);
  }
  
protected:
  friend class ClassificationAccuracyScoreObjectClass;
  
  ScalarVariableMeanPtr accuracy;
};

typedef ReferenceCountedObjectPtr<ClassificationAccuracyScoreObject> ClassificationAccuracyScoreObjectPtr;

class ClassificationAccuracyEvaluator : public Evaluator
{
public:
  virtual TypePtr getRequiredPredictedElementsType() const
    {return doubleVectorClass(enumValueType, probabilityType);}
  
  virtual TypePtr getRequiredSupervisionElementsType() const
    {return enumValueType;}
  
protected:
  virtual ScoreObjectPtr createEmptyScoreObject() const
    {return new ClassificationAccuracyScoreObject();}

  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject, ScoreObjectPtr& result) const
  {
    ClassificationAccuracyScoreObjectPtr score = result.staticCast<ClassificationAccuracyScoreObject>();

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

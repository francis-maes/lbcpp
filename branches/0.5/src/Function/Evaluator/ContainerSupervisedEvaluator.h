/*-----------------------------------------.---------------------------------.
| Filename: ContainerElementsEvaluator.h   | Container Evaluator             |
| Author  : Francis Maes                   |                                 |
| Started : 01/03/2011 16:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SUPERVISED_EVALUATOR_CONTAINER_H_
# define LBCPP_FUNCTION_SUPERVISED_EVALUATOR_CONTAINER_H_

# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Core/Container.h>

namespace lbcpp
{

/*
 * Evaluate by-element within containers (not by-containers)
 */
class ElementContainerSupervisedEvaluator : public SupervisedEvaluator
{
public:
  ElementContainerSupervisedEvaluator(SupervisedEvaluatorPtr elementEvaluator)
    : elementEvaluator(elementEvaluator) {}
  ElementContainerSupervisedEvaluator() {}

  virtual TypePtr getRequiredPredictionType() const
    {return elementEvaluator->getRequiredPredictionType();}

  virtual TypePtr getRequiredSupervisionType() const
    {return elementEvaluator->getRequiredSupervisionType();}

  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return elementEvaluator->createEmptyScoreObject(context, function);}
  
  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const
  {
    ContainerPtr supervisionContainer = example->getVariable(1).dynamicCast<Container>();
    ContainerPtr predictedContainer = output.dynamicCast<Container>();
    size_t n = predictedContainer->getNumElements();
    jassert(supervisionContainer->getNumElements() == n);
    for (size_t i = 0; i < n; ++i)
    {
      Variable supervision = supervisionContainer->getElement(i);
      Variable predicted = predictedContainer->getElement(i);
      if (!supervision.exists())
        continue;
      if (!predicted.exists())
      {
        context.errorCallback(T("ElementContainerSupervisedEvaluator::updateScoreObject"), T("Missing prediction"));
        return false;
      }
      addPrediction(context, predicted, supervision, scores);
    }
    return true;
  }

  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const
    {elementEvaluator->addPrediction(context, prediction, supervision, result);}

  virtual void finalizeScoreObject(const ScoreObjectPtr& scores, const FunctionPtr& function) const
    {elementEvaluator->finalizeScoreObject(scores, function);}

protected:
  friend class ElementContainerSupervisedEvaluatorClass;

  SupervisedEvaluatorPtr elementEvaluator;
};

/*
 * Evaluate by-container (average score by-container)
 */
class ContainerScoreObject : public ScoreObject
{
public:
  ContainerScoreObject()
    : mean(0.0), meanVector(new ScalarVariableMean()) {}
  
  virtual double getScoreToMinimize() const 
    {return mean;}

  void push(double score)
    {meanVector->push(score);}
  
  virtual String toString() const
  {
    double count = meanVector->getCount();
    if (!count)
      return String::empty;
    return getName() + T(": ") + String(mean * 100.0, 2) + T("% (") + String((int)count) + T(" examples)");
  }
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    ScoreObject::clone(context, target);
    target.staticCast<ContainerScoreObject>()->meanVector = meanVector->cloneAndCast<ScalarVariableMean>(context);
  }
  
  void finalize()
    {mean = meanVector->getMean();}
  
protected:
  friend class ContainerScoreObjectClass;
  
  double mean;
  ScalarVariableMeanPtr meanVector;
};

class ContainerSupervisedEvaluator : public SupervisedEvaluator
{
public:
  ContainerSupervisedEvaluator(SupervisedEvaluatorPtr elementEvaluator)
    : elementEvaluator(elementEvaluator) {}
  ContainerSupervisedEvaluator() {}

  virtual TypePtr getRequiredPredictionType() const
    {return elementEvaluator->getRequiredPredictionType();}

  virtual TypePtr getRequiredSupervisionType() const
    {return elementEvaluator->getRequiredSupervisionType();}

  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const
    {return new ContainerScoreObject();}
  
  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const
  {
    ContainerPtr supervisionContainer = example->getVariable(1).dynamicCast<Container>();
    ContainerPtr predictedContainer = output.dynamicCast<Container>();
    size_t n = predictedContainer->getNumElements();
    jassert(supervisionContainer->getNumElements() == n);
    // Compute the score of the container
    ScoreObjectPtr scoreObject = elementEvaluator->createEmptyScoreObject(context, FunctionPtr());
    for (size_t i = 0; i < n; ++i)
    {
      Variable supervision = supervisionContainer->getElement(i);
      Variable predicted = predictedContainer->getElement(i);
      if (!supervision.exists())
        continue;
      if (!predicted.exists())
      {
        context.errorCallback(T("ContainerSupervisedEvaluator::updateScoreObject"), T("Missing prediction"));
        return false;
      }
      elementEvaluator->addPrediction(context, predicted, supervision, scoreObject);
    }
    elementEvaluator->finalizeScoreObject(scoreObject, FunctionPtr());
    // Push score obtained for the container
    scores.staticCast<ContainerScoreObject>()->push(scoreObject->getScoreToMinimize());

    return true;
  }

  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const
    {jassertfalse;}

  virtual void finalizeScoreObject(const ScoreObjectPtr& scores, const FunctionPtr& function) const
    {scores.staticCast<ContainerScoreObject>()->finalize();}

protected:
  friend class ContainerSupervisedEvaluatorClass;

  SupervisedEvaluatorPtr elementEvaluator;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SUPERVISED_EVALUATOR_CONTAINER_H_

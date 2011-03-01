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

/*
** ScoreObject
*/
class ScoreObject : public Object
{
public:
  virtual double getScoreToMinimize() const = 0;
};

typedef ReferenceCountedObjectPtr<ScoreObject> ScoreObjectPtr;

class CompositeScoreObject : public ScoreObject
{
public:
  CompositeScoreObject() : scoreToMinimizeIndex((size_t)positiveIntegerType->getMissingValue().getInteger()) {}

  virtual double getScoreToMinimize() const
    {return scoreToMinimizeIndex == (size_t)positiveIntegerType->getMissingValue().getInteger() ? 0.0 : scores[scoreToMinimizeIndex]->getScoreToMinimize();}

  void addScoreObject(const ScoreObjectPtr& score)
    {scores.push_back(score);}

  void setAsScoreToMinimize(size_t index)
    {jassert(index < scores.size()); scoreToMinimizeIndex = index;}
  
  const ScoreObjectPtr& getScoreObject(size_t index) const
    {jassert(index < scores.size()); return scores[index];}

protected:
  friend class CompositeScoreObjectClass;

  std::vector<ScoreObjectPtr> scores;
  size_t scoreToMinimizeIndex;
};

typedef ReferenceCountedObjectPtr<CompositeScoreObject> CompositeScoreObjectPtr;

extern ClassPtr scoreObjectClass;

/*
** Evaluator
*/
// Function, Container[InputsObject] -> ScoreObject
class Evaluator : public Function
{
public:
  /* Function */
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? functionClass : containerClass(objectClass);}

  virtual String getOutputPostFix() const
    {return T("Evaluated");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return scoreObjectClass;}

  /* Evaluator */
  virtual ScoreObjectPtr createEmptyScoreObject() const = 0;
  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const = 0;
  virtual void finalizeScoreObject(const ScoreObjectPtr& scores) const {}
  
protected:
  friend class ProxyEvaluator;
  friend struct EvaluateExampleWorkUnit;

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

  void computeEvaluatorMultiThread(ExecutionContext& context, const FunctionPtr& function, const ContainerPtr& examples, const ScoreObjectPtr& scores) const;
  void computeEvaluatorSingleThread(ExecutionContext& context, const FunctionPtr& function, const ContainerPtr& examples, const ScoreObjectPtr& scores) const;
};

class SupervisedEvaluator : public Evaluator
{
public:
  virtual TypePtr getRequiredPredictionType() const = 0;
  virtual TypePtr getRequiredSupervisionType() const = 0;

protected:
  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, const ScoreObjectPtr& result) const = 0;
  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& score, const ObjectPtr& inputsObject, const Variable& output) const;
};

typedef ReferenceCountedObjectPtr<SupervisedEvaluator> SupervisedEvaluatorPtr;

class CompositeEvaluator : public Evaluator
{
public:
  void addEvaluator(EvaluatorPtr evaluator)
    {evaluators.push_back(evaluator);}
  
  /* Evaluator */
  virtual ScoreObjectPtr createEmptyScoreObject() const;
  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const;
  virtual void finalizeScoreObject(const ScoreObjectPtr& scores) const;

protected:
  std::vector<EvaluatorPtr> evaluators;
};

class ProxyEvaluator : public Evaluator
{
public:
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

  virtual ScoreObjectPtr createEmptyScoreObject() const;
  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const;
  virtual void finalizeScoreObject(const ScoreObjectPtr& scores) const;

protected:
  EvaluatorPtr implementation;

  virtual EvaluatorPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const = 0;
};

enum BinaryClassificationScore
{
  binaryClassificationAccuracyScore = 0,
  binaryClassificationF1Score,
  binaryClassificationMCCScore
};

// Classification
extern SupervisedEvaluatorPtr binaryClassificationEvaluator();
extern SupervisedEvaluatorPtr rocAnalysisEvaluator(BinaryClassificationScore scoreToOptimize = binaryClassificationAccuracyScore);

extern SupervisedEvaluatorPtr classificationEvaluator();

// Multi-label Classification
extern SupervisedEvaluatorPtr multiLabelClassificationEvaluator();

// Regression
extern SupervisedEvaluatorPtr regressionEvaluator();

// Default supervised evaluator
extern EvaluatorPtr defaultSupervisedEvaluator();

// Container<T> -> T
extern EvaluatorPtr containerElementsEvaluator();
};

#endif // !LBCPP_FUNCTION_EVALUATOR_H_

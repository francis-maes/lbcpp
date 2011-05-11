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
class ScoreObject : public NameableObject
{
public:
  virtual double getScoreToMinimize() const = 0;
};

typedef ReferenceCountedObjectPtr<ScoreObject> ScoreObjectPtr;

class DummyScoreObject : public ScoreObject
{
public:
  virtual double getScoreToMinimize() const
    {return 0.0;}
};

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
  Evaluator() : useMultiThreading(false) {}

  /*
  ** Evaluator
  */
  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const = 0;
  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const = 0;
  virtual void finalizeScoreObject(const ScoreObjectPtr& scores, const FunctionPtr& function) const {}

  // multi threading
  bool doUseMultiThreading() const
    {return useMultiThreading;}

  void setUseMultiThreading(bool enabled)
    {useMultiThreading = enabled;}

  /*
  ** Function
  */
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? functionClass : containerClass(objectClass);}

  virtual String getOutputPostFix() const
    {return T("Evaluated");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return scoreObjectClass;}

protected:
  friend class ProxyEvaluator;
  friend struct EvaluateExampleWorkUnit;

  bool useMultiThreading;
  bool evaluateExample(ExecutionContext& context, const ScoreObjectPtr& scores, const FunctionPtr& function, const ObjectPtr& example, CriticalSection* lock = NULL) const;

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

  void computeEvaluatorMultiThread(ExecutionContext& context, const FunctionPtr& function, const ContainerPtr& examples, const ScoreObjectPtr& scores) const;
  void computeEvaluatorSingleThread(ExecutionContext& context, const FunctionPtr& function, const ContainerPtr& examples, const ScoreObjectPtr& scores) const;
};

class SupervisedEvaluator : public Evaluator
{
public:
  virtual TypePtr getRequiredPredictionType() const = 0;
  virtual TypePtr getRequiredSupervisionType() const = 0;

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
  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const;
  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const;
  virtual void finalizeScoreObject(const ScoreObjectPtr& scores, const FunctionPtr& function) const;

protected:
  std::vector<EvaluatorPtr> evaluators;
};

class ProxyEvaluator : public Evaluator
{
public:
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const;
  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const;
  virtual void finalizeScoreObject(const ScoreObjectPtr& scores, const FunctionPtr& function) const;

protected:
  EvaluatorPtr implementation;

  virtual EvaluatorPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const = 0;
};

class CallbackBasedEvaluator : public Evaluator
{
public:
  CallbackBasedEvaluator(EvaluatorPtr evaluator = EvaluatorPtr());

  virtual FunctionPtr getFunctionToListen(const FunctionPtr& evaluatedFunction) const = 0;

  /* Evaluator */
  virtual ScoreObjectPtr createEmptyScoreObject(ExecutionContext& context, const FunctionPtr& function) const;
  virtual bool updateScoreObject(ExecutionContext& context, const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const;
  virtual void finalizeScoreObject(const ScoreObjectPtr& scores, const FunctionPtr& function) const;

protected:
  friend class CallbackBasedEvaluatorClass;

  EvaluatorPtr evaluator;
  FunctionCallbackPtr callback; // pas bien: effet de bord
};

enum BinaryClassificationScore
{
  binaryClassificationAccuracyScore = 0,
  binaryClassificationF1Score,
  binaryClassificationMCCScore
};

// Classification
extern SupervisedEvaluatorPtr binaryClassificationEvaluator(BinaryClassificationScore scoreToOptimize = binaryClassificationAccuracyScore);
extern SupervisedEvaluatorPtr rocAnalysisEvaluator(BinaryClassificationScore scoreToOptimize = binaryClassificationAccuracyScore, bool saveConfusionMatrices = false);

extern SupervisedEvaluatorPtr classificationEvaluator();

// Multi-label Classification
extern SupervisedEvaluatorPtr multiLabelClassificationEvaluator();

// Regression
extern SupervisedEvaluatorPtr regressionEvaluator();

// Ranking
extern SupervisedEvaluatorPtr rankingEvaluator();

// Default supervised evaluator
extern EvaluatorPtr defaultSupervisedEvaluator();

// Save To Directory
extern EvaluatorPtr saveToDirectoryEvaluator(const File& directory, const String& extension = T(".xml"));

// Container[Pair[T_Input,T_Supervision]> -> T_Supervision
extern SupervisedEvaluatorPtr containerSupervisedEvaluator(SupervisedEvaluatorPtr elementEvaluator);

extern SupervisedEvaluatorPtr symmetricMatrixSupervisedEvaluator(SupervisedEvaluatorPtr elementEvaluator, size_t minimumDistanceFromDiagonal);

};

#endif // !LBCPP_FUNCTION_EVALUATOR_H_

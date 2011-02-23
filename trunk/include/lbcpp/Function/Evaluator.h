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
};

typedef ReferenceCountedObjectPtr<ScoreObject> ScoreObjectPtr;

extern ClassPtr scoreObjectClass;

class Evaluator : public Function
{
public:
  /* Function */
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return containerClass(anyType);}

  virtual String getOutputPostFix() const
    {return T("Evaluated");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return scoreObjectClass;}
};

class SupervisedEvaluator : public Evaluator
{
public:
  virtual TypePtr getRequiredPredictionType() const = 0;
  virtual TypePtr getRequiredSupervisionType() const = 0;

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return containerClass(index == 0 ? anyType : getRequiredPredictionType());}

protected:
  virtual ScoreObjectPtr createEmptyScoreObject() const = 0;
  virtual void addPrediction(ExecutionContext& context, const Variable& prediction, const Variable& supervision, ScoreObjectPtr& result) const = 0;
  virtual void finalizeScoreObject(ScoreObjectPtr& score) const = 0;

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
};

typedef ReferenceCountedObjectPtr<SupervisedEvaluator> SupervisedEvaluatorPtr;

class OutputEvaluator : public Evaluator
{
public:
  virtual TypePtr getRequiredOutputType() const = 0;

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return containerClass(index == 0 ? anyType : getRequiredOutputType());}

  virtual ScoreObjectPtr computeOutputEvaluator(ExecutionContext& context, const ContainerPtr& outputs) const = 0;

protected:
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
};

typedef ReferenceCountedObjectPtr<OutputEvaluator> OutputEvaluatorPtr;

// Classification
extern SupervisedEvaluatorPtr binaryClassificationEvaluator();
extern SupervisedEvaluatorPtr rocAnalysisEvaluator();

extern SupervisedEvaluatorPtr classificationEvaluator();

// Multi-label Classification
extern SupervisedEvaluatorPtr multiLabelClassificationEvaluator();

// Regression
extern SupervisedEvaluatorPtr regressionEvaluator();

extern EvaluatorPtr functionBasedEvaluator(const FunctionPtr& function);

class CompositeScoreObject : public ScoreObject
{
public:
  CompositeScoreObject() : scoreToMinimizeIndex((size_t)-1) {}

  virtual double getScoreToMinimize() const
    {return scoreToMinimizeIndex == (size_t)-1 ? 0.0 : scores[scoreToMinimizeIndex]->getScoreToMinimize();}

  void pushScoreObject(const ScoreObjectPtr& score)
    {scores.push_back(score);}

  void setAsScoreToMinimize(size_t index)
    {jassert(index < scores.size()); scoreToMinimizeIndex = index;}

protected:
  friend class CompositeScoreObjectClass;

  std::vector<ScoreObjectPtr> scores;
  size_t scoreToMinimizeIndex;
};

typedef ReferenceCountedObjectPtr<CompositeScoreObject> CompositeScoreObjectPtr;

};

#endif // !LBCPP_FUNCTION_EVALUATOR_H_

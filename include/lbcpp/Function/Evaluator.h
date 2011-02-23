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
  virtual void getScores(std::vector< std::pair<String, double> >& res) const = 0;
};

typedef ReferenceCountedObjectPtr<ScoreObject> ScoreObjectPtr;

extern ClassPtr scoreObjectClass;

class Evaluator : public Function
{
public:
  Evaluator()
    {numInputs = 2;}

  virtual TypePtr getRequiredPredictedElementsType() const = 0;

  virtual TypePtr getRequiredSupervisionElementsType() const = 0;

  /* Function */
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return containerClass(index == 0 ? getRequiredPredictedElementsType() : getRequiredSupervisionElementsType());}

  virtual String getOutputPostFix() const
    {return T("Evaluated");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return scoreObjectClass;}

protected:
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
  
  virtual ScoreObjectPtr createEmptyScoreObject() const = 0;

  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject, ScoreObjectPtr& result) const = 0;
};

// Classification
extern EvaluatorPtr binaryClassificationConfusionEvaluator();
extern EvaluatorPtr rocAnalysisEvaluator();

extern EvaluatorPtr classificationAccuracyEvaluator();

// Multi-label Classification
extern EvaluatorPtr multiLabelClassificationEvaluator();

// Regression
extern EvaluatorPtr regressionErrorEvaluator();

extern FunctionPtr functionBasedEvaluator(const FunctionPtr& function);

class CompositeScoreObject : public ScoreObject
{
public:
  CompositeScoreObject() : scoreToMinimizeIndex((size_t)-1) {}

  virtual double getScoreToMinimize() const
    {return scoreToMinimizeIndex == (size_t)-1 ? 0.0 : scores[scoreToMinimizeIndex]->getScoreToMinimize();}

  virtual void getScores(std::vector< std::pair<String, double> >& res) const;

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

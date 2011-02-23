/*-----------------------------------------.---------------------------------.
| Filename: FunctionBasedEvaluator.h       | Function Based Evaluator        |
| Author  : Julien Becker                  |                                 |
| Started : 23/02/2011 12:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_BASED_EVALUATOR_H_
# define LBCPP_FUNCTION_BASED_EVALUATOR_H_

# include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{

class FunctionBasedEvaluator : public Evaluator
{
public:
  FunctionBasedEvaluator(FunctionPtr function) : function(function) {}
  FunctionBasedEvaluator() {}

  virtual TypePtr getRequiredPredictedElementsType() const
    {return function->getRequiredInputType(0, 2);}

  virtual TypePtr getRequiredSupervisionElementsType() const
    {return function->getRequiredInputType(1, 2);}

protected:
  friend class FunctionBasedEvaluatorClass;

  FunctionPtr function;

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return function->compute(context, inputs);}
  
  virtual void finalizeScoreObject(ScoreObjectPtr& score) const
    {jassertfalse;}
  
  virtual ScoreObjectPtr createEmptyScoreObject() const
    {jassertfalse; return ScoreObjectPtr();}
  
  virtual void addPrediction(ExecutionContext& context, const Variable& predictedObject, const Variable& correctObject, ScoreObjectPtr& result) const
    {jassertfalse;}
};
  
};

#endif // !LBCPP_FUNCTION_BASED_EVALUATOR_H_

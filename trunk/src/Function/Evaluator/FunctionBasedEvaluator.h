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

  virtual TypePtr getRequiredPredictionType() const
    {return function->getRequiredInputType(0, 2);}

  virtual TypePtr getRequiredSupervisionType() const
    {return function->getRequiredInputType(1, 2);}

protected:
  friend class FunctionBasedEvaluatorClass;

  FunctionPtr function;

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return function->compute(context, inputs, getNumInputs());}
  
  virtual void finalizeScoreObject(const ScoreObjectPtr& score) const
    {jassertfalse;}
  
  virtual ScoreObjectPtr createEmptyScoreObject() const
    {jassertfalse; return ScoreObjectPtr();}
  
  virtual void updateScoreObject(const ScoreObjectPtr& scores, const ObjectPtr& example, const Variable& output) const
    {jassertfalse;}
};
  
};

#endif // !LBCPP_FUNCTION_BASED_EVALUATOR_H_

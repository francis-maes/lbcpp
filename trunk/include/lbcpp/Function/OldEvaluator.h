/*-----------------------------------------.---------------------------------.
| Filename: Evaluator.h                    | Base class for evaluators       |
| Author  : Francis Maes                   |                                 |
| Started : 27/04/2010 15:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OLD_EVALUATOR_H_
# define LBCPP_OLD_EVALUATOR_H_

# include <lbcpp/Core/Variable.h>
# include <lbcpp/Data/RandomVariable.h>
# include "predeclarations.h"
# include "Evaluator.h"

namespace lbcpp
{

class OldEvaluator : public NameableObject
{
public:
  OldEvaluator(const String& name) : NameableObject(name) {}
  OldEvaluator() {}

  virtual void addPrediction(ExecutionContext& context, const Variable& predicted, const Variable& correct) = 0;

  virtual void getScores(std::vector< std::pair<String, double> >& res) const = 0;
  virtual double getDefaultScore() const = 0;

  lbcpp_UseDebuggingNewOperator
};

}; /* namespace lbcpp */

#endif // !LBCPP_OLD_EVALUATOR_H_

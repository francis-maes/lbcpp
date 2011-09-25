/*-----------------------------------------.---------------------------------.
| Filename: FormulaLearnAndSearch.h        | Formula Learn&Search Algorithm  |
| Author  : Francis Maes                   |                                 |
| Started : 25/09/2011 21:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_FORMULA_LEARN_AND_SEARCH_H_
# define LBCPP_SEQUENTIAL_DECISION_FORMULA_LEARN_AND_SEARCH_H_

# include "BanditFormulaOptimizer.h"

namespace lbcpp
{

class FormulaLearnAndSearch : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {   
    //FormulaPool pool(objective);
    return Variable();
  }
  
protected:
  friend class FormulaLearnAndSearchClass;

  FormulaSearchProblemPtr problem;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_FORMULA_LEARN_AND_SEARCH_H_

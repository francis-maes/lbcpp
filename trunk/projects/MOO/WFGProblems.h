/*-----------------------------------------.---------------------------------.
| Filename: WFGProblems.h                  | The WFG benchmark suite         |
| Author  : Denny Verbeeck                 |                                 |
| Started : 14/03/2014 17:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef MOO_PROBLEM_WFG_H_
# define MOO_PROBLEM_WFG_H_

# include <ml/Problem.h>

namespace lbcpp
{

class WFGObjective : public Objective
{
public:
  WFGObjective() {}

protected:
  size_t k;         //!< number of position-related parameters
  size_t l;         //!< number of distance-related parameters
  size_t m;         //!< number of objectives
};

} /* namespace lbcpp */


#endif //!MOO_PROBLEM_WFG_H_
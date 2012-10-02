/*-----------------------------------------.---------------------------------.
| Filename: Solution.h                     | Solution                        |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SOLUTION_H_
# define LBCPP_ML_SOLUTION_H_

# include "Fitness.h"

namespace lbcpp
{

class Solution : public Object
{
public:
  Solution(const ObjectPtr& object, const FitnessPtr& fitness)
    : object(object), fitness(fitness) {}
  Solution() {}

  /*
  ** Object
  */
  const ObjectPtr& getObject() const
    {return object;}

  /*
  ** Fitness
  */
  const FitnessPtr& getFitness() const
    {return fitness;}

  const FitnessLimitsPtr& getFitnessLimits() const
    {return fitness->getLimits();}

protected:
  friend class SolutionClass;

  ObjectPtr object;
  FitnessPtr fitness;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SOLUTION_H_

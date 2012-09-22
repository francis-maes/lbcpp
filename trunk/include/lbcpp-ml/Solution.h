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

class MOOSolution : public Object
{
public:
  MOOSolution(const ObjectPtr& object, const MOOFitnessPtr& fitness)
    : object(object), fitness(fitness) {}
  MOOSolution() {}

  /*
  ** Object
  */
  const ObjectPtr& getObject() const
    {return object;}

  /*
  ** Fitness
  */
  const MOOFitnessPtr& getFitness() const
    {return fitness;}

  const MOOFitnessLimitsPtr& getFitnessLimits() const
    {return fitness->getLimits();}

protected:
  friend class MOOSolutionClass;

  ObjectPtr object;
  MOOFitnessPtr fitness;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SOLUTION_H_

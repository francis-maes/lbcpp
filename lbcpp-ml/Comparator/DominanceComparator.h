/*-----------------------------------------.---------------------------------.
| Filename: DominanceComparator.h          | Dominance Comparator            |
| Author  : Francis Maes                   |                                 |
| Started : 1/09/2012 16:03                |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_COMPARATOR_DOMINANCE_H_
# define LBCPP_ML_COMPARATOR_DOMINANCE_H_

# include <lbcpp-ml/Comparator.h>
# include <lbcpp-ml/SolutionSet.h>

namespace lbcpp
{

class DominanceComparator : public MOOSolutionComparator
{
public:
  virtual void initialize(const MOOSolutionSetPtr& solutions)
    {this->solutions = solutions;}

  virtual int compare(size_t index1, size_t index2)
  {
    MOOFitnessPtr fitness1 = solutions->getFitness(index1);
    MOOFitnessPtr fitness2 = solutions->getFitness(index2);

    // TODO: optimized version to avoid two calls to MOOFitness::strictlyDominates()

    //MOOFitnessLimitsPtr limits = solution1->getFitnessLimits();
    //jassert(limits == solution2->getFitnessLimits());

    if (fitness1->strictlyDominates(fitness2))
      return -1;
    else if (fitness2->strictlyDominates(fitness1))
      return 1;
    else
      return 0;
  }

private:
  MOOSolutionSetPtr solutions;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_COMPARATOR_DOMINANCE_H_

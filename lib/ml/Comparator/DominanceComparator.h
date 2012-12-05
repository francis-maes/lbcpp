/*-----------------------------------------.---------------------------------.
| Filename: DominanceComparator.h          | Dominance Comparator            |
| Author  : Francis Maes                   |                                 |
| Started : 1/09/2012 16:03                |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_COMPARATOR_DOMINANCE_H_
# define ML_COMPARATOR_DOMINANCE_H_

# include <ml/SolutionComparator.h>
# include <ml/SolutionContainer.h>

namespace lbcpp
{

class DominanceComparator : public SolutionComparator
{
public:
  virtual void initialize(const SolutionContainerPtr& solutions)
    {this->solutions = solutions;}

  virtual int compare(size_t index1, size_t index2)
  {
    FitnessPtr fitness1 = solutions->getFitness(index1);
    FitnessPtr fitness2 = solutions->getFitness(index2);

    // TODO: optimized version to avoid two calls to Fitness::strictlyDominates()

    //FitnessLimitsPtr limits = solution1->getFitnessLimits();
    //jassert(limits == solution2->getFitnessLimits());

    if (fitness1->strictlyDominates(fitness2))
      return -1;
    else if (fitness2->strictlyDominates(fitness1))
      return 1;
    else
      return 0;
  }

private:
  SolutionContainerPtr solutions;
};

}; /* namespace lbcpp */

#endif // !ML_COMPARATOR_DOMINANCE_H_

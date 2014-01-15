/*-----------------------------------------.---------------------------------.
| Filename: SolutionComparator.h           | Solution Comparator             |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SOLUTION_COMPARATOR_H_
# define ML_SOLUTION_COMPARATOR_H_

# include "Fitness.h"

namespace lbcpp
{

class SolutionComparator : public Object
{
public:
  virtual void initialize(const SolutionContainerPtr& solutions) = 0;

  // returns -1 if solution1 is prefered, +1 if solution2 is prefered and 0 if there is no preference between the two solutions
  virtual int compareSolutions(size_t index1, size_t index2) = 0;
};

extern SolutionComparatorPtr objectiveComparator(size_t index);
extern SolutionComparatorPtr lexicographicComparator();
extern SolutionComparatorPtr dominanceComparator();
extern SolutionComparatorPtr paretoRankAndCrowdingDistanceComparator();
extern SolutionComparatorPtr spea2Comparator();

}; /* namespace lbcpp */

#endif // !ML_SOLUTION_COMPARATOR_H_

/*-----------------------------------------.---------------------------------.
| Filename: ObjectiveComparator.h          | Objective Comparator            |
| Author  : Francis Maes                   |                                 |
| Started : 1/09/2012 16:07                |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_COMPARATOR_OBJECTIVE_H_
# define ML_COMPARATOR_OBJECTIVE_H_

# include <ml/SolutionComparator.h>
# include <ml/SolutionContainer.h>

namespace lbcpp
{

class ObjectiveComparator : public SolutionComparator
{
public:
  ObjectiveComparator(size_t index = (size_t)-1) : index(index) {}

  virtual void initialize(const SolutionContainerPtr& solutions)
    {this->solutions = solutions;}

  virtual int compareSolutions(size_t index1, size_t index2)
  {
    double value1 = solutions->getFitness(index1)->getValue(index);
    double value2 = solutions->getFitness(index2)->getValue(index);
    double deltaValue = solutions->getFitnessLimits()->getObjectiveSign(index) * (value2 - value1);
    return deltaValue > 0 ? 1 : (deltaValue < 0 ? -1 : 0);
  }

protected:
  friend class ObjectiveComparatorClass;

  size_t index;

private:
  SolutionContainerPtr solutions;
};

}; /* namespace lbcpp */

#endif // !ML_COMPARATOR_OBJECTIVE_H_

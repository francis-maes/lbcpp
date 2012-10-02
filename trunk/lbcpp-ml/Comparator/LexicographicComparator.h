/*-----------------------------------------.---------------------------------.
| Filename: LexicographicComparator.h      | Lexicographic Comparator        |
| Author  : Francis Maes                   |                                 |
| Started : 1/09/2012 16:07                |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_COMPARATOR_LEXICOGRAPHIC_H_
# define LBCPP_ML_COMPARATOR_LEXICOGRAPHIC_H_

# include <lbcpp-ml/SolutionComparator.h>
# include <lbcpp-ml/SolutionSet.h>

namespace lbcpp
{

class LexicographicComparator : public SolutionComparator
{
public:
  virtual void initialize(const SolutionSetPtr& solutions)
    {this->solutions = solutions;}

  virtual int compare(size_t index1, size_t index2)
  {
    FitnessPtr fitness1 = solutions->getFitness(index1);
    FitnessPtr fitness2 = solutions->getFitness(index2);
    FitnessLimitsPtr limits = solutions->getFitnessLimits();

    for (size_t i = 0; i < limits->getNumObjectives(); ++i)
    {
      double value1 = fitness1->getValue(i);
      double value2 = fitness2->getValue(i);
      if (value1 != value2)
      {
        double deltaValue = limits->getObjectiveSign(i) * (value2 - value1);
        return deltaValue > 0 ? 1 : -1;
      }
    }
    return 0;
  }

private:
  SolutionSetPtr solutions;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_COMPARATOR_LEXICOGRAPHIC_H_

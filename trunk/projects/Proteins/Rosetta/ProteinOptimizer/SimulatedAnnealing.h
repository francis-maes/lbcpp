/*-----------------------------------------.---------------------------------.
| Filename: SimulatedAnnealing.h           | SimulatedAnnealing              |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Mar 1, 2012  8:21:03 AM        |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_SIMULATEDANNEALING_H_
# define LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_SIMULATEDANNEALING_H_

# include "../Data/Pose.h"
# include "GeneralOptimizer.h"

namespace lbcpp
{

class SimulatedAnnealing : public GeneralOptimizer
{
public:
  SimulatedAnnealing();

  virtual void optimize(ExecutionContext& context)
  {

  }

protected:
  friend class SimulatedAnnealingClass;

};

typedef ReferenceCountedObjectPtr<SimulatedAnnealing> SimulatedAnnealingPtr;

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_SIMULATEDANNEALING_H_


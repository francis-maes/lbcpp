/*-----------------------------------------.---------------------------------.
| Filename: SequentialOptimizer.h          | SequentialOptimizer             |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 29 avr. 2011  22:51:54         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_PROTEIN_SEQUENTIAL_OPTIMIZER_H_
# define LBCPP_PROTEINS_ROSETTA_PROTEIN_SEQUENTIAL_OPTIMIZER_H_

# include "precompiled.h"
# include "../ProteinOptimizer.h"
# include "SimulatedAnnealingOptimizer.h"

namespace lbcpp
{
class ProteinSequentialOptimizer;
typedef ReferenceCountedObjectPtr<ProteinSequentialOptimizer> ProteinSequentialOptimizerPtr;

class ProteinSequentialOptimizer: public ProteinOptimizer
{
public:
  ProteinSequentialOptimizer(long long seedForRandom = 0) :
    ProteinOptimizer(seedForRandom)
  {
  }

  ProteinSequentialOptimizer(ExecutionContextPtr context, String name, double frequencyCallback,
      long long seedForRandom = 0) :
    ProteinOptimizer(context, name, frequencyCallback, juce::File(), -1, seedForRandom)
  {
  }

  core::pose::PoseOP apply(core::pose::PoseOP& pose, ProteinMoverPtr& mover)
  {
    return sequentialOptimization(pose, mover);
  }

  /*
   * Performs sequential simulation on the pose object. This function adds a residue
   * at each iteration and then performs optimization on the resulting protein object.
   * The purpose is to fold the protein as it was being cronstructed.
   * @param pose the initial conformation
   * @param mover pointer to a mover that perturbs the object at each iteration.
   * @return the new conformation
   */
  core::pose::PoseOP sequentialOptimization(core::pose::PoseOP& pose, ProteinMoverPtr& mover)
  {
    core::pose::PoseOP acc = new core::pose::Pose();

    double initialTemperature = 4.0;
    double finalTemperature = 0.01;
    int numberDecreasingSteps = 100;
    int maxSteps = 0;
    int factor = 5000;
    bool store = getVerbosity();
    setVerbosity(false);

    for (int i = 1; i <= pose->n_residue(); i++)
    {
      ProteinSimulatedAnnealingOptimizerPtr optimizer = new ProteinSimulatedAnnealingOptimizer();

      maxSteps = (int)std::max((int)((factor * log(i)) + 1), numberDecreasingSteps);
      acc->append_residue_by_bond(pose->residue(i), true);
      core::pose::PoseOP tempPose = optimizer->simulatedAnnealingOptimization(acc, mover,
          initialTemperature, finalTemperature, numberDecreasingSteps, maxSteps);
      //core::pose::PoseOP tempPose = monteCarloOptimization(acc, mover, optArgs, finalTemperature,
      //    maxSteps);
      if (tempPose.get() == NULL)
      {
        return NULL;
      }
      (*acc) = (*tempPose);
    }

    setVerbosity(store);
    return acc;
  }
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_SEQUENTIAL_OPTIMIZER_H_

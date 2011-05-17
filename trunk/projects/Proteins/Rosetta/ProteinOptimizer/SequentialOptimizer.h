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
# include "../Sampler.h"
# include "../ProteinMover.h"
# include "../Sampler/ProteinMoverSampler.h"

namespace lbcpp
{
class ProteinSequentialOptimizer;
typedef ReferenceCountedObjectPtr<ProteinSequentialOptimizer> ProteinSequentialOptimizerPtr;

class ProteinSequentialOptimizer: public ProteinOptimizer
{
public:
  ProteinSequentialOptimizer() :
    ProteinOptimizer()
  {
  }

  ProteinSequentialOptimizer(String name) :
    ProteinOptimizer(name, -1, -1, juce::File())
  {
    if (name.isEmpty())
      this->name = String("Default");
    else
      this->name = name;
  }
/*
  core::pose::PoseOP apply(core::pose::PoseOP& pose, ExecutionContext& context,
      RandomGeneratorPtr& random)
  {
    SamplerPtr sampler;
    return sequentialOptimization(pose, sampler, context, random);
  }*/

  virtual void apply(ExecutionContext& context, const core::pose::PoseOP& pose,
                     const RandomGeneratorPtr& random, const SamplerPtr& sampler, core::pose::PoseOP& res)
  {
#ifdef LBCPP_PROTEIN_ROSETTA
    res = sequentialOptimization(pose, sampler, context, random);
#else
    jassert(false);
#endif // LBCPP_PROTEIN_ROSETTA
  }

#ifdef LBCPP_PROTEIN_ROSETTA

  /*
   * Performs sequential simulation on the pose object. This function adds a residue
   * at each iteration and then performs optimization on the resulting protein object.
   * The purpose is to fold the protein as it was being cronstructed.
   * @param pose the initial conformation
   * @param mover pointer to a mover that perturbs the object at each iteration.
   * @return the new conformation
   */
  core::pose::PoseOP sequentialOptimization(const core::pose::PoseOP& pose,
      const SamplerPtr& sampler, ExecutionContext& context, const RandomGeneratorPtr& random)
  {
    core::pose::PoseOP acc = new core::pose::Pose();

    double initialTemperature = 4.0;
    double finalTemperature = 0.01;
    int numberDecreasingSteps = 100;
    int timesReinitialization = 5;
    int maxSteps = 0;
    int factor = 5000;
    bool store = getVerbosity();
    setVerbosity(false);

    for (size_t i = 1; i <= pose->n_residue(); i++)
    {
      maxSteps = (int)std::max((int)((factor * log((double)i)) + 1), numberDecreasingSteps);

      ProteinSimulatedAnnealingOptimizerPtr optimizer = new ProteinSimulatedAnnealingOptimizer(
          initialTemperature, finalTemperature, numberDecreasingSteps, maxSteps,
          timesReinitialization, name);

      acc->append_residue_by_bond(pose->residue(i), true);

      SamplerPtr samp = new ProteinMoverSampler(acc->n_residue());

      core::pose::PoseOP tempPose;
//      core::pose::PoseOP tempPose = optimizer->simulatedAnnealingOptimization(acc, samp.staticCast<
//          Sampler> (), context, random, initialTemperature, finalTemperature,
//          numberDecreasingSteps, maxSteps, timesReinitialization);

      if (tempPose.get() == NULL)
      {
        return NULL;
      }
      (*acc) = (*tempPose);
    }
    std::cout << "test 5" << std::endl;
    setVerbosity(store);
    return acc;
  }
#endif // LBCPP_PROTEIN_ROSETTA

};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_SEQUENTIAL_OPTIMIZER_H_

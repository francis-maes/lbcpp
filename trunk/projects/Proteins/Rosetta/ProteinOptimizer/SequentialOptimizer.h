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
# include "../Sampler/ProteinMoverSampler.h"
# include "../Sampler/PhiPsiMoverSampler.h"
# include "../Sampler/ShearMoverSampler.h"
# include "../Sampler/RigidBodyGeneralMoverSampler.h"
# include "../Sampler/RigidBodySpinMoverSampler.h"
# include "../Sampler/RigidBodyTransMoverSampler.h"

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

  core::pose::PoseOP apply(core::pose::PoseOP& pose, ExecutionContext& context,
      RandomGeneratorPtr& random)
  {
    ProteinMoverSamplerPtr sampler;
    return sequentialOptimization(pose, sampler, context, random);
  }

  core::pose::PoseOP apply(core::pose::PoseOP& pose, ProteinMoverSamplerPtr& sampler,
      ExecutionContext& context, RandomGeneratorPtr& random)
  {
    return sequentialOptimization(pose, sampler, context, random);
  }

  /*
   * Performs sequential simulation on the pose object. This function adds a residue
   * at each iteration and then performs optimization on the resulting protein object.
   * The purpose is to fold the protein as it was being cronstructed.
   * @param pose the initial conformation
   * @param mover pointer to a mover that perturbs the object at each iteration.
   * @return the new conformation
   */
  core::pose::PoseOP sequentialOptimization(core::pose::PoseOP& pose,
      ProteinMoverSamplerPtr& sampler, ExecutionContext& context, RandomGeneratorPtr& random)
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

    for (int i = 1; i <= pose->n_residue(); i++)
    {
      maxSteps = (int)std::max((int)((factor * log(i)) + 1), numberDecreasingSteps);

      ProteinSimulatedAnnealingOptimizerPtr optimizer = new ProteinSimulatedAnnealingOptimizer(
          initialTemperature, finalTemperature, numberDecreasingSteps, maxSteps,
          timesReinitialization, name);

      acc->append_residue_by_bond(pose->residue(i), true);

      PhiPsiMoverSamplerPtr samp0 = new PhiPsiMoverSampler(acc->n_residue(), 0, 25, 0, 25);
      ShearMoverSamplerPtr samp1 = new ShearMoverSampler(acc->n_residue(), 0, 25, 0, 25);
      RigidBodyTransMoverSamplerPtr samp2 = new RigidBodyTransMoverSampler(acc->n_residue(), 0.5,
          0.25);
      RigidBodySpinMoverSamplerPtr samp3 = new RigidBodySpinMoverSampler(acc->n_residue(), 0, 20);
      RigidBodyGeneralMoverSamplerPtr samp4 = new RigidBodyGeneralMoverSampler(acc->n_residue(),
          0.5, 0.25, 0, 20);

      std::vector<Variable> samplers;
      samplers.push_back(Variable(samp0));
      samplers.push_back(Variable(samp1));
      samplers.push_back(Variable(samp2));
      samplers.push_back(Variable(samp3));
      samplers.push_back(Variable(samp4));
      ProteinMoverSamplerPtr samp(new ProteinMoverSampler(5, samplers));

      core::pose::PoseOP tempPose = optimizer->simulatedAnnealingOptimization(acc, samp, context,
          random, initialTemperature, finalTemperature, numberDecreasingSteps, maxSteps,
          timesReinitialization);

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
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_SEQUENTIAL_OPTIMIZER_H_

/*-----------------------------------------.---------------------------------.
| Filename: RosettaTest.h                  | Rosetta Test                    |
| Author  : Francis Maes                   |                                 |
| Started : 30/03/2011 13:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_TEST_H_
# define LBCPP_PROTEINS_ROSETTA_TEST_H_

# include "precompiled.h"
# include "../Data/Protein.h"
# include "../Data/AminoAcid.h"
# include "../Data/Residue.h"
# include "../Data/Formats/PDBFileGenerator.h"
# include "../Evaluator/QScoreEvaluator.h"
# include "RosettaUtils.h"
# include "ProteinMover.h"
# include "ProteinOptimizer.h"
# include "ProteinOptimizer/GreedyOptimizer.h"
# include "ProteinOptimizer/SequentialOptimizer.h"
# include "ProteinOptimizer/SimulatedAnnealingOptimizer.h"
# include "ProteinOptimizer/MonteCarloOptimizer.h"
# include <iostream>
# include <fstream>
# include <string>
# include <vector>
# include <cmath>
# include <time.h>
# include "ProteinMover/PhiPsiMover.h"
# include "ProteinMover/RigidBodySpinMover.h"
# include "ProteinMover/ShearMover.h"
# include "ProteinMover/RigidBodyGeneralMover.h"
# include "ProteinMover/RigidBodyTransMover.h"
# include "Sampler.h"
# include "Sampler/GaussianContinuousSampler.h"
# include "Sampler/ParzenContinuousSampler.h"
# include "Sampler/EnumerationDiscreteSampler.h"
# include "Sampler/ProteinMoverSampler.h"
# include "Sampler/PhiPsiMoverSampler.h"
# include "Sampler/ShearMoverSampler.h"
# include "Sampler/RigidBodyGeneralMoverSampler.h"
# include "Sampler/RigidBodyTransMoverSampler.h"
# include "Sampler/RigidBodySpinMoverSampler.h"

# undef T
#  include <core/pose/Pose.hh>
#  include <core/chemical/util.hh>
#  include <core/io/pdb/pose_io.hh>
#  include <core/io/pdb/file_data.hh>
#  include <core/kinematics/FoldTree.hh>
#  include <protocols/init.hh>
# define T JUCE_T

using namespace std;

namespace lbcpp
{

class RosettaTest: public WorkUnit
{
private:
  friend class RosettaTestClass;
  size_t arg;
  String proteinsDir;

public:
  virtual Variable run(ExecutionContext& context)
  {
    rosettaInitialization(context, false);
    RandomGeneratorPtr random = new RandomGenerator(0);

    core::pose::PoseOP pose = new core::pose::Pose();
    makePoseFromSequence(pose, T("AAAA"));

    PhiPsiMoverSamplerPtr samp0 = new PhiPsiMoverSampler(pose->n_residue(), 0, 25, 0, 25);
    ShearMoverSamplerPtr samp1 = new ShearMoverSampler(pose->n_residue(), 0, 25, 0, 25);
    RigidBodyTransMoverSamplerPtr samp2 = new RigidBodyTransMoverSampler(pose->n_residue(), 0.5,
        0.25);
    RigidBodySpinMoverSamplerPtr samp3 = new RigidBodySpinMoverSampler(pose->n_residue(), 0, 20);
    RigidBodyGeneralMoverSamplerPtr samp4 = new RigidBodyGeneralMoverSampler(pose->n_residue(),
        0.5, 0.25, 0, 20);

    std::vector<Variable> samplers;
    samplers.push_back(Variable(samp0));
    samplers.push_back(Variable(samp1));
    samplers.push_back(Variable(samp2));
    samplers.push_back(Variable(samp3));
    samplers.push_back(Variable(samp4));
    ProteinMoverSamplerPtr samp(new ProteinMoverSampler(5, samplers));
    //ProteinGreedyOptimizerPtr o = new ProteinGreedyOptimizer(1000);
    //ProteinMonteCarloOptimizerPtr o = new ProteinMonteCarloOptimizer(2.0, 1000, 5, String("AAA MC"));
    ProteinSimulatedAnnealingOptimizerPtr o = new ProteinSimulatedAnnealingOptimizer(4.0, 0.01, 50,
        10000, 5, String("AAA SA"));
    //ProteinSequentialOptimizerPtr o = new ProteinSequentialOptimizer("AAAA");

    core::pose::PoseOP temppose = o->apply(pose, samp, context, random);

    context.informationCallback(T("RosettaTest done."));
    return Variable();

  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_ROSETTA_TEST_H_

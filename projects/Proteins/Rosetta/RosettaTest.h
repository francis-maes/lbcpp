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
# include "Sampler/SimpleResidueSampler.h"

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

    ProteinPtr proteinTarget = Protein::createFromXml(context, context.getFile(T("1LEA_2.xml")));
    ProteinPtr proteinRef = Protein::createFromXml(context, context.getFile(T("1LEA.xml")));
    core::pose::PoseOP target = convertProteinToPose(context, proteinTarget);
    core::pose::PoseOP reference = convertProteinToPose(context, proteinRef);
    cout << "score ref : " << getConformationScore(reference) << endl;
    cout << "score target : " << getConformationScore(target) << endl;

    PhiPsiMoverSamplerPtr samp0 = new PhiPsiMoverSampler(proteinTarget->getLength(), 0, 25, 0, 25);
    ShearMoverSamplerPtr samp1 = new ShearMoverSampler(proteinTarget->getLength(), 0, 25, 0, 25);
    RigidBodyTransMoverSamplerPtr samp2 = new RigidBodyTransMoverSampler(proteinTarget->getLength(), 0, 0.5);

    std::vector<Variable> samplers;

    samplers.push_back(Variable(samp0));
    samplers.push_back(Variable(samp2));
    samplers.push_back(Variable(samp1));

    ProteinMoverSamplerPtr samp = new ProteinMoverSampler(3, samplers);

    ProteinEDAOptimizerPtr opti = new ProteinEDAOptimizer(0.5);
    ProteinMoverSamplerPtr out = opti->findBestMovers(context, random, target, reference, samp, 2,
        5, 0.5);
    out->saveToFile(context, context.getFile(T("outSampler.xml")));




    context.informationCallback(T("RosettaTest done."));
    return Variable();

  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_ROSETTA_TEST_H_

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
# include "Sampler/ParzenContinuousSampler.h"
# include "Sampler/EnumerationDiscreteSampler.h"
# include "Sampler/ProteinMoverSampler.h"
# include "Sampler/PhiPsiMoverSampler.h"
# include "Sampler/ShearMoverSampler.h"
# include "Sampler/RigidBodyGeneralMoverSampler.h"
# include "Sampler/RigidBodyTransMoverSampler.h"
# include "Sampler/RigidBodySpinMoverSampler.h"
# include "Sampler/SimpleResidueSampler.h"
# include "Sampler/GaussianMultivariateSampler.h"
# include "Sampler/DualResidueSampler.h"

# undef T
#  include <core/pose/Pose.hh>
#  include <core/chemical/util.hh>
#  include <core/io/pdb/pose_io.hh>
#  include <core/io/pdb/file_data.hh>
#  include <core/kinematics/FoldTree.hh>
# define T JUCE_T

using namespace std;

namespace lbcpp
{
void printMover(ProteinMoverPtr& t);
void outputEnergiesAndQScores(ExecutionContext& context, String referenceDirectory, String targetDirectory);

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



    context.informationCallback(T("RosettaTest done."));
    return Variable();

  }
};

void printMover(ProteinMoverPtr& t)
{
  if (t.isInstanceOf<PhiPsiMover> ())
  {
    PhiPsiMoverPtr t0 = (PhiPsiMoverPtr)t;
    std::cout << "PhiPsi" << " r : " << t0->getResidueIndex() << ", phi : " << t0->getDeltaPhi()
        << ", psi : " << t0->getDeltaPsi() << std::endl;
  }
  else if (t.isInstanceOf<ShearMover> ())
  {
    ShearMoverPtr t0 = (ShearMoverPtr)t;
    std::cout << "Shear " << " r : " << t0->getResidueIndex() << ", phi : " << t0->getDeltaPhi()
        << ", psi : " << t0->getDeltaPsi() << std::endl;
  }
  else if (t.isInstanceOf<RigidBodyTransMover> ())
  {
    RigidBodyTransMoverPtr t0 = (RigidBodyTransMoverPtr)t;
    std::cout << "Trans" << " r1 : " << t0->getIndexResidueOne() << ", r2 : "
        << t0->getIndexResidueTwo() << ", magnitude : " << t0->getMagnitude() << std::endl;
  }
  else if (t.isInstanceOf<RigidBodySpinMover> ())
  {
    RigidBodySpinMoverPtr t0 = (RigidBodySpinMoverPtr)t;
    std::cout << "Spin" << " r1 : " << t0->getIndexResidueOne() << ", r2 : "
        << t0->getIndexResidueTwo() << ", amplitude : " << t0->getAmplitude() << std::endl;
  }
  else if (t.isInstanceOf<RigidBodyGeneralMover> ())
  {
    RigidBodyGeneralMoverPtr t0 = (RigidBodyGeneralMoverPtr)t;
    std::cout << "General" << " r1 : " << t0->getIndexResidueOne() << ", r2 : "
        << t0->getIndexResidueTwo() << ", magnitude : " << t0->getMagnitude() << ", amplitude : "
        << t0->getAmplitude() << std::endl;
  }
  else
  {
    std::cout << "Another mover......" << std::endl;
  }
}

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_ROSETTA_TEST_H_

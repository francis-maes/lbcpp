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
# include "Sampler.h"
# include "Sampler/ParzenContinuousSampler.h"
# include "Sampler/EnumerationDiscreteSampler.h"
# include "Sampler/SimpleResidueSampler.h"
# include "Sampler/GaussianMultivariateSampler.h"
# include "Sampler/ResiduePairSampler.h"
# include "Sampler/MultiVariateGaussianSampler.h"

# ifdef LBCPP_PROTEIN_ROSETTA
#  undef T
#  include <core/pose/Pose.hh>
#  include <core/chemical/util.hh>
#  include <core/io/pdb/pose_io.hh>
#  include <core/io/pdb/file_data.hh>
#  include <core/kinematics/FoldTree.hh>
#  define T JUCE_T
# endif // LBCPP_PROTEIN_ROSETTA

using namespace std;

namespace lbcpp
{
void outputEnergiesAndQScores(ExecutionContext& context, String referenceDirectory, String targetDirectory);

class RosettaTest : public WorkUnit
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

    SamplerPtr samp = new ProteinMoverSampler(5);
    File output = context.getFile(T("tempdataset/1M4F.xml"));
    ProteinPtr prottemp = Protein::createFromXml(context, output);
    core::pose::PoseOP pose;
    convertProteinToPose(context, prottemp, pose);
//    // phipsisampler
//    CompositeSamplerPtr ppsres = simpleResidueSampler(5);
//    ContinuousSamplerPtr ppsphi = gaussianSampler(0, 25);
//    ContinuousSamplerPtr ppspsi = gaussianSampler(0, 25);
//    CompositeSamplerPtr phipsi = objectCompositeSampler(phiPsiMoverClass, ppsres, ppsphi, ppspsi);
//    // shearsampler
//    CompositeSamplerPtr sres = simpleResidueSampler(5);
//    ContinuousSamplerPtr sphi = gaussianSampler(0, 25);
//    ContinuousSamplerPtr spsi = gaussianSampler(0, 25);
//    CompositeSamplerPtr shear = objectCompositeSampler(shearMoverClass, sres, sphi, spsi);
//    // rigidbody
//    CompositeSamplerPtr rbres = residuePairSampler(5);
//    ContinuousSamplerPtr rbmagn = gaussianSampler(0.5, 0.25);
//    ContinuousSamplerPtr rbamp = gaussianSampler(0, 25);
//    CompositeSamplerPtr rigidbody = objectCompositeSampler(rigidBodyMoverClass, rbres, rbmagn,
//        rbamp);
//    std::vector<SamplerPtr> samplers;
//
//    samplers.push_back(phipsi);

//    ClassPtr actionClass = denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);
//    DenseDoubleVectorPtr proba = new DenseDoubleVector(actionClass, 1, 1);
//    CompositeSamplerPtr samp = mixtureSampler(proba, samplers);
//    ProteinMoverSamplerPtr moverSampler = samp;

    std::vector<Variable> learning;

    // phipsi

    learning.push_back(phiPsiMover(1, 34, -123));
    learning.push_back(phiPsiMover(0, 30, -122));
    learning.push_back(phiPsiMover(2, 27, -121));
    learning.push_back(phiPsiMover(3, 33, -121));
    // shear
    learning.push_back(shearMover(3, 0.9, 4.5));
    learning.push_back(shearMover(4, 0.7, 4.3));
    learning.push_back(shearMover(3, 0.8, 3.4));
    // general
    learning.push_back(rigidBodyMover(3, 5, 2.8, -3.4));
    learning.push_back(rigidBodyMover(3, 5, 2.5, -2.4));
    learning.push_back(rigidBodyMover(1, 3, 0.8, 3.4));
    learning.push_back(rigidBodyMover(0, 4, 1.2, 2.4));
    learning.push_back(rigidBodyMover(2, 4, 0.3, 3.4));
    learning.push_back(rigidBodyMover(1, 3, 0.76, 4.2));
    learning.push_back(rigidBodyMover(1, 3, 0.76, 4.2));
    learning.push_back(rigidBodyMover(0, 3, 1.01, 4));
    // spin
    learning.push_back(rigidBodyMover(0, 3, 0.0, 11.3));
    learning.push_back(rigidBodyMover(1, 3, 0.0, 12.4));
    learning.push_back(rigidBodyMover(3, 5, 0.0, 9.3));
    learning.push_back(rigidBodyMover(2, 5, 0.0, 10.2));
    // trans
    learning.push_back(rigidBodyMover(4, 1, 10.2, 0.0));
    learning.push_back(rigidBodyMover(4, 1, 9.2, 0.0));
    learning.push_back(rigidBodyMover(4, 0, 12.1, 0.0));
    learning.push_back(rigidBodyMover(1, 3, -0.3, 0.0));
    learning.push_back(rigidBodyMover(0, 2, -2.1, 0.0));
    learning.push_back(rigidBodyMover(0, 3, -1.3, 0.0));

    samp->learn(context, learning);

    random = new RandomGenerator(0);

    File outfile = context.getFile(T("protSampler.xml"));
    samp->saveToFile(context, outfile);

    int count0 = 0;
    int count1 = 0;
    int count2 = 0;
    int num = 50;
    for (int i = 0; i < num; i++)
    {

      Variable v = samp->sample(context, random);

      ProteinMoverPtr t = v.getObjectAndCast<ProteinMover> ();
      cout << (const char* )t->toString() << endl;
      if (t.isInstanceOf<PhiPsiMover> ())
        count0++;
      else if (t.isInstanceOf<ShearMover> ())
        count1++;
      else if (t.isInstanceOf<RigidBodyMover> ())
        count2++;
    }

    cout << "il y a  " << endl;
    cout << "phipsi : " << (double)count0 / (double)num << endl;
    cout << "shear : " << (double)count1 / (double)num << endl;
    cout << "rigidbody : " << (double)count2 / (double)num << endl;

//    CompositeSamplerPtr rebirth = Variable::createFromFile(context, outfile).getObjectAndCast<CompositeSampler> ();
//    CompositeSamplerPtr rebirth2 = samp->cloneAndCast<CompositeSampler> ();
//    //rebirth->saveToFile(context, context.getFile(T("rebirth.xml")));
//    count0 = 0;
//    count1 = 0;
//    count2 = 0;
//    RandomGeneratorPtr random2 = new RandomGenerator(0);
//    RandomGeneratorPtr random3 = new RandomGenerator(0);
//    for (int i = 0; i < num; i++)
//    {
//
//      Variable v = rebirth->sample(context, random2);
//      Variable v2 = rebirth2->sample(context, random3);
//
//      ProteinMoverPtr t = v.getObjectAndCast<ProteinMover> ();
//      ProteinMoverPtr t2 = v2.getObjectAndCast<ProteinMover> ();
//      cout << (const char* )t->toString() << endl;
//      cout << (const char* )t2->toString() << endl;
//      if (t.isInstanceOf<PhiPsiMover> ())
//        count0++;
//      else if (t.isInstanceOf<ShearMover> ())
//        count1++;
//      else if (t.isInstanceOf<RigidBodyMover> ())
//        count2++;
//    }
//
//    cout << "il y a  " << endl;
//    cout << "phipsi : " << (double)count0 / (double)num << endl;
//    cout << "shear : " << (double)count1 / (double)num << endl;
//    cout << "rigidbody : " << (double)count2 / (double)num << endl;

    context.informationCallback(T("RosettaTest done."));
    return Variable();

  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_ROSETTA_TEST_H_

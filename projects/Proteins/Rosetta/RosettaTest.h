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
# include "ProteinMover/ShearMover.h"
# include "ProteinMover/RigidBodyMover.h"
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

    DoubleMatrixPtr initMeans = new DoubleMatrix(3, 1, 0.0);
    initMeans->setValue(0, 0, 2);
    initMeans->setValue(1, 0, 10);
    initMeans->setValue(2, 0, -5);

    DoubleMatrixPtr initStd = new DoubleMatrix(3, 3, 0.0);
    initStd->setValue(0, 0, 2);
    initStd->setValue(0, 1, 0.01);
    initStd->setValue(0, 2, 0.2);
    initStd->setValue(1, 0, 0.01);
    initStd->setValue(1, 1, 1);
    initStd->setValue(1, 2, 0.2);
    initStd->setValue(2, 0, 0.2);
    initStd->setValue(2, 1, 0.2);
    initStd->setValue(2, 2, 3);
    MultiVariateGaussianSamplerPtr mvgsamp = new MultiVariateGaussianSampler(initMeans, initStd);

    std::vector<Variable> learningmvg;
    DoubleMatrixPtr temp = new DoubleMatrix(3, 1, 0.0);
    temp->setValue(0, 0, 4);
    temp->setValue(1, 0, 2);
    temp->setValue(2, 0, 0.6);
    learningmvg.push_back(temp);
    temp = new DoubleMatrix(3, 1, 0.0);
    temp->setValue(0, 0, 4.2);
    temp->setValue(1, 0, 2.1);
    temp->setValue(2, 0, 0.59);
    learningmvg.push_back(temp);
    temp = new DoubleMatrix(3, 1, 0.0);
    temp->setValue(0, 0, 3.9);
    temp->setValue(1, 0, 2);
    temp->setValue(2, 0, 0.58);
    learningmvg.push_back(temp);
    temp = new DoubleMatrix(3, 1, 0.0);
    temp->setValue(0, 0, 4.3);
    temp->setValue(1, 0, 2.1);
    temp->setValue(2, 0, 0.62);
    learningmvg.push_back(temp);
    temp = new DoubleMatrix(3, 1, 0.0);
    temp->setValue(0, 0, 4.1);
    temp->setValue(1, 0, 2.2);
    temp->setValue(2, 0, 0.63);
    learningmvg.push_back(temp);
    mvgsamp->learn(context, learningmvg);

    for (int i = 0; i < 20; i++)
    {
      DoubleMatrixPtr result = mvgsamp->sample(context, random).getObjectAndCast<DoubleMatrix>();
      cout << (const char* )result->toString() << endl;
    }

    // phipsisampler
    SimpleResidueSamplerPtr ppsres = new SimpleResidueSampler(5);
    ContinuousSamplerPtr ppsphi = gaussianSampler(50, 2);
    ContinuousSamplerPtr ppspsi = gaussianSampler(-1000, 10);
    CompositeSamplerPtr phipsi = objectCompositeSampler(phiPsiMoverClass, ppsres, ppsphi, ppspsi);
    // shearsampler
    SimpleResidueSamplerPtr sres = new SimpleResidueSampler(5);
    ContinuousSamplerPtr sphi = gaussianSampler(2000, 2);
    ContinuousSamplerPtr spsi = gaussianSampler(-10, 1);
    CompositeSamplerPtr shear = objectCompositeSampler(shearMoverClass, sres, sphi, spsi);
    // rigidbody
    ResiduePairSamplerPtr rbres = new ResiduePairSampler(5);
    ContinuousSamplerPtr rbmagn = gaussianSampler(0.5, 0.01);
    ContinuousSamplerPtr rbamp = gaussianSampler(20, 1);
    CompositeSamplerPtr rigidbody = objectCompositeSampler(rigidBodyMoverClass, rbres,rbmagn, rbamp);
    std::vector<SamplerPtr> samplers;
    samplers.push_back(phipsi);
    samplers.push_back(shear);
    samplers.push_back(rigidbody);
    ClassPtr actionClass = denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);
    DenseDoubleVectorPtr proba = new DenseDoubleVector(actionClass, 3, 0.33);
    proba->setValue(0, 0.5);
    proba->setValue(1, 0.25);
    proba->setValue(2, 0.25);
    CompositeSamplerPtr samp = mixtureSampler(proba, samplers);

    std::vector<Variable> learning;

    // phipsi

    learning.push_back(new PhiPsiMover(1, 34, -123));
    learning.push_back(new PhiPsiMover(0, 30, -122));
    learning.push_back(new PhiPsiMover(2, 27, -121));
    learning.push_back(new PhiPsiMover(3, 33, -121));
    // shear
    learning.push_back(new ShearMover(3, 0.9, 4.5));
    learning.push_back(new ShearMover(4, 0.7, 4.3));
    learning.push_back(new ShearMover(3, 0.8, 3.4));
    // general
    learning.push_back(new RigidBodyMover(3, 5, 2.8, -3.4));
    learning.push_back(new RigidBodyMover(3, 5, 2.5, -2.4));
    learning.push_back(new RigidBodyMover(1, 3, 0.8, 3.4));
    learning.push_back(new RigidBodyMover(0, 4, 1.2, 2.4));
    learning.push_back(new RigidBodyMover(2, 4, 0.3, 3.4));
    learning.push_back(new RigidBodyMover(1, 3, 0.76, 4.2));
    learning.push_back(new RigidBodyMover(1, 3, 0.76, 4.2));
    learning.push_back(new RigidBodyMover(0, 3, 1.01, 4));
    // spin
    learning.push_back(new RigidBodyMover(0, 3, 0.0, 11.3));
    learning.push_back(new RigidBodyMover(1, 3, 0.0, 12.4));
    learning.push_back(new RigidBodyMover(3, 5, 0.0, 9.3));
    learning.push_back(new RigidBodyMover(2, 5, 0.0, 10.2));
    // trans
    learning.push_back(new RigidBodyMover(4, 1, 10.2, 0.0));
    learning.push_back(new RigidBodyMover(4, 1, 9.2, 0.0));
    learning.push_back(new RigidBodyMover(4, 0, 12.1, 0.0));
    learning.push_back(new RigidBodyMover(1, 3, -0.3, 0.0));
    learning.push_back(new RigidBodyMover(0, 2, -2.1, 0.0));
    learning.push_back(new RigidBodyMover(0, 3, -1.3, 0.0));

    //samp->learn(context, learning);

    random = new RandomGenerator(0);

    File outfile = context.getFile(T("protSampler.xml"));
    samp->saveToFile(context, outfile);

    int count0 = 0;
    int count1 = 0;
    int count2 = 0;
    int num = 10000;
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

    CompositeSamplerPtr rebirth = Variable::createFromFile(context, outfile).getObjectAndCast<CompositeSampler> ();
    CompositeSamplerPtr rebirth2 = samp->cloneAndCast<CompositeSampler> ();
    //rebirth->saveToFile(context, context.getFile(T("rebirth.xml")));
    count0 = 0;
    count1 = 0;
    count2 = 0;
    RandomGeneratorPtr random2 = new RandomGenerator(0);
    RandomGeneratorPtr random3 = new RandomGenerator(0);
    for (int i = 0; i < num; i++)
    {

      Variable v = rebirth->sample(context, random2);
      Variable v2 = rebirth2->sample(context, random3);

      ProteinMoverPtr t = v.getObjectAndCast<ProteinMover> ();
      ProteinMoverPtr t2 = v2.getObjectAndCast<ProteinMover> ();
      cout << (const char* )t->toString() << endl;
      cout << (const char* )t2->toString() << endl;
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

    context.informationCallback(T("RosettaTest done."));
    return Variable();

  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_ROSETTA_TEST_H_

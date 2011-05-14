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

//    RandomGeneratorPtr random2 = new RandomGenerator(0);
//    RandomGeneratorPtr random3 = new RandomGenerator(0);
//    ContinuousSamplerPtr gauss = gaussianSampler(2, 1);
//    ContinuousSamplerPtr temp = gauss->cloneAndCast<ContinuousSampler>();
//    DiscretizeSamplerPtr disc = new DiscretizeSampler(0, 5, gauss);
//
//    for (int i = 0; i < 5; i++)
//    {
//      std::cout << "val : " << i << " : " << gauss->sample(context, random).getDouble() << std::endl;
//      std::cout << "val : " << i << " : " << temp->sample(context, random2).getDouble() << "clone " << std::endl;
//      std::cout << "val : " << i << " : " << disc->sample(context, random3).getInteger() << std::endl;
//    }

    PhiPsiMoverSamplerPtr samp0 = new PhiPsiMoverSampler(5, -20, 5, 50, 2);
    ShearMoverSamplerPtr samp1 = new ShearMoverSampler(5, 0, 20, 30, 5);
    //    RigidBodyTransMoverSamplerPtr samp2 = new RigidBodyTransMoverSampler(5, 0.5, 0.05);
    //    RigidBodySpinMoverSamplerPtr samp3 = new RigidBodySpinMoverSampler(5, 0, 20);
    RigidBodyGeneralMoverSamplerPtr samp4 = new RigidBodyGeneralMoverSampler(5, 0.5, 0.05, 0, 20);

    std::vector<Variable> samplers;
    //samplers.push_back(Variable(samp2));
    samplers.push_back(Variable(samp0));
    samplers.push_back(Variable(samp1));
    samplers.push_back(Variable(samp4));
    //samplers.push_back(Variable(samp3));

    ProteinMoverSamplerPtr samp = new ProteinMoverSampler(3, samplers);

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

    samp->learn(context, learning);

    random = new RandomGenerator(0);

    File outfile = context.getFile(T("protSampler.xml"));
    samp->saveToFile(context, outfile);

    int count0 = 0;
    int count1 = 0;
    int count2 = 0;
    int count3 = 0;
    int count4 = 0;
    int num = 20;
    for (int i = 0; i < num; i++)
    {

      Variable v = samp->sample(context, random);

      ProteinMoverPtr t = v.getObjectAndCast<ProteinMover> ();

      if (t.isInstanceOf<PhiPsiMover> ())
      {
        PhiPsiMoverPtr t0 = (PhiPsiMoverPtr)t;
        cout << i << " = " << "phipsi" << " r : " << t0->getResidueIndex() << ", phi : "
            << t0->getDeltaPhi() << ", psi : " << t0->getDeltaPsi() << endl;
        count0++;
      }
      else if (t.isInstanceOf<ShearMover> ())
      {
        ShearMoverPtr t0 = (ShearMoverPtr)t;
        cout << i << " = " << "shear " << " r : " << t0->getResidueIndex() << ", phi : "
            << t0->getDeltaPhi() << ", psi : " << t0->getDeltaPsi() << endl;
        count1++;
      }
      else if (t.isInstanceOf<RigidBodyMover> ())
      {
        RigidBodyMoverPtr t0 = (RigidBodyMoverPtr)t;
        cout << i << " = " << "general" << " r1 : " << t0->getIndexResidueOne() << ", r2 : "
            << t0->getIndexResidueTwo() << ", magnitude : " << t0->getMagnitude()
            << ", amplitude : " << t0->getAmplitude() << endl;
        count4++;
      }
    }

    cout << "il y a  " << endl;
    cout << "phipsi : " << (double)count0 / (double)num << endl;
    cout << "shear : " << (double)count1 / (double)num << endl;
    cout << "rigidbodytrans : " << (double)count2 / (double)num << endl;
    cout << "rigidbodyspin : " << (double)count3 / (double)num << endl;
    cout << "rigidbodygeneral : " << (double)count4 / (double)num << endl;

    ProteinMoverSamplerPtr rebirth = Variable::createFromFile(context, outfile).getObjectAndCast<
        ProteinMoverSampler> ();
    ProteinMoverSamplerPtr rebirth2 = samp->cloneAndCast<ProteinMoverSampler> ();
    //rebirth->saveToFile(context, context.getFile(T("rebirth.xml")));
    count0 = 0;
    count1 = 0;
    count2 = 0;
    count3 = 0;
    count4 = 0;
    RandomGeneratorPtr random2 = new RandomGenerator(0);
    RandomGeneratorPtr random3 = new RandomGenerator(0);
    for (int i = 0; i < num; i++)
    {

      Variable v = rebirth->sample(context, random2);
      Variable v2 = rebirth2->sample(context, random3);

      ProteinMoverPtr t = v.getObjectAndCast<ProteinMover> ();
      ProteinMoverPtr t2 = v2.getObjectAndCast<ProteinMover> ();

      if (t.isInstanceOf<PhiPsiMover> ())
      {
        PhiPsiMoverPtr t0 = (PhiPsiMoverPtr)t;
        cout << i << " = " << "phipsi" << " r : " << t0->getResidueIndex() << ", phi : "
            << t0->getDeltaPhi() << ", psi : " << t0->getDeltaPsi() << endl;
        PhiPsiMoverPtr t0b = (PhiPsiMoverPtr)t2;
        cout << i << " = " << "phipsi" << " r : " << t0b->getResidueIndex() << ", phi : "
            << t0b->getDeltaPhi() << ", psi : " << t0b->getDeltaPsi() << endl;
        count0++;
      }
      else if (t.isInstanceOf<ShearMover> ())
      {
        ShearMoverPtr t0 = (ShearMoverPtr)t;
        cout << i << " = " << "shear " << " r : " << t0->getResidueIndex() << ", phi : "
            << t0->getDeltaPhi() << ", psi : " << t0->getDeltaPsi() << endl;
        ShearMoverPtr t0b = (ShearMoverPtr)t2;
        cout << i << " = " << "shear " << " r : " << t0b->getResidueIndex() << ", phi : "
            << t0b->getDeltaPhi() << ", psi : " << t0b->getDeltaPsi() << endl;
        count1++;
      }
      else if (t.isInstanceOf<RigidBodyMover> ())
      {
        RigidBodyMoverPtr t0 = (RigidBodyMoverPtr)t;
        cout << i << " = " << "general" << " r1 : " << t0->getIndexResidueOne() << ", r2 : "
            << t0->getIndexResidueTwo() << ", magnitude : " << t0->getMagnitude()
            << ", amplitude : " << t0->getAmplitude() << endl;
        RigidBodyMoverPtr t0b = (RigidBodyMoverPtr)t2;
        cout << i << " = " << "general" << " r1 : " << t0b->getIndexResidueOne() << ", r2 : "
            << t0b->getIndexResidueTwo() << ", magnitude : " << t0b->getMagnitude()
            << ", amplitude : " << t0b->getAmplitude() << endl;
        count4++;
      }
    }

    cout << "il y a  " << endl;
    cout << "phipsi : " << (double)count0 / (double)num << endl;
    cout << "shear : " << (double)count1 / (double)num << endl;
    cout << "rigidbodytrans : " << (double)count2 / (double)num << endl;
    cout << "rigidbodyspin : " << (double)count3 / (double)num << endl;
    cout << "rigidbodygeneral : " << (double)count4 / (double)num << endl;

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
  else if (t.isInstanceOf<RigidBodyMover> ())
  {
    RigidBodyMoverPtr t0 = (RigidBodyMoverPtr)t;
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

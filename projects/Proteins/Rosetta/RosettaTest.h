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
# include "Sampler/GaussianMultivariateSampler.h"
# include "Sampler/DualResidueSampler.h"

# undef T
#  include <core/pose/Pose.hh>
#  include <core/chemical/util.hh>
#  include <core/io/pdb/pose_io.hh>
#  include <core/io/pdb/file_data.hh>
#  include <core/kinematics/FoldTree.hh>
//#  include <protocols/init.hh>
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

    PhiPsiMoverSamplerPtr samp0 = new PhiPsiMoverSampler(5, -20, 5, 50, 2);
    ShearMoverSamplerPtr samp1 = new ShearMoverSampler(5, 0, 20, 30, 5);
    RigidBodyTransMoverSamplerPtr samp2 = new RigidBodyTransMoverSampler(5, 0.5, 0.05);
    RigidBodySpinMoverSamplerPtr samp3 = new RigidBodySpinMoverSampler(5, 0, 20);
    RigidBodyGeneralMoverSamplerPtr samp4 = new RigidBodyGeneralMoverSampler(5, 0.5, 0.05, 0, 20);

    std::vector<Variable> samplers;
    samplers.push_back(Variable(samp2));
    samplers.push_back(Variable(samp0));
    samplers.push_back(Variable(samp1));
    samplers.push_back(Variable(samp4));
    samplers.push_back(Variable(samp3));


    ProteinMoverSamplerPtr samp = new ProteinMoverSampler(5, samplers);

    std::vector<std::pair<Variable, Variable> > learning;

    // phipsi
    learning.push_back(std::pair<Variable, Variable>(Variable(new PhiPsiMover(1, 34, -123)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new PhiPsiMover(0, 30, -122)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new PhiPsiMover(2, 27, -121)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new PhiPsiMover(3, 33, -121)),
        Variable()));
    // shear
    learning.push_back(std::pair<Variable, Variable>(Variable(new ShearMover(3, 0.9, 4.5)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new ShearMover(4, 0.7, 4.3)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new ShearMover(3, 0.8, 3.4)),
        Variable()));
    // general
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodyGeneralMover(3, 5, 2.8,
        -3.4)), Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodyGeneralMover(3, 5, 2.5,
        -2.4)), Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodyGeneralMover(1, 3, 0.8,
        3.4)), Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodyGeneralMover(0, 4, 1.2,
        2.4)), Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodyGeneralMover(2, 4, 0.3,
        3.4)), Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodyGeneralMover(1, 3, 0.76,
        4.2)), Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodyGeneralMover(1, 3, 0.76,
        4.2)), Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodyGeneralMover(0, 3, 1.01,
        4)), Variable()));
    // spin
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodySpinMover(0, 3, 11.3)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodySpinMover(1, 3, 12.4)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodySpinMover(3, 5, 9.3)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodySpinMover(2, 5, 10.2)),
        Variable()));
    // trans
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodyTransMover(4, 1, 10.2)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodyTransMover(4, 1, 9.2)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodyTransMover(4, 0, 12.1)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodyTransMover(1, 3, -0.3)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodyTransMover(0, 2, -2.1)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodyTransMover(0, 3, -1.3)),
        Variable()));

    samp->learn(context, random, learning);
    random = new RandomGenerator(0);
    RandomGeneratorPtr random2 = new RandomGenerator(0);

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
      else if (t.isInstanceOf<RigidBodyTransMover> ())
      {
        RigidBodyTransMoverPtr t0 = (RigidBodyTransMoverPtr)t;
        cout << i << " = " << "trans" << " r1 : " << t0->getIndexResidueOne() << ", r2 : "
            << t0->getIndexResidueTwo() << ", magnitude : " << t0->getMagnitude() << endl;
        count2++;
      }
      else if (t.isInstanceOf<RigidBodySpinMover> ())
      {
        RigidBodySpinMoverPtr t0 = (RigidBodySpinMoverPtr)t;
        cout << i << " = " << "spin" << " r1 : " << t0->getIndexResidueOne() << ", r2 : "
            << t0->getIndexResidueTwo() << ", amplitude : " << t0->getAmplitude() << endl;
        count3++;
      }
      else if (t.isInstanceOf<RigidBodyGeneralMover> ())
      {
        RigidBodyGeneralMoverPtr t0 = (RigidBodyGeneralMoverPtr)t;
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
    ProteinMoverSamplerPtr rebirth2 = samp->cloneAndCast<ProteinMoverSampler>();
    rebirth->saveToFile(context, context.getFile(T("rebirth.xml")));
    count0 = 0;
    count1 = 0;
    count2 = 0;
    count3 = 0;
    count4 = 0;
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
      else if (t.isInstanceOf<RigidBodyTransMover> ())
      {
        RigidBodyTransMoverPtr t0 = (RigidBodyTransMoverPtr)t;
        cout << i << " = " << "trans" << " r1 : " << t0->getIndexResidueOne() << ", r2 : "
            << t0->getIndexResidueTwo() << ", magnitude : " << t0->getMagnitude() << endl;
        RigidBodyTransMoverPtr t0b = (RigidBodyTransMoverPtr)t2;
        cout << i << " = " << "trans" << " r1 : " << t0b->getIndexResidueOne() << ", r2 : "
            << t0b->getIndexResidueTwo() << ", magnitude : " << t0b->getMagnitude() << endl;
        count2++;
      }
      else if (t.isInstanceOf<RigidBodySpinMover> ())
      {
        RigidBodySpinMoverPtr t0 = (RigidBodySpinMoverPtr)t;
        cout << i << " = " << "spin" << " r1 : " << t0->getIndexResidueOne() << ", r2 : "
            << t0->getIndexResidueTwo() << ", amplitude : " << t0->getAmplitude() << endl;
        RigidBodySpinMoverPtr t0b = (RigidBodySpinMoverPtr)t2;
        cout << i << " = " << "spin" << " r1 : " << t0b->getIndexResidueOne() << ", r2 : "
            << t0b->getIndexResidueTwo() << ", amplitude : " << t0b->getAmplitude() << endl;
        count3++;
      }
      else if (t.isInstanceOf<RigidBodyGeneralMover> ())
      {
        RigidBodyGeneralMoverPtr t0 = (RigidBodyGeneralMoverPtr)t;
        cout << i << " = " << "general" << " r1 : " << t0->getIndexResidueOne() << ", r2 : "
            << t0->getIndexResidueTwo() << ", magnitude : " << t0->getMagnitude()
            << ", amplitude : " << t0->getAmplitude() << endl;
        RigidBodyGeneralMoverPtr t0b = (RigidBodyGeneralMoverPtr)t2;
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

    //==============TEST serialisation Mover ==============================
    //    RigidBodySpinMoverPtr p0 = new RigidBodySpinMover(1, 4, 12.8);
    //
    //    File outfile = context.getFile(T("moverPP.xml"));
    //    p0->saveToFile(context, outfile);
    //
    //    p0->setIndexResidueOne(5);
    //    p0->setIndexResidueTwo(10);
    //    RigidBodySpinMoverPtr t0 = Variable::createFromFile(context, outfile).getObjectAndCast<
    //        RigidBodySpinMover> ();
    //    t0->setAmplitude(0.9);
    //    //t0->setMagnitude(1.1);
    //
    //    cout << "phipsi true " << " r1 : " << p0->getIndexResidueOne() << ", r2 : "
    //        << p0->getIndexResidueTwo() /*<< ", mag : " << p0->getMagnitude()*/<< ", amp : "<< p0->getAmplitude() <<endl;
    //
    //    cout << "phipsi true " << " r1 : " << t0->getIndexResidueOne() << ", r2 : "
    //        << t0->getIndexResidueTwo() /*<< ", mag : " << t0->getMagnitude() */<< ", amp : "<< t0->getAmplitude() << endl;
    // =========================================================================

    // ================TEST serialisation Sampler PhiPsi et Shear ============================
    //    cout << "test 1 " << endl;
    //    ShearMoverSamplerPtr p0 = new ShearMoverSampler(10, 20, 0.2, -10.5, 1);
    //    cout << "test 2 " << endl;
    //    File outfile = context.getFile(T("moverPP.xml"));
    //    cout << "test 3 " << endl;
    //    samp1->saveToFile(context, outfile);
    //    cout << "test 4 " << endl;
    //
    //    ShearMoverSamplerPtr t0 = Variable::createFromFile(context, outfile).getObjectAndCast<
    //        ShearMoverSampler> ();
    //    cout << "test 5 " << endl;
    //
    //    for (int i = 0; i < 20; i++)
    //    {
    //      ShearMoverPtr p0 = samp1->sample(context, random).getObjectAndCast<ShearMover> ();
    //      ShearMoverPtr p1 = t0->sample(context, random2).getObjectAndCast<ShearMover> ();
    //
    //      cout << "(" << p0->getResidueIndex() << ", " << p0->getDeltaPhi() << ", "
    //          << p0->getDeltaPsi() << ") <-----> ";
    //      cout << "(" << p1->getResidueIndex() << ", " << p1->getDeltaPhi() << ", "
    //          << p1->getDeltaPsi() << ")" << endl;
    //    }
    //    cout << "Done." << endl;
    //===================================================================================================

    // ================TEST serialisation RigidBody ============================
    //    File outfile = context.getFile(T("moverPP.xml"));
    //    samp4->saveToFile(context, outfile);
    //
    //    RigidBodyGeneralMoverSamplerPtr
    //        t0 = Variable::createFromFile(context, outfile).getObjectAndCast<
    //            RigidBodyGeneralMoverSampler> ();
    //
    //    for (int i = 0; i < 20; i++)
    //    {
    //      RigidBodyGeneralMoverPtr p0 = samp4->sample(context, random).getObjectAndCast<
    //          RigidBodyGeneralMover> ();
    //      RigidBodyGeneralMoverPtr p1 = t0->sample(context, random2).getObjectAndCast<
    //          RigidBodyGeneralMover> ();
    //
    //      cout << "(" << p0->getIndexResidueOne() << ", " << p0->getIndexResidueTwo() << ", "
    //          << p0->getAmplitude() << ", " << p0->getMagnitude() << ") <-----> ";
    //      cout << "(" << p1->getIndexResidueOne() << ", " << p1->getIndexResidueTwo() << ", "
    //          << p1->getAmplitude() << ", " << p1->getMagnitude() << ")" << endl;
    //    }
    //===================================================================================================

    context.informationCallback(T("RosettaTest done."));
    return Variable();

  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_ROSETTA_TEST_H_

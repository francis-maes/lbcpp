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
  RigidBodySpinMoverPtr mover;
  std::vector<double> parameters;

public:
  virtual Variable run(ExecutionContext& context)
  {
    RandomGeneratorPtr random = new RandomGenerator(0);

    PhiPsiMoverSamplerPtr samp0 = new PhiPsiMoverSampler(5, -20, 5, 50, 2);
    ShearMoverSamplerPtr samp1 = new ShearMoverSampler(5, 0, 20, 30, 5);
    RigidBodyTransMoverSamplerPtr samp2 = new RigidBodyTransMoverSampler(5, 0.5, 0.05);
    RigidBodySpinMoverSamplerPtr samp3 = new RigidBodySpinMoverSampler(5, 0, 20);
    RigidBodyGeneralMoverSamplerPtr samp4 = new RigidBodyGeneralMoverSampler(5, 0.5, 0.05, 0, 20);

    std::vector<Variable> samplers;
    samplers.push_back(Variable(samp0));
    samplers.push_back(Variable(samp1));
    samplers.push_back(Variable(samp2));
    samplers.push_back(Variable(samp3));
    samplers.push_back(Variable(samp4));

    ProteinMoverSampler samp(5, samplers);

    std::vector<std::pair<Variable, Variable> > learning;

    learning.push_back(std::pair<Variable, Variable>(Variable(new PhiPsiMover(1, 34, -123)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new PhiPsiMover(0, 30, -122)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new PhiPsiMover(2, 27, -121)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new PhiPsiMover(3, 33, -121)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new ShearMover(3, 0.9, 4.5)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new ShearMover(4, 0.7, 4.3)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new ShearMover(3, 0.8, 3.4)),
        Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodyGeneralMover(1, 3, 0.8,
        3.4)), Variable()));
    learning.push_back(std::pair<Variable, Variable>(
        Variable(new RigidBodyGeneralMover(0, 3, 1, 4)), Variable()));
    learning.push_back(std::pair<Variable, Variable>(Variable(new RigidBodySpinMover(1, 4, 10)),
        Variable()));

    samp.learn(context, random, learning);

    int count0 = 0;
    int count1 = 0;
    int count2 = 0;
    int count3 = 0;
    int count4 = 0;
    int num = 10000;
    for (int i = 0; i < num; i++)
    {

      Variable v = samp.sample(context, random);

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

    //    RigidBodyGeneralMoverPtr m0 = new RigidBodyGeneralMover(0, 4, 4.5, -5.3);
    //    RigidBodyGeneralMoverPtr m1 = new RigidBodyGeneralMover(0, 3, -5.5, -4.7);
    //    RigidBodyGeneralMoverPtr m2 = new RigidBodyGeneralMover(0, 4, 6, -6);
    //    RigidBodyGeneralMoverPtr m3 = new RigidBodyGeneralMover(0, 4, 3, -4);
    //    RigidBodyGeneralMoverPtr m4 = new RigidBodyGeneralMover(0, 4, 5.2, -3.6);
    //    RigidBodyGeneralMoverPtr m5 = new RigidBodyGeneralMover(0, 4, 5.1, -4.9);
    //    RigidBodyGeneralMoverPtr m6 = new RigidBodyGeneralMover(0, 3, 4.9, -5.5);
    //    RigidBodyGeneralMoverPtr m7 = new RigidBodyGeneralMover(0, 4, 3.9, -5.6);
    //    RigidBodyGeneralMoverPtr m8 = new RigidBodyGeneralMover(1, 4, 4.56, -6.4);
    //    RigidBodyGeneralMoverPtr m9 = new RigidBodyGeneralMover(1, 4, 16.34, -5);
    //
    //    std::vector<std::pair<Variable, Variable> > dataset;
    //    dataset.push_back(std::pair<Variable, Variable>(m0, Variable()));
    //    dataset.push_back(std::pair<Variable, Variable>(m1, Variable()));
    //    dataset.push_back(std::pair<Variable, Variable>(m2, Variable()));
    //    dataset.push_back(std::pair<Variable, Variable>(m3, Variable()));
    //    dataset.push_back(std::pair<Variable, Variable>(m4, Variable()));
    //    dataset.push_back(std::pair<Variable, Variable>(m5, Variable()));
    //    dataset.push_back(std::pair<Variable, Variable>(m6, Variable()));
    //    dataset.push_back(std::pair<Variable, Variable>(m7, Variable()));
    //    dataset.push_back(std::pair<Variable, Variable>(m8, Variable()));
    //    dataset.push_back(std::pair<Variable, Variable>(m9, Variable()));
    //
    //    samp.learn(context, random, dataset);
    //    cout << "test 2 " << endl;
    //    for (int i = 0; i < 20; i++)
    //    {
    //      Variable v = samp.sample(context, random);
    //      RigidBodyGeneralMoverPtr t = v.getObjectAndCast<RigidBodyGeneralMover> ();
    //      cout << i << " = " << " r1 : " << t->getIndexResidueOne() << ", r2 : "
    //          << t->getIndexResidueTwo() << ", magnitude : " << t->getMagnitude() << ", amplitude : "
    //          << t->getAmplitude() << endl;
    //    }

    context.informationCallback(T("RosettaTest done."));
    return Variable();

  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_ROSETTA_TEST_H_

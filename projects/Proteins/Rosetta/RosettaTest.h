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
# include "Sampler/SimpleResidueSampler.h"
# include "Sampler/ResiduePairSampler.h"

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

# ifdef LBCPP_PROTEIN_ROSETTA
# endif // LBCPP_PROTEIN_ROSETTA
    // --------------- univarate mixture learning
    //    SamplerPtr gauss1 = gaussianSampler(-2, 5);
    //    SamplerPtr gauss2 = gaussianSampler(9, 5);
    //    std::vector<SamplerPtr> mixtsamp;
    //    mixtsamp.push_back(gauss1);
    //    mixtsamp.push_back(gauss2);
    //    DenseDoubleVectorPtr probas = new DenseDoubleVector(2, 0);
    //    probas->setValue(0, 0.3);
    //    probas->setValue(1, 0.7);
    //    CompositeSamplerPtr mix = mixtureSampler(probas, mixtsamp);
    //    cout << "=========== test sample =======================" << endl;
    //    ofstream ofs("/Users/alex/Documents/MATLAB/unigauss.data");
    //
    //    for (int i = 0; i < 10000; i++)
    //      ofs << mix->sample(context, random).getDouble() << endl;
    //
    //    ofs.flush();
    //    ofs.close();
    //
    //    VariableVectorPtr learning = new VariableVector(0);
    //
    //    std::vector<double> mm(8);
    //    mm[0] = -10.2;
    //    mm[1] = -12.3;
    //    mm[2] = -9.2;
    //    mm[3] = -15.2;
    //    mm[4] = 98.5;
    //    mm[5] = 99.0;
    //    mm[6] = 100.5;
    //    mm[7] = 100.0;
    //
    //    for (size_t i = 0; i < mm.size(); i++)
    //      learning->append(Variable(mm[i]));
    //
    //    cout << "=========== learning =======================" << endl;
    //    mix->learn(context, ContainerPtr(), learning, ContainerPtr(), ContainerPtr());
    //
    //    cout << "=========== sampling again =======================" << endl;
    //    ofstream ofs2("/Users/alex/Documents/MATLAB/multiLearned.data");
    //    for (double i = 0; i < 10000; i++)
    //      ofs2 << mix->sample(context, random) << endl;
    //
    //    ofs2.flush();
    //    ofs2.close();
    //
    //    mix->saveToFile(context, context.getFile(T("testunimulti.xml")));
    //
    //    random = new RandomGenerator(0);
    //    RandomGeneratorPtr random2 = new RandomGenerator(0);
    //    SamplerPtr
    //        mix2 =
    //            Variable::createFromFile(context, context.getFile(T("testunimulti.xml"))).getObjectAndCast<
    //                CompositeSampler> ();
    //    for (size_t i = 0; i < 10; i++)
    //    {
    //      cout << mix->sample(context, random).getDouble() << endl;
    //      cout << mix2->sample(context, random2).getDouble() << endl;
    //    }

    // --------------- multivarate mixture learning
            DoubleMatrixPtr means0 = new DoubleMatrix(2, 1, 0.0);
            DoubleMatrixPtr means1 = new DoubleMatrix(2, 1, 0.0);
            means0->setValue(0, 0, -4.9);
            means0->setValue(1, 0, -4.9);
            means1->setValue(0, 0, 25.0);
            means1->setValue(1, 0, 55.0);

            DoubleMatrixPtr covarianceMatrix0 = new DoubleMatrix(2, 2, 0.0);
            covarianceMatrix0->setValue(0, 0, 2.0);
            covarianceMatrix0->setValue(0, 1, 0.01);
            covarianceMatrix0->setValue(1, 0, 0.01);
            covarianceMatrix0->setValue(1, 1, 2.0);
            DoubleMatrixPtr covarianceMatrix1 = new DoubleMatrix(2, 2, 0.0);
            covarianceMatrix1->setValue(0, 0, 2.0);
            covarianceMatrix1->setValue(0, 1, 0.01);
            covarianceMatrix1->setValue(1, 0, 0.01);
            covarianceMatrix1->setValue(1, 1, 2.0);

            ScalarContinuousSamplerPtr gauss0 = multiVariateGaussianSampler(means0, covarianceMatrix0);
            ScalarContinuousSamplerPtr gauss1 = multiVariateGaussianSampler(means1, covarianceMatrix1);
            DenseDoubleVectorPtr probas = new DenseDoubleVector(2, 0);
            probas->setValue(0, 0.3);
            probas->setValue(1, 0.7);
            std::vector<SamplerPtr> mixtsamp;
            mixtsamp.push_back(gauss0);
            mixtsamp.push_back(gauss1);
            CompositeSamplerPtr mix = mixtureSampler(probas, mixtsamp);

        cout << "=========== test sample =======================" << endl;
        ofstream ofs("/Users/alex/Documents/MATLAB/multi.data");

//            for (int i = 0; i < 10000; i++)
//            {
//              DoubleMatrixPtr out = mix->sample(context, random).getObjectAndCast<DoubleMatrix> ();
//              ofs << out->getValue(0, 0) << ", " << out->getValue(1, 0) << endl;
//            }

        for (double i = -100; i <= 100; i++)
        {
          for (double j = -100; j <= 100; j++)
          {
            DoubleMatrixPtr probs = new DoubleMatrix(1, 1);
            DoubleMatrixPtr point = new DoubleMatrix(2, 1, i);
            point->setValue(1, 0, j);
            ObjectVectorPtr conteneur = new ObjectVector(doubleMatrixClass(doubleType), 0);
            conteneur->append(point);
            ScalarContinuousSamplerPtr tempsampler = gauss0;
            tempsampler->computeProbabilities(conteneur, probs, 0);
            ofs << probs->getValue(0, 0) << " ";
          }
          ofs << endl;
        }

        ofs.flush();
        ofs.close();

        ObjectVectorPtr learning = new ObjectVector(doubleMatrixClass(doubleType), 0);

        std::vector<DoubleMatrixPtr> mm(8);
        mm[0] = new DoubleMatrix(2, 1, 0.0);
        mm[0]->setElement(0, 0, Variable(-13.54634));
        mm[0]->setElement(1, 0, Variable(-13.5464));
        mm[1] = new DoubleMatrix(2, 1, 0.0);
        mm[1]->setElement(0, 0, Variable(-16.2462));
        mm[1]->setElement(1, 0, Variable(-18.14646));
        mm[2] = new DoubleMatrix(2, 1, 0.0);
        mm[2]->setElement(0, 0, Variable(-12.5644));
        mm[2]->setElement(1, 0, Variable(-19.5356));
        mm[3] = new DoubleMatrix(2, 1, 0.0);
        mm[3]->setElement(0, 0, Variable(-15.59));
        mm[3]->setElement(1, 0, Variable(-17.57));
        mm[4] = new DoubleMatrix(2, 1, 0.0);
        mm[4]->setElement(0, 0, Variable(20.57));
        mm[4]->setElement(1, 0, Variable(96.52));
        mm[5] = new DoubleMatrix(2, 1, 0.0);
        mm[5]->setElement(0, 0, Variable(19.95));
        mm[5]->setElement(1, 0, Variable(90.52));
        mm[6] = new DoubleMatrix(2, 1, 0.0);
        mm[6]->setElement(0, 0, Variable(25.52));
        mm[6]->setElement(1, 0, Variable(89.3));
        mm[7] = new DoubleMatrix(2, 1, 0.0);
        mm[7]->setElement(0, 0, Variable(17.9));
        mm[7]->setElement(1, 0, Variable(88.2));

        for (size_t i = 0; i < mm.size(); i++)
          learning->append(mm[i]);

        cout << "=========== learning =======================" << endl;
        mix->learn(context, ContainerPtr(), learning, DenseDoubleVectorPtr(), ContainerPtr(), ContainerPtr(), DenseDoubleVectorPtr());

        cout << "=========== sampling again =======================" << endl;
        ofstream ofs2("/Users/alex/Documents/MATLAB/multiLearned.data");
        for (double i = 0; i < 10000; i++)
        {
          DoubleMatrixPtr out = mix->sample(context, random).getObjectAndCast<Matrix> ();
          ofs2 << out->getValue(0, 0) << ", " << out->getValue(1, 0) << endl;
        }

        ofs2.flush();
        ofs2.close();

        mix->saveToFile(context, context.getFile(T("testgaussmulti.xml")));

        random = new RandomGenerator(0);
        RandomGeneratorPtr random2 = new RandomGenerator(0);
        SamplerPtr mix2 = Variable::createFromFile(context, context.getFile(
            T("testgaussmulti.xml"))).getObjectAndCast<CompositeSampler> ();
        for (size_t i = 0; i < 5; i++)
        {
          DoubleMatrixPtr out = mix->sample(context, random).getObjectAndCast<Matrix> ();
          cout << out->getValue(0, 0) << ", " << out->getValue(1, 0) << endl;
          DoubleMatrixPtr out2 = mix2->sample(context, random2).getObjectAndCast<Matrix> ();
          cout << out2->getValue(0, 0) << ", " << out2->getValue(1, 0) << endl;
        }

    // --------------- gaussian log value function
    //    ScalarVectorFunctionPtr func = gaussianLogValueFunction();
    //    DenseDoubleVectorPtr params = new DenseDoubleVector(2, 0);
    //    params->setValue(0, 3.0);
    //    params->setValue(1, 1.5);
    //    Variable val(0.13);
    //    double result = 0;
    //    DenseDoubleVectorPtr grad = new DenseDoubleVector(2, 0);
    //    double weight = 1;
    //    func->computeScalarVectorFunction(params, &val, &result, &grad, weight);
    //    cout << result << endl;
    //    cout << (const char* )grad->toString() << endl;

    // --------------- Samplers
//    SamplerPtr samp = new ProteinMoverSampler(5);
//    ObjectVectorPtr learning = new ObjectVector(proteinMoverClass, 0);
//
//    // phipsi
//    learning->append(phiPsiMover(1, 34, -123));
//    learning->append(phiPsiMover(0, 30, -122));
//    learning->append(phiPsiMover(2, 27, -121));
//    learning->append(phiPsiMover(3, 33, -121));
//    learning->append(phiPsiMover(1, 34, -123));
//    learning->append(phiPsiMover(0, 30, -122));
//    learning->append(phiPsiMover(2, 27, -121));
//    // shear
//    learning->append(shearMover(3, 0.9, 4.5));
//    learning->append(shearMover(4, 0.7, 4.3));
//    learning->append(shearMover(3, 0.8, 3.4));
//    // general
//    learning->append(rigidBodyMover(3, 5, 2.8, -3.4));
//    learning->append(rigidBodyMover(2, 5, 2.5, -2.4));
//    learning->append(rigidBodyMover(1, 3, 0.8, 3.4));
//    learning->append(rigidBodyMover(3, 4, 1.2, 2.4));
//    learning->append(rigidBodyMover(3, 5, 0.3, 3.4));
//    // spin
//    learning->append(rigidBodyMover(3, 5, 0.0, 11.3));
//    learning->append(rigidBodyMover(4, 0, 0.0, 12.4));
//    // trans
//    learning->append(rigidBodyMover(4, 1, 10.2, 0.0));
//    learning->append(rigidBodyMover(4, 2, 9.2, 0.0));
//    learning->append(rigidBodyMover(4, 0, 12.1, 0.0));
//
//    samp->learn(context, ContainerPtr(), learning);
//
//    random = new RandomGenerator(0);
//
//    int count0 = 0;
//    int count1 = 0;
//    int count2 = 0;
//    int num = 10;
//    for (int i = 0; i < num; i++)
//    {
//      Variable v = samp->sample(context, random);
//
//      ProteinMoverPtr t = v.getObjectAndCast<ProteinMover> ();
//      cout << i << " : " << (const char*)t->toString() << endl;
//
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
//
//    ObjectVectorPtr learning2 = new ObjectVector(proteinMoverClass, 0);
//    learning2->append(phiPsiMover(1, 34, -123));
//    learning2->append(phiPsiMover(0, 30, -122));
//    learning2->append(phiPsiMover(2, 27, -121));
//    learning2->append(phiPsiMover(3, 33, -121));
//    learning2->append(phiPsiMover(1, 34, -123));
//    learning2->append(phiPsiMover(0, 30, -122));
//    learning2->append(phiPsiMover(2, 27, -121));
//
//    RandomGeneratorPtr random2 = new RandomGenerator(0);
//    //        SamplerPtr phipsisampler = objectCompositeSampler(phiPsiMoverClass, new SimpleResidueSampler(5), gaussianSampler(0, 25), gaussianSampler(0, 25));
//    //        phipsisampler->learn(context, ContainerPtr(), learning2);
//    //        for (int i = 0; i < num; i++)
//    //        {
//    //          Variable v = phipsisampler->sample(context, random2);
//    //          ProteinMoverPtr t = v.getObjectAndCast<ProteinMover>();
//    //          cout << i << " : " << (const char* )t->toString() << endl;
//    //        }
//
//    File outfile = context.getFile(T("protSampler.xml"));
//    samp->saveToFile(context, outfile);
//
//    CompositeSamplerPtr rebirth = Variable::createFromFile(context, outfile).getObjectAndCast<
//        CompositeSampler> ();
//    CompositeSamplerPtr rebirth2 = samp->cloneAndCast<CompositeSampler> ();
//    count0 = 0;
//    count1 = 0;
//    count2 = 0;
//    random2 = new RandomGenerator(0);
//    RandomGeneratorPtr random3 = new RandomGenerator(0);
//    for (int i = 0; i < num; i++)
//    {
//
//      Variable v = rebirth->sample(context, random2);
//      Variable v2 = rebirth2->sample(context, random3);
//
//      ProteinMoverPtr t = v.getObjectAndCast<ProteinMover> ();
//      ProteinMoverPtr t2 = v2.getObjectAndCast<ProteinMover> ();
//      cout << (const char*)t->toString() << endl;
//      cout << (const char*)t2->toString() << endl;
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

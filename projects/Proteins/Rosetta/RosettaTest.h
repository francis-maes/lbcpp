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
# include "Sampler/GeneralProteinMoverSampler.h"
# include "Sampler/ConditionalProteinMoverSampler.h"
# include "RosettaSandBox.h"

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
extern EnumerationPtr featuresMoverEnumerationEnumeration;

void generateMoversDataSet(VectorPtr& inputs, VectorPtr& samples);

class RosettaTest : public WorkUnit
{
private:
  friend class RosettaTestClass;
  size_t arg;
  double value;
  String proteinsDir;

public:
  virtual Variable run(ExecutionContext& context)
  {
    rosettaInitialization(context, false);
    RandomGeneratorPtr random = new RandomGenerator(0);

# ifdef LBCPP_PROTEIN_ROSETTA
# endif // LBCPP_PROTEIN_ROSETTA

    // -------------- max ent
    VectorPtr inputs;
    VectorPtr samples;
    generateMoversDataSet(inputs, samples);

    ConditionalProteinMoverSamplerPtr MEsampler = new ConditionalProteinMoverSampler(0);

    MEsampler->learn(context, inputs, samples, DenseDoubleVectorPtr(), ContainerPtr(),
        ContainerPtr(), DenseDoubleVectorPtr());

    ClassPtr inputClass = denseDoubleVectorClass(featuresMoverEnumerationEnumeration, doubleType);
    random = new RandomGenerator();

    context.enterScope(T("Samples 1"));
    DenseDoubleVectorPtr input = new DenseDoubleVector(inputClass, 3, 0.0);
    input->setValue(0, 20.0); // length of protein
    input->setValue(1, 1.0); // first distribution
    Variable inputVariable = input;
    for (size_t i = 0; i < 100; ++i)
    {
      Variable sample = MEsampler->sample(context, random, &inputVariable);
      context.resultCallback(T("sample ") + String((int)i + 1), sample);
    }
    context.leaveScope();

    context.enterScope(T("Samples 2"));
    input = new DenseDoubleVector(inputClass, 3, 0.0);
    input->setValue(0, 10.0); // length of protein
    input->setValue(2, 1.0); // second distribution
    inputVariable = input;
    for (size_t i = 0; i < 100; ++i)
    {
      Variable sample = MEsampler->sample(context, random, &inputVariable);
      context.resultCallback(T("sample ") + String((int)i + 1), sample);
    }
    context.leaveScope();

# if 0
    // --------------- Conditional sampler
    ConditionalProteinMoverSamplerPtr gen = new ConditionalProteinMoverSampler(0);
    ConditionalSimpleResidueSamplerPtr simple = new ConditionalSimpleResidueSampler();
    ConditionalResiduePairSamplerPtr pair = new ConditionalResiduePairSampler();

    VariableVectorPtr samples = new VariableVector(0);
    VectorPtr inputs = vector(denseDoubleVectorClass(falseOrTrueEnumeration, doubleType), 0);
    DenseDoubleVectorPtr features1 = new DenseDoubleVector(2, 1.0);
    features1->setValue(0, 0.21);
    DenseDoubleVectorPtr features2 = new DenseDoubleVector(2, 1.0);
    features2->setValue(0, 0.22);
    DenseDoubleVectorPtr features3 = new DenseDoubleVector(2, 1.0);
    features3->setValue(0, 0.23);
    DenseDoubleVectorPtr features4 = new DenseDoubleVector(2, 1.0);
    features4->setValue(0, 0.51);
    DenseDoubleVectorPtr features5 = new DenseDoubleVector(2, 1.0);
    features5->setValue(0, 0.49);
    DenseDoubleVectorPtr features6 = new DenseDoubleVector(2, 1.0);
    features6->setValue(0, 0.54);
    DenseDoubleVectorPtr features7 = new DenseDoubleVector(2, 1.0);
    features7->setValue(0, 0.52);
    Variable input1(features1);
    Variable input2(features2);
    Variable input3(features3);
    Variable input4(features4);
    Variable input5(features5);
    Variable input6(features6);
    Variable input7(features7);
    inputs->append(features1);
    inputs->append(features2);
    inputs->append(features3);
    inputs->append(features4);
    inputs->append(features5);
    inputs->append(features6);
    inputs->append(features7);
    //    inputs->append(input1);
    //    inputs->append(input2);
    //    inputs->append(input3);
    //    inputs->append(input4);
    //    inputs->append(input5);
    //    inputs->append(input6);
    //    inputs->append(input7);
    Variable temp1(Variable(10.0));
    Variable temp2(Variable(11.0));
    Variable temp3(Variable(12.0));
    Variable temp4(Variable(33.0));
    Variable temp5(Variable(34.0));
    Variable temp6(Variable(32.0));
    Variable temp7(Variable(31.0));
    samples->append(temp1);
    samples->append(temp2);
    samples->append(temp3);
    samples->append(temp4);
    samples->append(temp5);
    samples->append(temp6);
    samples->append(temp7);

    VectorPtr samples2 = new DenseDoubleVector(7, 0.0);
    samples2->setElement(0, (double)0.10);
    samples2->setElement(1, (double)0.11);
    samples2->setElement(2, (double)0.12);
    samples2->setElement(3, (double)0.33);
    samples2->setElement(4, (double)0.34);
    samples2->setElement(5, (double)0.32);
    samples2->setElement(6, (double)0.31);

    simple->learn(context, inputs, samples2, DenseDoubleVectorPtr(), ContainerPtr(), ContainerPtr(),
        DenseDoubleVectorPtr());

    for (size_t j = 10; j <= 50; j += 10)
    {
      for (size_t i = 0; i < 5; i++)
      {
        DenseDoubleVectorPtr features = new DenseDoubleVector(2, 1.0);
        features->setValue(0, (double)j / 100);
        Variable input(features);

        //ProteinMoverPtr mover =
        //    gen->sample(context, random, &input).getObjectAndCast<ProteinMover> ();
        Variable mover =
                    simple->sample(context, random, &input);
        //cout << (const char*)mover->toString() << endl;
        cout << (const char*)mover.toString() << endl;
      }
      cout << "===" << endl;
    }

//    VariableVectorPtr samples = new VariableVector(0);
//    VariableVectorPtr inputs = new VariableVector(0);
//    Variable input1(Variable((int)20));
//    Variable input2(Variable((int)20));
//    Variable input3(Variable((int)20));
//    Variable input4(Variable((int)50));
//    Variable input5(Variable((int)50));
//    Variable input6(Variable((int)50));
//    Variable input7(Variable((int)50));
//    inputs->append(input1);
//    inputs->append(input2);
//    inputs->append(input3);
//    inputs->append(input4);
//    inputs->append(input5);
//    inputs->append(input6);
//    inputs->append(input7);
//    RigidBodyMoverPtr temp1 = new RigidBodyMover(10, 15, 4.5, -23.3);
//    RigidBodyMoverPtr temp2 = new RigidBodyMover(11, 15, 4.1, -22.3);
//    RigidBodyMoverPtr temp3 = new RigidBodyMover(12, 17, 4.2, -21.3);
//    RigidBodyMoverPtr temp4 = new RigidBodyMover(22, 26, 5.5, -24.3);
//    RigidBodyMoverPtr temp5 = new RigidBodyMover(20, 26, 3.5, -23.1);
//    PhiPsiMoverPtr temp6 = new PhiPsiMover(24, 34.2, 87.3);
//    PhiPsiMoverPtr temp7 = new PhiPsiMover(25, 33.2, 84.3);
//    samples->append(temp1);
//    samples->append(temp2);
//    samples->append(temp3);
//    samples->append(temp4);
//    samples->append(temp5);
//    samples->append(temp6);
//    samples->append(temp7);
//
//    gen->learn(context, inputs, samples, DenseDoubleVectorPtr(), ContainerPtr(), ContainerPtr(),
//        DenseDoubleVectorPtr());
//
//    int count1 = 0;
//    int count2 = 0;
//    int count3 = 0;
//    for (size_t j = 10; j < 40; j += 10)
//    {
//      for (size_t i = 0; i < 20; i++)
//      {
//        Variable input((int)j);
//        ProteinMoverPtr mover =
//            gen->sample(context, random, &input).getObjectAndCast<ProteinMover> ();
//        cout << (const char*)mover->toString() << endl;
//
//        if (mover.isInstanceOf<PhiPsiMover> ())
//          count1++;
//        else if (mover.isInstanceOf<ShearMover> ())
//          count2++;
//        else if (mover.isInstanceOf<RigidBodyMover> ())
//          count3++;
//      }
//      cout << "===" << endl;
//    }
//
//    int num = count1 + count2 + count3;
//    cout << "il y a  " << endl;
//    cout << "phipsi : " << (double)count1 / (double)num << endl;
//    cout << "shear : " << (double)count2 / (double)num << endl;
//    cout << "rigidbody : " << (double)count3 / (double)num << endl;
# endif
#  if 0
    // --------------- residuepair sampler
    cout << "======================= dual residue sampler ===================" << endl;
    ofstream ofsdual("/Users/alex/Documents/MATLAB/residuepair_init.data");
    ResiduePairSamplerPtr dual = new ResiduePairSampler(50);
    random = new RandomGenerator(0);
    for (double i = 0; i < 20; i++)
    {
      PairPtr out = dual->sample(context, random).getObjectAndCast<Pair> ();
      ofsdual << out->getFirst().getInteger() << " "
      << out->getSecond().getInteger() << endl;
    }

    ofsdual.flush();
    ofsdual.close();

    dual->saveToFile(context, context.getFile(T("dualresidue.xml")));

    std::vector<PairPtr> mm(8);
    mm[0] = new Pair(4, 21);
    mm[1] = new Pair(6, 20);
    mm[2] = new Pair(5, 19);
    mm[3] = new Pair(7, 20);
    mm[4] = new Pair(46, 5);
    mm[5] = new Pair(47, 3);
    mm[6] = new Pair(47, 2);
    mm[7] = new Pair(45, 4);

    ObjectVectorPtr dataset = new ObjectVector(pairClass(integerType, integerType), 0);

    for (int i = 0; i < mm.size(); i++)
    dataset->append(mm[i]);

    dual->learn(context, ContainerPtr(), dataset, DenseDoubleVectorPtr(), ContainerPtr(),
        ContainerPtr(), DenseDoubleVectorPtr());
    dual->computeProbabilities();

    //====================================================

    dual->saveToFile(context, context.getFile(T("dualresidue_afterlearn.xml")));
    ResiduePairSamplerPtr dual2 = Variable::createFromFile(context, context.getFile(
            T("dualresidue_afterlearn.xml"))).getObjectAndCast<ResiduePairSampler> ();
    ofstream ofsduallearned("/Users/alex/Documents/MATLAB/residuepair_learned.data");
    random = new RandomGenerator(0);
    RandomGeneratorPtr random2 = new RandomGenerator(0);
    for (double i = 0; i < 2000; i++)
    {
      PairPtr out = dual->sample(context, random).getObjectAndCast<Pair> ();
      cout << out->getFirst().getInteger() << ", " << out->getSecond().getInteger() << endl;
      PairPtr out2 = dual2->sample(context, random2).getObjectAndCast<Pair> ();
      cout << out2->getFirst().getInteger() << ", " << out2->getSecond().getInteger() << endl;
      ofsduallearned << out2->getFirst().getInteger() << ", "
      << out2->getSecond().getInteger() << endl;
    }

    ofsduallearned.flush();
    ofsduallearned.close();
#  endif

#  if 0
    // --------------- univarate mixture learning
    SamplerPtr gauss1 = gaussianSampler(-2, 5);
    SamplerPtr gauss2 = gaussianSampler(9, 5);
    std::vector<SamplerPtr> mixtsamp;
    mixtsamp.push_back(gauss1);
    mixtsamp.push_back(gauss2);
    DenseDoubleVectorPtr probas = new DenseDoubleVector(2, 0);
    probas->setValue(0, 0.3);
    probas->setValue(1, 0.7);
    CompositeSamplerPtr mix = mixtureSampler(probas, mixtsamp);
    cout << "=========== test sample =======================" << endl;
    ofstream ofs("/Users/alex/Documents/MATLAB/unigauss.data");

    for (int i = 0; i < 10000; i++)
    ofs << mix->sample(context, random).getDouble() << endl;

    ofs.flush();
    ofs.close();

    VariableVectorPtr learning = new VariableVector(0);

    std::vector<double> mm(8);
    mm[0] = -10.2;
    mm[1] = -12.3;
    mm[2] = -9.2;
    mm[3] = -15.2;
    mm[4] = 98.5;
    mm[5] = 99.0;
    mm[6] = 100.5;
    mm[7] = 100.0;

    for (size_t i = 0; i < mm.size(); i++)
    learning->append(Variable(mm[i]));

    cout << "=========== learning =======================" << endl;
    mix->learn(context, ContainerPtr(), learning, ContainerPtr(), ContainerPtr());

    cout << "=========== sampling again =======================" << endl;
    ofstream ofs2("/Users/alex/Documents/MATLAB/multiLearned.data");
    for (double i = 0; i < 10000; i++)
    ofs2 << mix->sample(context, random) << endl;

    ofs2.flush();
    ofs2.close();

    mix->saveToFile(context, context.getFile(T("testunimulti.xml")));

    random = new RandomGenerator(0);
    RandomGeneratorPtr random2 = new RandomGenerator(0);
    SamplerPtr
    mix2 =
    Variable::createFromFile(context, context.getFile(T("testunimulti.xml"))).getObjectAndCast<
    CompositeSampler> ();
    for (size_t i = 0; i < 10; i++)
    {
      cout << mix->sample(context, random).getDouble() << endl;
      cout << mix2->sample(context, random2).getDouble() << endl;
    }
#  endif

# if 0
    // --------------- multivarate mixture learning
    DoubleMatrixPtr means0 = new DoubleMatrix(2, 1, 0.0);
    DoubleMatrixPtr means1 = new DoubleMatrix(2, 1, 0.0);
    means0->setValue(0, 0, -14.9);
    means0->setValue(1, 0, 4.9);
    means1->setValue(0, 0, 15.0);
    means1->setValue(1, 0, -5.0);

    DoubleMatrixPtr covarianceMatrix0 = new DoubleMatrix(2, 2, 0.0);
    covarianceMatrix0->setValue(0, 0, 5.0);
    covarianceMatrix0->setValue(0, 1, 0.01);
    covarianceMatrix0->setValue(1, 0, 0.01);
    covarianceMatrix0->setValue(1, 1, 5.0);
    DoubleMatrixPtr covarianceMatrix1 = new DoubleMatrix(2, 2, 0.0);
    covarianceMatrix1->setValue(0, 0, 5.0);
    covarianceMatrix1->setValue(0, 1, 0.01);
    covarianceMatrix1->setValue(1, 0, 0.01);
    covarianceMatrix1->setValue(1, 1, 5.0);

    ScalarContinuousSamplerPtr gauss0 = multiVariateGaussianSampler(means0, covarianceMatrix0);
    ScalarContinuousSamplerPtr gauss1 = multiVariateGaussianSampler(means1, covarianceMatrix1);
    DenseDoubleVectorPtr probas = new DenseDoubleVector(2, 0);
    probas->setValue(0, 0.5);
    probas->setValue(1, 0.5);
    std::vector<SamplerPtr> mixtsamp;
    mixtsamp.push_back(gauss0);
    mixtsamp.push_back(gauss1);
    CompositeSamplerPtr mix = mixtureSampler(probas, mixtsamp);

    cout << "=========== test sample =======================" << endl;
    ofstream ofs("/Users/alex/Documents/MATLAB/multi.data");

    ObjectVectorPtr learning = new ObjectVector(doubleMatrixClass(doubleType), 0);

    std::vector<DoubleMatrixPtr> mm(8);
    mm[0] = new DoubleMatrix(2, 1, 0.0);
    mm[0]->setValue(0, 0, -12.54634);
    mm[0]->setValue(1, 0, 8.5464);
    mm[1] = new DoubleMatrix(2, 1, 0.0);
    mm[1]->setValue(0, 0, -8.2462);
    mm[1]->setValue(1, 0, 12.14646);
    mm[2] = new DoubleMatrix(2, 1, 0.0);
    mm[2]->setValue(0, 0, -10.34644);
    mm[2]->setValue(1, 0, 8.5356);
    mm[3] = new DoubleMatrix(2, 1, 0.0);
    mm[3]->setValue(0, 0, -11.59);
    mm[3]->setValue(1, 0, 12.57);
    mm[4] = new DoubleMatrix(2, 1, 0.0);
    mm[4]->setValue(0, 0, 20.57);
    mm[4]->setValue(1, 0, -26.52);
    mm[5] = new DoubleMatrix(2, 1, 0.0);
    mm[5]->setValue(0, 0, 20.95);
    mm[5]->setValue(1, 0, -25.52);
    mm[6] = new DoubleMatrix(2, 1, 0.0);
    mm[6]->setValue(0, 0, 20.52);
    mm[6]->setValue(1, 0, -26.3);
    mm[7] = new DoubleMatrix(2, 1, 0.0);
    mm[7]->setValue(0, 0, 20.2);
    mm[7]->setValue(1, 0, -25.2);

    for (size_t i = 0; i < mm.size(); i++)
    learning->append(mm[i]);

    cout << "=========== learning =======================" << endl;
    mix->learn(context, ContainerPtr(), learning, DenseDoubleVectorPtr(), ContainerPtr(),
        ContainerPtr(), DenseDoubleVectorPtr());

    File outmix = context.getFile(T("mixturesampler_mix.xml"));
    File outp = context.getFile(T("mixturesampler_p.xml"));
    mix->saveToFile(context, outmix);

    for (int i = 0; i < 10; i++)
    {
      DoubleMatrixPtr out = mix->sample(context, random).getObjectAndCast<DoubleMatrix> ();
      cout << out->getValue(0, 0) << ", " << out->getValue(1, 0) << endl;
    }

    for (double i = -50; i <= 50; i++)
    {
      for (double j = -50; j <= 50; j++)
      {
        DoubleMatrixPtr point = new DoubleMatrix(2, 1, i);
        point->setValue(1, 0, j);
        ObjectVectorPtr conteneur = new ObjectVector(doubleMatrixClass(doubleType), 0);
        conteneur->append(point);
        DenseDoubleVectorPtr probs = mix->computeProbabilities(ContainerPtr(), conteneur);
        ofs << probs->getValue(0) << " ";
      }
      ofs << endl;
    }

    ofs.flush();
    ofs.close();

    //            cout << "=========== sampling again =======================" << endl;
    //            ofstream ofs2("/Users/alex/Documents/MATLAB/multiLearned.data");
    //            for (double i = 0; i < 10000; i++)
    //            {
    //              DoubleMatrixPtr out = mix->sample(context, random).getObjectAndCast<Matrix> ();
    //              ofs2 << out->getValue(0, 0) << ", " << out->getValue(1, 0) << endl;
    //            }
    //
    //            ofs2.flush();
    //            ofs2.close();
    //
    //            mix->saveToFile(context, context.getFile(T("testgaussmulti.xml")));
    //
    //            random = new RandomGenerator(0);
    //            RandomGeneratorPtr random2 = new RandomGenerator(0);
    //            SamplerPtr mix2 = Variable::createFromFile(context, context.getFile(
    //                T("testgaussmulti.xml"))).getObjectAndCast<CompositeSampler> ();
    //            for (size_t i = 0; i < 5; i++)
    //            {
    //              DoubleMatrixPtr out = mix->sample(context, random).getObjectAndCast<Matrix> ();
    //              cout << out->getValue(0, 0) << ", " << out->getValue(1, 0) << endl;
    //              DoubleMatrixPtr out2 = mix2->sample(context, random2).getObjectAndCast<Matrix> ();
    //              cout << out2->getValue(0, 0) << ", " << out2->getValue(1, 0) << endl;
    //            }
#  endif

#  if 0
    // --------------- gaussian log value function
    ScalarVectorFunctionPtr func = gaussianLogValueFunction();
    DenseDoubleVectorPtr params = new DenseDoubleVector(2, 0);
    params->setValue(0, 3.0);
    params->setValue(1, 1.5);
    Variable val(0.13);
    double result = 0;
    DenseDoubleVectorPtr grad = new DenseDoubleVector(2, 0);
    double weight = 1;
    func->computeScalarVectorFunction(params, &val, &result, &grad, weight);
    cout << result << endl;
    cout << (const char* )grad->toString() << endl;
#  endif

#  if 0
    // --------------- Samplers
    SamplerPtr samp = new ProteinMoverSampler(5);
    ObjectVectorPtr learning = new ObjectVector(proteinMoverClass, 0);

    // phipsi
    learning->append(phiPsiMover(1, 34, -123));
    learning->append(phiPsiMover(0, 30, -122));
    learning->append(phiPsiMover(2, 27, -121));
    learning->append(phiPsiMover(3, 33, -121));
    learning->append(phiPsiMover(1, 34, -123));
    learning->append(phiPsiMover(0, 30, -122));
    learning->append(phiPsiMover(2, 27, -121));
    // shear
    learning->append(shearMover(3, 0.9, 4.5));
    learning->append(shearMover(4, 0.7, 4.3));
    learning->append(shearMover(3, 0.8, 3.4));
    // general
    learning->append(rigidBodyMover(3, 5, 2.8, -3.4));
    learning->append(rigidBodyMover(2, 5, 2.5, -2.4));
    learning->append(rigidBodyMover(1, 3, 0.8, 3.4));
    learning->append(rigidBodyMover(3, 4, 1.2, 2.4));
    learning->append(rigidBodyMover(3, 5, 0.3, 3.4));
    // spin
    learning->append(rigidBodyMover(3, 5, 0.0, 11.3));
    learning->append(rigidBodyMover(4, 0, 0.0, 12.4));
    // trans
    learning->append(rigidBodyMover(4, 1, 10.2, 0.0));
    learning->append(rigidBodyMover(4, 2, 9.2, 0.0));
    learning->append(rigidBodyMover(4, 0, 12.1, 0.0));

    samp->learn(context, ContainerPtr(), learning);

    random = new RandomGenerator(0);

    int count0 = 0;
    int count1 = 0;
    int count2 = 0;
    int num = 10;
    for (int i = 0; i < num; i++)
    {
      Variable v = samp->sample(context, random);

      ProteinMoverPtr t = v.getObjectAndCast<ProteinMover> ();
      cout << i << " : " << (const char*)t->toString() << endl;

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

    ObjectVectorPtr learning2 = new ObjectVector(proteinMoverClass, 0);
    learning2->append(phiPsiMover(1, 34, -123));
    learning2->append(phiPsiMover(0, 30, -122));
    learning2->append(phiPsiMover(2, 27, -121));
    learning2->append(phiPsiMover(3, 33, -121));
    learning2->append(phiPsiMover(1, 34, -123));
    learning2->append(phiPsiMover(0, 30, -122));
    learning2->append(phiPsiMover(2, 27, -121));

    RandomGeneratorPtr random2 = new RandomGenerator(0);
    //        SamplerPtr phipsisampler = objectCompositeSampler(phiPsiMoverClass, new SimpleResidueSampler(5), gaussianSampler(0, 25), gaussianSampler(0, 25));
    //        phipsisampler->learn(context, ContainerPtr(), learning2);
    //        for (int i = 0; i < num; i++)
    //        {
    //          Variable v = phipsisampler->sample(context, random2);
    //          ProteinMoverPtr t = v.getObjectAndCast<ProteinMover>();
    //          cout << i << " : " << (const char* )t->toString() << endl;
    //        }

    File outfile = context.getFile(T("protSampler.xml"));
    samp->saveToFile(context, outfile);

    CompositeSamplerPtr rebirth = Variable::createFromFile(context, outfile).getObjectAndCast<
    CompositeSampler> ();
    CompositeSamplerPtr rebirth2 = samp->cloneAndCast<CompositeSampler> ();
    count0 = 0;
    count1 = 0;
    count2 = 0;
    random2 = new RandomGenerator(0);
    RandomGeneratorPtr random3 = new RandomGenerator(0);
    for (int i = 0; i < num; i++)
    {

      Variable v = rebirth->sample(context, random2);
      Variable v2 = rebirth2->sample(context, random3);

      ProteinMoverPtr t = v.getObjectAndCast<ProteinMover> ();
      ProteinMoverPtr t2 = v2.getObjectAndCast<ProteinMover> ();
      cout << (const char*)t->toString() << endl;
      cout << (const char*)t2->toString() << endl;
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

#  endif

    context.informationCallback(T("RosettaTest done."));
    return Variable();

  }
};

void generateMoversDataSet(VectorPtr& inputs, VectorPtr& samples)
{
  size_t numExamples = 100;
  ClassPtr inputClass = denseDoubleVectorClass(featuresMoverEnumerationEnumeration, doubleType);
  inputs = vector(inputClass);
  samples = vector(proteinMoverClass);

  DenseDoubleVectorPtr input = new DenseDoubleVector(inputClass, 3, 0.0);
  input->setValue(0, 20.0); // length of protein
  input->setValue(1, 1.0); // first distribution
  for (size_t i = 0; i < numExamples / 2; ++i)
  {
    inputs->append(input);
    samples->append(phiPsiMover(15, 32, -123));

    inputs->append(input);
    samples->append(phiPsiMover(12, 34, -120));

    inputs->append(input);
    samples->append(phiPsiMover(14, 38, -121));

    inputs->append(input);
    samples->append(phiPsiMover(13, 30, -122));
  }

  input = new DenseDoubleVector(inputClass, 3, 0.0);
  input->setValue(0, 10.0); // length of protein
  input->setValue(2, 1.0); // second distribution
  for (size_t i = 0; i < numExamples / 2; ++i)
  {
    inputs->append(input);
    samples->append(shearMover(3, 0.9, 4.5));

    inputs->append(input);
    samples->append(shearMover(4, 0.7, 4.3));

    inputs->append(input);
    samples->append(shearMover(3, 0.8, 3.4));

    inputs->append(input);
    samples->append(shearMover(2, 1.0, 4.53));

//    inputs->append(input);
//    samples->append(shearMover(4, 1.1, 4.32));
//
//    inputs->append(input);
//    samples->append(shearMover(5, 0.853, 3.45));

    // general
    inputs->append(input);
    samples->append(rigidBodyMover(1, 3, 20.8, -3.4));

    inputs->append(input);
    samples->append(rigidBodyMover(0, 3, 20.5, -2.4));

    inputs->append(input);
    samples->append(rigidBodyMover(1, 3, 20.18, -3.4));

    inputs->append(input);
    samples->append(rigidBodyMover(0, 4, 21.2, -2.4));

    inputs->append(input);
    samples->append(rigidBodyMover(0, 4, 20.3, -3.4));

    inputs->append(input);
    samples->append(rigidBodyMover(1, 3, 20.76, -4.2));

    inputs->append(input);
    samples->append(rigidBodyMover(1, 3, 20.76, -4.2));

    inputs->append(input);
    samples->append(rigidBodyMover(0, 3, 21.01, -4));
  }
}

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_ROSETTA_TEST_H_

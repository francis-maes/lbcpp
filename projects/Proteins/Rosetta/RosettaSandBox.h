/*-----------------------------------------.---------------------------------.
| Filename: RosettaSandBox.h               | Rosetta Sand Box                |
| Author  : Francis Maes                   |                                 |
| Started : 15/05/2011 19:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_SAND_BOX
# define LBCPP_PROTEINS_ROSETTA_SAND_BOX

# include "../Data/Protein.h"
# include "../Data/AminoAcid.h"
# include "../Data/Residue.h"
# include "../Data/Formats/PDBFileGenerator.h"
# include "../Evaluator/QScoreEvaluator.h"
# include "RosettaUtils.h"
# include "ProteinMover.h"
# include "ProteinMover/PhiPsiMover.h"
# include "ProteinMover/ShearMover.h"
# include "ProteinMover/RigidBodyMover.h"
# include "Sampler/SimpleResidueSampler.h"
# include "Sampler/ResiduePairSampler.h"
# include "Sampler/ProteinMoverSampler.h"
# include "Sampler.h"
# include "Sampler/GeneralProteinMoverSampler.h"
# include "RosettaProtein.h"
//using namespace std;

namespace lbcpp
{

class RosettaSandBox : public WorkUnit
{
public:
  RosettaSandBox() : numExamples(100) {}

  void generateConditionalGaussianDataSet(VectorPtr& inputs, VectorPtr& samples)
  {
    size_t n = numExamples;

    ClassPtr inputClass = denseDoubleVectorClass(falseOrTrueEnumeration, doubleType);
    RandomGeneratorPtr random = new RandomGenerator();
    inputs = vector(inputClass, n);
    samples = new DenseDoubleVector(n, 0.0);

    // in interval x \in [0, 1], mean goes from 0.5 to -0.5 and stddev goes from 0.0 to 1.0
    double meanOrigin = 0.5;
    double meanCoefficient = -1;
    double stddevOrigin = 0.0;
    double stddevCoefficient = 1.0;

    for (size_t i = 0; i < n; ++i)
    {
# if 0
      double x = random->sampleDouble(0, 1);
      double mean = meanOrigin + x * meanCoefficient;
      double stddev = stddevOrigin + x * stddevCoefficient;
# else
      double x = 0;
      double mean = 0;
      double stddev = 0;
      if ((i % 2) == 0)
      {
        x = random->sampleDoubleFromGaussian(0.20, 0.01);
        mean = 390;
        stddev = 5;
      }
      else
      {
        x = random->sampleDoubleFromGaussian(0.50, 0.02);
        mean = 450;
        stddev = 5;
      }
# endif

      double y = random->sampleDoubleFromGaussian(mean, stddev);
      DenseDoubleVectorPtr input = new DenseDoubleVector(inputClass);
      input->setValue(0, 1.0);
      input->setValue(1, x);
      inputs->setElement(i, input);
      samples->setElement(i, y);
    }
  }

  void generateMoversDataSet(VectorPtr& inputs, VectorPtr& samples)
  {
#ifdef LBCPP_PROTEIN_ROSETTA
    rosettaInitialization(defaultExecutionContext(), false);
    ClassPtr inputClass = denseDoubleVectorClass(falseOrTrueEnumeration, doubleType);
    //inputs = vector(inputClass);
    //inputs = new ObjectVector(doubleVectorClass(), 0); // not work
    inputs = vector(doubleVectorClass(), 0);
    samples = vector(proteinMoverClass);

    core::pose::PoseOP pose = new core::pose::Pose();
    String acc = T("AAAAAAAAAPSHHH");
    makePoseFromSequence(pose, acc);
    RosettaWorkerPtr worker1 = new RosettaWorker(pose, 2, 1, 0, 0, 0);
    RosettaProteinPtr protein = new RosettaProtein(pose, 1, 0, 0, 0);
    RosettaProteinFeaturesPtr feat = new RosettaProteinFeatures(1, 0, 0, 0);
    feat->initialize(defaultExecutionContext(), rosettaProteinClass);

    DenseDoubleVectorPtr input = new DenseDoubleVector(inputClass);
    input->setValue(0, 1.0); // first distribution
    for (size_t i = 0; i < numExamples / 2; ++i)
    {
      //inputs->append(input);
      inputs->append(feat->compute(defaultExecutionContext(), protein));
      samples->append(phiPsiMover(0, 32, -123));

      //inputs->append(input);
      inputs->append(feat->compute(defaultExecutionContext(), protein));
      samples->append(phiPsiMover(1, 34, -120));

      //inputs->append(input);
      inputs->append(feat->compute(defaultExecutionContext(), protein));
      samples->append(phiPsiMover(2, 38, -121));

      //inputs->append(input);
      inputs->append(feat->compute(defaultExecutionContext(), protein));
      samples->append(phiPsiMover(3, 30, -122));
    }


    acc = T("AAAAAAAAAAAAAAAAAAAAAAAAPSHHH");
    makePoseFromSequence(pose, acc);
    RosettaProteinPtr protein2 = new RosettaProtein(pose, 1, 0, 0, 0);


    input = new DenseDoubleVector(inputClass);
    input->setValue(1, 1.0); // second distribution
    for (size_t i = 0; i < numExamples / 2; ++i)
    {
      //inputs->append(input);
      inputs->append(feat->compute(defaultExecutionContext(), protein2));
      samples->append(shearMover(3, 0.9, 4.5));

      //inputs->append(input);
      inputs->append(feat->compute(defaultExecutionContext(), protein2));
      samples->append(shearMover(4, 0.7, 4.3));

      //inputs->append(input);
      inputs->append(feat->compute(defaultExecutionContext(), protein2));
      samples->append(shearMover(3, 0.8, 3.4));

      // general
      //inputs->append(input);
      inputs->append(feat->compute(defaultExecutionContext(), protein2));
      samples->append(rigidBodyMover(3, 5, 2.8, -3.4));

      //inputs->append(input);
      inputs->append(feat->compute(defaultExecutionContext(), protein2));
      samples->append(rigidBodyMover(3, 5, 2.5, -2.4));

      //inputs->append(input);
      inputs->append(feat->compute(defaultExecutionContext(), protein2));
      samples->append(rigidBodyMover(1, 3, 0.8, 3.4));

      //inputs->append(input);
      inputs->append(feat->compute(defaultExecutionContext(), protein2));
      samples->append(rigidBodyMover(0, 4, 1.2, 2.4));

      //inputs->append(input);
      inputs->append(feat->compute(defaultExecutionContext(), protein2));
      samples->append(rigidBodyMover(2, 4, 0.3, 3.4));

      //inputs->append(input);
      inputs->append(feat->compute(defaultExecutionContext(), protein2));
      samples->append(rigidBodyMover(1, 3, 0.76, 4.2));

      //inputs->append(input);
      inputs->append(feat->compute(defaultExecutionContext(), protein2));
      samples->append(rigidBodyMover(1, 3, 0.76, 4.2));

      //inputs->append(input);
      inputs->append(feat->compute(defaultExecutionContext(), protein2));
      samples->append(rigidBodyMover(0, 3, 1.01, 4));
    }
#else
    jassert(false);
#endif // LBCPP_PROTEIN_ROSETTA
  }

  void displayDataSet(ExecutionContext& context, const String& name, const ContainerPtr& inputs, const ContainerPtr& samples)
  {
    size_t n = samples->getNumElements();

    TypePtr elementsType = inputs->getElementsType();
    bool isDoubleVector = elementsType->inheritsFrom(denseDoubleVectorClass());
    EnumerationPtr inputsEnumeration;
    if (isDoubleVector)
      inputsEnumeration = DoubleVector::getElementsEnumeration(elementsType);

    context.enterScope(name);
    for (size_t i = 0; i < n; ++i)
    {
      context.enterScope(T("Sample ") + String((int)i + 1));
      
      if (isDoubleVector)
      {
        DenseDoubleVectorPtr input = inputs->getElement(i).getObjectAndCast<DenseDoubleVector>();
        if (inputsEnumeration == falseOrTrueEnumeration)
          context.resultCallback(T("input"), input->getValue(1));
        else
          for (size_t j = 0; j < inputsEnumeration->getNumElements(); ++j)
            context.resultCallback(inputsEnumeration->getElementName(j), input->getValue(j));
      }
      else
        context.resultCallback(T("input"), inputs->getElement(i));

      context.resultCallback(T("sample"), samples->getElement(i));
      context.leaveScope(true);
    }
    context.leaveScope(n);
  }

  virtual Variable run(ExecutionContext& context)
  {
    VectorPtr inputs;
    VectorPtr samples;
    RandomGeneratorPtr random = new RandomGenerator();
# if 0
    generateConditionalGaussianDataSet(inputs, samples);
    displayDataSet(context, T("Conditional gaussian dataset"), inputs, samples);

    //SamplerPtr sampler = conditionalGaussianSampler();
    SamplerPtr sampler = new ClamperSampler(360, 480);
    sampler->learn(context, inputs, samples);

    context.enterScope(T("Testing conditional gaussian"));
    for (size_t i = 0; i < 1000; ++i)
    {
      context.enterScope(T("Sample ") + String((int)i + 1));
# if 0
      double x = random->sampleDouble(0.0, 1.0);
#else
      double x = random->sampleDouble(0.0, 1.0);
# endif
      context.resultCallback(T("x"), x);
      DenseDoubleVectorPtr features = new DenseDoubleVector(falseOrTrueEnumeration, doubleType);
      features->setValue(0, 1.0);
      features->setValue(1, x);
      Variable featuresVariable(features);

      double y = sampler->sample(context, random, &featuresVariable).getDouble();
      context.resultCallback(T("y"), y);
      context.leaveScope(true);
    }
    context.leaveScope(true);
# endif
#if 1
    generateMoversDataSet(inputs, samples);
    SamplerPtr moverClassSampler = maximumEntropySampler(proteinMoverEnumerationEnumeration);
    SamplerPtr MEsampler = proteinMoverSampler(moverClassSampler, 1000);

    MEsampler->learn(context, inputs, samples);
  
//    ClassPtr inputClass = denseDoubleVectorClass(falseOrTrueEnumeration, doubleType);
//    random = new RandomGenerator();
//
//    context.enterScope(T("Samples 1"));
//    DenseDoubleVectorPtr input = new DenseDoubleVector(inputClass);
//    input->setValue(0, 1.0); // first distribution
//    Variable inputVariable = input;
//    for (size_t i = 0; i < 100; ++i)
//    {
//      Variable sample = MEsampler->sample(context, random, &inputVariable);
//      context.resultCallback(T("sample ") + String((int)i + 1), sample);
//    }
//    context.leaveScope();
//
//    context.enterScope(T("Samples 2"));
//    input = new DenseDoubleVector(inputClass);
//    input->setValue(1, 1.0); // second distribution
//    inputVariable = input;
//    for (size_t i = 0; i < 100; ++i)
//    {
//      Variable sample = MEsampler->sample(context, random, &inputVariable);
//      context.resultCallback(T("sample ") + String((int)i + 1), sample);
//    }
//    context.leaveScope();
#endif // 0
#if 0

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

    SamplerPtr sampler = new ProteinMoverSampler(5);
    sampler->learn(context, learning);
    context.resultCallback(T("sampler"), sampler);
#endif 


    return Variable();

  }

private:
  friend class RosettaSandBoxClass;

  size_t numExamples;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_ROSETTA_TEST_H_

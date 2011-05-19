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
# include <lbcpp/Learning/LossFunction.h>
# include <lbcpp/Learning/Numerical.h>
# include <lbcpp/Core/Vector.h>
//using namespace std;

namespace lbcpp
{

class MaximumEntropySampler : public DiscreteSampler
{
public:
  MaximumEntropySampler(TypePtr outputType) : outputType(outputType)
  {
    StochasticGDParametersPtr learnerParameters = new StochasticGDParameters(constantIterationFunction(1.0), maxIterationsStoppingCriterion(100));
    learnerParameters->setLossFunction(logBinomialMultiClassLossFunction());
    learnerParameters->setEvaluateAtEachIteration(false);

    predictor = linearMultiClassClassifier(learnerParameters);
    predictor->setEvaluator(EvaluatorPtr()); // todo: log-likelyhood evaluator
  }

  MaximumEntropySampler() {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    DenseDoubleVectorPtr probabilities = predictor->compute(context, inputs[0], Variable::missingValue(outputType)).getObjectAndCast<DenseDoubleVector>();
    if (!probabilities)
      return Variable();
    return Variable(random->sampleWithNormalizedProbabilities(probabilities->getValues()), probabilities->getElementsEnumeration());
  }

  static ContainerPtr mergeInputAndOutputs(const ContainerPtr& inputs, const ContainerPtr& samples)
  {
    size_t n = inputs->getNumElements();
    ClassPtr pairType = pairClass(inputs->getElementsType(), samples->getElementsType());
    ObjectVectorPtr res = new ObjectVector(pairType, n);
    for (size_t i = 0; i < n; ++i)
      res->set(i, new Pair(pairType, inputs->getElement(i), samples->getElement(i)));
    return res;
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
                                                    const ContainerPtr& validationInputs, const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& supervisionWeights)
  {
    ContainerPtr trainingData = mergeInputAndOutputs(trainingInputs, trainingSamples);
    ContainerPtr validationData;
    if (validationSamples)
      validationData = mergeInputAndOutputs(validationInputs, validationSamples);
    predictor->train(context, trainingData, validationData, T("Training maxent"));
  }

protected:
  friend class MaximumEntropySamplerClass;

  TypePtr outputType; // tmp, todo: merge with Functions
  FunctionPtr predictor;
};

class RosettaSandBox : public WorkUnit
{
public:
  void generateDataSet(VectorPtr& inputs, VectorPtr& samples)
  {
    ClassPtr inputClass = denseDoubleVectorClass(falseOrTrueEnumeration, doubleType);
    inputs = vector(inputClass);
    samples = vector(proteinMoverClass);

    DenseDoubleVectorPtr input = new DenseDoubleVector(inputClass);
    input->setValue(0, 1.0); // first distribution
    for (size_t i = 0; i < 10; ++i)
    {
      inputs->append(input);
      samples->append(phiPsiMover(0, 32, -123));

      inputs->append(input);
      samples->append(phiPsiMover(1, 34, -120));

      inputs->append(input);
      samples->append(phiPsiMover(2, 38, -121));

      inputs->append(input);
      samples->append(phiPsiMover(3, 30, -122));
    }

    input = new DenseDoubleVector(inputClass);
    input->setValue(1, 1.0); // second distribution
    for (size_t i = 0; i < 10; ++i)
    {
      inputs->append(input);
      samples->append(shearMover(3, 0.9, 4.5));

      inputs->append(input);
      samples->append(shearMover(4, 0.7, 4.3));

      inputs->append(input);
      samples->append(shearMover(3, 0.8, 3.4));

      // general
      inputs->append(input);
      samples->append(rigidBodyMover(3, 5, 2.8, -3.4));

      inputs->append(input);
      samples->append(rigidBodyMover(3, 5, 2.5, -2.4));

      inputs->append(input);
      samples->append(rigidBodyMover(1, 3, 0.8, 3.4));

      inputs->append(input);
      samples->append(rigidBodyMover(0, 4, 1.2, 2.4));

      inputs->append(input);
      samples->append(rigidBodyMover(2, 4, 0.3, 3.4));

      inputs->append(input);
      samples->append(rigidBodyMover(1, 3, 0.76, 4.2));

      inputs->append(input);
      samples->append(rigidBodyMover(1, 3, 0.76, 4.2));

      inputs->append(input);
      samples->append(rigidBodyMover(0, 3, 1.01, 4));
    }
  }

  virtual Variable run(ExecutionContext& context)
  {
    VectorPtr inputs;
    VectorPtr samples;
    generateDataSet(inputs, samples);

    SamplerPtr moverClassSampler = new MaximumEntropySampler(proteinMoverEnumerationEnumeration);
    SamplerPtr sampler = proteinMoverSampler(moverClassSampler, 1000);

    sampler->learn(context, inputs, samples);
  
    ClassPtr inputClass = denseDoubleVectorClass(falseOrTrueEnumeration, doubleType);
    RandomGeneratorPtr random = new RandomGenerator();

    context.enterScope(T("Samples 1"));
    DenseDoubleVectorPtr input = new DenseDoubleVector(inputClass);
    input->setValue(0, 1.0); // first distribution
    Variable inputVariable = input;
    for (size_t i = 0; i < 100; ++i)
    {
      Variable sample = sampler->sample(context, random, &inputVariable);
      context.resultCallback(T("sample ") + String((int)i + 1), sample);
    }
    context.leaveScope();

    context.enterScope(T("Samples 2"));
    input = new DenseDoubleVector(inputClass);
    input->setValue(1, 1.0); // second distribution
    inputVariable = input;
    for (size_t i = 0; i < 100; ++i)
    {
      Variable sample = sampler->sample(context, random, &inputVariable);
      context.resultCallback(T("sample ") + String((int)i + 1), sample);
    }
    context.leaveScope();

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
  friend class RosettaWorkUnitClass;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_ROSETTA_TEST_H_

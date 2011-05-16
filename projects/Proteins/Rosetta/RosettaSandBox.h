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
  MaximumEntropySampler()
  {
    StochasticGDParametersPtr learnerParameters = new StochasticGDParameters(constantIterationFunction(1.0));
    learnerParameters->setLossFunction(logBinomialMultiClassLossFunction());
    predictor = linearMultiClassClassifier(learnerParameters);
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    DenseDoubleVectorPtr probabilities = predictor->compute(context, inputs[0], Variable()).getObjectAndCast<DenseDoubleVector>();
    if (!probabilities)
      return Variable();
    return Variable(random->sampleWithNormalizedProbabilities(probabilities->getValues()), probabilities->getElementsEnumeration());
  }

  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    ContainerPtr trainingData = new ObjectVector(dataset[0].getType(), dataset.size());
    for (size_t i = 0; i < dataset.size(); ++i)
      trainingData->setElement(i, dataset[i]);
    predictor->train(context, trainingData, ContainerPtr(), T("Training maxent"));
  }

protected:
  friend class MaximumEntropySamplerClass;

  FunctionPtr predictor;
};

class RosettaSandBox : public WorkUnit
{
public:
  void generateDataSet(std::vector<Variable>& res)
  {
    ClassPtr inputClass = denseDoubleVectorClass(falseOrTrueEnumeration, doubleType);
    TypePtr outputType = proteinMoverEnumerationEnumeration;

    DenseDoubleVectorPtr input = new DenseDoubleVector(inputClass);
    input->setValue(0, 1.0); // first distribution
    for (size_t i = 0; i < 100; ++i)
    {
      res.push_back(new Pair(input, Variable(0, outputType)));
      res.push_back(new Pair(input, Variable(0, outputType)));
      res.push_back(new Pair(input, Variable(0, outputType)));
      res.push_back(new Pair(input, Variable(1, outputType)));
    }

    input = new DenseDoubleVector(inputClass);
    input->setValue(1, 1.0); // second distribution
    for (size_t i = 0; i < 100; ++i)
    {
      res.push_back(new Pair(input, Variable(2, outputType)));
      res.push_back(new Pair(input, Variable(2, outputType)));
      res.push_back(new Pair(input, Variable(2, outputType)));
      res.push_back(new Pair(input, Variable(1, outputType)));
    }
  }

  virtual Variable run(ExecutionContext& context)
  {
    std::vector<Variable> learning;
    generateDataSet(learning);

    SamplerPtr sampler = new MaximumEntropySampler();
    sampler->learn(context, learning);
  
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

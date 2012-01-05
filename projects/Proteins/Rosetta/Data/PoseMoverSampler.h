/*-----------------------------------------.---------------------------------.
| Filename: PoseMoverSampler.h             | PoseMoverSampler                |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 14, 2011  9:12:04 AM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_POSEMOVERSAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_POSEMOVERSAMPLER_H_

# include <lbcpp/Sampler/Sampler.h>
# include "MoverSampler/SimpleResidueSampler.h"
# include "MoverSampler/PairResidueSampler.h"
# include "PoseMover.h"

namespace lbcpp
{

class PoseMoverSampler : public CompositeSampler
{
public:
  PoseMoverSampler(DiscreteSamplerPtr classSampler, size_t numResidues)
  {
    createObjectSamplers(numResidues);
    samplers.push_back(classSampler);
  }

  PoseMoverSampler(size_t numResidues)
  {
    createObjectSamplers(numResidues);
    samplers.push_back(enumerationSampler(poseMoverEnumerationEnumeration));
  }

  PoseMoverSampler() {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    size_t index = (size_t)samplers.back()->sample(context, random, inputs).getInteger();
    jassert(index < samplers.size() - 1);
    return samplers[index]->sample(context, random, inputs);
  }

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples, const DenseDoubleVectorPtr& weights, std::vector<ContainerPtr>& subInputs, std::vector<ContainerPtr>& subSamples, std::vector<ContainerPtr>& subWeights) const
  {
    size_t n = samples->getNumElements();

    size_t numSamplers = samplers.size();
    subInputs.resize(numSamplers);
    subSamples.resize(numSamplers);
    subWeights.resize(numSamplers);

    VectorPtr classSamples = vector(poseMoverEnumerationEnumeration, n);
    for (size_t i = 0; i < numSamplers - 1; ++i)
    {
      subSamples[i] = new ObjectVector(poseMoverClass, 0);
      if (inputs)
        subInputs[i] = vector(inputs->getElementsType());
    }
    subInputs.back() = inputs;
    subSamples.back() = classSamples;

    for (size_t i = 0; i < n; ++i)
    {
      Variable element = samples->getElement(i);
      TypePtr type = element.getType();

      size_t target = 0;
      if (type == phiPsiMoverClass)
        target = 0;
      else if (type == shearMoverClass)
        target = 1;
      else if (type == rigidBodyMoverClass)
        target = 2;
      else
        jassert(false);
      classSamples->setElement(i, Variable(target, poseMoverEnumerationEnumeration));

      subSamples[target].staticCast<Vector>()->append(element);
      if (inputs)
        subInputs[target].staticCast<Vector>()->append(inputs->getElement(i));
    }
  }

protected:
  friend class PoseMoverSamplerClass;

  void createObjectSamplers(size_t numResidues)
  {
    DenseDoubleVectorPtr r1 = new DenseDoubleVector(numResidues, 1.0 / (double)numResidues);
    DenseDoubleVectorPtr r2 = new DenseDoubleVector(numResidues, 1.0 / (double)numResidues);
    DiscreteSamplerPtr res1 = enumerationSampler(r1);
    DiscreteSamplerPtr res2 = enumerationSampler(r2);

    size_t num = 3;
    std::vector<SamplerPtr> mixtsamp1Phi = generateGaussian(num);
    std::vector<SamplerPtr> mixtsamp1Psi = generateGaussian(num);
    std::vector<SamplerPtr> mixtsamp2Phi = generateGaussian(num);
    std::vector<SamplerPtr> mixtsamp2Psi = generateGaussian(num);
    std::vector<SamplerPtr> mixtsampMag = generateGaussian(2);
    std::vector<SamplerPtr> mixtsampAmp = generateGaussian(2);

    DenseDoubleVectorPtr probas1Phi = new DenseDoubleVector(num, 1.0 / num);
    DenseDoubleVectorPtr probas1Psi = new DenseDoubleVector(num, 1.0 / num);
    DenseDoubleVectorPtr probas2Phi = new DenseDoubleVector(num, 1.0 / num);
    DenseDoubleVectorPtr probas2Psi = new DenseDoubleVector(num, 1.0 / num);
    DenseDoubleVectorPtr probasMag = new DenseDoubleVector(2, 0.5);
    DenseDoubleVectorPtr probasAmp = new DenseDoubleVector(2, 0.5);
    CompositeSamplerPtr p1Phi = mixtureSampler(probas1Phi, mixtsamp1Phi);
    CompositeSamplerPtr p1Psi = mixtureSampler(probas1Psi, mixtsamp1Psi);
    CompositeSamplerPtr p2Phi = mixtureSampler(probas2Phi, mixtsamp2Phi);
    CompositeSamplerPtr p2Psi = mixtureSampler(probas2Psi, mixtsamp2Psi);
    CompositeSamplerPtr pMag = mixtureSampler(probasMag, mixtsampMag);
    CompositeSamplerPtr pAmp = mixtureSampler(probasAmp, mixtsampAmp);

    samplers.push_back(objectCompositeSampler(phiPsiMoverClass, res1, p1Phi, p1Psi));
    samplers.push_back(objectCompositeSampler(shearMoverClass, res2, p2Phi, p2Psi));
    samplers.push_back(objectCompositeSampler(rigidBodyMoverClass, new PairResidueSampler(numResidues), pMag, pAmp));

    // OR
    //samplers.push_back(objectCompositeSampler(phiPsiMoverClass, new SimpleResidueSampler(numResidues), gaussianSampler(0, 50), gaussianSampler(0, 50)));
    //samplers.push_back(objectCompositeSampler(shearMoverClass, new SimpleResidueSampler(numResidues), gaussianSampler(0, 50), gaussianSampler(0, 50)));
    //samplers.push_back(objectCompositeSampler(rigidBodyMoverClass, new PairResidueSampler(numResidues), gaussianSampler(1, 1), gaussianSampler(0, 50)));
  }

  std::vector<SamplerPtr> generateGaussian(size_t num)
  {
    std::vector<SamplerPtr> mixtsamp;
    for (size_t i = 0; i < num; i++)
      mixtsamp.push_back(gaussianSampler(0, 25));
    return mixtsamp;
  }
};

typedef ReferenceCountedObjectPtr<PoseMoverSampler> PoseMoverSamplerPtr;


extern CompositeSamplerPtr pairResidueSampler(size_t numResidues);
extern CompositeSamplerPtr simpleResidueSampler(size_t numResidues);

extern CompositeSamplerPtr poseMoverSampler(size_t numResidues);
extern CompositeSamplerPtr poseMoverSampler(DiscreteSamplerPtr classSampler, size_t numResidues);

extern SamplerPtr gaussianMultivariateSampler();


}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_POSEMOVERSAMPLER_H_

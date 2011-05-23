/*-----------------------------------------.---------------------------------.
| Filename: ProteinMoverSampler.h          | ProteinMoverSampler             |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 16 mai 2011  07:06:13          |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_PROTEIN_MOVER_SAMPLER_H_
# define LBCPP_SAMPLER_PROTEIN_MOVER_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"
# include "SimpleResidueSampler.h"
# include "ResiduePairSampler.h"
# include "../ProteinMover.h"
# include "SmoothEnumerationSampler.h"

namespace lbcpp
{

class ProteinMoverSampler : public CompositeSampler
{
public:
  ProteinMoverSampler(DiscreteSamplerPtr classSampler, size_t numResidues)
  {
    createObjectSamplers(numResidues);
    samplers.push_back(classSampler);
  }

  ProteinMoverSampler(size_t numResidues)
  {
    createObjectSamplers(numResidues);
    samplers.push_back(new SmoothEnumerationSampler(proteinMoverEnumerationEnumeration, 0.1, 0.1));
  }

  ProteinMoverSampler() {}

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

    VectorPtr classSamples = vector(proteinMoverEnumerationEnumeration, n);
    for (size_t i = 0; i < numSamplers - 1; ++i)
    {
      subSamples[i] = new ObjectVector(proteinMoverClass, 0);
      if (inputs)
        subInputs[i] = vector(inputs->getElementsType());
    }
    subInputs.back() = inputs;
    subSamples.back() = classSamples;

    for (size_t i = 0; i < n; ++i)
    {
      Variable element = samples->getElement(i);
      TypePtr type = element.getType();

      size_t target;
      if (type == phiPsiMoverClass)
        target = 0;
      else if (type == shearMoverClass)
        target = 1;
      else if (type == rigidBodyMoverClass)
        target = 2;
      else
        jassert(false);
      classSamples->setElement(i, Variable(target, proteinMoverEnumerationEnumeration));

      subSamples[target].staticCast<Vector>()->append(element);
      if (inputs)
        subInputs[target].staticCast<Vector>()->append(inputs->getElement(i));
    }
  }

protected:
  friend class ProteinMoverSamplerClass;

  void createObjectSamplers(size_t numResidues)
  {
    DenseDoubleVectorPtr r1 = new DenseDoubleVector(numResidues, 1.0 / (double)numResidues);
    DenseDoubleVectorPtr r2 = new DenseDoubleVector(numResidues, 1.0 / (double)numResidues);
    DiscreteSamplerPtr res1 = new SmoothEnumerationSampler(r1, 0.1, 1.0 / (5 * (double)numResidues), 0, numResidues - 1);
    DiscreteSamplerPtr res2 = new SmoothEnumerationSampler(r2, 0.1, 1.0 / (5 * (double)numResidues), 0, numResidues - 1);

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
    samplers.push_back(objectCompositeSampler(rigidBodyMoverClass, new ResiduePairSampler(numResidues), pMag, pAmp));

    // OR
    //      samplers.push_back(objectCompositeSampler(phiPsiMoverClass, new SimpleResidueSampler(numResidues), gaussianSampler(0, 25), gaussianSampler(0, 25)));
    //      samplers.push_back(objectCompositeSampler(shearMoverClass, new SimpleResidueSampler(numResidues), gaussianSampler(0, 25), gaussianSampler(0, 25)));
    //      samplers.push_back(objectCompositeSampler(rigidBodyMoverClass, new ResiduePairSampler(numResidues), gaussianSampler(1, 1), gaussianSampler(0, 25)));
  }

  std::vector<SamplerPtr> generateGaussian(size_t num)
  {
    std::vector<SamplerPtr> mixtsamp;
    for (size_t i = 0; i < num; i++)
      mixtsamp.push_back(gaussianSampler(0, 25));
    return mixtsamp;
  }
};

typedef ReferenceCountedObjectPtr<ProteinMoverSampler> ProteinMoverSamplerPtr;

}; /* namespace lbcpp */

#endif //! LBCPP_SAMPLER_PROTEIN_MOVER_SAMPLER_H_

/*-----------------------------------------.---------------------------------.
| Filename: SubsetVectorSampler.h          | Subset Vector Sampler           |
| Author  : Francis Maes                   |                                 |
| Started : 15/11/2012 16:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SAMPLER_SUBSET_VECTOR_H_
# define ML_SAMPLER_SUBSET_VECTOR_H_

# include <ml/Sampler.h>

namespace lbcpp
{

class SubsetVectorSampler : public Sampler
{
public:
  SubsetVectorSampler(SamplerPtr vectorSampler, size_t subsetSize)
    : vectorSampler(vectorSampler), subsetSize(subsetSize) {}
  SubsetVectorSampler() : subsetSize(0) {}
  
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
    {vectorSampler->initialize(context, domain);}

  virtual ObjectPtr sample(ExecutionContext& context) const 
  {
    VectorPtr elements = vectorSampler->sample(context);
    if (elements->getNumElements() <= subsetSize)
      return elements;
    std::vector<size_t> order;
    context.getRandomGenerator()->sampleOrder(elements->getNumElements(), order);
    VectorPtr res = vector(elements->getElementsType(), subsetSize);
    for (size_t i = 0; i < subsetSize; ++i)
      res->setElement(i, elements->getElement(order[i]));
    return res;
  }

protected:
  friend class SubsetVectorSamplerClass;

  SamplerPtr vectorSampler;
  size_t subsetSize;
};

}; /* namespace lbcpp */

#endif // ML_SAMPLER_SUBSET_VECTOR_H_

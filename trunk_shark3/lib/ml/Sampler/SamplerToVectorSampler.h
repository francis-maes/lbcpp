/*-----------------------------------------.---------------------------------.
 | Filename: SamplerToVectorSampler.h       | Sampler To Vector Sampler       |
 | Author  : Denny Verbeeck                 |                                 |
 | Started : 25/02/2013 23:25               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef ML_SAMPLER_TO_VECTOR_H_
# define ML_SAMPLER_TO_VECTOR_H_

# include <ml/Sampler.h>

namespace lbcpp
{

/** This class transforms a normal sampler to a vector sampler.
 *  The result of its sample() function is an OVector of numSamples samples,
 *  each created by calling the normal sampler's sample() function.
 **/
  
class SamplerToVectorSampler : public Sampler
{
public:
  SamplerToVectorSampler(SamplerPtr sampler, size_t numSamples)
    : sampler(sampler), numSamples(numSamples) {}
  SamplerToVectorSampler() : numSamples(0) {}
  
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
    {sampler->initialize(context, domain.staticCast<VectorDomain>()->getElementsDomain());}
  
  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    jassert(sampler && numSamples);
    OVectorPtr res = vector(objectClass, numSamples);
    for (size_t i = 0; i < numSamples; ++i)
      res->set(i, sampler->sample(context));
    return res;
  }
  
protected:
  friend class SamplerToVectorSamplerClass;
  
  SamplerPtr sampler;
  size_t numSamples;
};

}; /* namespace lbcpp */
  
#endif // !ML_SAMPLER_TO_VECTOR_H_

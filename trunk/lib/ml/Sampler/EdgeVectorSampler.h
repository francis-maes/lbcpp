/*-----------------------------------------.---------------------------------.
 | Filename: EdgeVectorSampler.h           | Edge Vector Sampler             |
 | Author  : Denny Verbeeck                |                                 |
 | Started : 14/03/2013 13:58              |                                 |
 `-----------------------------------------/                                 |
                                |                                            |
                                `-------------------------------------------*/

#ifndef ML_SAMPLER_EDGE_VECTOR_H_
# define ML_SAMPLER_EDGE_VECTOR_H_

# include <ml/Sampler.h>

namespace lbcpp
{

class EdgeVectorSampler : public Sampler
{
public:
  EdgeVectorSampler() : domain(NULL) {}

  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
    {this->domain = domain.staticCast<VectorDomain>()->getElementsDomain().staticCast<ScalarVectorDomain>();}
  
  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    OVectorPtr res = vector(denseDoubleVectorClass(), 2 * domain->getNumDimensions());
    size_t count = 0;
    for (size_t d = 0; d < domain->getNumDimensions(); ++d) // for each dimension we need to create two samples
    {
      DenseDoubleVectorPtr sampleLowerbound = domain->sampleUniformly(context.getRandomGenerator());
      DenseDoubleVectorPtr sampleUpperbound = domain->sampleUniformly(context.getRandomGenerator());
      sampleLowerbound->setValue(d, domain->getLowerLimit(d));
      sampleUpperbound->setValue(d, domain->getUpperLimit(d));
      res->set(count++, sampleLowerbound);
      res->set(count++, sampleUpperbound);
    }
    return res;
  }
  
protected:
  friend class EdgeVectorSamplerClass;
  
  ScalarVectorDomainPtr domain;                         /**< Pointer to the domain in which we sample             */
};

}; /* namespace lbcpp */

#endif // !ML_SAMPLER_EDGE_VECTOR_H_

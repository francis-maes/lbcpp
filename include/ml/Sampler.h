/*-----------------------------------------.---------------------------------.
| Filename: Sampler.h                      | Sampler                         |
| Author  : Francis Maes                   |  (represents a distribution)    |
| Started : 22/09/2012 20:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SAMPLER_H_
# define ML_SAMPLER_H_

# include "Domain.h"
# include "SolutionContainer.h"

namespace lbcpp
{

class Sampler : public Object
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
    {}

  virtual ObjectPtr sample(ExecutionContext& context) const = 0;
  virtual bool isDeterministic() const // returns true if the sampler has became deterministic
    {return false;}

  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& objects)
    {jassertfalse;}

  virtual void learn(ExecutionContext& context, const SolutionVectorPtr& solutions)
    {jassertfalse;}

  virtual void reinforce(ExecutionContext& context, const ObjectPtr& object, double weight)
    {jassertfalse;}
};

extern SamplerPtr uniformScalarVectorSampler();
extern SamplerPtr diagonalGaussianSampler();
extern SamplerPtr diagonalGaussianDistributionSampler();

extern SamplerPtr binaryMixtureSampler(SamplerPtr sampler1, SamplerPtr sampler2, double probability = 0.5);
extern SamplerPtr subsetVectorSampler(SamplerPtr vectorSampler, size_t subsetSize);

class DecoratorSampler : public Sampler
{
public:
  DecoratorSampler(SamplerPtr sampler = SamplerPtr())
    : sampler(sampler) {}
  
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
    {sampler->initialize(context, domain);}

  virtual ObjectPtr sample(ExecutionContext& context) const
    {return sampler->sample(context);}

  virtual bool isDeterministic() const
    {return sampler->isDeterministic();}

  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& objects)
    {sampler->learn(context, objects);}

  virtual void learn(ExecutionContext& context, const SolutionVectorPtr& solutions)
    {sampler->learn(context, solutions);}

  virtual void reinforce(ExecutionContext& context, const ObjectPtr& object, double weight)
    {sampler->reinforce(context, object, weight);}

protected:
  friend class DecoratorSamplerClass;

  SamplerPtr sampler;
};

typedef ReferenceCountedObjectPtr<DecoratorSampler> DecoratorSamplerPtr;

}; /* namespace lbcpp */

#endif // !ML_SAMPLER_H_

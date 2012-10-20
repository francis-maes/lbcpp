/*-----------------------------------------.---------------------------------.
| Filename: ExpressionSampler.h            | Expression Sampler              |
| Author  : Francis Maes                   |                                 |
| Started : 04/10/2012 16:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_EXPRESSION_SAMPLER_H_
# define LBCPP_ML_EXPRESSION_SAMPLER_H_

# include "ExpressionDomain.h"
# include "Sampler.h"

namespace lbcpp
{

class ExpressionSampler : public Sampler
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
    {this->domain = domain.staticCast<ExpressionDomain>();}

protected:
  ExpressionDomainPtr domain;
};

typedef ReferenceCountedObjectPtr<ExpressionSampler> ExpressionSamplerPtr;

class DepthControlledExpressionSampler : public ExpressionSampler
{
public:
  DepthControlledExpressionSampler(size_t minDepth = 2, size_t maxDepth = 5)
    : minDepth(minDepth), maxDepth(maxDepth) {}
  
  virtual ExpressionPtr sampleTree(RandomGeneratorPtr random, size_t minDepth, size_t maxDepth) const = 0;

  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    ExpressionSampler::initialize(context, domain);
    terminals = this->domain->getTerminals();
  }
  
  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    ExpressionPtr res = sampleTree(random, minDepth, random->sampleSize(minDepth, maxDepth + 1));
    jassert(res->getDepth() >= minDepth && res->getDepth() <= maxDepth);
    return res;
  }

protected:
  friend class DepthControlledExpressionSamplerClass;

  size_t minDepth;
  size_t maxDepth;
  std::vector<ExpressionPtr> terminals;
};

typedef ReferenceCountedObjectPtr<DepthControlledExpressionSampler> DepthControlledExpressionSamplerPtr;

extern DepthControlledExpressionSamplerPtr fullExpressionSampler(size_t minDepth = 2, size_t maxDepth = 5);
extern DepthControlledExpressionSamplerPtr growExpressionSampler(size_t minDepth = 2, size_t maxDepth = 5);

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_SAMPLER_H_

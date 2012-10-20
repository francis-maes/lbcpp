/*-----------------------------------------.---------------------------------.
| Filename: ExpressionSampler.cpp          | Expression Sampler              |
| Author  : Francis Maes                   |                                 |
| Started : 20/10/2012 22:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp-ml/ExpressionSampler.h>
using namespace lbcpp;

/*
** ExpressionSampler
*/
void ExpressionSampler::initialize(ExecutionContext& context, const DomainPtr& domain)
  {this->domain = domain.staticCast<ExpressionDomain>();}

/*
** DepthControlledExpressionSampler
*/
DepthControlledExpressionSampler::DepthControlledExpressionSampler(size_t minDepth, size_t maxDepth)
  : minDepth(minDepth), maxDepth(maxDepth) {}
  
void DepthControlledExpressionSampler::initialize(ExecutionContext& context, const DomainPtr& domain)
{
  ExpressionSampler::initialize(context, domain);
  terminals = this->domain->getTerminals();
}
  
ObjectPtr DepthControlledExpressionSampler::sample(ExecutionContext& context) const
{
  RandomGeneratorPtr random = context.getRandomGenerator();
  ExpressionPtr res = sampleTree(random, minDepth, random->sampleSize(minDepth, maxDepth + 1));
  jassert(res->getDepth() >= minDepth && res->getDepth() <= maxDepth);
  return res;
}

/*
** ExpressionPerturbator
*/
void ExpressionPerturbator::initialize(ExecutionContext& context, const DomainPtr& domain)
  {this->domain = domain.staticCast<ExpressionDomain>();}

ObjectPtr ExpressionPerturbator::sample(ExecutionContext& context, const ObjectPtr& object) const
{
  const ExpressionPtr& expression = object.staticCast<Expression>();
  if (!expression->getNumSubNodes())
    return expression; // do not apply mutations on leaf nodes
  for (size_t attempt = 0; attempt < 2; ++attempt)
  {
    ExpressionPtr res = sampleExpression(context, expression);
    if (res)
      return res;
  }
  return expression;
}

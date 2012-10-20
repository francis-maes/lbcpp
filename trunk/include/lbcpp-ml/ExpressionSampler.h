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
# include "Perturbator.h"

namespace lbcpp
{

class ExpressionSampler : public Sampler
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain);

protected:
  ExpressionDomainPtr domain;
};

typedef ReferenceCountedObjectPtr<ExpressionSampler> ExpressionSamplerPtr;

class DepthControlledExpressionSampler : public ExpressionSampler
{
public:
  DepthControlledExpressionSampler(size_t minDepth = 2, size_t maxDepth = 5);
  
  virtual ExpressionPtr sampleTree(RandomGeneratorPtr random, size_t minDepth, size_t maxDepth) const = 0;

  virtual void initialize(ExecutionContext& context, const DomainPtr& domain);
  virtual ObjectPtr sample(ExecutionContext& context) const;

protected:
  friend class DepthControlledExpressionSamplerClass;

  size_t minDepth;
  size_t maxDepth;
  std::vector<ExpressionPtr> terminals;
};

typedef ReferenceCountedObjectPtr<DepthControlledExpressionSampler> DepthControlledExpressionSamplerPtr;

extern DepthControlledExpressionSamplerPtr fullExpressionSampler(size_t minDepth = 2, size_t maxDepth = 5);
extern DepthControlledExpressionSamplerPtr growExpressionSampler(size_t minDepth = 2, size_t maxDepth = 5);

class ExpressionPerturbator : public Perturbator
{
public:
  virtual ExpressionPtr sampleExpression(ExecutionContext& context, const ExpressionPtr& expression) const = 0;

  virtual void initialize(ExecutionContext& context, const DomainPtr& domain);
  virtual ObjectPtr sample(ExecutionContext& context, const ObjectPtr& object) const;

protected:
  ExpressionDomainPtr domain;
};

typedef ReferenceCountedObjectPtr<ExpressionPerturbator> ExpressionPerturbatorPtr;

extern ExpressionPerturbatorPtr shrinkExpressionPerturbator();
extern ExpressionPerturbatorPtr swapExpressionPerturbator(double functionSelectionProbability);
extern ExpressionPerturbatorPtr insertExpressionPerturbator(size_t maxDepth = 17);
extern ExpressionPerturbatorPtr kozaExpressionPerturbator(DepthControlledExpressionSamplerPtr sampler, size_t maxRegenerationDepth = 5, size_t maxDepth = 17);
extern BinaryPerturbatorPtr subTreeCrossOverExpressionPerturbator(double functionSelectionProbability, size_t maxDepth = 17);

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_SAMPLER_H_

/*-----------------------------------------.---------------------------------.
| Filename: Sampler.h                      | Sampler base class              |
| Author  : Francis Maes                   |                                 |
| Started : 13/05/2011 16:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_H_
# define LBCPP_SAMPLER_H_

# include "../Data/RandomGenerator.h"
# include "../Core/Variable.h"
# include "../Core/Vector.h"

namespace lbcpp
{ 

/*
** Sampler Base Class
*/
class Sampler : public Object
{
public:
  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const = 0;

  virtual void learn(ExecutionContext& context, const std::vector<std::pair<Variable, Variable> >& dataset) = 0;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class SamplerClass;
};
typedef ReferenceCountedObjectPtr<Sampler> SamplerPtr;

/*
** Continuous Sampler
*/
class ContinuousSampler : public Sampler
{
public:
  virtual double computeExpectation(const Variable* inputs = NULL) const
    {jassert(false); return 0.0;}

  lbcpp_UseDebuggingNewOperator
};
typedef ReferenceCountedObjectPtr<ContinuousSampler> ContinuousSamplerPtr;

extern ContinuousSamplerPtr gaussianSampler(double mean, double stddev);

/*
** Discrete Sampler
*/
class DiscreteSampler : public Sampler
{
public:
  lbcpp_UseDebuggingNewOperator
};
typedef ReferenceCountedObjectPtr<DiscreteSampler> DiscreteSamplerPtr;

/*
** Composite Sampler
*/
class CompositeSampler : public Sampler
{
public:
  CompositeSampler(size_t numSamplers)
    : samplers(numSamplers) {}
  CompositeSampler() {}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class CompositeSamplerClass;

  std::vector<SamplerPtr> samplers;
};

typedef ReferenceCountedObjectPtr<CompositeSampler> CompositeSamplerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_H_

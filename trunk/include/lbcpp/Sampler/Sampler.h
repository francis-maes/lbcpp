/*-----------------------------------------.---------------------------------.
| Filename: Sampler.h                      | Sampler base class              |
| Author  : Francis Maes                   |                                 |
| Started : 13/05/2011 16:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_H_
# define LBCPP_SAMPLER_H_

# include "../Data/DoubleVector.h"
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

  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset) = 0;

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

extern ContinuousSamplerPtr gaussianSampler(double mean = 0.0, double stddev = 1.0);

/*
** Discrete Sampler
*/
class DiscreteSampler : public Sampler
{
public:
  lbcpp_UseDebuggingNewOperator
};
typedef ReferenceCountedObjectPtr<DiscreteSampler> DiscreteSamplerPtr;

extern DiscreteSamplerPtr enumerationSampler(EnumerationPtr enumeration);
extern DiscreteSamplerPtr enumerationSampler(const DenseDoubleVectorPtr& probabilities);

/*
** Composite Sampler
*/
class CompositeSampler : public Sampler
{
public:
  CompositeSampler(size_t numSamplers)
    : samplers(numSamplers) {}
  CompositeSampler(const std::vector<SamplerPtr>& samplers)
    : samplers(samplers) {}
  CompositeSampler() {}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class CompositeSamplerClass;

  std::vector<SamplerPtr> samplers;
};

typedef ReferenceCountedObjectPtr<CompositeSampler> CompositeSamplerPtr;

extern CompositeSamplerPtr objectCompositeSampler(ClassPtr objectClass, const SamplerPtr& firstVariableSampler);
extern CompositeSamplerPtr objectCompositeSampler(ClassPtr objectClass, const SamplerPtr& firstVariableSampler, const SamplerPtr& secondVariableSampler);
extern CompositeSamplerPtr objectCompositeSampler(ClassPtr objectClass, const SamplerPtr& firstVariableSampler, const SamplerPtr& secondVariableSampler, const SamplerPtr& thirdVariableSampler);
extern CompositeSamplerPtr objectCompositeSampler(ClassPtr objectClass, const std::vector<SamplerPtr>& variableSamplers);

extern CompositeSamplerPtr independentDoubleVectorSampler(EnumerationPtr elementsEnumeration, SamplerPtr elementSamplerModel);
extern CompositeSamplerPtr independentDoubleVectorSampler(size_t numElements, SamplerPtr elementSamplerModel);
extern CompositeSamplerPtr independentDoubleMatrixSampler(size_t numRows, size_t numColumns, SamplerPtr elementSamplerModel);

extern CompositeSamplerPtr mixtureSampler(const DenseDoubleVectorPtr& probabilities, const std::vector<SamplerPtr>& samplers);

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_H_

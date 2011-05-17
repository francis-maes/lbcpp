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
# include "../Data/Matrix.h"

namespace lbcpp
{ 

/*
** Sampler Base Class
*/
class Sampler : public Object
{
public:
  virtual Variable computeExpectation(const Variable* inputs = NULL) const
    {jassert(false); return Variable();}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const = 0;

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, 
                                                const ContainerPtr& validationInputs = ContainerPtr(), const ContainerPtr& validationSamples = ContainerPtr())
    {jassert(false);}

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
  lbcpp_UseDebuggingNewOperator
};
typedef ReferenceCountedObjectPtr<ContinuousSampler> ContinuousSamplerPtr;

class ScalarContinuousSampler : public ContinuousSampler
{
public:

  virtual void computeProbabilities(const ContainerPtr& data, DoubleMatrixPtr& probabilities, size_t numColumnToFill) const = 0;
  virtual void updateParameters(const ContainerPtr& data, const DoubleMatrixPtr& probabilitiesForAllModels, size_t numColumn) = 0;

  lbcpp_UseDebuggingNewOperator
};
typedef ReferenceCountedObjectPtr<ScalarContinuousSampler> ScalarContinuousSamplerPtr;

extern ScalarContinuousSamplerPtr uniformScalarSampler(double minValue = 0.0, double maxValue = 1.0);
extern ScalarContinuousSamplerPtr gaussianSampler(double mean = 0.0, double stddev = 1.0);

/*
** Discrete Sampler
*/
class DiscreteSampler : public Sampler
{
public:
  lbcpp_UseDebuggingNewOperator
};
typedef ReferenceCountedObjectPtr<DiscreteSampler> DiscreteSamplerPtr;

extern DiscreteSamplerPtr bernoulliSampler(double probability, double minProbability = 0.0, double maxProbability = 1.0);
extern DiscreteSamplerPtr enumerationSampler(EnumerationPtr enumeration);
extern DiscreteSamplerPtr enumerationSampler(const DenseDoubleVectorPtr& probabilities);

extern DiscreteSamplerPtr discretizeSampler(const ContinuousSamplerPtr& sampler, int minValue = INT_MIN, int maxValue = INT_MAX);

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

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, 
                                                const ContainerPtr& validationInputs, const ContainerPtr& validationSamples);

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class CompositeSamplerClass;

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples, std::vector<ContainerPtr>& subInputs, std::vector<ContainerPtr>& subSamples) const
    {jassert(false);}

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

extern CompositeSamplerPtr zeroOrScalarContinuousSampler(DiscreteSamplerPtr equalZeroSampler, ScalarContinuousSamplerPtr scalarSampler);

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_H_

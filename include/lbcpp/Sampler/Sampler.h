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

class Sampler;
typedef ReferenceCountedObjectPtr<Sampler> SamplerPtr;
class ContinuousSampler;
typedef ReferenceCountedObjectPtr<ContinuousSampler> ContinuousSamplerPtr;

/*
** Sampler Base Class
*/
class Sampler : public Object
{
public:
  virtual Variable computeExpectation(const Variable* inputs = NULL) const
    {jassert(false); return Variable();}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const = 0;

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights = DenseDoubleVectorPtr(),
                                                const ContainerPtr& validationInputs = ContainerPtr(), const ContainerPtr& validationSamples = ContainerPtr(), const DenseDoubleVectorPtr& validationWeights = DenseDoubleVectorPtr())
    {jassert(false);}

  virtual DenseDoubleVectorPtr computeProbabilities(const ContainerPtr& inputs, const ContainerPtr& samples) const
    {jassert(false); return DenseDoubleVectorPtr();}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class SamplerClass;
};

class ConstantSampler : public Sampler
{
public:
  ConstantSampler(const Variable& value = Variable())
    : value(value) {}
 
  virtual Variable computeExpectation(const Variable* inputs = NULL) const;
  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const;
  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights = DenseDoubleVectorPtr(),
                                                const ContainerPtr& validationInputs = ContainerPtr(), const ContainerPtr& validationSamples = ContainerPtr(), const DenseDoubleVectorPtr& validationWeights = DenseDoubleVectorPtr());
  virtual DenseDoubleVectorPtr computeProbabilities(const ContainerPtr& inputs, const ContainerPtr& samples) const;

protected:
  friend class ConstantSamplerClass;
  Variable value;
};

extern SamplerPtr constantSampler(const Variable& value);

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

extern DiscreteSamplerPtr maximumEntropySampler(TypePtr outputType);

/*
** Continuous Sampler
*/
class ContinuousSampler : public Sampler
{
public:
  lbcpp_UseDebuggingNewOperator
};

extern ContinuousSamplerPtr multiVariateGaussianSampler(const DoubleMatrixPtr& initialMean, const DoubleMatrixPtr& initialStdDev);

class ScalarContinuousSampler : public ContinuousSampler
{
public:
  lbcpp_UseDebuggingNewOperator
};
typedef ReferenceCountedObjectPtr<ScalarContinuousSampler> ScalarContinuousSamplerPtr;

extern ScalarContinuousSamplerPtr uniformScalarSampler(double minValue = 0.0, double maxValue = 1.0);
extern ScalarContinuousSamplerPtr gaussianSampler(double mean = 0.0, double stddev = 1.0);

extern ScalarContinuousSamplerPtr conditionalGaussianSampler();

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

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
                                                  const ContainerPtr& validationInputs, const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& supervisionWeights);

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class CompositeSamplerClass;

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples, const DenseDoubleVectorPtr& weights, std::vector<ContainerPtr>& subInputs, std::vector<ContainerPtr>& subSamples, std::vector<ContainerPtr>& subWeights) const
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

/*
** DecoratorSampler
*/
class DecoratorSampler : public Sampler
{
public:
  DecoratorSampler(SamplerPtr sampler)
    : sampler(sampler) {}
  DecoratorSampler() {}
 
  virtual Variable computeExpectation(const Variable* inputs = NULL) const;
  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const;
  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights = DenseDoubleVectorPtr(),
                                                const ContainerPtr& validationInputs = ContainerPtr(), const ContainerPtr& validationSamples = ContainerPtr(), const DenseDoubleVectorPtr& validationWeights = DenseDoubleVectorPtr());

  virtual DenseDoubleVectorPtr computeProbabilities(const ContainerPtr& inputs, const ContainerPtr& samples) const;

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

protected:
  friend class DecoratorSamplerClass;

  SamplerPtr sampler;
};

typedef ReferenceCountedObjectPtr<DecoratorSampler> DecoratorSamplerPtr;

extern DecoratorSamplerPtr rejectionSampler(SamplerPtr sampler, FunctionPtr predicate);

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_H_

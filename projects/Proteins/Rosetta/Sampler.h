/*-----------------------------------------.---------------------------------.
| Filename: Sampler.h                      | Sampler                         |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 30 avr. 2011  10:41:07         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_SAMPLER_H_

# include "precompiled.h"

namespace lbcpp
{
class Sampler;
typedef ReferenceCountedObjectPtr<Sampler> SamplerPtr;

class Sampler: public Object
{
public:
  Sampler() :
    Object()
  {
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const=0;
  virtual void learn(ExecutionContext& context, const RandomGeneratorPtr& random,
      const std::vector<std::pair<Variable, Variable> >& dataset)=0;
  virtual SamplerPtr clone()=0;

protected:
  friend class SamplerClass;
};

class ContinuousSampler;
typedef ReferenceCountedObjectPtr<ContinuousSampler> ContinuousSamplerPtr;

class ContinuousSampler: public Sampler
{
public:
  ContinuousSampler() :
    mean(0), std(1)
  {
  }

  ContinuousSampler(double mean, double std) :
    mean(mean), std(std)
  {
  }

  /**
   * dataset = first : a Variable of double type containing the data observed.
   *           second : not yet used.
   */
  double getMean(const std::vector<std::pair<Variable, Variable> >& dataset)
  {
    double temporaryMean = 0;
    if (dataset.size() > 0)
    {
      for (int i = 0; i < dataset.size(); i++)
        temporaryMean += dataset[i].first.getDouble();
      temporaryMean = temporaryMean / dataset.size();
    }
    return temporaryMean;
  }

  /**
   * dataset = first : a Variable of double type containing the data observed.
   *           second : not yet used.
   */
  double getVariance(const std::vector<std::pair<Variable, Variable> >& dataset, double mean)
  {
    double temporaryVariance = 0;
    if (dataset.size() > 1)
    {
      for (int i = 0; i < dataset.size(); i++)
        temporaryVariance += std::pow(dataset[i].first.getDouble() - mean, 2);
      temporaryVariance = temporaryVariance / dataset.size();
    }
    if (temporaryVariance <= 0)
      temporaryVariance = 0.3 * mean;
    return temporaryVariance;
  }

protected:
  friend class ContinuousSamplerClass;
  double mean;
  double std;
};

class DiscreteSampler;
typedef ReferenceCountedObjectPtr<DiscreteSampler> DiscreteSamplerPtr;

class DiscreteSampler: public Sampler
{
public:

protected:
  friend class DiscreteSamplerClass;
};

class SequentialSampler;
typedef ReferenceCountedObjectPtr<SequentialSampler> SequentialSamplerPtr;

class SequentialSampler: public Sampler
{
public:

protected:
  friend class SequentialSamplerClass;
  std::vector<Variable> samplers; // each Variable contains a pointer to the corresponding sampler
};

class CompositeSampler;
typedef ReferenceCountedObjectPtr<CompositeSampler> CompositeSamplerPtr;

class CompositeSampler: public Sampler
{
public:
  CompositeSampler() :
    Sampler()
  {
  }

  CompositeSampler(size_t numSamplers) :
    Sampler()
  {
    sons = std::vector<Variable>(numSamplers);
  }

protected:
  friend class CompositeSamplerClass;
  std::vector<Variable> sons; // each Variable contains a pointer to the corresponding sampler
};

extern SamplerPtr gaussianContinuousSampler();
extern SamplerPtr enumerationDiscreteSampler();
extern SamplerPtr parzenContinuousSampler();
extern SamplerPtr proteinMoverSampler();
extern SamplerPtr phiPsiMoverSampler();
extern SamplerPtr shearMoverSampler();
extern SamplerPtr rigidBodyTransMoverSampler();
extern SamplerPtr rigidBodySpinMoverSampler();
extern SamplerPtr rigidBodyGeneralMoverSampler();
extern SamplerPtr simpleResidueSampler();

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_SAMPLER_H_

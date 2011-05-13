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
# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

class DiscreteSampler;
typedef ReferenceCountedObjectPtr<DiscreteSampler> DiscreteSamplerPtr;

class DiscreteSampler : public Sampler
{
public:

protected:
  friend class DiscreteSamplerClass;
};

class CompositeSampler;
typedef ReferenceCountedObjectPtr<CompositeSampler> CompositeSamplerPtr;

class CompositeSampler : public Sampler
{
public:
  CompositeSampler(size_t numSamplers)
    : sons(numSamplers)
  {
  }

  CompositeSampler()
    : Sampler()
  {
  }


protected:
  friend class CompositeSamplerClass;
  std::vector<SamplerPtr> sons; // each Variable contains a pointer to the corresponding sampler
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
extern SamplerPtr gaussianMultivariateSampler();

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_SAMPLER_H_

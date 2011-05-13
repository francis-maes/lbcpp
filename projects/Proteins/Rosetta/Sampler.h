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

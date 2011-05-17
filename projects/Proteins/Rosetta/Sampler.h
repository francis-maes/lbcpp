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

extern CompositeSamplerPtr residuePairSampler(size_t numResidues);
extern CompositeSamplerPtr simpleResidueSampler(size_t numResidues);

extern CompositeSamplerPtr proteinMoverSampler(size_t numResidues);
extern CompositeSamplerPtr proteinMoverSampler(DiscreteSamplerPtr classSampler, size_t numResidues);

extern ScalarContinuousSamplerPtr multiVariateGaussianSampler(const DoubleMatrixPtr& initialMean, const DoubleMatrixPtr& initialStdDev);

extern SamplerPtr parzenContinuousSampler();
extern SamplerPtr gaussianMultivariateSampler();


}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_SAMPLER_H_

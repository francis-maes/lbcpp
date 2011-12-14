/*-----------------------------------------.---------------------------------.
| Filename: ProteinMoverSampler.h          | ProteinMoverSampler             |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 14, 2011  9:12:04 AM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_PROTEINMOVERSAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_PROTEINMOVERSAMPLER_H_

# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

extern CompositeSamplerPtr residuePairSampler(size_t numResidues);
extern CompositeSamplerPtr simpleResidueSampler(size_t numResidues);

extern CompositeSamplerPtr proteinMoverSampler(size_t numResidues);
extern CompositeSamplerPtr proteinMoverSampler(DiscreteSamplerPtr classSampler, size_t numResidues);

extern SamplerPtr gaussianMultivariateSampler();


}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_PROTEINMOVERSAMPLER_H_

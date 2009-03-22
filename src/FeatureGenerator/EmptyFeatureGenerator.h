/*-----------------------------------------.---------------------------------.
| Filename: EmptyFeatureGenerator.h        | The Empty Feature Generator     |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2009 17:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_FEATURE_GENERATOR_EMPTY_H_
# define CRALGO_FEATURE_GENERATOR_EMPTY_H_

# include <cralgo/FeatureGenerator.h>
# include <cralgo/impl/Bridge/FeatureGeneratorDefaultImplementations.hpp>

namespace cralgo
{

class EmptyFeatureGenerator
  : public FeatureGeneratorDefaultImplementations<EmptyFeatureGenerator, FlatFeatureGenerator>
{
public:
  template<class VisitorType>
  void staticFeatureGenerator(VisitorType& visitor, FeatureDictionaryPtr dictionary) const
    {}
    
  FeatureDictionaryPtr getDictionary() const
    {return FeatureDictionaryPtr();} // can be combined with any other dictionary
};

}; /* namespace cralgo */

#endif // !CRALGO_FEATURE_GENERATOR_EMPTY_H_

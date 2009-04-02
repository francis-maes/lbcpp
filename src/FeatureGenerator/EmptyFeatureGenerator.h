/*-----------------------------------------.---------------------------------.
| Filename: EmptyFeatureGenerator.h        | The Empty Feature Generator     |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2009 17:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LCPP_FEATURE_GENERATOR_EMPTY_H_
# define LCPP_FEATURE_GENERATOR_EMPTY_H_

# include <lcpp/FeatureGenerator.h>
# include <lcpp/impl/Bridge/FeatureGeneratorDefaultImplementations.hpp>

namespace lcpp
{

class EmptyFeatureGenerator
  : public FeatureGeneratorDefaultImplementations<EmptyFeatureGenerator, FlatFeatureGenerator>
{
public:
  template<class VisitorType>
  void staticFeatureGenerator(VisitorType& visitor) const
    {}
    
  FeatureDictionaryPtr getDictionary() const
    {return FeatureDictionaryPtr();} // can be combined with any other dictionary
};

}; /* namespace lcpp */

#endif // !LCPP_FEATURE_GENERATOR_EMPTY_H_

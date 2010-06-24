/*-----------------------------------------.---------------------------------.
| Filename: EmptyFeatureGenerator.h        | The Empty Feature Generator     |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2009 17:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_EMPTY_H_
# define LBCPP_FEATURE_GENERATOR_EMPTY_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include <lbcpp/impl/Bridge/FeatureGeneratorDefaultImplementations.hpp>

namespace lbcpp
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

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_EMPTY_H_

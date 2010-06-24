/*-----------------------------------------.---------------------------------.
| Filename: UnitFeatureGenerator.h         | The Unit Feature Generator      |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2009 17:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_UNIT_H_
# define LBCPP_FEATURE_GENERATOR_UNIT_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include <lbcpp/FeatureGenerator/impl/FeatureGeneratorDefaultImplementations.hpp>

namespace lbcpp
{

class UnitFeatureGenerator : public FeatureGeneratorDefaultImplementations<UnitFeatureGenerator, FlatFeatureGenerator>
{
public:
  template<class VisitorType>
  void staticFeatureGenerator(VisitorType& visitor) const
    {visitor.featureSense_(getDictionary(), (size_t)0);}
    
  FeatureDictionaryPtr getDictionary() const
  {
    static FeatureDictionaryPtr dictionary;
    if (!dictionary)
      dictionary = FeatureDictionaryManager::getInstance().getOrCreateRootDictionary(T("unit"), true, false);
    return dictionary;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_UNIT_H_

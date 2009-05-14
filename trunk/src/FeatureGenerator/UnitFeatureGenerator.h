/*-----------------------------------------.---------------------------------.
| Filename: UnitFeatureGenerator.h         | The Unit Feature Generator      |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2009 17:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_UNIT_H_
# define LBCPP_FEATURE_GENERATOR_UNIT_H_

# include <lbcpp/FeatureGenerator.h>
# include <lbcpp/impl/Bridge/FeatureGeneratorDefaultImplementations.hpp>

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
    static FeatureDictionaryPtr dictionary = createDictionary();
    return dictionary;
  }
  
private:
  static FeatureDictionaryPtr createDictionary()
  {
    FeatureDictionaryPtr res = new FeatureDictionary("unit");
    res->getFeatures()->add("unit");
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_UNIT_H_

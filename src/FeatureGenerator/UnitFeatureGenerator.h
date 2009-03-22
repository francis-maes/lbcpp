/*-----------------------------------------.---------------------------------.
| Filename: UnitFeatureGenerator.h         | The Unit Feature Generator      |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2009 17:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_FEATURE_GENERATOR_UNIT_H_
# define CRALGO_FEATURE_GENERATOR_UNIT_H_

# include <cralgo/FeatureGenerator.h>
# include <cralgo/impl/Bridge/FeatureGeneratorDefaultImplementations.hpp>

namespace cralgo
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

}; /* namespace cralgo */

#endif // !CRALGO_FEATURE_GENERATOR_UNIT_H_

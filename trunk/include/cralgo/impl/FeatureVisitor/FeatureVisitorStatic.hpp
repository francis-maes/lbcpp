/*-----------------------------------------.---------------------------------.
| Filename: FeatureVisitorStatic.hpp       | Static Feature Visitor          |
| Author  : Francis Maes                   |   Base Class                    |
| Started : 27/02/2009 18:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_FEATURE_VISITOR_STATIC_H_
# define CRALGO_FEATURE_VISITOR_STATIC_H_

# include "../../FeatureGenerator.h"
# include "../../EditableFeatureGenerator.h"
# include "../Object.hpp"

namespace cralgo {
namespace impl {

template<class ExactType>
struct FeatureVisitor : public Object<ExactType>
{
  // override these functions:
  bool featureEnter(cralgo::FeatureDictionaryPtr dictionary, size_t number)
    {return false;}
  void featureSense(cralgo::FeatureDictionaryPtr dictionary, size_t number, double value = 1.0)
    {}
  void featureLeave()
    {}
    
  void featureCall(cralgo::FeatureDictionaryPtr dictionary, cralgo::FeatureGeneratorPtr featureGenerator);  
  void featureCall(cralgo::FeatureDictionaryPtr dictionary, size_t number, cralgo::FeatureGeneratorPtr featureGenerator)
  {
    if (_this().featureEnter(dictionary, number))
    {
      _this().featureCall(featureGenerator->getDictionary(), featureGenerator);
      _this().featureLeave();
    }
  }

  // conversion functions
  bool featureEnter_(cralgo::FeatureDictionaryPtr dictionary, const std::string& scopeName)
    {return _this().featureEnter(dictionary, dictionary->getScopes()->add(scopeName));}
    
  bool featureEnter_(cralgo::FeatureDictionaryPtr dictionary, const char* scopeName)
    {return _this().featureEnter(dictionary, dictionary->getScopes()->add(scopeName));}
    
  bool featureEnter_(cralgo::FeatureDictionaryPtr dictionary, size_t number)
    {return _this().featureEnter(dictionary, number);}
  
  void featureSense_(cralgo::FeatureDictionaryPtr dictionary, const std::string& featureName, double value = 1.0)
    {_this().featureSense(dictionary, dictionary->getFeatures()->add(featureName), value);}
    
  void featureSense_(cralgo::FeatureDictionaryPtr dictionary, const char* featureName, double value = 1.0)
    {_this().featureSense(dictionary, dictionary->getFeatures()->add(featureName), value);}
    
  void featureSense_(cralgo::FeatureDictionaryPtr dictionary, size_t number, double value = 1.0)
    {_this().featureSense(dictionary, number, value);}
  
  void featureLeave_()
    {_this().featureLeave();}
  
protected:
  ExactType& _this() {return *static_cast<ExactType* >(this);}
};

}; /* namespace cralgo */
}; /* namespace impl */

#endif // !CRALGO_FEATURE_VISITOR_STATIC_H_

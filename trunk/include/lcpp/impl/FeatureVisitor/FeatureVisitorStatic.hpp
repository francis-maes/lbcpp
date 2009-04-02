/*-----------------------------------------.---------------------------------.
| Filename: FeatureVisitorStatic.hpp       | Static Feature Visitor          |
| Author  : Francis Maes                   |   Base Class                    |
| Started : 27/02/2009 18:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LCPP_FEATURE_VISITOR_STATIC_H_
# define LCPP_FEATURE_VISITOR_STATIC_H_

# include "../../FeatureGenerator.h"
# include "../../EditableFeatureGenerator.h"
# include "../Object.hpp"

namespace lcpp {
namespace impl {

template<class ExactType>
struct FeatureVisitor : public Object<ExactType>
{
  // override these functions:
  bool featureEnter(lcpp::FeatureDictionaryPtr dictionary, size_t number)
    {return false;}
  void featureSense(lcpp::FeatureDictionaryPtr dictionary, size_t number, double value = 1.0)
    {}
  void featureLeave()
    {}
    
  void featureCall(lcpp::FeatureDictionaryPtr dictionary, lcpp::FeatureGeneratorPtr featureGenerator);  
  void featureCall(lcpp::FeatureDictionaryPtr dictionary, size_t number, lcpp::FeatureGeneratorPtr featureGenerator)
  {
    if (_this().featureEnter(dictionary, number))
    {
      _this().featureCall(featureGenerator->getDictionary(), featureGenerator);
      _this().featureLeave();
    }
  }

  // conversion functions
  bool featureEnter_(lcpp::FeatureDictionaryPtr dictionary, const std::string& scopeName)
    {return _this().featureEnter(dictionary, dictionary->getScopes()->add(scopeName));}
    
  bool featureEnter_(lcpp::FeatureDictionaryPtr dictionary, const char* scopeName)
    {return _this().featureEnter(dictionary, dictionary->getScopes()->add(scopeName));}
    
  bool featureEnter_(lcpp::FeatureDictionaryPtr dictionary, size_t number)
    {return _this().featureEnter(dictionary, number);}
  
  void featureSense_(lcpp::FeatureDictionaryPtr dictionary, const std::string& featureName, double value = 1.0)
    {_this().featureSense(dictionary, dictionary->getFeatures()->add(featureName), value);}
    
  void featureSense_(lcpp::FeatureDictionaryPtr dictionary, const char* featureName, double value = 1.0)
    {_this().featureSense(dictionary, dictionary->getFeatures()->add(featureName), value);}
    
  void featureSense_(lcpp::FeatureDictionaryPtr dictionary, size_t number, double value = 1.0)
    {_this().featureSense(dictionary, number, value);}
  
  void featureLeave_()
    {_this().featureLeave();}
  
protected:
  ExactType& _this() {return *static_cast<ExactType* >(this);}
};

}; /* namespace lcpp */
}; /* namespace impl */

#endif // !LCPP_FEATURE_VISITOR_STATIC_H_

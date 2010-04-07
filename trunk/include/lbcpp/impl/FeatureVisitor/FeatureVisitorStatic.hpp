/*-----------------------------------------.---------------------------------.
| Filename: FeatureVisitorStatic.hpp       | Static Feature Visitor          |
| Author  : Francis Maes                   |   Base Class                    |
| Started : 27/02/2009 18:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_FEATURE_VISITOR_STATIC_H_
# define LBCPP_FEATURE_VISITOR_STATIC_H_

# include "../../FeatureGenerator.h"
# include "../../EditableFeatureGenerator.h"
# include "../Object.hpp"

namespace lbcpp {
namespace impl {

template<class ExactType>
struct FeatureVisitor : public Object<ExactType>
{
  // override these functions:
  bool featureEnter(lbcpp::FeatureDictionaryPtr dictionary, size_t number, lbcpp::FeatureDictionaryPtr subDictionary)
    {return false;}
  void featureSense(lbcpp::FeatureDictionaryPtr dictionary, size_t number, double value = 1.0)
    {}
  void featureLeave()
    {}
  void featureCall(lbcpp::FeatureDictionaryPtr dictionary, size_t scopeNumber, lbcpp::FeatureGeneratorPtr featureGenerator);

  /*
  ** Conversion functions
  ** These are the functions called by the generated code
  */
  bool featureEnter_(lbcpp::FeatureDictionaryPtr dictionary, const String& scopeName, lbcpp::FeatureDictionaryPtr subDictionary)
    {return _this().featureEnter(dictionary, dictionary->getScopes()->add(scopeName), subDictionary);}
    
  bool featureEnter_(lbcpp::FeatureDictionaryPtr dictionary, const char* scopeName, lbcpp::FeatureDictionaryPtr subDictionary)
    {return _this().featureEnter(dictionary, dictionary->getScopes()->add(scopeName), subDictionary);}
    
  bool featureEnter_(lbcpp::FeatureDictionaryPtr dictionary, size_t number, lbcpp::FeatureDictionaryPtr subDictionary)
    {return _this().featureEnter(dictionary, number, subDictionary);}
  
  void featureSense_(lbcpp::FeatureDictionaryPtr dictionary, const String& featureName, double value = 1.0)
    {_this().featureSense(dictionary, dictionary->getFeatures()->add(featureName), value);}
    
  void featureSense_(lbcpp::FeatureDictionaryPtr dictionary, const char* featureName, double value = 1.0)
    {_this().featureSense(dictionary, dictionary->getFeatures()->add(featureName), value);}
    
  void featureSense_(lbcpp::FeatureDictionaryPtr dictionary, size_t number, double value = 1.0)
    {_this().featureSense(dictionary, number, value);}
  
  void featureLeave_()
    {_this().featureLeave();}

  void featureCall_(lbcpp::FeatureDictionaryPtr dictionary, size_t scopeIndex, lbcpp::FeatureGeneratorPtr featureGenerator)
    {return _this().featureCall(dictionary, scopeIndex, featureGenerator);}

  void featureCall_(lbcpp::FeatureDictionaryPtr dictionary, const String& scopeName, lbcpp::FeatureGeneratorPtr featureGenerator)
    {return _this().featureCall(dictionary, dictionary->getScopes()->add(scopeName), featureGenerator);}
    
  void featureCall_(lbcpp::FeatureDictionaryPtr dictionary, const char* scopeName, lbcpp::FeatureGeneratorPtr featureGenerator)
    {return _this().featureCall(dictionary, dictionary->getScopes()->add(scopeName), featureGenerator);}
  
protected:
  ExactType& _this() {return *static_cast<ExactType* >(this);}
};

}; /* namespace lbcpp */
}; /* namespace impl */

#endif // !LBCPP_FEATURE_VISITOR_STATIC_H_

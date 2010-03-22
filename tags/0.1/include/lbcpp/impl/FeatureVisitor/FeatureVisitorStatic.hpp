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
  bool featureEnter(lbcpp::FeatureDictionaryPtr dictionary, size_t number)
    {return false;}
  void featureSense(lbcpp::FeatureDictionaryPtr dictionary, size_t number, double value = 1.0)
    {}
  void featureLeave()
    {}
    
  void featureCall(lbcpp::FeatureDictionaryPtr dictionary, lbcpp::FeatureGeneratorPtr featureGenerator);  
  void featureCall(lbcpp::FeatureDictionaryPtr dictionary, size_t number, lbcpp::FeatureGeneratorPtr featureGenerator)
  {
    if (_this().featureEnter(dictionary, number))
    {
      _this().featureCall(featureGenerator->getDictionary(), featureGenerator);
      _this().featureLeave();
    }
  }

  // conversion functions
  bool featureEnter_(lbcpp::FeatureDictionaryPtr dictionary, const std::string& scopeName)
    {return _this().featureEnter(dictionary, dictionary->getScopes()->add(scopeName));}
    
  bool featureEnter_(lbcpp::FeatureDictionaryPtr dictionary, const char* scopeName)
    {return _this().featureEnter(dictionary, dictionary->getScopes()->add(scopeName));}
    
  bool featureEnter_(lbcpp::FeatureDictionaryPtr dictionary, size_t number)
    {return _this().featureEnter(dictionary, number);}
  
  void featureSense_(lbcpp::FeatureDictionaryPtr dictionary, const std::string& featureName, double value = 1.0)
    {_this().featureSense(dictionary, dictionary->getFeatures()->add(featureName), value);}
    
  void featureSense_(lbcpp::FeatureDictionaryPtr dictionary, const char* featureName, double value = 1.0)
    {_this().featureSense(dictionary, dictionary->getFeatures()->add(featureName), value);}
    
  void featureSense_(lbcpp::FeatureDictionaryPtr dictionary, size_t number, double value = 1.0)
    {_this().featureSense(dictionary, number, value);}
  
  void featureLeave_()
    {_this().featureLeave();}
  
protected:
  ExactType& _this() {return *static_cast<ExactType* >(this);}
};

}; /* namespace lbcpp */
}; /* namespace impl */

#endif // !LBCPP_FEATURE_VISITOR_STATIC_H_

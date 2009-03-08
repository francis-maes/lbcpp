/*-----------------------------------------.---------------------------------.
| Filename: StaticInterface.h              | Specification of the static     |
| Author  : Francis Maes                   |   interface                     |
| Started : 01/03/2009 21:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_BRIDGE_STATIC_INTERFACE_HPP_
# define CRALGO_BRIDGE_STATIC_INTERFACE_HPP_

namespace cralgo
{

template<class ExactType>
struct StaticFeatureVisitor
{
  // override these functions:
  bool featureEnter(cralgo::FeatureDictionary& dictionary, size_t number)
    {return false;}
  void featureSense(cralgo::FeatureDictionary& dictionary, size_t number, double value = 1.0)
    {}
  void featureCall(cralgo::FeatureDictionary& dictionary, cralgo::FeatureGeneratorPtr featureGenerator);
  
  void featureCall(cralgo::FeatureDictionary& dictionary, size_t number, cralgo::FeatureGeneratorPtr featureGenerator)
  {
    if (_this().featureEnter(dictionary, number))
    {
      _this().featureCall(featureGenerator->getDefaultDictionary(), featureGenerator);
      _this().featureLeave();
    }
  }
  
  void featureLeave()
    {}

  // conversion functions
  bool featureEnter_(cralgo::FeatureDictionary& dictionary, const std::string& scopeName)
    {return _this().featureEnter(dictionary, dictionary.getScopes().add(scopeName));}
    
  bool featureEnter_(cralgo::FeatureDictionary& dictionary, const char* scopeName)
    {return _this().featureEnter(dictionary, dictionary.getScopes().add(scopeName));}
    
  bool featureEnter_(cralgo::FeatureDictionary& dictionary, size_t number)
    {return _this().featureEnter(dictionary, number);}
  
  void featureSense_(cralgo::FeatureDictionary& dictionary, const std::string& featureName, double value = 1.0)
    {_this().featureSense(dictionary, dictionary.getFeatures().add(featureName), value);}
    
  void featureSense_(cralgo::FeatureDictionary& dictionary, const char* featureName, double value = 1.0)
    {_this().featureSense(dictionary, dictionary.getFeatures().add(featureName), value);}
    
  void featureSense_(cralgo::FeatureDictionary& dictionary, size_t number, double value = 1.0)
    {_this().featureSense(dictionary, number, value);}
  
  void featureLeave_()
    {_this().featureLeave();}
  
protected:
  ExactType& _this() {return *static_cast<ExactType* >(this);}
};

struct StaticCallback
{
  template<class ScopeType>
  void enterLocalScope(size_t scopeNumber, ScopeType& scope) {}
  
  void leaveLocalScope() {}

  template<class ContainerType, class ChooseType>
  void choose(const ContainerType& choices, const ChooseType& dummy) {}
  
  void reward(double r) {}
};

/* TODO

struct StaticPolicy : public StaticCallback
{
  template<class CRAlgorithmType, class ContainerType, class ChooseType>
  const void* choose(const CRAlgorithmType& crAlgorithm, const ContainerType& choices, const ChooseType& dummy)
    {assert(false); return NULL;}

  void reward(double r) {}
};
*/

}; /* namespace cralgo */

#endif // !CRALGO_BRIDGE_STATIC_INTERFACE_HPP_


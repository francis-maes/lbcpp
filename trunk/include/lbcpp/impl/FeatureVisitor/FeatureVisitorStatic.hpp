/*-----------------------------------------.---------------------------------.
| Filename: FeatureVisitorStatic.hpp       | Static Feature Visitor          |
| Author  : Francis Maes                   |   Base Class                    |
| Started : 27/02/2009 18:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_FEATURE_VISITOR_STATIC_H_
# define LBCPP_FEATURE_VISITOR_STATIC_H_

# include "../../FeatureGenerator/FeatureGenerator.h"
# include "../../FeatureGenerator/EditableFeatureGenerator.h"
# include "../Object.hpp"

namespace lbcpp {
namespace impl {

template<class ExactType>
struct FeatureVisitor : public Object<ExactType>
{
  // override these functions:
  bool featureEnter(lbcpp::FeatureDictionaryPtr dictionary, size_t number, lbcpp::FeatureDictionaryPtr subDictionary, double weight)
    {return false;}
  void featureSense(lbcpp::FeatureDictionaryPtr dictionary, size_t number, double value)
    {}
  void featureLeave()
    {}
  void featureCall(lbcpp::FeatureDictionaryPtr dictionary, size_t scopeNumber, lbcpp::FeatureGeneratorPtr featureGenerator, double weight);

  /*
  ** Conversion functions
  ** These are the functions called by the generated code
  */
  bool featureEnter_(lbcpp::FeatureDictionaryPtr dictionary, const String& scopeName, lbcpp::FeatureDictionaryPtr subDictionary)
    {return callFeatureEnter(dictionary, dictionary->getScopes()->add(scopeName), subDictionary, 1.0);}
    
  bool featureEnter_(lbcpp::FeatureDictionaryPtr dictionary, const char* scopeName, lbcpp::FeatureDictionaryPtr subDictionary)
    {return callFeatureEnter(dictionary, dictionary->getScopes()->add(scopeName), subDictionary, 1.0);}
    
  bool featureEnter_(lbcpp::FeatureDictionaryPtr dictionary, size_t number, lbcpp::FeatureDictionaryPtr subDictionary)
    {return callFeatureEnter(dictionary, number, subDictionary, 1.0);}
  
  void featureSense_(lbcpp::FeatureDictionaryPtr dictionary, const String& featureName, double value = 1.0)
    {callFeatureSense(dictionary, dictionary->getFeatures()->add(featureName), value);}
    
  void featureSense_(lbcpp::FeatureDictionaryPtr dictionary, const char* featureName, double value = 1.0)
    {callFeatureSense(dictionary, dictionary->getFeatures()->add(featureName), value);}
    
  void featureSense_(lbcpp::FeatureDictionaryPtr dictionary, size_t number, double value = 1.0)
    {callFeatureSense(dictionary, number, value);}
  
  void featureLeave_()
    {callFeatureLeave();}

  void featureCall_(lbcpp::FeatureGeneratorPtr featureGenerator, lbcpp::FeatureDictionaryPtr dictionary, size_t scopeIndex, double weight = 1.0)
    {callFeatureCall(dictionary, scopeIndex, featureGenerator, weight);}

  void featureCall_(lbcpp::FeatureGeneratorPtr featureGenerator, lbcpp::FeatureDictionaryPtr dictionary, const String& scopeName, double weight = 1.0)
    {callFeatureCall(dictionary, dictionary->getScopes()->add(scopeName), featureGenerator, weight);}
    
  void featureCall_(lbcpp::FeatureGeneratorPtr featureGenerator, lbcpp::FeatureDictionaryPtr dictionary, const char* scopeName, double weight = 1.0)
    {callFeatureCall(dictionary, dictionary->getScopes()->add(scopeName), featureGenerator, weight);}
  
protected:
  ExactType& _this() {return *static_cast<ExactType* >(this);}
  
  void assertScopeNumberIsValid(size_t number)
    {jassert((int)number >= 0);}
  
  bool callFeatureEnter(lbcpp::FeatureDictionaryPtr dictionary, size_t number, lbcpp::FeatureDictionaryPtr subDictionary, double weight)
  {
    jassert(isNumberValid(weight));
    assertScopeNumberIsValid(number);
    return _this().featureEnter(dictionary, number, subDictionary, weight);
  }

  void callFeatureSense(lbcpp::FeatureDictionaryPtr dictionary, size_t number, double value)
  {
    jassert(isNumberValid(value));
    assertScopeNumberIsValid(number);
    _this().featureSense(dictionary, number, value);
  }

  void callFeatureLeave()
    {_this().featureLeave();}

  void callFeatureCall(lbcpp::FeatureDictionaryPtr dictionary, size_t scopeNumber, lbcpp::FeatureGeneratorPtr featureGenerator, double weight)
  {
    jassert(isNumberValid(weight));
    assertScopeNumberIsValid(scopeNumber);
    if (weight && featureGenerator)
      _this().featureCall(dictionary, scopeNumber, featureGenerator, weight);
  }
};

template<class ExactType>
class WeightStackBasedFeatureVisitor : public FeatureVisitor<ExactType>
{
public:
  WeightStackBasedFeatureVisitor(double weight = 1.0)
    : currentWeight(weight) {}

  bool featureEnter(lbcpp::FeatureDictionaryPtr dictionary, size_t number, lbcpp::FeatureDictionaryPtr subDictionary, double weight)
  {
    weightStack.push_back(currentWeight);
    currentWeight *= weight;
    return currentWeight != 0.0;
  }

  void featureLeave()
  {
    jassert(weightStack.size());
    currentWeight = weightStack.back();
    weightStack.pop_back();
  }

protected:
  double currentWeight;

private:
  std::vector<double> weightStack;
};

template<class ExactType, class VectorType>
class VectorStackBasedFeatureVisitor
  : public FeatureVisitor< ExactType >
{
public:
 typedef ReferenceCountedObjectPtr<VectorType> VectorPtr;
  
  VectorStackBasedFeatureVisitor(VectorPtr vector, double weight = 1.0)
    : currentVector(vector), currentWeight(1.0) {}
  
  bool featureEnter(lbcpp::FeatureDictionaryPtr dictionary, size_t number, lbcpp::FeatureDictionaryPtr subDictionary, double weight)
  {
    jassert(currentVector && currentVector->getDictionary()->checkEquals(dictionary));
    VectorPtr subVector = ((ExactType* )this)->getCurrentSubVector(number, subDictionary);
    if (!subVector)
      return false;
    currentVectorStack.push_back(std::make_pair(currentVector, currentWeight));
    currentVector = subVector;
    currentWeight *= weight;
    return true;
  }
    
  void featureLeave()
  {
    jassert(currentVectorStack.size() > 0);
    currentVector = currentVectorStack.back().first;
    currentWeight = currentVectorStack.back().second;
    currentVectorStack.pop_back();    
    jassert(currentVector);
  }
   
protected:
  VectorPtr currentVector;
  double currentWeight;

  VectorPtr getCurrentSubVector(size_t number, lbcpp::FeatureDictionaryPtr subDictionary)
  {
    VectorPtr& subVector = currentVector->getSubVector(number);
    if (!subVector)
      subVector = VectorPtr(new VectorType(subDictionary));
    else
      subVector->ensureDictionary(subDictionary);
    return subVector;
  }

private:
  std::vector< std::pair<VectorPtr, double> > currentVectorStack;
};

}; /* namespace lbcpp */
}; /* namespace impl */

#endif // !LBCPP_FEATURE_VISITOR_STATIC_H_

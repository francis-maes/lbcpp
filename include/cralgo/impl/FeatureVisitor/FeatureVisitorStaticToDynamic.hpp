/*-----------------------------------------.---------------------------------.
| Filename: FeatureVisitorStaticToDyn...hpp| Feature Visitor                 |
| Author  : Francis Maes                   |   Static To Dynamic Bridge      |
| Started : 27/02/2009 18:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_FEATURE_VISITOR_STATIC_TO_DYNAMIC_H_
# define CRALGO_FEATURE_VISITOR_STATIC_TO_DYNAMIC_H_

# include "FeatureVisitorStatic.hpp"
# include "../StaticToDynamic.hpp"

namespace cralgo {
namespace impl {

STATIC_TO_DYNAMIC_CLASS(FeatureVisitor, Object)
  
  virtual bool featureEnter(FeatureDictionary& dictionary, size_t index)
    {return BaseClass::impl.featureEnter(dictionary, index);}
    
  virtual void featureSense(FeatureDictionary& dictionary, size_t index, double value)
    {BaseClass::impl.featureSense(dictionary, index, value);}

  virtual void featureCall(FeatureDictionary& dictionary, size_t scopeIndex, FeatureGeneratorPtr featureGenerator)
    {BaseClass::impl.featureCall(dictionary, scopeIndex, featureGenerator);}

  virtual void featureCall(FeatureDictionary& dictionary, FeatureGeneratorPtr featureGenerator)
    {BaseClass::impl.featureCall(dictionary, featureGenerator);}

  virtual void featureLeave()
    {BaseClass::impl.featureLeave();}
    
STATIC_TO_DYNAMIC_ENDCLASS(FeatureVisitor);

template<class ExactType>
inline void FeatureVisitor<ExactType>::featureCall(cralgo::FeatureDictionary& dictionary, cralgo::FeatureGeneratorPtr featureGenerator)
{
  FeatureVisitorPtr featureVisitor = staticToDynamic(_this());
  featureGenerator->accept(featureVisitor, &dictionary);
}

}; /* namespace cralgo */
}; /* namespace impl */

#endif // !CRALGO_FEATURE_VISITOR_STATIC_TO_DYNAMIC_H_

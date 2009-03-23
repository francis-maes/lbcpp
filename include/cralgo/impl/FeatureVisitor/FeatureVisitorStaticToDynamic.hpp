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
  
  virtual bool featureEnter(FeatureDictionaryPtr dictionary, size_t index)
    {return BaseClass::impl.featureEnter(dictionary, index);}
    
  virtual void featureSense(FeatureDictionaryPtr dictionary, size_t index, double value)
    {BaseClass::impl.featureSense(dictionary, index, value);}

  virtual void featureCall(FeatureDictionaryPtr dictionary, size_t scopeIndex, FeatureGeneratorPtr featureGenerator)
    {BaseClass::impl.featureCall(dictionary, scopeIndex, featureGenerator);}

  virtual void featureCall(FeatureDictionaryPtr dictionary, FeatureGeneratorPtr featureGenerator)
    {BaseClass::impl.featureCall(dictionary, featureGenerator);}

  virtual void featureLeave()
    {BaseClass::impl.featureLeave();}
    
STATIC_TO_DYNAMIC_ENDCLASS(FeatureVisitor);

template<class ImplementationType>
class StaticToDynamicFeatureVisitorRef : public cralgo::FeatureVisitor
{
public:
  StaticToDynamicFeatureVisitorRef(ImplementationType& impl)
    : impl(impl)  {}

  virtual bool featureEnter(FeatureDictionaryPtr dictionary, size_t index)
    {return impl.featureEnter(dictionary, index);}
    
  virtual void featureSense(FeatureDictionaryPtr dictionary, size_t index, double value)
    {impl.featureSense(dictionary, index, value);}

  virtual void featureCall(FeatureDictionaryPtr dictionary, size_t scopeIndex, FeatureGeneratorPtr featureGenerator)
    {impl.featureCall(dictionary, scopeIndex, featureGenerator);}

  virtual void featureCall(FeatureDictionaryPtr dictionary, FeatureGeneratorPtr featureGenerator)
    {impl.featureCall(dictionary, featureGenerator);}

  virtual void featureLeave()
    {impl.featureLeave();}
        
  ImplementationType& impl;
};

template<class ExactType>
inline void FeatureVisitor<ExactType>::featureCall(cralgo::FeatureDictionaryPtr dictionary, cralgo::FeatureGeneratorPtr featureGenerator)
{
  StaticToDynamicFeatureVisitorRef<ExactType> dynamicVisitor(_this());
  StaticallyAllocatedReferenceCountedObjectPtr<cralgo::FeatureVisitor> dynamicVisitorPtr(dynamicVisitor);
  {
    EditableFeatureGeneratorPtr editable = featureGenerator.dynamicCast<EditableFeatureGenerator>();
    if (editable)
      editable->staticFeatureGenerator(_this());
    else
      featureGenerator->accept(dynamicVisitorPtr);
  }
}

}; /* namespace cralgo */
}; /* namespace impl */

#endif // !CRALGO_FEATURE_VISITOR_STATIC_TO_DYNAMIC_H_

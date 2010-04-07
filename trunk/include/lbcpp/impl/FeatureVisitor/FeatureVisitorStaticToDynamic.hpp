/*-----------------------------------------.---------------------------------.
| Filename: FeatureVisitorStaticToDyn...hpp| Feature Visitor                 |
| Author  : Francis Maes                   |   Static To Dynamic Bridge      |
| Started : 27/02/2009 18:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_FEATURE_VISITOR_STATIC_TO_DYNAMIC_H_
# define LBCPP_FEATURE_VISITOR_STATIC_TO_DYNAMIC_H_

# include "FeatureVisitorStatic.hpp"
# include "../StaticToDynamic.hpp"
# include <lbcpp/FeatureVisitor.h>

namespace lbcpp {
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
class StaticToDynamicFeatureVisitorRef : public lbcpp::FeatureVisitor
{
public:
  StaticToDynamicFeatureVisitorRef(ImplementationType& impl)
    : impl(impl)  {}

  virtual bool featureEnter(FeatureDictionaryPtr dictionary, size_t index)
    {return impl.featureEnter(dictionary, index);}
    
  virtual void featureSense(FeatureDictionaryPtr dictionary, size_t index, double value)
    {impl.featureSense(dictionary, index, value);}

  virtual void featureCall(FeatureGeneratorPtr featureGenerator)
    {impl.featureCall(featureGenerator);}

  virtual void featureCall(FeatureDictionaryPtr dictionary, size_t scopeIndex, FeatureGeneratorPtr featureGenerator)
    {impl.featureCall(dictionary, scopeIndex, featureGenerator);}

  virtual void featureLeave()
    {impl.featureLeave();}
        
  ImplementationType& impl;
};

template<class ExactType>
inline FeatureVisitorPtr staticToDynamic(impl::FeatureVisitor<ExactType>& implementation)
  {return FeatureVisitorPtr(new StaticToDynamicFeatureVisitorRef<ExactType>(static_cast<ExactType& >(implementation)));}

template<class ExactType>
inline void FeatureVisitor<ExactType>::featureCall(lbcpp::FeatureGeneratorPtr featureGenerator)
{
  EditableFeatureGeneratorPtr editable = featureGenerator.dynamicCast<EditableFeatureGenerator>();
  if (editable)
    editable->staticFeatureGenerator(_this());
  else
  {
    StaticToDynamicFeatureVisitorRef<ExactType> dynamicVisitor(_this());
    StaticallyAllocatedReferenceCountedObjectPtr<lbcpp::FeatureVisitor> dynamicVisitorPtr(dynamicVisitor);
    featureGenerator->accept(dynamicVisitorPtr);
  }
}

}; /* namespace lbcpp */
}; /* namespace impl */

#endif // !LBCPP_FEATURE_VISITOR_STATIC_TO_DYNAMIC_H_

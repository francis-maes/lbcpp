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

namespace cralgo {
namespace impl {

template<class ImplementationType>
class StaticToDynamicFeatureVisitor : public cralgo::FeatureVisitor
{
public:
  StaticToDynamicFeatureVisitor(const ImplementationType& impl)
    : impl(impl) {}
    
  ImplementationType impl;
  
  virtual bool featureEnter(FeatureDictionary& dictionary, size_t index)
    {return impl.featureEnter(dictionary, index);}
    
  virtual void featureSense(FeatureDictionary& dictionary, size_t index, double value)
    {impl.featureSense(dictionary, index, value);}

  virtual void featureCall(FeatureDictionary& dictionary, size_t scopeIndex, FeatureGeneratorPtr featureGenerator)
    {impl.featureCall(dictionary, scopeIndex, featureGenerator);}

  virtual void featureCall(FeatureDictionary& dictionary, FeatureGeneratorPtr featureGenerator)
    {impl.featureCall(dictionary, featureGenerator);}

  virtual void featureLeave()
    {impl.featureLeave();}
};

template<class ExactType>
inline void FeatureVisitor<ExactType>::featureCall(cralgo::FeatureDictionary& dictionary, cralgo::FeatureGeneratorPtr featureGenerator)
{
  FeatureVisitorPtr featureVisitor = staticToDynamic(_this());
  featureGenerator->accept(featureVisitor, &dictionary);
}

template<class ExactType>
inline FeatureVisitorPtr staticToDynamic(const FeatureVisitor<ExactType>& staticVisitor)
  {return FeatureVisitorPtr(new StaticToDynamicFeatureVisitor<ExactType>(static_cast<const ExactType& >(staticVisitor)));}

}; /* namespace cralgo */
}; /* namespace impl */

#endif // !CRALGO_FEATURE_VISITOR_STATIC_TO_DYNAMIC_H_

/*-----------------------------------------.---------------------------------.
| Filename: FeatureVisitorDynamicToSta..hpp| Dynamic to Static wrapper       |
| Author  : Francis Maes                   |   for feature visitors          |
| Started : 27/02/2009 18:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_FEATURE_VISITOR_DYNAMIC_TO_STATIC_H_
# define LBCPP_FEATURE_VISITOR_DYNAMIC_TO_STATIC_H_

# include "FeatureVisitorStatic.hpp"

namespace lbcpp {
namespace impl {

struct DynamicToStaticFeatureVisitor : public FeatureVisitor<DynamicToStaticFeatureVisitor>
{
  DynamicToStaticFeatureVisitor(FeatureVisitorPtr target)
    : target(target) {}
  
  bool featureEnter(lbcpp::FeatureDictionaryPtr dictionary, size_t number)
    {return target->featureEnter(dictionary, number);}

  void featureSense(lbcpp::FeatureDictionaryPtr dictionary, size_t number, double value = 1.0)
    {target->featureSense(dictionary, number, value);}
  
  void featureCall(lbcpp::FeatureGeneratorPtr featureGenerator)
    {target->featureCall(featureGenerator);}

  void featureCall(lbcpp::FeatureDictionaryPtr dictionary, size_t number, lbcpp::FeatureGeneratorPtr featureGenerator)
    {FeatureVisitor<DynamicToStaticFeatureVisitor>::featureCall(dictionary, number, featureGenerator);}
  
  void featureLeave()
    {target->featureLeave();}

  FeatureVisitorPtr getTarget()
    {return target;}

private:
  FeatureVisitorPtr target;
};

inline DynamicToStaticFeatureVisitor dynamicToStatic(FeatureVisitorPtr visitor)
  {return DynamicToStaticFeatureVisitor(visitor);}

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_STATIC_FEATURE_VISITOR_DYNAMIC_TO_STATIC_HPP_

/*-----------------------------------------.---------------------------------.
| Filename: DotProductVisitor.hpp          | Visitors for dot-product        |
| Author  : Francis Maes                   |  operations                     |
| Started : 01/03/2009 18:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_FEATURE_VISITOR_DOT_PRODUCT_HPP_
# define CRALGO_FEATURE_VISITOR_DOT_PRODUCT_HPP_

# include "FeatureVisitorStatic.hpp"
# include "../../DenseVector.h"

namespace cralgo {
namespace impl {

class DotProductDenseVectorVisitor : public FeatureVisitor<DotProductDenseVectorVisitor>
{
public:
  DotProductDenseVectorVisitor(DenseVectorPtr vector)
    : currentVector(vector), res(0.0) {}
  
  bool featureEnter(FeatureDictionaryPtr dictionary, size_t index)
  {
    DenseVectorPtr& subVector = currentVector->getSubVector(index);
    if (!subVector)
      return false;
    currentVectorStack.push_back(currentVector);
    currentVector = subVector;
    return true;
  }
  
  void featureSense(FeatureDictionaryPtr dictionary, size_t index, double value)
    {res += currentVector->get(index) * value;}
  
  void featureLeave()
  {
    assert(currentVectorStack.size() > 0);
    currentVector = currentVectorStack.back();
    currentVectorStack.pop_back();    
  }
  
  double getResult() const
    {return res;}
  
private:
  std::vector<DenseVectorPtr> currentVectorStack;
  DenseVectorPtr currentVector;
  double res;
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_FEATURE_VISITOR_DOT_PRODUCT_HPP_


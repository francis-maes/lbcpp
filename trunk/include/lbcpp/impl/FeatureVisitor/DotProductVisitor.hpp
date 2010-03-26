/*-----------------------------------------.---------------------------------.
| Filename: DotProductVisitor.hpp          | Visitors for dot-product        |
| Author  : Francis Maes                   |  operations                     |
| Started : 01/03/2009 18:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_VISITOR_DOT_PRODUCT_HPP_
# define LBCPP_FEATURE_VISITOR_DOT_PRODUCT_HPP_

# include "FeatureVisitorStatic.hpp"
# include "../../DenseVector.h"

namespace lbcpp {
namespace impl {

template<class ExactType, class VectorType>
class DotProductVectorVisitor : public FeatureVisitor<ExactType>
{
public:
  typedef ReferenceCountedObjectPtr<VectorType> VectorPtr;
  
  DotProductVectorVisitor(VectorPtr vector)
    : currentVector(vector), res(0.0) {}
  
  bool featureEnter(FeatureDictionaryPtr dictionary, size_t index)
  {
    VectorPtr& subVector = currentVector->getSubVector(index);
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
    jassert(currentVectorStack.size() > 0);
    currentVector = currentVectorStack.back();
    currentVectorStack.pop_back();    
  }
  
  double getResult() const
    {return res;}
  
private:
  std::vector<VectorPtr> currentVectorStack;
  VectorPtr currentVector;
  double res;
};

class DotProductDenseVectorVisitor 
  : public DotProductVectorVisitor<DotProductDenseVectorVisitor, DenseVector>
{
public:
  DotProductDenseVectorVisitor(DenseVectorPtr vector)
    : DotProductVectorVisitor<DotProductDenseVectorVisitor, DenseVector>(vector) {}
};

class DotProductSparseVectorVisitor 
  : public DotProductVectorVisitor<DotProductSparseVectorVisitor, SparseVector>
{
public:
  DotProductSparseVectorVisitor(SparseVectorPtr vector)
    : DotProductVectorVisitor<DotProductSparseVectorVisitor, SparseVector>(vector) {}
};

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_VISITOR_DOT_PRODUCT_HPP_

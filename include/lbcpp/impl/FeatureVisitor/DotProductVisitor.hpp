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
class DotProductVectorVisitor : public VectorStackBasedFeatureVisitor<ExactType, VectorType>
{
public:
  typedef VectorStackBasedFeatureVisitor<ExactType, VectorType> BaseClass;
  typedef typename BaseClass::VectorPtr VectorPtr;
  
  DotProductVectorVisitor(VectorPtr vector)
    : BaseClass(vector), res(0.0) {jassert(BaseClass::currentVector);}
    
  void featureSense(FeatureDictionaryPtr dictionary, size_t index, double value)
    {res += getCurrentVector().get(index) * value * BaseClass::currentWeight;}
  
  void featureCall(lbcpp::FeatureDictionaryPtr dictionary, size_t scopeNumber, lbcpp::FeatureGeneratorPtr featureGenerator, double weight)
  {
    jassert(BaseClass::currentVector);
    VectorPtr subVector = getCurrentSubVector(scopeNumber, featureGenerator->getDictionary());
    if (subVector)
    {
      weight *= BaseClass::currentWeight;
      if (weight)
        res += featureGenerator->dotProduct(subVector) * weight;
    }
  }

  double getResult() const
    {return res;}
  
  VectorPtr getCurrentSubVector(size_t number, lbcpp::FeatureDictionaryPtr subDictionary)
  {
    VectorPtr res = getCurrentVector().getSubVector(number);
    jassert(!res || res->getDictionary() == subDictionary);
    return res;
  }
  
  const VectorType& getCurrentVector() const
    {jassert(BaseClass::currentVector); return const_cast<const VectorType& >(*BaseClass::currentVector.get());}

private:
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

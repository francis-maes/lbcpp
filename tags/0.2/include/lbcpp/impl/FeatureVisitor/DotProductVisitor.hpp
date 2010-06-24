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
# include "../../FeatureGenerator/DenseVector.h"

namespace lbcpp {
namespace impl {

template<class ExactType, class VectorType>
class DotProductVectorVisitor : public VectorStackBasedFeatureVisitor<ExactType, VectorType>
{
public:
  typedef VectorStackBasedFeatureVisitor<ExactType, VectorType> BaseClass;
  typedef typename BaseClass::VectorPtr VectorPtr;
  
  DotProductVectorVisitor(VectorPtr vector, FeatureGenerator::DotProductCache* cache)
    : BaseClass(vector), cache(cache), res(0.0) {jassert(BaseClass::currentVector);}
    
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
        res += makeDotProduct(featureGenerator, subVector) * weight;
    }
  }

  double getResult() const
    {return res;}
  
  VectorPtr getCurrentSubVector(size_t number, lbcpp::FeatureDictionaryPtr subDictionary)
  {
    VectorPtr res = getCurrentVector().getSubVector(number);
    if (res)
      res->getDictionary()->checkEquals(subDictionary);
    return res;
  }
  
  const VectorType& getCurrentVector() const
    {jassert(BaseClass::currentVector); return const_cast<const VectorType& >(*BaseClass::currentVector.get());}

private:
  FeatureGenerator::DotProductCache* cache;
  double res;

  double makeDotProduct(FeatureGeneratorPtr featureGenerator, VectorPtr vector)
  {
    // only dot products with sparse vectors are cache
    if (!cache || !featureGenerator.dynamicCast<SparseVector>())
      return featureGenerator->dotProduct(vector, cache);

    std::pair<FeatureGeneratorPtr, FeatureGeneratorPtr> key(featureGenerator, vector);
    FeatureGenerator::DotProductCache::const_iterator it = cache->find(key);
    if (it == cache->end())
    {
      double res = featureGenerator->dotProduct(vector, cache);
      (*cache)[key] = res;
      return res;
    }
    else
    {
      jassert(featureGenerator->dotProduct(vector) == it->second);
      return it->second;
    }
  }
};

class DotProductDenseVectorVisitor 
  : public DotProductVectorVisitor<DotProductDenseVectorVisitor, DenseVector>
{
public:
  DotProductDenseVectorVisitor(DenseVectorPtr vector, FeatureGenerator::DotProductCache* cache = NULL)
    : DotProductVectorVisitor<DotProductDenseVectorVisitor, DenseVector>(vector, cache) {}
};

class DotProductSparseVectorVisitor 
  : public DotProductVectorVisitor<DotProductSparseVectorVisitor, SparseVector>
{
public:
  DotProductSparseVectorVisitor(SparseVectorPtr vector, FeatureGenerator::DotProductCache* cache = NULL)
    : DotProductVectorVisitor<DotProductSparseVectorVisitor, SparseVector>(vector, cache) {}
};

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_VISITOR_DOT_PRODUCT_HPP_

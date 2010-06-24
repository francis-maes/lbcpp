/*-----------------------------------------.---------------------------------.
| Filename: CreateVectorVisitor.hpp        | Create a vector                 |
| Author  : Francis Maes                   |                                 |
| Started : 27/02/2009 18:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_STATIC_FEATURE_VISITOR_CREATE_VECTOR_HPP_
# define LBCPP_STATIC_FEATURE_VISITOR_CREATE_VECTOR_HPP_

# include "FeatureVisitorStatic.hpp"
# include "../../FeatureGenerator/SparseVector.h"

namespace lbcpp {
namespace impl {

template<class ExactType, class VectorType>
struct CreateVectorVisitor : public VectorStackBasedFeatureVisitor< ExactType, VectorType >
{
  typedef VectorStackBasedFeatureVisitor< ExactType, VectorType > BaseClass;
  typedef typename BaseClass::VectorPtr VectorPtr;
  
  CreateVectorVisitor(FeatureDictionaryPtr dictionary) 
    : BaseClass(new VectorType(dictionary)) {result = BaseClass::currentVector;}
  
  void featureSense(lbcpp::FeatureDictionaryPtr dictionary, size_t number, double value = 1.0)
  {
    jassert(BaseClass::currentVector->getDictionary() == dictionary);
    value *= BaseClass::currentWeight;
    if (value)
      BaseClass::currentVector->get(number) += value;
  }

  void featureCall(lbcpp::FeatureDictionaryPtr dictionary, size_t scopeNumber, lbcpp::FeatureGeneratorPtr featureGenerator, double weight)
  {
    VectorPtr subVector = ((ExactType* )this)->getCurrentSubVector(scopeNumber, featureGenerator->getDictionary());
    featureGenerator->addWeightedTo(subVector, weight);
  }

  VectorPtr getResult() const
    {return result;}
  
private:
  VectorPtr result;
};

struct CreateSparseVectorVisitor : public CreateVectorVisitor<CreateSparseVectorVisitor, SparseVector>
{
  CreateSparseVectorVisitor(FeatureDictionaryPtr dictionary) 
    : CreateVectorVisitor<CreateSparseVectorVisitor, SparseVector>(dictionary) {}
};

struct CreateDenseVectorVisitor : public CreateVectorVisitor<CreateDenseVectorVisitor, DenseVector>
{
  CreateDenseVectorVisitor(FeatureDictionaryPtr dictionary) 
    : CreateVectorVisitor<CreateDenseVectorVisitor, DenseVector>(dictionary) {}
};

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_STATIC_FEATURE_VISITOR_CREATE_VECTOR_HPP_

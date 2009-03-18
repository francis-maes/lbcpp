/*-----------------------------------------.---------------------------------.
| Filename: CreateSparseVectorVisitor.hpp  | Create a sparse vector          |
| Author  : Francis Maes                   |                                 |
| Started : 27/02/2009 18:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_STATIC_FEATURE_VISITOR_CREATE_SPARSE_VECTOR_HPP_
# define CRALGO_STATIC_FEATURE_VISITOR_CREATE_SPARSE_VECTOR_HPP_

# include "FeatureVisitorStatic.hpp"
# include "../../SparseVector.h"

namespace cralgo {
namespace impl {

template<class ExactType, class VectorType>
struct CreateVectorVisitor : public FeatureVisitor< ExactType >
{
  typedef ReferenceCountedObjectPtr<VectorType> VectorTypePtr;
  
  CreateVectorVisitor(FeatureDictionaryPtr dictionary) 
    : vector(new VectorType(dictionary)) {currentVector = vector;}
  
  bool featureEnter(cralgo::FeatureDictionaryPtr dictionary, size_t number)
  {
    currentVectorStack.push_back(currentVector);
    VectorTypePtr& v = currentVector->getSubVector(number);
    if (!v)
      v = new VectorType(dictionary->getSubDictionary(number));
    currentVector = v;
    return true;
  }

  void featureSense(cralgo::FeatureDictionaryPtr dictionary, size_t number, double value = 1.0)
    {currentVector->set(number, value);}

  void featureLeave()
  {
    assert(currentVectorStack.size() > 0);
    currentVector = currentVectorStack.back();
    currentVectorStack.pop_back();
  }
  
  VectorTypePtr getResult() const
    {return vector;}
  
private:
  VectorTypePtr vector;
  std::vector<VectorTypePtr> currentVectorStack;
  VectorTypePtr currentVector;
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
}; /* namespace cralgo */

#endif // !CRALGO_STATIC_FEATURE_VISITOR_CREATE_SPARSE_VECTOR_HPP_

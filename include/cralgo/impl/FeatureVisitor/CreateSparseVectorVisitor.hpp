/*-----------------------------------------.---------------------------------.
| Filename: CreateSparseVectorVisitor.hpp  | Create a sparse vector          |
| Author  : Francis Maes                   |                                 |
| Started : 27/02/2009 18:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_STATIC_FEATURE_VISITOR_CREATE_SPARSE_VECTOR_HPP_
# define CRALGO_STATIC_FEATURE_VISITOR_CREATE_SPARSE_VECTOR_HPP_

# include "StaticFeatureVisitor.hpp"
# include "../../SparseVector.h"

namespace cralgo
{

struct CreateSparseVectorVisitor : public StaticFeatureVisitor<CreateSparseVectorVisitor>
{
  CreateSparseVectorVisitor(FeatureDictionary& dictionary) 
    : vector(new SparseVector(dictionary)) {currentVector = vector;}
  
  bool featureEnter(cralgo::FeatureDictionary& dictionary, size_t number)
  {
    currentVectorStack.push_back(currentVector);
    SparseVectorPtr& v = currentVector->getSubVector(number);
    if (!v)
      v = SparseVectorPtr(new SparseVector(dictionary.getSubDictionary(number)));
    currentVector = v;
    return true;
  }

  void featureSense(cralgo::FeatureDictionary& dictionary, size_t number, double value = 1.0)
    {currentVector->set(number, value);}

  void featureLeave()
  {
    assert(currentVectorStack.size() > 0);
    currentVector = currentVectorStack.back();
    currentVectorStack.pop_back();
  }
  
  SparseVectorPtr getResult() const
    {return vector;}
  
private:
  SparseVectorPtr vector;
  std::vector<SparseVectorPtr> currentVectorStack;
  SparseVectorPtr currentVector;
};

}; /* namespace cralgo */

#endif // !CRALGO_STATIC_FEATURE_VISITOR_CREATE_SPARSE_VECTOR_HPP_

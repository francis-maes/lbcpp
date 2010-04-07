/*-----------------------------------------.---------------------------------.
| Filename: AssignmentVisitor.hpp          | Visitors for assignment         |
| Author  : Francis Maes                   |  operations                     |
| Started : 28/02/2009 13:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_VISITOR_ASSIGNMENT_HPP_
# define LBCPP_FEATURE_VISITOR_ASSIGNMENT_HPP_

# include "FeatureVisitorStatic.hpp"
# include "../../DenseVector.h"

namespace lbcpp {
namespace impl {

struct AssignmentVectorOperation
{
  void process(double& lValue, double rValue)
    {jassert(false);}
  
  template<class VectorPtr>
  void call(VectorPtr vector, FeatureGeneratorPtr featureGenerator)
    {jassert(false);}
};

struct AddVectorOperation : public AssignmentVectorOperation
{
  void process(double& lValue, const double rValue)
    {lValue += rValue;}
  
  template<class VectorPtr>
  void call(VectorPtr vector, FeatureGeneratorPtr featureGenerator)
    {featureGenerator->addTo(vector);}
};

struct SubstractVectorOperation : public AssignmentVectorOperation
{
  void process(double& lValue, const double rValue)
    {lValue += rValue;}

  template<class VectorPtr>
  void call(VectorPtr vector, FeatureGeneratorPtr featureGenerator)
    {featureGenerator->substractFrom(vector);}
};

struct AddWeightedVectorOperation : public AssignmentVectorOperation
{
  AddWeightedVectorOperation(double weight) 
    : weight(weight) {}
    
  double weight;
  
  void process(double& lValue, const double rValue)
    {lValue += rValue * weight;}

  template<class VectorPtr>
  void call(VectorPtr vector, FeatureGeneratorPtr featureGenerator)
    {featureGenerator->addWeightedTo(vector, weight);}
};

struct AddWeightedSignsVectorOperation : public AssignmentVectorOperation
{
  AddWeightedSignsVectorOperation(double weight)
    : weight(weight) {}
  
  double weight;
  
  inline void process(double& lvalue, const double rvalue)
    {lvalue += rvalue > 0 ? weight : (rvalue < 0 ? -weight : 0);}

  template<class VectorPtr>
  void call(VectorPtr vector, FeatureGeneratorPtr featureGenerator)
    {featureGenerator->addWeightedSignsTo(vector, weight);}
};

template<class ExactType, class VectorType, class OperationType>
class AssignmentToVectorVisitor
  : public FeatureVisitor< ExactType >
{
public:
  typedef ReferenceCountedObjectPtr<VectorType> VectorPtr;
  
  AssignmentToVectorVisitor(VectorPtr target, OperationType& operation)
    : operation(operation), currentVector(target) {}
  
  bool featureEnter(lbcpp::FeatureDictionaryPtr dictionary, size_t number, lbcpp::FeatureDictionaryPtr subDictionary)
  {
    currentVectorStack.push_back(currentVector);
    currentVector = getCurrentSubVector(number, subDictionary);
    return true;
  }
  
  void featureSense(lbcpp::FeatureDictionaryPtr dictionary, size_t number, double value = 1.0)
  {
    jassert(currentVector->getDictionary() == dictionary);
    operation.process(currentVector->get(number), value);
  }
  
  void featureLeave()
  {
    jassert(currentVectorStack.size() > 0);
    currentVector = currentVectorStack.back();
    currentVectorStack.pop_back();    
  }
  
  void featureCall(lbcpp::FeatureDictionaryPtr dictionary, size_t scopeNumber, lbcpp::FeatureGeneratorPtr featureGenerator)
  {
    VectorPtr subVector = getCurrentSubVector(scopeNumber, featureGenerator->getDictionary());
    operation.call(subVector, featureGenerator);
  }
  
private:
  OperationType& operation;
  std::vector<VectorPtr> currentVectorStack;
  VectorPtr currentVector;

  VectorPtr getCurrentSubVector(size_t number, lbcpp::FeatureDictionaryPtr subDictionary)
  {
    VectorPtr& subVector = currentVector->getSubVector(number);
    if (!subVector)
      subVector = VectorPtr(new VectorType(subDictionary));
    return subVector;
  }
};

template<class OperationType>
class AssignmentToDenseVisitor : public AssignmentToVectorVisitor<AssignmentToDenseVisitor<OperationType>, DenseVector, OperationType>
{
public:
  AssignmentToDenseVisitor(DenseVectorPtr target, OperationType& operation)
    : AssignmentToVectorVisitor<AssignmentToDenseVisitor<OperationType>, DenseVector, OperationType>(target, operation) {}
};

template<class OperationType>
class AssignmentToSparseVisitor : public AssignmentToVectorVisitor<AssignmentToSparseVisitor<OperationType>, SparseVector, OperationType>
{
public:
  AssignmentToSparseVisitor(SparseVectorPtr target, OperationType& operation)
    : AssignmentToVectorVisitor<AssignmentToSparseVisitor<OperationType>, SparseVector, OperationType>(target, operation) {}
};

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_VISITOR_ASSIGNMENT_HPP_


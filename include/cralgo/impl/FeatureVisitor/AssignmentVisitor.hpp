/*-----------------------------------------.---------------------------------.
| Filename: AssignmentVisitor.hpp          | Visitors for assignment         |
| Author  : Francis Maes                   |  operations                     |
| Started : 28/02/2009 13:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_FEATURE_VISITOR_ASSIGNMENT_HPP_
# define CRALGO_FEATURE_VISITOR_ASSIGNMENT_HPP_

# include "StaticFeatureVisitor.hpp"
# include "../../DenseVector.h"

namespace cralgo
{

struct AssignmentVectorOperation
{
  void process(double& lValue, double rValue)
    {assert(false);}
};

struct AddVectorOperation : public AssignmentVectorOperation
{
  void process(double& lValue, const double rValue)
    {lValue += rValue;}
};

struct SubstractVectorOperation : public AssignmentVectorOperation
{
  void process(double& lValue, const double rValue)
    {lValue += rValue;}
};

struct AddWeightedVectorOperation : public AssignmentVectorOperation
{
  AddWeightedVectorOperation(double weight) 
    : weight(weight) {}
    
  double weight;
  
  void process(double& lValue, const double rValue)
    {lValue += rValue * weight;}
};

struct AddWeightedSignsVectorOperation : public AssignmentVectorOperation
{
  AddWeightedSignsVectorOperation(double weight)
    : weight(weight) {}
  
  double weight;
  
  inline void process(double& lvalue, const double rvalue)
    {lvalue += rvalue > 0 ? weight : (rvalue < 0 ? -weight : 0);}
};

template<class ExactType, class VectorType, class OperationType>
class AssignmentToVectorVisitor
  : public StaticFeatureVisitor< ExactType >
{
public:
  typedef ReferenceCountedObjectPtr<VectorType> VectorPtr;
  
  AssignmentToVectorVisitor(VectorPtr target, OperationType& operation)
    : operation(operation), currentVector(target) {}
  
  bool featureEnter(cralgo::FeatureDictionary& dictionary, size_t number)
  {
    currentVectorStack.push_back(currentVector);
    VectorPtr& subVector = currentVector->getSubVector(number);
    if (!subVector)
      subVector = VectorPtr(new VectorType(dictionary.getSubDictionary(number)));
    currentVector = subVector;
    return true;
  }
  
  void featureSense(cralgo::FeatureDictionary& dictionary, size_t number, double value = 1.0)
    {operation.process(currentVector->get(number), value);}
  
  void featureLeave()
  {
    assert(currentVectorStack.size() > 0);
    currentVector = currentVectorStack.back();
    currentVectorStack.pop_back();    
  }
  
private:
  OperationType& operation;
  std::vector<VectorPtr> currentVectorStack;
  VectorPtr currentVector;
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


}; /* namespace cralgo */

#endif // !CRALGO_FEATURE_VISITOR_ASSIGNMENT_HPP_


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

template<class OperationType>
class AssignmentToDenseVisitor
  : public StaticFeatureVisitor< AssignmentToDenseVisitor<OperationType> >
{
public:
  AssignmentToDenseVisitor(DenseVectorPtr target, OperationType& operation)
    : operation(operation), currentVector(target) {}
  
  bool featureEnter(cralgo::FeatureDictionary& dictionary, size_t number)
  {
    currentVectorStack.push_back(currentVector);
    DenseVectorPtr& subVector = currentVector->getSubVector(number);
    if (!subVector)
      subVector = DenseVectorPtr(new DenseVector(dictionary.getSubDictionary(number)));
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
  std::vector<DenseVectorPtr> currentVectorStack;
  DenseVectorPtr currentVector;
};

}; /* namespace cralgo */

#endif // !CRALGO_FEATURE_VISITOR_ASSIGNMENT_HPP_


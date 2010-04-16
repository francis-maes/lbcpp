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
  void call(VectorPtr vector, FeatureGeneratorPtr featureGenerator, double weight)
    {jassert(false);}
};

struct AddVectorOperation : public AssignmentVectorOperation
{
  void process(double& lValue, const double rValue)
    {lValue += rValue;}
  
  template<class VectorPtr>
  void call(VectorPtr vector, FeatureGeneratorPtr featureGenerator, double weight)
    {weight == 1.0 ? featureGenerator->addTo(vector) : featureGenerator->addWeightedTo(vector, weight);}
};

struct SubstractVectorOperation : public AssignmentVectorOperation
{
  void process(double& lValue, const double rValue)
    {lValue += rValue;}

  template<class VectorPtr>
  void call(VectorPtr vector, FeatureGeneratorPtr featureGenerator, double weight)
    {weight == 1.0 ? featureGenerator->substractFrom(vector) : featureGenerator->addWeightedTo(vector, -weight);}
};

struct AddWeightedVectorOperation : public AssignmentVectorOperation
{
  AddWeightedVectorOperation(double weight) 
    : weight(weight) {}
    
  double weight;
  
  void process(double& lValue, const double rValue)
    {lValue += rValue * weight;}

  template<class VectorPtr>
  void call(VectorPtr vector, FeatureGeneratorPtr featureGenerator, double subWeight)
    {featureGenerator->addWeightedTo(vector, weight * subWeight);}
};

struct AddWeightedSignsVectorOperation : public AssignmentVectorOperation
{
  AddWeightedSignsVectorOperation(double weight)
    : weight(weight) {}
  
  double weight;
  
  inline void process(double& lvalue, const double rvalue)
    {lvalue += rvalue > 0 ? weight : (rvalue < 0 ? -weight : 0);}

  template<class VectorPtr>
  void call(VectorPtr vector, FeatureGeneratorPtr featureGenerator, double subWeight)
    {featureGenerator->addWeightedSignsTo(vector, weight * subWeight);}
};

template<class ExactType, class VectorType, class OperationType>
class AssignmentToVectorVisitor
  : public VectorStackBasedFeatureVisitor< ExactType, VectorType >
{
public:
  typedef VectorStackBasedFeatureVisitor< ExactType, VectorType > BaseClass;
  typedef typename BaseClass::VectorPtr VectorPtr;
  
  AssignmentToVectorVisitor(VectorPtr target, OperationType& operation)
    : BaseClass(target), operation(operation) {}
  
  void featureSense(lbcpp::FeatureDictionaryPtr dictionary, size_t number, double value)
  {
    jassert(BaseClass::currentVector->getDictionary() == dictionary);
    value *= BaseClass::currentWeight;
    if (value)
      operation.process(BaseClass::currentVector->get(number), value);
  }
    
  void featureCall(lbcpp::FeatureDictionaryPtr dictionary, size_t scopeNumber, lbcpp::FeatureGeneratorPtr featureGenerator, double weight)
  {
    VectorPtr subVector = BaseClass::getCurrentSubVector(scopeNumber, featureGenerator->getDictionary());
    weight *= BaseClass::currentWeight;
    if (weight)
      operation.call(subVector, featureGenerator, weight);
  }
  
private:
  OperationType& operation;
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


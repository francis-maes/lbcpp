/*-----------------------------------------.---------------------------------.
| Filename: UnaryOperationVisitor.hpp      | Visitors for const unary        |
| Author  : Francis Maes                   |  operations                     |
| Started : 01/03/2009 19:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_FEATURE_VISITOR_UNARY_OPERATION_HPP_
# define CRALGO_FEATURE_VISITOR_UNARY_OPERATION_HPP_

# include "StaticFeatureVisitor.hpp"
# include "../../DenseVector.h"

namespace cralgo
{

struct ConstUnaryVectorOperation
{
  void process(const double value) {assert(false);}
};

struct ComputeL0NormVectorOperation : public ConstUnaryVectorOperation
{
  ComputeL0NormVectorOperation() : res(0) {}
  size_t res;
  void process(const double value) {if (value) ++res;}
};

struct ComputeL1NormVectorOperation : public ConstUnaryVectorOperation
{
  ComputeL1NormVectorOperation() : res(0.0) {}
  double res;
  void process(const double value) {res += fabs(value);}
};

struct ComputeSumOfSquaresVectorOperation : public ConstUnaryVectorOperation
{
  ComputeSumOfSquaresVectorOperation() : res(0.0) {}
  double res;
  void process(const double value) {res += value * value;}
};

template<class OperationType>
class UnaryOperationVisitor
  : public StaticFeatureVisitor< UnaryOperationVisitor<OperationType> >
{
public:
  UnaryOperationVisitor(OperationType& operation)
    : operation(operation) {}
  
  bool featureEnter(cralgo::FeatureDictionary& dictionary, size_t number)
    {return true;}

  void featureSense(cralgo::FeatureDictionary& dictionary, size_t number, double value = 1.0)
    {operation.process(value);}
  
  void featureLeave()
    {}
  
private:
  OperationType& operation;
};

}; /* namespace cralgo */

#endif // !CRALGO_FEATURE_VISITOR_UNARY_OPERATION_HPP_


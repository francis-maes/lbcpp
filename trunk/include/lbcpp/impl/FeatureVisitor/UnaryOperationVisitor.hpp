/*-----------------------------------------.---------------------------------.
| Filename: UnaryOperationVisitor.hpp      | Visitors for const unary        |
| Author  : Francis Maes                   |  operations                     |
| Started : 01/03/2009 19:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_VISITOR_UNARY_OPERATION_HPP_
# define LBCPP_FEATURE_VISITOR_UNARY_OPERATION_HPP_

# include "FeatureVisitorStatic.hpp"
# include "../../DenseVector.h"

namespace lbcpp {
namespace impl {

struct ConstUnaryVectorOperation
{
  void process(const double value)
    {jassert(false);}
  
  void call(FeatureGeneratorPtr featureGenerator, double weight)
    {jassert(false);}
};

struct ComputeL0NormVectorOperation : public ConstUnaryVectorOperation
{
  ComputeL0NormVectorOperation() : res(0) {}
  size_t res;

  void process(const double value)
    {if (value) ++res;}

  void call(FeatureGeneratorPtr featureGenerator, double weight)
    {if (weight) res += featureGenerator->l0norm();}
};

struct ComputeL1NormVectorOperation : public ConstUnaryVectorOperation
{
  ComputeL1NormVectorOperation() : res(0.0) {}
  double res;

  void process(const double value)
    {res += fabs(value);}

  void call(FeatureGeneratorPtr featureGenerator, double weight)
    {if (weight) res += featureGenerator->l1norm() * weight;}
};

struct ComputeSumOfSquaresVectorOperation : public ConstUnaryVectorOperation
{
  ComputeSumOfSquaresVectorOperation() : res(0.0) {}
  double res;

  void process(const double value)
    {res += value * value;}

  void call(FeatureGeneratorPtr featureGenerator, double weight)
    {if (weight) res += featureGenerator->sumOfSquares() * weight * weight;}
};

template<class OperationType>
class UnaryOperationVisitor
  : public WeightStackBasedFeatureVisitor< UnaryOperationVisitor<OperationType> >
{
public:
  typedef WeightStackBasedFeatureVisitor< UnaryOperationVisitor<OperationType> > BaseClass;

  UnaryOperationVisitor(OperationType& operation)
    : operation(operation) {}
  
  void featureSense(lbcpp::FeatureDictionaryPtr dictionary, size_t number, double value)
  {
    value *= BaseClass::currentWeight;
    if (value)
      operation.process(value);
  }
  
  void featureLeave()
    {}

  void featureCall(lbcpp::FeatureDictionaryPtr dictionary, size_t scopeNumber, lbcpp::FeatureGeneratorPtr featureGenerator, double weight)
  {
    weight *= BaseClass::currentWeight;
    if (weight)
      operation.call(featureGenerator, weight);
  }
  
private:
  OperationType& operation;
};

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_VISITOR_UNARY_OPERATION_HPP_


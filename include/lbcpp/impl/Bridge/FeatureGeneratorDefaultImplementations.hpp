/*-----------------------------------------.---------------------------------.
| Filename: FeatureGeneratorDefaultImpl...h| Feature generator default       |
| Author  : Francis Maes                   |   implementations               |
| Started : 01/03/2009 18:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_FEATURE_GENERATOR_DEFAULT_IMPLEMENTATIONS_HPP_
# define LBCPP_FEATURE_GENERATOR_DEFAULT_IMPLEMENTATIONS_HPP_

# include "../FeatureVisitor/FeatureVisitorStaticToDynamic.hpp"
# include "../FeatureVisitor/FeatureVisitorDynamicToStatic.hpp"
# include "../FeatureVisitor/CreateVectorVisitor.hpp"
# include "../FeatureVisitor/UnaryOperationVisitor.hpp"
# include "../FeatureVisitor/AssignmentVisitor.hpp"
# include "../FeatureVisitor/DotProductVisitor.hpp"
# include "../FeatureVisitor/StringDescriptionVisitor.hpp"

namespace lbcpp
{

#define FEATURE_GENERATOR_DEFAULT_IMPL(ReturnType) \
  template<class ExactType, class BaseType> \
  inline ReturnType FeatureGeneratorDefaultImplementations<ExactType, BaseType>::

FEATURE_GENERATOR_DEFAULT_IMPL(void) accept(FeatureVisitorPtr visitor) const
{
  impl::DynamicToStaticFeatureVisitor staticVisitor(visitor);
  _this().staticFeatureGenerator(staticVisitor);  
}

FEATURE_GENERATOR_DEFAULT_IMPL(SparseVectorPtr) toSparseVector() const
{
  impl::CreateSparseVectorVisitor staticVisitor(_this().getDictionary());
  _this().staticFeatureGenerator(staticVisitor);
  return staticVisitor.getResult();
}

FEATURE_GENERATOR_DEFAULT_IMPL(DenseVectorPtr) toDenseVector() const
{
  impl::CreateDenseVectorVisitor staticVisitor(_this().getDictionary());
  _this().staticFeatureGenerator(staticVisitor);
  return staticVisitor.getResult();
}

FEATURE_GENERATOR_DEFAULT_IMPL(String) toString() const
{
  impl::StringDescriptionVisitor staticVisitor;
  _this().staticFeatureGenerator(staticVisitor);
  return staticVisitor.getResult();
}

FEATURE_GENERATOR_DEFAULT_IMPL(size_t) l0norm() const
{
  impl::ComputeL0NormVectorOperation op;
  impl::UnaryOperationVisitor<impl::ComputeL0NormVectorOperation> staticVisitor(op);
  _this().staticFeatureGenerator(staticVisitor);
  return op.res;
}

FEATURE_GENERATOR_DEFAULT_IMPL(double) l1norm() const
{
  impl::ComputeL1NormVectorOperation op;
  impl::UnaryOperationVisitor<impl::ComputeL1NormVectorOperation> staticVisitor(op);
  _this().staticFeatureGenerator(staticVisitor);
  return op.res;
}

FEATURE_GENERATOR_DEFAULT_IMPL(double) sumOfSquares() const
{
  impl::ComputeSumOfSquaresVectorOperation op;
  impl::UnaryOperationVisitor<impl::ComputeSumOfSquaresVectorOperation> staticVisitor(op);
  _this().staticFeatureGenerator(staticVisitor);
  return op.res;
}

FEATURE_GENERATOR_DEFAULT_IMPL(void) addTo(DenseVectorPtr target) const
{
  target->ensureDictionary(_this().getDictionary());
  impl::AddVectorOperation op;
  impl::AssignmentToDenseVisitor<impl::AddVectorOperation> staticVisitor(target, op);
  _this().staticFeatureGenerator(staticVisitor);
}

FEATURE_GENERATOR_DEFAULT_IMPL(void) addTo(SparseVectorPtr target) const
{
  target->ensureDictionary(_this().getDictionary());
  impl::AddVectorOperation op;
  impl::AssignmentToSparseVisitor<impl::AddVectorOperation> staticVisitor(target, op);
  _this().staticFeatureGenerator(staticVisitor);
}

FEATURE_GENERATOR_DEFAULT_IMPL(void) substractFrom(DenseVectorPtr target) const
{
  target->ensureDictionary(_this().getDictionary());
  impl::SubstractVectorOperation op;
  impl::AssignmentToDenseVisitor<impl::SubstractVectorOperation> staticVisitor(target, op);
  _this().staticFeatureGenerator(staticVisitor);
}

FEATURE_GENERATOR_DEFAULT_IMPL(void) substractFrom(SparseVectorPtr target) const
{
  target->ensureDictionary(_this().getDictionary());
  impl::SubstractVectorOperation op;
  impl::AssignmentToSparseVisitor<impl::SubstractVectorOperation> staticVisitor(target, op);
  _this().staticFeatureGenerator(staticVisitor);
}

FEATURE_GENERATOR_DEFAULT_IMPL(void) addWeightedTo(DenseVectorPtr target, double weight) const
{
  target->ensureDictionary(_this().getDictionary());
  if (!weight)
    return;  
  impl::AddWeightedVectorOperation op(weight);
  impl::AssignmentToDenseVisitor<impl::AddWeightedVectorOperation> staticVisitor(target, op);
  _this().staticFeatureGenerator(staticVisitor);
}

FEATURE_GENERATOR_DEFAULT_IMPL(void) addWeightedTo(SparseVectorPtr target, double weight) const
{
  target->ensureDictionary(_this().getDictionary());
  if (!weight)
    return;  
  impl::AddWeightedVectorOperation op(weight);
  impl::AssignmentToSparseVisitor<impl::AddWeightedVectorOperation> staticVisitor(target, op);
  _this().staticFeatureGenerator(staticVisitor);
}

FEATURE_GENERATOR_DEFAULT_IMPL(void) addWeightedSignsTo(DenseVectorPtr target, double weight) const
{
  target->ensureDictionary(_this().getDictionary());
  if (!weight)
    return;  
  impl::AddWeightedSignsVectorOperation op(weight);
  impl::AssignmentToDenseVisitor<impl::AddWeightedSignsVectorOperation> staticVisitor(target, op);
  _this().staticFeatureGenerator(staticVisitor);
}

FEATURE_GENERATOR_DEFAULT_IMPL(double) dotProduct(const SparseVectorPtr vector) const
{
  vector->ensureDictionary(_this().getDictionary());
  impl::DotProductSparseVectorVisitor staticVisitor(vector);
  _this().staticFeatureGenerator(staticVisitor);
  return staticVisitor.getResult();
}

FEATURE_GENERATOR_DEFAULT_IMPL(double) dotProduct(const DenseVectorPtr vector) const
{
  vector->ensureDictionary(_this().getDictionary());
  impl::DotProductDenseVectorVisitor staticVisitor(vector);
  _this().staticFeatureGenerator(staticVisitor);
  return staticVisitor.getResult();
}

FEATURE_GENERATOR_DEFAULT_IMPL(double) dotProduct(const FeatureGeneratorPtr featureGenerator) const
{
  if (featureGenerator->isDense())
    return dotProduct(featureGenerator->toDenseVector());
  else
    return dotProduct(featureGenerator->toSparseVector());
}

# undef FEATURE_GENERATOR_DEFAULT_IMPL

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_DEFAULT_IMPLEMENTATIONS_HPP_

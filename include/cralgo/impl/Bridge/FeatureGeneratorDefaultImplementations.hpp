/*-----------------------------------------.---------------------------------.
| Filename: FeatureGeneratorDefaultImpl...h| Feature generator default       |
| Author  : Francis Maes                   |   implementations               |
| Started : 01/03/2009 18:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_FEATURE_GENERATOR_DEFAULT_IMPLEMENTATIONS_HPP_
# define CRALGO_FEATURE_GENERATOR_DEFAULT_IMPLEMENTATIONS_HPP_

# include "../FeatureVisitor/FeatureVisitorStaticToDynamic.hpp"
# include "../FeatureVisitor/FeatureVisitorDynamicToStatic.hpp"
# include "../FeatureVisitor/CreateSparseVectorVisitor.hpp"
# include "../FeatureVisitor/UnaryOperationVisitor.hpp"
# include "../FeatureVisitor/AssignmentVisitor.hpp"
# include "../FeatureVisitor/DotProductVisitor.hpp"
# include "../FeatureVisitor/StringDescriptionVisitor.hpp"

namespace cralgo
{

#define FEATURE_GENERATOR_DEFAULT_IMPL(ReturnType) \
  template<class ExactType, class BaseType> \
  inline ReturnType FeatureGeneratorDefaultImplementations<ExactType, BaseType>::

FEATURE_GENERATOR_DEFAULT_IMPL(void) accept(FeatureVisitorPtr visitor, FeatureDictionary* dictionary) const
{
  impl::DynamicToStaticFeatureVisitor staticVisitor(visitor);
  featureGenerator(staticVisitor, dictionary);  
}

FEATURE_GENERATOR_DEFAULT_IMPL(SparseVectorPtr) toSparseVector(FeatureDictionary* dictionary) const
{
  impl::CreateSparseVectorVisitor staticVisitor(selectDictionary(dictionary));
  featureGenerator(staticVisitor, dictionary);
  return staticVisitor.getResult();
}

FEATURE_GENERATOR_DEFAULT_IMPL(DenseVectorPtr) toDenseVector(FeatureDictionary* dictionary) const
{
  impl::CreateDenseVectorVisitor staticVisitor(selectDictionary(dictionary));
  featureGenerator(staticVisitor, dictionary);
  return staticVisitor.getResult();
}

FEATURE_GENERATOR_DEFAULT_IMPL(std::string) toString(FeatureDictionary* dictionary) const
{
  impl::StringDescriptionVisitor staticVisitor;
  featureGenerator(staticVisitor, dictionary);
  return staticVisitor.getResult();
}

FEATURE_GENERATOR_DEFAULT_IMPL(size_t) l0norm() const
{
  impl::ComputeL0NormVectorOperation op;
  impl::UnaryOperationVisitor<impl::ComputeL0NormVectorOperation> staticVisitor(op);
  featureGenerator(staticVisitor, NULL);
  return op.res;
}

FEATURE_GENERATOR_DEFAULT_IMPL(double) l1norm() const
{
  impl::ComputeL1NormVectorOperation op;
  impl::UnaryOperationVisitor<impl::ComputeL1NormVectorOperation> staticVisitor(op);
  featureGenerator(staticVisitor, NULL);
  return op.res;
}

FEATURE_GENERATOR_DEFAULT_IMPL(double) sumOfSquares() const
{
  impl::ComputeSumOfSquaresVectorOperation op;
  impl::UnaryOperationVisitor<impl::ComputeSumOfSquaresVectorOperation> staticVisitor(op);
  featureGenerator(staticVisitor, NULL);
  return op.res;
}

FEATURE_GENERATOR_DEFAULT_IMPL(void) addTo(DenseVectorPtr target, FeatureDictionary* dictionary) const
{
  target->ensureDictionary(selectDictionary(dictionary));
  impl::AddVectorOperation op;
  impl::AssignmentToDenseVisitor<impl::AddVectorOperation> staticVisitor(target, op);
  featureGenerator(staticVisitor, dictionary);
}

FEATURE_GENERATOR_DEFAULT_IMPL(void) substractFrom(DenseVectorPtr target, FeatureDictionary* dictionary) const
{
  target->ensureDictionary(selectDictionary(dictionary));
  impl::SubstractVectorOperation op;
  impl::AssignmentToDenseVisitor<impl::SubstractVectorOperation> staticVisitor(target, op);
  featureGenerator(staticVisitor, dictionary);
}

FEATURE_GENERATOR_DEFAULT_IMPL(void) addWeightedTo(DenseVectorPtr target, double weight, FeatureDictionary* dictionary) const
{
  target->ensureDictionary(selectDictionary(dictionary));
  impl::AddWeightedVectorOperation op(weight);
  impl::AssignmentToDenseVisitor<impl::AddWeightedVectorOperation> staticVisitor(target, op);
  featureGenerator(staticVisitor, dictionary);
}

FEATURE_GENERATOR_DEFAULT_IMPL(void) addWeightedTo(SparseVectorPtr target, double weight, FeatureDictionary* dictionary) const
{
  target->ensureDictionary(selectDictionary(dictionary));
  impl::AddWeightedVectorOperation op(weight);
  impl::AssignmentToSparseVisitor<impl::AddWeightedVectorOperation> staticVisitor(target, op);
  featureGenerator(staticVisitor, dictionary);
}

FEATURE_GENERATOR_DEFAULT_IMPL(void) addWeightedSignsTo(DenseVectorPtr target, double weight, FeatureDictionary* dictionary) const
{
  target->ensureDictionary(selectDictionary(dictionary));
  impl::AddWeightedSignsVectorOperation op(weight);
  impl::AssignmentToDenseVisitor<impl::AddWeightedSignsVectorOperation> staticVisitor(target, op);
  featureGenerator(staticVisitor, dictionary);
}

FEATURE_GENERATOR_DEFAULT_IMPL(double) dotProduct(const DenseVectorPtr vector, FeatureDictionary* dictionary) const
{
  vector->ensureDictionary(selectDictionary(dictionary));
  impl::DotProductDenseVectorVisitor staticVisitor(vector);
  featureGenerator(staticVisitor, dictionary);
  return staticVisitor.getResult();
}

# undef FEATURE_GENERATOR_DEFAULT_IMPL

}; /* namespace cralgo */

#endif // !CRALGO_FEATURE_GENERATOR_DEFAULT_IMPLEMENTATIONS_HPP_

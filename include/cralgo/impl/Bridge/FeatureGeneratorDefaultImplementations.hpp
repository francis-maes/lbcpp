/*-----------------------------------------.---------------------------------.
| Filename: FeatureGeneratorDefaultImpl...h| Feature generator default       |
| Author  : Francis Maes                   |   implementations               |
| Started : 01/03/2009 18:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_FEATURE_GENERATOR_DEFAULT_IMPLEMENTATIONS_HPP_
# define CRALGO_FEATURE_GENERATOR_DEFAULT_IMPLEMENTATIONS_HPP_

# include "../FeatureVisitor/StaticFeatureVisitor.hpp"
# include "../FeatureVisitor/DynamicToStaticFeatureVisitor.hpp"
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

FEATURE_GENERATOR_DEFAULT_IMPL(void) accept(FeatureVisitor& visitor, FeatureDictionary* dictionary) const
{
  DynamicToStaticFeatureVisitor staticVisitor(visitor);
  featureGenerator(staticVisitor, dictionary);  
}

FEATURE_GENERATOR_DEFAULT_IMPL(SparseVectorPtr) toSparseVector(FeatureDictionary* dictionary) const
{
  CreateSparseVectorVisitor staticVisitor(selectDictionary(dictionary));
  featureGenerator(staticVisitor, dictionary);
  return staticVisitor.getResult();
}

FEATURE_GENERATOR_DEFAULT_IMPL(DenseVectorPtr) toDenseVector(FeatureDictionary* dictionary) const
{
  CreateDenseVectorVisitor staticVisitor(selectDictionary(dictionary));
  featureGenerator(staticVisitor, dictionary);
  return staticVisitor.getResult();
}

FEATURE_GENERATOR_DEFAULT_IMPL(std::string) toString(FeatureDictionary* dictionary) const
{
  StringDescriptionVisitor staticVisitor;
  featureGenerator(staticVisitor, dictionary);
  return staticVisitor.getResult();
}

FEATURE_GENERATOR_DEFAULT_IMPL(size_t) l0norm() const
{
  ComputeL0NormVectorOperation op;
  UnaryOperationVisitor<ComputeL0NormVectorOperation> staticVisitor(op);
  featureGenerator(staticVisitor, NULL);
  return op.res;
}

FEATURE_GENERATOR_DEFAULT_IMPL(double) l1norm() const
{
  ComputeL1NormVectorOperation op;
  UnaryOperationVisitor<ComputeL1NormVectorOperation> staticVisitor(op);
  featureGenerator(staticVisitor, NULL);
  return op.res;
}

FEATURE_GENERATOR_DEFAULT_IMPL(double) sumOfSquares() const
{
  ComputeSumOfSquaresVectorOperation op;
  UnaryOperationVisitor<ComputeSumOfSquaresVectorOperation> staticVisitor(op);
  featureGenerator(staticVisitor, NULL);
  return op.res;
}

FEATURE_GENERATOR_DEFAULT_IMPL(void) addTo(DenseVectorPtr target, FeatureDictionary* dictionary) const
{
  target->ensureDictionary(selectDictionary(dictionary));
  AddVectorOperation op;
  AssignmentToDenseVisitor<AddVectorOperation> staticVisitor(target, op);
  featureGenerator(staticVisitor, dictionary);
}

FEATURE_GENERATOR_DEFAULT_IMPL(void) substractFrom(DenseVectorPtr target, FeatureDictionary* dictionary) const
{
  target->ensureDictionary(selectDictionary(dictionary));
  SubstractVectorOperation op;
  AssignmentToDenseVisitor<SubstractVectorOperation> staticVisitor(target, op);
  featureGenerator(staticVisitor, dictionary);
}

FEATURE_GENERATOR_DEFAULT_IMPL(void) addWeightedTo(DenseVectorPtr target, double weight, FeatureDictionary* dictionary) const
{
  target->ensureDictionary(selectDictionary(dictionary));
  AddWeightedVectorOperation op(weight);
  AssignmentToDenseVisitor<AddWeightedVectorOperation> staticVisitor(target, op);
  featureGenerator(staticVisitor, dictionary);
}

FEATURE_GENERATOR_DEFAULT_IMPL(void) addWeightedSignsTo(DenseVectorPtr target, double weight, FeatureDictionary* dictionary) const
{
  target->ensureDictionary(selectDictionary(dictionary));
  AddWeightedSignsVectorOperation op(weight);
  AssignmentToDenseVisitor<AddWeightedSignsVectorOperation> staticVisitor(target, op);
  featureGenerator(staticVisitor, dictionary);
}

FEATURE_GENERATOR_DEFAULT_IMPL(double) dotProduct(const DenseVectorPtr vector, FeatureDictionary* dictionary) const
{
  vector->ensureDictionary(selectDictionary(dictionary));
  DotProductDenseVectorVisitor staticVisitor(vector);
  featureGenerator(staticVisitor, dictionary);
  return staticVisitor.getResult();
}


# undef FEATURE_GENERATOR_DEFAULT_IMPL

}; /* namespace cralgo */

#endif // !CRALGO_FEATURE_GENERATOR_DEFAULT_IMPLEMENTATIONS_HPP_

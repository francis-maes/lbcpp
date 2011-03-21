/*-----------------------------------------.---------------------------------.
| Filename: CartesianProductFeatureGene...h| Cartesian Product               |
| Author  : Julien Becker                  |          Feature Generator      |
| Started : 17/02/2011 15:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_GENERIC_CARTESIAN_PRODUCT_H_
# define LBCPP_FEATURE_GENERATOR_GENERIC_CARTESIAN_PRODUCT_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

// DoubleVector[n], DoubleVector[m] -> DoubleVector[n x m]
class CartesianProductFeatureGenerator : public FeatureGenerator
{
public:
  CartesianProductFeatureGenerator(bool lazy)
    : FeatureGenerator(lazy) {}
  CartesianProductFeatureGenerator() {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return (TypePtr)doubleVectorClass();}
  
  virtual String getOutputPostFix() const
    {return T("CartesianProduct");}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    EnumerationPtr firstEnum = DoubleVector::getElementsEnumeration(inputVariables[0]->getType());
    EnumerationPtr secondEnum = DoubleVector::getElementsEnumeration(inputVariables[1]->getType());
    jassert(firstEnum && secondEnum);
    numSecondElements = secondEnum->getNumElements();
    return cartesianProductEnumerationEnumeration(firstEnum, secondEnum);
  }

  virtual size_t l0norm(const Variable* inputs) const
  {
    const DoubleVectorPtr& v1 = inputs[0].getObjectAndCast<DoubleVector>();
    const DoubleVectorPtr& v2 = inputs[1].getObjectAndCast<DoubleVector>();
    return v1 && v2 ? v1->l0norm() * v2->l0norm() : 0;
  }

  virtual double sumOfSquares(const Variable* inputs) const
  {
    const DoubleVectorPtr& v1 = inputs[0].getObjectAndCast<DoubleVector>();
    const DoubleVectorPtr& v2 = inputs[1].getObjectAndCast<DoubleVector>();
    return v1 && v2 ? v1->sumOfSquares() * v2->sumOfSquares() : 0;
  }

  virtual double dotProduct(const Variable* inputs, const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector) const
  {
    const DoubleVectorPtr& v1 = inputs[0].getObjectAndCast<DoubleVector>();
    const DoubleVectorPtr& v2 = inputs[1].getObjectAndCast<DoubleVector>();

    if (!v1 || !v2)
      return 0.0;

    double res = 0.0;
    SparseDoubleVectorPtr s1 = v1->toSparseVector();
    size_t n1 = s1->getNumValues();
    if (!n1)
      return 0.0;

    for (size_t i = 0; i < n1; ++i)
    {
      const std::pair<size_t, double>& indexAndWeight1 = s1->getValue(i);
      if (indexAndWeight1.second)
      {
        size_t startIndex = indexAndWeight1.first * numSecondElements;
        res += indexAndWeight1.second * v2->dotProduct(denseVector, offsetInDenseVector + startIndex);
      }
    }
    return res;
  }

  virtual void addWeightedTo(const Variable* inputs, const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector, double weight) const
  {
    const DoubleVectorPtr& v1 = inputs[0].getObjectAndCast<DoubleVector>();
    const DoubleVectorPtr& v2 = inputs[1].getObjectAndCast<DoubleVector>();

    if (!v1 || !v2)
      return;

    SparseDoubleVectorPtr s1 = v1->toSparseVector();
    size_t n1 = s1->getNumValues();
    if (!n1)
      return;

    for (size_t i = 0; i < n1; ++i)
    {
      const std::pair<size_t, double>& indexAndWeight1 = s1->getValue(i);
      if (indexAndWeight1.second)
      {
        size_t startIndex = indexAndWeight1.first * numSecondElements;
        v2->addWeightedTo(denseVector, offsetInDenseVector + startIndex, weight * indexAndWeight1.second);
      }
    }
  }

  virtual void appendTo(const Variable* inputs, const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const
  {
    const DoubleVectorPtr& v1 = inputs[0].getObjectAndCast<DoubleVector>();
    const DoubleVectorPtr& v2 = inputs[1].getObjectAndCast<DoubleVector>();

    if (!v1 || !v2)
      return;
    
    SparseDoubleVectorPtr s1 = v1->toSparseVector();
    size_t n1 = s1->getNumValues();
    if (!n1)
      return;

    for (size_t i = 0; i < n1; ++i)
    {
      const std::pair<size_t, double>& indexAndWeight1 = s1->getValue(i);
      if (indexAndWeight1.second)
      {
        size_t startIndex = indexAndWeight1.first * numSecondElements;
        v2->appendTo(sparseVector, offsetInSparseVector + startIndex, weight * indexAndWeight1.second);
      }
    }
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const DoubleVectorPtr& v1 = inputs[0].getObjectAndCast<DoubleVector>();
    const DoubleVectorPtr& v2 = inputs[1].getObjectAndCast<DoubleVector>();
    if (!v1 || !v2)
      return;

    SparseDoubleVectorPtr s1 = v1->toSparseVector();
    size_t n1 = s1->getNumValues();

    SparseDoubleVectorPtr s2 = v2->toSparseVector();
    size_t n2 = s2->getNumValues();

    if (!n1 || !n2)
      return;

    for (size_t i = 0; i < n1; ++i)
    {
      const std::pair<size_t, double>& indexAndWeight1 = s1->getValue(i);
      if (!indexAndWeight1.second)
        continue;
      size_t startIndex = indexAndWeight1.first * numSecondElements;
      for (size_t j = 0; j < n2; ++j)
      {
        const std::pair<size_t, double>& indexAndWeight2 = s2->getValue(j);
        callback.sense(startIndex + indexAndWeight2.first, indexAndWeight1.second * indexAndWeight2.second);
      }
    }
  }
  
protected:
  size_t numSecondElements;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_GENERIC_CARTESIAN_PRODUCT_H_

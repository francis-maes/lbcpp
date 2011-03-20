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

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    SparseDoubleVectorPtr s1 = inputs[0].getObjectAndCast<DoubleVector>()->toSparseVector();
    SparseDoubleVectorPtr s2 = inputs[1].getObjectAndCast<DoubleVector>()->toSparseVector();

    if (s1 && s2 && s1->getValues().size() && s2->getValues().size())
    {
      for (size_t i = 0; i < s1->getValues().size(); ++i)
      {
        std::pair<size_t, double> indexAndWeight1 = s1->getValues()[i];
        if (!indexAndWeight1.second)
          continue;
        size_t startIndex = indexAndWeight1.first * numSecondElements;
        for (size_t j = 0; j < s2->getValues().size(); ++j)
        {
          std::pair<size_t, double> indexAndWeight2 = s2->getValues()[j];
          callback.sense(startIndex + indexAndWeight2.first, indexAndWeight1.second * indexAndWeight2.second);
        }
      }
    }
  }
  
protected:
  size_t numSecondElements;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_GENERIC_CARTESIAN_PRODUCT_H_

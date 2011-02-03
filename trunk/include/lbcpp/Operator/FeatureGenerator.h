/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerator.h             | Base class for Feature          |
| Author  : Francis Maes                   |  Geneators                      |
| Started : 01/02/2011 16:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPERATOR_FEATURE_GENERATOR_H_
# define LBCPP_OPERATOR_FEATURE_GENERATOR_H_

# include "VariableGenerator.h"

namespace lbcpp
{

class DoubleVector;
typedef ReferenceCountedObjectPtr<DoubleVector> DoubleVectorPtr;

class SparseDoubleVector;
typedef ReferenceCountedObjectPtr<SparseDoubleVector> SparseDoubleVectorPtr;

class DenseDoubleVector;
typedef ReferenceCountedObjectPtr<DenseDoubleVector> DenseDoubleVectorPtr;

class LazyDoubleVector;
typedef ReferenceCountedObjectPtr<LazyDoubleVector> LazyDoubleVectorPtr;

class CompositeDoubleVector;
typedef ReferenceCountedObjectPtr<CompositeDoubleVector> CompositeDoubleVectorPtr;

class DoubleVector : public Vector
{
public:
  DoubleVector(ClassPtr thisClass)
    : Vector(thisClass) {}
  DoubleVector() {}

  virtual EnumerationPtr getElementsEnumeration() const
    {return thisClass->getTemplateArgument(0);}

  virtual TypePtr getElementsType() const
    {return thisClass->getTemplateArgument(1);}
};

extern ClassPtr doubleVectorClass(TypePtr elementsEnumeration = positiveIntegerEnumerationEnumeration, TypePtr elementsType = doubleType);
extern ClassPtr sparseDoubleVectorClass(TypePtr elementsEnumeration = positiveIntegerEnumerationEnumeration, TypePtr elementsType = doubleType);
extern ClassPtr denseDoubleVectorClass(TypePtr elementsEnumeration = positiveIntegerEnumerationEnumeration, TypePtr elementsType = doubleType);
extern ClassPtr lazyDoubleVectorClass(TypePtr elementsEnumeration = positiveIntegerEnumerationEnumeration, TypePtr elementsType = doubleType);
extern ClassPtr compositeDoubleVectorClass(TypePtr elementsEnumeration = positiveIntegerEnumerationEnumeration, TypePtr elementsType = doubleType);

class FeatureGenerator : public VariableGenerator
{
public:
  virtual EnumerationPtr getFeaturesEnumeration(ExecutionContext& context, TypePtr& elementsType) = 0;

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

protected:
  EnumerationPtr featuresEnumeration;
  TypePtr featuresType;

  bool getDoubleVectorParameters(ExecutionContext& context, TypePtr type, EnumerationPtr& elementsEnumeration, TypePtr& elementsType) const;

  TypePtr computeOutputType(ExecutionContext& context);
};

typedef ReferenceCountedObjectPtr<FeatureGenerator> FeatureGeneratorPtr;


class SparseDoubleVector : public DoubleVector
{
public:
  SparseDoubleVector(EnumerationPtr elementsEnumeration, TypePtr elementsType)
    : DoubleVector(sparseDoubleVectorClass(elementsEnumeration, elementsType)) {}
  SparseDoubleVector(ClassPtr thisClass)
    : DoubleVector(thisClass) {}
  SparseDoubleVector() {}

  void appendValue(size_t index, double value)
  {
    jassert(values.empty() || index > values.back().first);
    values.push_back(std::make_pair(index, value));
  }

  // Vector
  virtual void clear()
    {values.clear();}

  virtual void reserve(size_t size)
    {jassert(false);}

  virtual void resize(size_t size)
    {jassert(false);}

  virtual void prepend(const Variable& value)
    {jassert(false);}

  virtual void append(const Variable& value)
    {jassert(false);}

  virtual void remove(size_t index)
    {jassert(false);}

  // Container
  virtual size_t getNumElements() const
    {return values.empty() ? 0 : values.back().first + 1;}

  virtual Variable getElement(size_t index) const
    {jassert(false); return Variable();}

  virtual void setElement(size_t index, const Variable& value)
  {}

private:
  std::vector< std::pair<size_t, double> > values;
};

class DenseDoubleVector : public DoubleVector
{
public:
  DenseDoubleVector(EnumerationPtr elementsEnumeration, TypePtr elementsType = doubleType)
    : DoubleVector(denseDoubleVectorClass(elementsEnumeration, elementsType)) {}
  DenseDoubleVector() {}
  
    // Vector
  virtual void clear()
    {jassert(false);}

  virtual void reserve(size_t size)
    {jassert(false);}

  virtual void resize(size_t size)
    {jassert(false);}

  virtual void prepend(const Variable& value)
    {jassert(false);}

  virtual void append(const Variable& value)
    {jassert(false);}

  virtual void remove(size_t index)
    {jassert(false);}

  // Container
  virtual size_t getNumElements() const
    {jassert(false); return 0;}

  virtual Variable getElement(size_t index) const
    {jassert(false); return Variable();}

  virtual void setElement(size_t index, const Variable& value)
    {jassert(false);}

private:
  std::vector<double>* values;
  bool ownValues;
};

class LazyDoubleVector : public DoubleVector
{
public:
  LazyDoubleVector(FeatureGeneratorPtr featureGenerator, const Variable* inputs)
    : featureGenerator(featureGenerator), inputs(featureGenerator->getNumInputs())
  {
    for (size_t i = 0; i < this->inputs.size(); ++i)
      this->inputs[i] = inputs[i];
  }
  LazyDoubleVector() {}

    // Vector
  virtual void clear()
    {jassert(false);}

  virtual void reserve(size_t size)
    {jassert(false);}

  virtual void resize(size_t size)
    {jassert(false);}

  virtual void prepend(const Variable& value)
    {jassert(false);}

  virtual void append(const Variable& value)
    {jassert(false);}

  virtual void remove(size_t index)
    {jassert(false);}

  // Container
  virtual size_t getNumElements() const
    {jassert(false); return 0;}

  virtual Variable getElement(size_t index) const
    {jassert(false); return Variable();}

  virtual void setElement(size_t index, const Variable& value)
    {jassert(false);}

private:
  FeatureGeneratorPtr featureGenerator;
  std::vector<Variable> inputs;
};

class CompositeDoubleVector : public DoubleVector
{
public:
  CompositeDoubleVector(EnumerationPtr elementsEnumeration, TypePtr elementsType)
    : DoubleVector(compositeDoubleVectorClass(elementsEnumeration, elementsType)) {}
  CompositeDoubleVector() {}
  
  virtual size_t getNumElements() const
    {return numElements;}

  virtual Variable getElement(size_t index) const
    {jassert(false); return Variable();}

  virtual void setElement(size_t index, const Variable& value)
    {jassert(false);}

  void appendSubVector(size_t shift, const DoubleVectorPtr& vector)
    {vectors.push_back(std::make_pair(shift, vector));}
  
  virtual void clear()
    {vectors.clear();}

  virtual void reserve(size_t size)
    {}

  virtual void resize(size_t size)
    {numElements = size;}

  virtual void prepend(const Variable& value)
    {jassert(false);}

  virtual void append(const Variable& value)
    {jassert(false);}

  virtual void remove(size_t index)
    {jassert(false);}

private:
  std::vector< std::pair<size_t, DoubleVectorPtr> > vectors;
  size_t numElements;
};


extern FeatureGeneratorPtr concatenateFeatureGenerator();

extern FeatureGeneratorPtr enumerationFeatureGenerator();
extern FeatureGeneratorPtr enumerationDistributionFeatureGenerator();

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_FEATURE_GENERATOR_H_

/*-----------------------------------------.---------------------------------.
| Filename: DoubleVector.cpp               | Base class for Double Vectors   |
| Author  : Francis Maes                   |                                 |
| Started : 03/02/2011 16:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/DoubleVector.h>
#include <lbcpp/FeatureGenerator/FeatureGenerator.h>
#include "../Core/Object/SparseVectorHelper.h"
using namespace lbcpp;

/*
** DoubleVector
*/
bool DoubleVector::getTemplateParameters(ExecutionContext& context, TypePtr type, EnumerationPtr& elementsEnumeration, TypePtr& elementsType)
{
  TypePtr dvType = type->findBaseTypeFromTemplateName(T("DoubleVector"));
  if (!dvType)
  {
    context.errorCallback(type->getName() + T(" is not a DoubleVector"));
    return false;
  }
  jassert(dvType->getNumTemplateArguments() == 2);
  elementsEnumeration = dvType->getTemplateArgument(0);
  elementsType = dvType->getTemplateArgument(1);
  return true;
}  

/*
** SparseDoubleVector
*/
SparseDoubleVector::SparseDoubleVector(EnumerationPtr elementsEnumeration, TypePtr elementsType)
  : DoubleVector(sparseDoubleVectorClass(elementsEnumeration, elementsType)), lastIndex(-1)  {}

SparseDoubleVector::SparseDoubleVector(ClassPtr thisClass)
  : DoubleVector(thisClass), lastIndex(-1) {}

SparseDoubleVector::SparseDoubleVector()
  : lastIndex(-1) {}

// double vector
void SparseDoubleVector::appendTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector) const
{
  if (values.empty())
    return;

  jassert((int)offsetInSparseVector > sparseVector->lastIndex);
  std::vector<std::pair<size_t, double> >& targetValues = sparseVector->getValues();
  targetValues.reserve(targetValues.size() + values.size());
  for (size_t i = 0; i < values.size(); ++i)
    targetValues.push_back(std::make_pair(values[i].first + offsetInSparseVector, values[i].second));
  sparseVector->updateLastIndex();
}

void SparseDoubleVector::addWeightedTo(const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector, double weight) const
{
  denseVector->ensureSize(offsetInDenseVector + (size_t)(lastIndex + 1));
  double* target = denseVector->getValuePointer(offsetInDenseVector);
  for (size_t i = 0; i < values.size(); ++i)
    target[values[i].first] += values[i].second * weight;
}

double SparseDoubleVector::dotProduct(const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector) const
{
  double res = 0.0;
  const double* target = denseVector->getValuePointer(offsetInDenseVector);
  for (size_t i = 0; i < values.size(); ++i)
    res += target[values[i].first] * values[i].second;
  return res;
}

// vector
void SparseDoubleVector::clear()
  {values.clear();}

void SparseDoubleVector::reserve(size_t size)
  {jassert(false);}

void SparseDoubleVector::resize(size_t size)
  {jassert(false);}

void SparseDoubleVector::prepend(const Variable& value)
  {jassert(false);}

void SparseDoubleVector::append(const Variable& value)
  {jassert(false);}

void SparseDoubleVector::remove(size_t index)
  {jassert(false);}

// Container
size_t SparseDoubleVector::getNumElements() const
  {return (size_t)(lastIndex + 1);}

Variable SparseDoubleVector::getElement(size_t index) const
{
  const double* value = SparseDoubleVectorHelper::get(values, index);
  return Variable(value ? *value : 0.0, getElementsType());
}

void SparseDoubleVector::setElement(size_t index, const Variable& value)
{
  jassert(value.exists() && value.isDouble());
  if ((int)index > lastIndex)
  {
    appendValue(index, value.getDouble());
    return;
  }
  else
    SparseDoubleVectorHelper::set(values, index, value.getDouble());
}

/*
** DenseDoubleVector
*/
DenseDoubleVector::DenseDoubleVector(ClassPtr thisClass, std::vector<double>& values)
  : DoubleVector(thisClass), values(&values), ownValues(false) {}

DenseDoubleVector::DenseDoubleVector(ClassPtr thisClass, size_t initialSize, double initialValue)
  : DoubleVector(thisClass), ownValues(true)
{
  if (initialSize == (size_t)-1)
    initialSize = getElementsEnumeration()->getNumElements();
  values = new std::vector<double>(initialSize, initialValue);
}

DenseDoubleVector::DenseDoubleVector()
  : values(NULL), ownValues(false)
{
}

DenseDoubleVector::~DenseDoubleVector()
  {if (values && ownValues) delete values;}

void DenseDoubleVector::ensureSize(size_t minimumSize)
{
  if (!values)
  {
    values = new std::vector<double>(minimumSize, 0.0);
    ownValues = true;
  }
  else if (values->size() < minimumSize)
    values->resize(minimumSize, 0.0);

}

void DenseDoubleVector::multiplyByScalar(double value)
{
  jassert(values);
  if (value == 1.0)
    return;
  else if (value == 0.0)
    memset(&(*values)[0], 0, sizeof (double) * values->size());
  else
  {
    double* pointer = getValuePointer(0);
    double* limit = getValuePointer(values->size());
    while (pointer < limit)
    {
      (*pointer) *= value;
      ++pointer;
    }
  }      
}

// DoubleVector
void DenseDoubleVector::appendTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector) const
{
  if (!values)
    return;

  jassert((int)offsetInSparseVector > sparseVector->getLastIndex());
  std::vector< std::pair<size_t, double> >& target = sparseVector->getValues();

  target.reserve(target.size() + values->size());
  const double* source = getValuePointer(0);
  for (size_t i = 0; i < values->size(); ++i)
    if (source[i])
      target.push_back(std::make_pair(offsetInSparseVector + i, source[i]));
  sparseVector->updateLastIndex();
}

void DenseDoubleVector::addWeightedTo(const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector, double weight) const
{
  if (values)
  {
    denseVector->ensureSize(offsetInDenseVector + values->size());
    double* target = denseVector->getValuePointer(offsetInDenseVector);
    const double* source = getValuePointer(0);
    const double* limit = getValuePointer(values->size());
    while (source < limit)
    {
      *target += weight * (*source);
      ++source;
      ++target;
    }
  }
}

double DenseDoubleVector::dotProduct(const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector) const
{
  if (!values || !denseVector->values)
    return 0.0;
  const double* target = denseVector->getValuePointer(offsetInDenseVector);
  const double* source = getValuePointer(0);
  const double* limit = getValuePointer(values->size());
  double res = 0.0;
  while (source < limit)
  {
    res += (*source) * (*target);
    ++source;
    ++target;
  }
  return res;
}

  // Vector
void DenseDoubleVector::clear()
{
  if (values)
    values->clear();
}

void DenseDoubleVector::reserve(size_t size)
{
  if (!values)
  {
    values = new std::vector<double>();
    ownValues = true;
  }
  values->reserve(size);
}

void DenseDoubleVector::resize(size_t size)
{
  if (!values)
  {
    values = new std::vector<double>();
    ownValues = true;
  }
  values->resize(size);
}

void DenseDoubleVector::prepend(const Variable& value)
{
  // this operation is not permitted
  jassert(false);
  //values->insert(values->begin(), value.getDouble());
}

void DenseDoubleVector::append(const Variable& value)
  {values->push_back(value.getDouble());}

void DenseDoubleVector::remove(size_t index)
{
  // this operation is not permitted
  jassert(false);
}

// Container
size_t DenseDoubleVector::getNumElements() const
{return values ? values->size() : 0;}

Variable DenseDoubleVector::getElement(size_t index) const
  {return Variable(values && index < values->size() ? (*values)[index] : 0.0, getElementsType());}

void DenseDoubleVector::setElement(size_t index, const Variable& value)
  {jassert(index < values->size() && value.isDouble()); (*values)[index] = value.getDouble();}

// Object
void DenseDoubleVector::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  const DenseDoubleVectorPtr& target = t.staticCast<DenseDoubleVector>();
  if (values)
  {
    target->values = new std::vector<double>(*values);
    target->ownValues = true;
  }
}

/*
** LazyDoubleVector
*/
LazyDoubleVector::LazyDoubleVector(FeatureGeneratorPtr featureGenerator, const Variable* inputs)
  : DoubleVector(lazyDoubleVectorClass(featureGenerator->getFeaturesEnumeration(), featureGenerator->getFeaturesType())),
    featureGenerator(featureGenerator), inputs(featureGenerator->getNumInputs())
{
  for (size_t i = 0; i < this->inputs.size(); ++i)
    this->inputs[i] = inputs[i];
}

// DoubleVector
void LazyDoubleVector::appendTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector) const
{
  if (computedVector)
    computedVector->appendTo(sparseVector, offsetInSparseVector);
  else
    featureGenerator->appendTo(&inputs[0], sparseVector, offsetInSparseVector);
}

void LazyDoubleVector::addWeightedTo(const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector, double weight) const
{
  if (computedVector)
    computedVector->addWeightedTo(denseVector, offsetInDenseVector, weight);
  else
    featureGenerator->addWeightedTo(&inputs[0], denseVector, offsetInDenseVector, weight);
}

double LazyDoubleVector::dotProduct(const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector) const
{
  if (computedVector)
    return computedVector->dotProduct(denseVector, offsetInDenseVector);
  else
    return featureGenerator->dotProduct(&inputs[0], denseVector, offsetInDenseVector);
}

  // Vector
void LazyDoubleVector::clear()
  {jassert(false);}

void LazyDoubleVector::reserve(size_t size)
  {jassert(false);}

void LazyDoubleVector::resize(size_t size)
  {jassert(false);}

void LazyDoubleVector::prepend(const Variable& value)
  {jassert(false);}

void LazyDoubleVector::append(const Variable& value)
  {jassert(false);}

void LazyDoubleVector::remove(size_t index)
  {jassert(false);}

void LazyDoubleVector::ensureIsComputed()
{
  if (!computedVector)
    computedVector = featureGenerator->toComputedVector(&inputs[0]);
}

// Container
size_t LazyDoubleVector::getNumElements() const
  {return getElementsEnumeration()->getNumElements();}

Variable LazyDoubleVector::getElement(size_t index) const
{
  const_cast<LazyDoubleVector* >(this)->ensureIsComputed();
  return computedVector->getElement(index);
}

void LazyDoubleVector::setElement(size_t index, const Variable& value)
{
  // not allowed
  jassert(false);
}

/*
** CompositeDoubleVector
*/
// DoubleVector
void CompositeDoubleVector::appendTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector) const
{
  for (size_t i = 0; i < vectors.size(); ++i)
  {
    const std::pair<size_t, DoubleVectorPtr>& subVector = vectors[i];
    subVector.second->appendTo(sparseVector, offsetInSparseVector + subVector.first);
  }
}

void CompositeDoubleVector::addWeightedTo(const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector, double weight) const
{
  for (size_t i = 0; i < vectors.size(); ++i)
  {
    const std::pair<size_t, DoubleVectorPtr>& subVector = vectors[i];
    subVector.second->addWeightedTo(denseVector, offsetInDenseVector + subVector.first, weight);
  }
}

double CompositeDoubleVector::dotProduct(const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector) const
{
  double res = 0.0;
  for (size_t i = 0; i < vectors.size(); ++i)
  {
    const std::pair<size_t, DoubleVectorPtr>& subVector = vectors[i];
    res += subVector.second->dotProduct(denseVector, offsetInDenseVector + subVector.first);
  }
  return res;
}

// Container
size_t CompositeDoubleVector::getNumElements() const
{
  if (vectors.empty())
    return 0;
  const std::pair<size_t, DoubleVectorPtr>& lastSubVector = vectors.back();
  return lastSubVector.first + lastSubVector.second->getNumElements();
}

Variable CompositeDoubleVector::getElement(size_t index) const
{
  if (vectors.empty())
    return missingElement();
  
  if (index < vectors[0].first)
    return missingElement();

  int i = 1;
  while (i < (int)vectors.size() && vectors[i].first < index)
    ++i;
  --i;
  jassert(i >= 0 && i < (int)vectors.size());
  return vectors[i].second->getElement(index - vectors[i].first);
}

void CompositeDoubleVector::setElement(size_t index, const Variable& value)
{
  if (vectors.empty())
    {jassert(false); return;}
  
  if (index < vectors[0].first)
    {jassert(false); return;}

  int i = 1;
  while (i < (int)vectors.size() && vectors[i].first < index)
    ++i;
  --i;
  jassert(i >= 0 && i < (int)vectors.size());
  return vectors[i].second->setElement(index - vectors[i].first, value);
}

void CompositeDoubleVector::clear()
  {vectors.clear();}

void CompositeDoubleVector::reserve(size_t size)
  {}

void CompositeDoubleVector::resize(size_t size)
  {}

void CompositeDoubleVector::prepend(const Variable& value)
  {jassert(false);}

void CompositeDoubleVector::append(const Variable& value)
  {jassert(false);}

void CompositeDoubleVector::remove(size_t index)
  {jassert(false);}

void CompositeDoubleVector::appendSubVector(size_t shift, const DoubleVectorPtr& subVector)
{
  jassert(subVector);
  jassert(vectors.empty() || shift >= (vectors.back().first + vectors.back().second->getElementsEnumeration()->getNumElements()));
  vectors.push_back(std::make_pair(shift, subVector));
}

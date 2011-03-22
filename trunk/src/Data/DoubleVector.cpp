/*-----------------------------------------.---------------------------------.
| Filename: DoubleVector.cpp               | Base class for Double Vectors   |
| Author  : Francis Maes                   |                                 |
| Started : 03/02/2011 16:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Data/DoubleVector.h>
#include <lbcpp/FeatureGenerator/FeatureGenerator.h>
#include "../Core/Object/SparseVectorHelper.h"
#include "../FeatureGenerator/FeatureGeneratorCallbacks.hpp"
using namespace lbcpp;

/*
** Default implementations
*/
template<class CallbackType>
inline void computeFeatures(const SparseDoubleVector& sparseVector, CallbackType& callback)
{
  const std::pair<size_t, double>* ptr = sparseVector.getValues();
  const std::pair<size_t, double>* limit = ptr + sparseVector.getNumValues();
  while (ptr < limit)
  {
    callback.sense(ptr->first, ptr->second);
    ++ptr;
  }
}

template<class CallbackType>
inline void computeFeatures(const DenseDoubleVector& denseVector, CallbackType& callback)
{
  //const std::vector<double>& values = denseVector.getValues();
  const double* source = denseVector.getValuePointer(0);
  const double* limit = source + denseVector.getNumElements();
  size_t i = 0;
  while (source < limit)
  {
    callback.sense(i, *source);
    ++i;
    ++source;
  }
}

template<class CallbackType>
inline void computeFeatures(const CompositeDoubleVector& compositeVector, CallbackType& callback)
{
  size_t n = compositeVector.getNumSubVectors();
  for (size_t i = 0; i < n; ++i)
    callback.sense(compositeVector.getSubVectorOffset(i), compositeVector.getSubVector(i), 1.0);
}

template<class CallbackType>
inline void computeFeatures(const LazyDoubleVector& lazyVector, CallbackType& callback)
{
  const DoubleVectorPtr& computed = lazyVector.getComputedVector();
  if (computed)
    callback.sense(0, computed, 1.0);
  else
    callback.sense(0, lazyVector.getFeatureGenerator(), &lazyVector.getInputs()[0], 1.0);
}


template<class VectorType>
inline size_t defaultL0Norm(const VectorType& vector)
{
  ComputeL0NormFeatureGeneratorCallback callback;
  computeFeatures(vector, callback);
  return callback.res;
}

template<class VectorType>
inline double defaultEntropy(const VectorType& vector)
{
  ComputeEntropyFeatureGeneratorCallback callback;
  computeFeatures(vector, callback);
  return callback.res;
}

template<class VectorType>
inline double defaultSumOfSquares(const VectorType& vector)
{
  ComputeSumOfSquaresFeatureGeneratorCallback callback;
  computeFeatures(vector, callback);
  return callback.res;
}

template<class VectorType>
inline double defaultGetExtremumValue(const VectorType& vector, bool lookForMaximum, size_t* index)
{
  ComputeExtremumValueFeatureGeneratorCallback callback(lookForMaximum, index);
  computeFeatures(vector, callback);
  return callback.res;
}

/*
** DoubleVector
*/
EnumerationPtr DoubleVector::getElementsEnumeration(TypePtr doubleVectorType)
{
  TypePtr dvType = doubleVectorType->findBaseTypeFromTemplateName(T("DoubleVector"));
  if (!dvType)
    return EnumerationPtr();
  jassert(dvType->getNumTemplateArguments() == 2);
  TypePtr res = dvType->getTemplateArgument(0);
  jassert(res);
  return res;
}

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
  if (elementsEnumeration->getName() == T("EnumValue"))
  {
    context.errorCallback(T("No elements enumeration in DoubleVector"));
    return false;
  }
  elementsType = dvType->getTemplateArgument(1);
  return true;
}  

SparseDoubleVectorPtr DoubleVector::toSparseVector() const
{
  SparseDoubleVectorPtr res(new SparseDoubleVector(getElementsEnumeration(), getElementsType()));
  appendTo(res, 0, 1.0);
  return res;
}

int DoubleVector::getIndexOfMaximumValue() const
{
  size_t res;
  return getExtremumValue(true, &res) == -DBL_MAX ? -1 : (int)res;
}

int DoubleVector::getIndexOfMinimumValue() const
{
  size_t res;
  return getExtremumValue(false, &res) == DBL_MAX ? -1 : (int)res;
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

void SparseDoubleVector::saveToXml(XmlExporter& exporter) const
{
  Object::saveToXml(exporter);
  size_t n = values.size();
  String res;
  for (size_t i = 0; i < n; ++i)
    if (values[i].second)
      res += String((int)values[i].first) + T(":") + String(values[i].second) + T(" ");
  exporter.addTextElement(res.trimEnd());
}

bool SparseDoubleVector::loadFromXml(XmlImporter& importer)
{
  if (!Object::loadFromXml(importer))
    return false;
  StringArray tokens;
  tokens.addTokens(importer.getAllSubText(), true);
  TypePtr elementType = getElementsType();
  for (size_t i = 0; i < (size_t)tokens.size(); ++i)
  {
    int e = tokens[i].indexOfChar(T(':'));
    int index = tokens[i].substring(0, e).getIntValue();
    if (index < 0)
      return false;
    setElement((size_t)index, tokens[i].substring(e + 1).getDoubleValue());
  }
  return true;
}

size_t SparseDoubleVector::incrementValue(size_t index, double delta)
{
  if ((int)index > lastIndex)
  {
    size_t res = values.size();
    appendValue(index, delta);
    return res;
  }
  int position = SparseVectorHelper<double>::findIndex(values, index);
  if (position >= 0)
  {
    values[position].second += delta;
    return (size_t)index;
  }
  size_t res;
  SparseVectorHelper<double>::insert(values, index, delta, &res);
  return res;
}

// double vector
double SparseDoubleVector::entropy() const
  {return defaultEntropy(*this);}

size_t SparseDoubleVector::l0norm() const
  {return values.size();} // /!\ this may be an overestimate when values contains some null values

double SparseDoubleVector::sumOfSquares() const
  {return defaultSumOfSquares(*this);}

double SparseDoubleVector::getExtremumValue(bool lookForMaximum, size_t* index) const
  {return defaultGetExtremumValue(*this, lookForMaximum, index);}

void SparseDoubleVector::multiplyByScalar(double scalar)
{
  if (scalar == 0.0)
    values.clear();
  else if (scalar == 1.0)
    return;
  for (size_t i = 0; i < values.size(); ++i)
    values[i].second *= scalar;
}

void SparseDoubleVector::appendTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const
{
  if (values.empty())
    return;

  jassert((int)offsetInSparseVector > sparseVector->lastIndex);
  std::vector<std::pair<size_t, double> >& targetValues = sparseVector->values;
  //targetValues.reserve(targetValues.size() + values.size());
  for (size_t i = 0; i < values.size(); ++i)
    targetValues.push_back(std::make_pair(values[i].first + offsetInSparseVector, values[i].second * weight));
  sparseVector->updateLastIndex();
}

void SparseDoubleVector::addWeightedTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const
{
  jassert(false);
  // FIXME: implement
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
  {return (size_t)juce::jmax((int)getElementsEnumeration()->getNumElements(), lastIndex + 1);}

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

DenseDoubleVector::DenseDoubleVector(EnumerationPtr enumeration, TypePtr elementsType, size_t initialSize, double initialValue)
  : DoubleVector(denseDoubleVectorClass(enumeration, elementsType)), ownValues(true)
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

void DenseDoubleVector::saveToXml(XmlExporter& exporter) const
{
  Object::saveToXml(exporter);
  size_t n = values->size();
  String res;
  for (size_t i = 0; i < n; ++i)
    res += String((*values)[i]) + T(" ");
  exporter.addTextElement(res.trimEnd());
}

bool DenseDoubleVector::loadFromXml(XmlImporter& importer)
{
  if (!Object::loadFromXml(importer))
    return false;
  StringArray tokens;
  tokens.addTokens(importer.getAllSubText(), true);
  size_t n = tokens.size();
  TypePtr elementType = getElementsType();
  ensureSize(n);
  for (size_t i = 0; i < n; ++i)
    getValueReference(i) = tokens[i].getDoubleValue();
  return true;
}

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

double DenseDoubleVector::computeLogSumOfExponentials() const
{
  double highestValue = getMaximumValue();
  double res = 0.0;
  for (size_t i = 0; i < values->size(); ++i)
    res += exp((*values)[i] - highestValue);
  return log(res) + highestValue;
}

// DoubleVector
double DenseDoubleVector::entropy() const
  {return defaultEntropy(*this);}

size_t DenseDoubleVector::l0norm() const
  {return values ? defaultL0Norm(*this) : 0;}

double DenseDoubleVector::sumOfSquares() const
  {return values ? defaultSumOfSquares(*this) : 0.0;}

double DenseDoubleVector::getExtremumValue(bool lookForMaximum, size_t* index) const
  {return defaultGetExtremumValue(*this, lookForMaximum, index);}

void DenseDoubleVector::appendTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const
{
  if (!values)
    return;

  jassert((int)offsetInSparseVector > sparseVector->getLastIndex());
  std::vector< std::pair<size_t, double> >& target = sparseVector->values;

  //target.reserve(target.size() + values->size());
  const double* source = getValuePointer(0);
  for (size_t i = 0; i < values->size(); ++i)
    if (source[i])
      target.push_back(std::make_pair(offsetInSparseVector + i, source[i] * weight));
  sparseVector->updateLastIndex();
}

void DenseDoubleVector::addWeightedTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const
{
  if (!values)
    return;

  jassert(false);
  // FIXME: implement
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
LazyDoubleVector::LazyDoubleVector(ClassPtr thisClass, FeatureGeneratorPtr featureGenerator, const Variable* inputs)
  : DoubleVector(thisClass), featureGenerator(featureGenerator), inputs(featureGenerator->getNumInputs())
{
  for (size_t i = 0; i < this->inputs.size(); ++i)
    this->inputs[i] = inputs[i];
}

// DoubleVector
double LazyDoubleVector::entropy() const
  {return defaultEntropy(*this);}

size_t LazyDoubleVector::l0norm() const
  {return defaultL0Norm(*this);}

double LazyDoubleVector::sumOfSquares() const
  {return defaultSumOfSquares(*this);}

double LazyDoubleVector::getExtremumValue(bool lookForMaximum, size_t* index) const
  {return defaultGetExtremumValue(*this, lookForMaximum, index);}

void LazyDoubleVector::multiplyByScalar(double scalar)
  {jassert(false);}

void LazyDoubleVector::appendTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const
{
  if (computedVector)
    computedVector->appendTo(sparseVector, offsetInSparseVector, weight);
  else
    featureGenerator->appendTo(&inputs[0], sparseVector, offsetInSparseVector, weight);
}

void LazyDoubleVector::addWeightedTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const
{
  if (computedVector)
    computedVector->addWeightedTo(sparseVector, offsetInSparseVector, weight);
  else
    featureGenerator->addWeightedTo(&inputs[0], sparseVector, offsetInSparseVector, weight);
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

SparseDoubleVectorPtr LazyDoubleVector::toSparseVector() const
{
  if (computedVector)
    return computedVector->toSparseVector();
  else
    return DoubleVector::toSparseVector();
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
double CompositeDoubleVector::entropy() const
  {return defaultEntropy(*this);}

size_t CompositeDoubleVector::l0norm() const
  {return defaultL0Norm(*this);}

double CompositeDoubleVector::sumOfSquares() const
  {return defaultSumOfSquares(*this);}

double CompositeDoubleVector::getExtremumValue(bool lookForMaximum, size_t* index) const
  {return defaultGetExtremumValue(*this, lookForMaximum, index);}

void CompositeDoubleVector::multiplyByScalar(double scalar)
{
  if (scalar == 1.0)
    return;
  for (size_t i = 0; i < vectors.size(); ++i)
    vectors[i].second->multiplyByScalar(scalar);
}

void CompositeDoubleVector::appendTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const
{
  for (size_t i = 0; i < vectors.size(); ++i)
  {
    const std::pair<size_t, DoubleVectorPtr>& subVector = vectors[i];
    subVector.second->appendTo(sparseVector, offsetInSparseVector + subVector.first, weight);
  }
}

void CompositeDoubleVector::addWeightedTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const
{
  for (size_t i = 0; i < vectors.size(); ++i)
  {
    const std::pair<size_t, DoubleVectorPtr>& subVector = vectors[i];
    subVector.second->addWeightedTo(sparseVector, offsetInSparseVector + subVector.first, weight);
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

void CompositeDoubleVector::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  const CompositeDoubleVectorPtr& target = t.staticCast<CompositeDoubleVector>();
  target->vectors.resize(vectors.size());
  for (size_t i = 0; i < vectors.size(); ++i)
  {
    target->vectors[i].first = vectors[i].first;
    target->vectors[i].second = vectors[i].second->cloneAndCast<DoubleVector>();
  }
}

bool CompositeDoubleVector::loadFromXml(XmlImporter& importer)
{
  int size = importer.getIntAttribute(T("numSubVectors"), -1);
  if (size < 0)
  {
    importer.getContext().errorCallback(T("Unknown number of sub vectors"));
    return false;
  }
  vectors.clear();
  vectors.reserve(size);
  forEachXmlChildElementWithTagName(*importer.getCurrentElement(), elt, T("vector"))
  {
    importer.enter(elt);
    int offset = importer.getIntAttribute(T("offset"), -1);
    Variable variable = importer.loadVariable(doubleVectorClass());
    if (offset < 0 || !variable.exists())
    {
      importer.getContext().errorCallback(T("Could not read sub vector"));
      return false;
    }
    vectors.push_back(std::make_pair((size_t)offset, variable.getObjectAndCast<DoubleVector>()));
    importer.leave();
  }
  if (vectors.size() != (size_t)size)
  {
    importer.getContext().errorCallback(T("Invalid number of sub vectors"));
    return false;
  }
  return true;
}

void CompositeDoubleVector::saveToXml(XmlExporter& exporter) const
{
  exporter.setAttribute(T("numSubVectors"), vectors.size());
  for (size_t i = 0; i < vectors.size(); ++i)
  {
    exporter.enter(T("vector"));
    exporter.setAttribute(T("offset"), vectors[i].first);
    exporter.writeVariable(vectors[i].second, doubleVectorClass());
    exporter.leave();
  }
}

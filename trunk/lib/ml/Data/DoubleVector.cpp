/*-----------------------------------------.---------------------------------.
| Filename: DoubleVector.cpp               | Base class for Double Vectors   |
| Author  : Francis Maes                   |                                 |
| Started : 03/02/2011 16:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <ml/DoubleVector.h>
#include <oil/Lua/Lua.h>
#include <oil/Core/Double.h>
#include <oil/Execution/ExecutionContext.h>
#include "FeatureGeneratorCallbacks.hpp"
using namespace lbcpp;

/*
** SparseVectorHelper
*/
template<class ElementType>
struct SparseVectorHelper
{
  typedef std::vector< std::pair<size_t, ElementType> > VectorType;
  
  static void set(VectorType& v, size_t index, const ElementType& value)
    {get(v, index) = value;}
  
  static void remove(VectorType& v, size_t index)
  {
    int i = findIndex(v, index);
    if (i >= 0)
      v.erase(v.begin() + i);
  }
  
  static ElementType& insert(VectorType& v, size_t index, const ElementType& value, size_t* position = NULL)
  {
    if (v.empty() || index > v.back().first)
    {
      if (position)
        *position = v.size();
      v.push_back(std::make_pair(index, value)); 
      return v.back().second;
    }
    else
    {
      int pos = dichotomicSearch(v, index, 0, v.size());
      jassert(pos >= 0 && pos <= (int)v.size());
      jassert(pos >= (int)v.size() || v[pos].first != index); // element should not already exists
      if (position)
        *position = (size_t)pos;
      v.insert(v.begin() + pos, std::make_pair(index, value)); // new element
      return v[pos].second;
    }
  }
  
  static ElementType& get(VectorType& v, size_t index, const ElementType& defaultValue = ElementType())
  {
    int res = findIndex(v, index);
    if (res >= 0)
      return v[res].second;
    return insert(v, index, defaultValue);
  }
  
  static const ElementType* get(const VectorType& v, size_t index)
  {
    int res = findIndex(v, index);
    return res < 0 ? NULL : &v[res].second;
  }
  
  static int findIndex(const VectorType& v, size_t index)
  {
    if (v.empty())
      return -1;
    size_t res = dichotomicSearch(v, index, 0, v.size());
    return res < v.size() && v[res].first == index ? (int)res : -1;
  }

private:
  static int findFirstEntryGreaterOrEqual(const VectorType& v, size_t index, size_t startingPoint = 0)
  {
    //assertIntegrity();
    jassert(startingPoint >= 0 && startingPoint <= v.size());
    if (startingPoint == v.size())
    {
      jassert(v.empty() || v.back().first < index);
      return v.size();
    }
    size_t res = dichotomicSearch(index, startingPoint, v.size());
    jassert(res >= startingPoint);
    jassert(res <= v.size());
    return res;
  }
  
  static size_t dichotomicSearch(const VectorType& v, size_t index, size_t begin, size_t end)
  {
    jassert(begin >= 0 && begin <= v.size() && end >= 0 && end <= v.size());
    jassert(begin <= end);
    jassert(begin == 0 || v[begin - 1].first < index);
    jassert(end == v.size() || v[end].first >= index);

    unsigned result;
    if (begin < v.size() && v[begin].first >= index)
      result = begin;
    else
      switch (end - begin)
      {
      case 0:
        result = begin;
        break;
        
      case 1:
        jassert(begin < v.size());
        result = v[begin].first >= index ? begin : (begin + 1);
        break;
        
      case 2:
        jassert(begin + 1 < v.size());
        result = v[begin].first >= index ? begin :
          (v[begin + 1].first >= index ? (begin + 1) : (begin + 2));
        break;
        
      case 3:
      case 4:
      case 5:
        for (result = begin; result < end; ++result)
        {
          jassert(result < v.size());
          if (v[result].first >= index)
            break;
        }
        break;
        
      default:
        {
          // estimate split point
/*          index_t beginIndex = begin < v.size() ? v[begin].first : v.back().first;
          index_t endIndex = end < v.size() ? v[end].first : v.back().first;
          double k = (index - beginIndex) / (double)(endIndex - beginIndex);
          unsigned split = (unsigned)(begin + k * (end - begin));
          
          // force to reduce from at least 2 candidates
          if (split < begin + 2) split = begin + 2;
          if (split > end - 2) split = end - 2;*/
          size_t split = (begin + end) / 2;
         
          jassert(split < v.size());
          if (v[split].first >= index)
            result = dichotomicSearch(v, index, begin, split);
          else
            result = dichotomicSearch(v, index, split, end);
        }
      };
    jassert(result == v.size() || v[result].first >= index);
    jassert(result == 0 || v[result - 1].first < index);
    return result;
  }
};

typedef SparseVectorHelper<double> SparseDoubleVectorHelper;

/*
** Default implementations
*/
template<class CallbackType>
inline void computeFeatures(const SparseDoubleVector& sparseVector, CallbackType& callback)
{
  if (!sparseVector.getNumValues())
    return;
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

template<class VectorType>
inline size_t defaultL0Norm(const VectorType& vector)
{
  ComputeL0NormFeatureGeneratorCallback callback;
  computeFeatures(vector, callback);
  return callback.res;
}

template<class VectorType>
inline double defaultL1Norm(const VectorType& vector)
{
  ComputeL1NormFeatureGeneratorCallback callback;
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
EnumerationPtr DoubleVector::getElementsEnumeration(ClassPtr doubleVectorType)
{
  ClassPtr dvType = doubleVectorType->findBaseTypeFromTemplateName(T("DoubleVector"));
  if (!dvType)
    return EnumerationPtr();
  jassert(dvType->getNumTemplateArguments() == 2);
  ClassPtr res = dvType->getTemplateArgument(0);
  jassert(res);
  return res;
}

bool DoubleVector::getTemplateParameters(ExecutionContext& context, ClassPtr type, EnumerationPtr& elementsEnumeration, ClassPtr& elementsType)
{
  ClassPtr dvType = type->findBaseTypeFromTemplateName(T("DoubleVector"));
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

DenseDoubleVectorPtr DoubleVector::toDenseDoubleVector() const
{
  const size_t n = getNumElements();
  DenseDoubleVectorPtr res(new DenseDoubleVector(getElementsEnumeration(), getElementsType(), n));
  if (n != 0)
  {
    double* ptr = res->getValuePointer(0);
    for (size_t i = 0; i < n; ++i, ++ptr)
      *ptr = Double::get(getElement(i));
  }
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

double DoubleVector::l2norm(const DoubleVectorPtr& vector) const
{
  size_t numElements = this->getNumElements();
  jassert(numElements == vector->getNumElements());
  double distance = 0;

  if (vector.isInstanceOf<SparseDoubleVector> ())
  {
    size_t indexInSparse = 0;
    SparseDoubleVectorPtr tmpVector = vector;
    const std::vector<std::pair<size_t, double> >& sparseValues = tmpVector->getValuesVector();

    for (size_t i = 0; i < numElements; i++)
    {
      if ((indexInSparse < sparseValues.size()) && (i == sparseValues[indexInSparse].first))
      {
        const double diff = Double::get(getElement(i)) - sparseValues[indexInSparse].second;
        distance += diff * diff;
        indexInSparse++;
      }
      else
      {
        const double diff = Double::get(getElement(i));
        distance += diff * diff;
      }
    }
  }
  else if (vector.isInstanceOf<DenseDoubleVector> ())
  {
    DenseDoubleVectorPtr tmpVector = vector;
    const std::vector<double>& denseValues = tmpVector->getValues();

    for (size_t i = 0; i < numElements; i++)
    {
      const double diff = Double::get(getElement(i)) - denseValues[i];
      distance += diff * diff;
    }
  }
  else
  {
    for (size_t i = 0; i < numElements; i++)
    {
      const double diff = Double::get(getElement(i)) - Double::get(vector->getElement(i));
      distance += diff * diff;
    }
  }

  return std::sqrt(distance);
}

// arguments: a, b
// <a, b>
int DoubleVector::dot(LuaState& state)
{
  DoubleVectorPtr vector1 = state.checkObject(1, doubleVectorClass()).staticCast<DoubleVector>();
  DoubleVectorPtr vector2 = state.checkObject(2, doubleVectorClass()).staticCast<DoubleVector>();
 
  DenseDoubleVectorPtr dv = vector1.dynamicCast<DenseDoubleVector>();
  if (dv)
  {
    state.pushNumber(vector2->dotProduct(dv, 0));
    return 1;
  }
  else
  {
    dv = vector2.dynamicCast<DenseDoubleVector>();
    if (dv)
    {
      state.pushNumber(vector1->dotProduct(dv, 0));
      return 1;
    }
    else
    {
      state.error("Invalid vector type");
      return 0;
    }
  }
}

// arguments: a, b, w=1
// a += b * w
int DoubleVector::add(LuaState& state)
{
  DoubleVectorPtr vector1 = state.checkObject(1, doubleVectorClass()).staticCast<DoubleVector>();
  DoubleVectorPtr vector2 = state.checkObject(2, doubleVectorClass()).staticCast<DoubleVector>();
  double weight = (state.getTop() == 3 ? state.checkNumber(3) : 1.0);

  DenseDoubleVectorPtr dv = vector1.dynamicCast<DenseDoubleVector>();
  if (dv)
    vector2->addWeightedTo(dv, 0, weight);
  else
  {
    SparseDoubleVectorPtr sv = vector1.dynamicCast<SparseDoubleVector>();
    if (sv)
      vector2->addWeightedTo(sv, 0, weight);
    else
      state.error("Invalid vector type");
  }
  return 0;
}

int DoubleVector::mul(LuaState& state)
{
  DoubleVectorPtr vector = state.checkObject(1, doubleVectorClass()).staticCast<DoubleVector>();
  double weight = state.checkNumber(2);
  vector->multiplyByScalar(weight);
  return 0;
}

int DoubleVector::l0norm(LuaState& state)
{
  DoubleVectorPtr vector = state.checkObject(1, doubleVectorClass()).staticCast<DoubleVector>();
  state.pushInteger(vector->l0norm());
  return 1;
}

int DoubleVector::l1norm(LuaState& state)
{
  DoubleVectorPtr vector = state.checkObject(1, doubleVectorClass()).staticCast<DoubleVector>();
  state.pushNumber(vector->l1norm());
  return 1;
}

int DoubleVector::l2norm(LuaState& state)
{
  DoubleVectorPtr vector = state.checkObject(1, doubleVectorClass()).staticCast<DoubleVector>();
  state.pushNumber(vector->l2norm());
  return 1;
}

int DoubleVector::argmin(LuaState& state)
{
  DoubleVectorPtr vector = state.checkObject(1, doubleVectorClass()).staticCast<DoubleVector>();
  int res = vector->getIndexOfMinimumValue();
  if (res < 0)
    return 0;
  state.pushNumber(res + 1);
  return 1;
}

int DoubleVector::argmax(LuaState& state)
{
  DoubleVectorPtr vector = state.checkObject(1, doubleVectorClass()).staticCast<DoubleVector>();
  int res = vector->getIndexOfMaximumValue();
  if (res < 0)
    return 0;
  state.pushNumber(res + 1);
  return 1;
}

// returns a + weight * b
static DoubleVectorPtr addWeighted(const DoubleVectorPtr& a, const DoubleVectorPtr& b, double weight)
{
  DoubleVectorPtr res = a->cloneAndCast<DoubleVector>();
  DenseDoubleVectorPtr denseRes = res.dynamicCast<DenseDoubleVector>();
  if (denseRes)
    b->addWeightedTo(denseRes, 0, weight);
  else
  {
    SparseDoubleVectorPtr sparseRes = res.dynamicCast<SparseDoubleVector>();
    if (sparseRes)
      b->addWeightedTo(sparseRes, 0, weight);
    else
      jassert(false);
  }
  return res;
}

int DoubleVector::__add(LuaState& state)
{
  DoubleVectorPtr other = state.checkObject(1, doubleVectorClass()).staticCast<DoubleVector>();
  state.pushObject(addWeighted(this, other, 1.0));
  return 1;
}

int DoubleVector::__sub(LuaState& state)
{
  DoubleVectorPtr other = state.checkObject(1, doubleVectorClass()).staticCast<DoubleVector>();
  state.pushObject(addWeighted(this, other, -1.0));
  return 1;
}

int DoubleVector::__mul(LuaState& state)
{
  double number = state.checkNumber(1);
  DoubleVectorPtr res = cloneAndCast<DoubleVector>();
  res->multiplyByScalar(number);
  state.pushObject(res);
  return 1;
}

int DoubleVector::__div(LuaState& state)
{
  return Object::__div(state); // not yet implemented
}

/*
** SparseDoubleVector
*/
SparseDoubleVector::SparseDoubleVector(EnumerationPtr elementsEnumeration, ClassPtr elementsType)
  : DoubleVector(sparseDoubleVectorClass(elementsEnumeration, elementsType)), lastIndex(-1)  {}

SparseDoubleVector::SparseDoubleVector(ClassPtr thisClass)
  : DoubleVector(thisClass), lastIndex(-1) {}

SparseDoubleVector::SparseDoubleVector(size_t initialReservedSize)
  : DoubleVector(sparseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleClass)), lastIndex(-1)
{
  values.reserve(initialReservedSize);
}

SparseDoubleVector::SparseDoubleVector()
  : lastIndex(-1) {}

void SparseDoubleVector::saveToXml(XmlExporter& exporter) const
{
  Object::saveToXml(exporter);
  size_t n = values.size();
  string res;
  for (size_t i = 0; i < n; ++i)
    if (values[i].second)
      res += string((int)values[i].first) + T(":") + string(values[i].second) + T(" ");
  exporter.addTextElement(res.trimEnd());
}

bool SparseDoubleVector::loadFromXml(XmlImporter& importer)
{
  if (!Object::loadFromXml(importer))
    return false;
  StringArray tokens;
  tokens.addTokens(importer.getAllSubText(), true);
  ClassPtr elementType = getElementsType();
  for (size_t i = 0; i < (size_t)tokens.size(); ++i)
  {
    int e = tokens[i].indexOfChar(T(':'));
    int index = tokens[i].substring(0, e).getIntValue();
    if (index < 0)
      return false;
    setElement((size_t)index, new Double(tokens[i].substring(e + 1).getDoubleValue()));
  }
  return true;
}

size_t SparseDoubleVector::getSizeInBytes(bool recursively) const
{
  size_t res = Object::getSizeInBytes(recursively);
  return res + sizeof (values) + values.size() * sizeof (std::pair<size_t, double>);
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

void SparseDoubleVector::pruneValues(double epsilon)
{
  for (size_t i = 0; i < values.size(); )
  {
    if (fabs(values[i].second) <= epsilon)
      values.erase(values.begin() + i);
    else
      ++i;
  }
  lastIndex = values.size() ? (int)values.back().first : -1;
}

// double vector
double SparseDoubleVector::entropy() const
  {return defaultEntropy(*this);}

size_t SparseDoubleVector::l0norm() const
  {return values.size();} // /!\ this may be an overestimate when values contains some null values

double SparseDoubleVector::l1norm() const
  {return defaultL1Norm(*this);}

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
  // TODO: more optimized implementation
  for (size_t i = 0; i < values.size(); ++i)
    sparseVector->incrementValue(values[i].first + offsetInSparseVector, values[i].second * weight);
}

void SparseDoubleVector::addWeightedTo(const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector, double weight) const
{
  if (!getNumValues())
    return;
  denseVector->ensureSize(offsetInDenseVector + (size_t)(lastIndex + 1));
  double* target = denseVector->getValuePointer(offsetInDenseVector);
  for (size_t i = 0; i < values.size(); ++i)
    target[values[i].first] += values[i].second * weight;
}

double SparseDoubleVector::dotProduct(const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector) const
{
  double res = 0.0;
  if (denseVector->getNumValues())
  {
    const double* target = denseVector->getValuePointer(offsetInDenseVector);
    size_t n = denseVector->getNumValues();
    for (size_t i = 0; i < values.size(); ++i)
    {
      size_t index = values[i].first;
      if (index >= n)
        break;
      res += target[index] * values[i].second;
    }
  }
  return res;
}

void SparseDoubleVector::computeFeatures(FeatureGeneratorCallback& callback) const
{
  for (size_t i = 0; i < values.size() && !callback.shouldStop(); ++i)
    callback.sense(values[i].first, values[i].second);
}

double SparseDoubleVector::l2norm(const DoubleVectorPtr& vector) const
{
  size_t indexInSparse = 0;
  size_t numElements = vector->getNumElements();
  jassert(lastIndex == (int)numElements);
  double distance = 0;

  if (vector.isInstanceOf<SparseDoubleVector> ())
  {
    size_t remoteIndexInSparse = 0;
    SparseDoubleVectorPtr tmpVector = vector;
    const std::vector<std::pair<size_t, double> >& sparseValues = tmpVector->getValuesVector();
    double distanceAggregator = 0;

    for (size_t i = 0; i < numElements; i++)
    {
      distanceAggregator = 0;

      if ((indexInSparse < values.size()) && (i == values[indexInSparse].first))
      {
        distanceAggregator += values[indexInSparse].second;
        indexInSparse++;
      }

      if ((remoteIndexInSparse < sparseValues.size()) && (i == sparseValues[remoteIndexInSparse].first))
      {
        distanceAggregator -= sparseValues[remoteIndexInSparse].second;
        remoteIndexInSparse++;
      }

      distance += distanceAggregator * distanceAggregator;
    }
  }
  else if (vector.isInstanceOf<DenseDoubleVector> ())
  {
    DenseDoubleVectorPtr tmpVector = vector;
    const std::vector<double>& denseValues = tmpVector->getValues();

    for (size_t i = 0; i < numElements; i++)
    {
      if ((indexInSparse < values.size()) && (i == values[indexInSparse].first))
      {
        const double diff = values[indexInSparse].second - denseValues[i];
        distance += diff * diff;
        indexInSparse++;
      }
      else
        distance += denseValues[i] * denseValues[i];
    }
  }
  else
  {
    for (size_t i = 0; i < numElements; i++)
    {
      if ((indexInSparse < values.size()) && (i == values[indexInSparse].first))
      {
        const double diff = values[indexInSparse].second - Double::get(vector->getElement(i));
        distance += diff * diff;
        indexInSparse++;
      }
      else
      {
        const double diff = Double::get(vector->getElement(i));
        distance += diff * diff;
      }
    }
  }

  return std::sqrt(distance);
}

// vector
void SparseDoubleVector::clear()
  {values.clear();}

void SparseDoubleVector::reserve(size_t size)
  {jassert(false);}

void SparseDoubleVector::resize(size_t size)
  {jassert(false);}

void SparseDoubleVector::prepend(const ObjectPtr& value)
  {jassert(false);}

void SparseDoubleVector::append(const ObjectPtr& value)
  {jassert(false);}

void SparseDoubleVector::remove(size_t index)
  {jassert(false);}

// Container
size_t SparseDoubleVector::getNumElements() const
  {return (size_t)juce::jmax((int)getElementsEnumeration()->getNumElements(), lastIndex + 1);}

ObjectPtr SparseDoubleVector::getElement(size_t index) const
{
  const double* value = SparseDoubleVectorHelper::get(values, index);
  if (!value)
    return ObjectPtr();
  jassert(*value != DVector::missingValue);
  return new Double(getElementsType(), *value);
}

void SparseDoubleVector::setElement(size_t index, const ObjectPtr& value)
{
  jassert(value);
  double dValue = Double::get(value);
  if ((int)index > lastIndex)
  {
    appendValue(index, dValue);
    return;
  }
  else
    SparseDoubleVectorHelper::set(values, index, dValue);
}

void SparseDoubleVector::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  SparseDoubleVectorPtr targetVector = target.staticCast<SparseDoubleVector>();
  targetVector->values = values;
  targetVector->lastIndex = lastIndex;
}

string SparseDoubleVector::toShortString() const
{
  string res;
  for (size_t i = 0; i < values.size(); ++i)
    if (values[i].second)
      res += getElementName(values[i].first) + T(":") + string(values[i].second) + T(" ");
  if (res.isNotEmpty())
    res.dropLastCharacters(1);
  return res;
}

int SparseDoubleVector::append(LuaState& state)
{
  if (state.getTop() == 3)
  {
    SparseDoubleVectorPtr vector = state.checkObject(1, sparseDoubleVectorClass()).staticCast<SparseDoubleVector>();
    int index = state.checkInteger(2);
    if (index <= 0)
      state.error("Invalid index in SparseDoubleVector::append");
    else
    {
      double value = state.checkNumber(3);
      vector->appendValue((size_t)(index - 1), value);
    }
  }
  //else
  //  DoubleVector::append(state);
  return 0;
}

int SparseDoubleVector::increment(LuaState& state)
{
  SparseDoubleVectorPtr vector = state.checkObject(1, sparseDoubleVectorClass()).staticCast<SparseDoubleVector>();
  int index = state.checkInteger(2);
  if (index <= 0)
    state.error("Invalid index in SparseDoubleVector::append");
  else
  {
    double value = state.checkNumber(3);
    if (value)
      vector->incrementValue((size_t)(index - 1), value);
  }
  return 0;
}

int SparseDoubleVector::__index(LuaState& state) const
{
  if (!state.isInteger(1))
    return Object::__index(state);

  int index = state.toInteger(1);
  if (index <= 0)
  {
    state.error("Invalid index in SparseDoubleVector::index()");
    return 0;
  }
  const double* value = SparseDoubleVectorHelper::get(values, (size_t)(index - 1));
  state.pushNumber(value ? *value : 0.0);
  return 1;
}

int SparseDoubleVector::__newIndex(LuaState& state)
{
  if (!state.isInteger(1))
    return Object::__newIndex(state);

  int index = state.toInteger(1);
  if (index <= 0)
    state.error("Invalid index in SparseDoubleVector::newIndex()");

  double value = state.checkNumber(2);
  if (index > lastIndex)
    appendValue(index, value);
  else
    SparseDoubleVectorHelper::set(values, (size_t)(index - 1), value);
  return 0;
}

/*
** DenseDoubleVector
*/
DenseDoubleVector::DenseDoubleVector(ClassPtr thisClass, std::vector<double>& values)
  : DoubleVector(thisClass), values(&values), ownValues(false)
{
  jassert(thisClass && thisClass->inheritsFrom(denseDoubleVectorClass()));
}

DenseDoubleVector::DenseDoubleVector(ClassPtr thisClass, size_t initialSize, double initialValue)
  : DoubleVector(thisClass), ownValues(true)
{
  jassert(thisClass && thisClass->inheritsFrom(denseDoubleVectorClass()));
  if (initialSize == (size_t)-1)
  {
    EnumerationPtr elementsEnumeration = getElementsEnumeration();
    initialSize = elementsEnumeration->getNumElements();
  }
  values = new std::vector<double>(initialSize, initialValue);
}

DenseDoubleVector::DenseDoubleVector(EnumerationPtr enumeration, ClassPtr elementsType, size_t initialSize, double initialValue)
  : DoubleVector(denseDoubleVectorClass(enumeration, elementsType)), ownValues(true)
{
  jassert(enumeration);
  jassert(((ObjectPtr)enumeration).isInstanceOf<Enumeration>());
  if (initialSize == (size_t)-1)
    initialSize = enumeration->getNumElements();
  values = new std::vector<double>(initialSize, initialValue);
}

DenseDoubleVector::DenseDoubleVector(size_t initialSize, double initialValue)
  : DoubleVector(denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleClass)), ownValues(true)
{
  jassert(thisClass);
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
  ClassPtr elementsType = getElementsType();
  string res;
  for (size_t i = 0; i < n; ++i)
  {
    double value = (*values)[i];
    jassert(value != DVector::missingValue);
    res += string(value) + T(" ");
  }
  exporter.addTextElement(res.trimEnd());
  exporter.setAttribute(T("size"), n);
}

bool DenseDoubleVector::loadFromXml(XmlImporter& importer)
{
  if (!Object::loadFromXml(importer))
    return false;

  bool ok = true;
  StringArray tokens;
  string allText = importer.getAllSubText();
  tokens.addTokens(allText, true);
  tokens.removeEmptyStrings();
  size_t n = tokens.size();
  if (importer.hasAttribute(T("size")))
  {
    int expectedSize = importer.getIntAttribute(T("size"), -1);
    if (expectedSize != (int)n)
    {
      importer.getContext().errorCallback(T("Invalid number of tokens: expected ") +
        string(expectedSize) + T(" values, found ") + string((int)n) + T(" values"));
      ok = false;
    }
  }

  ClassPtr elementsType = getElementsType();
  ensureSize(n);
  for (size_t i = 0; i < n; ++i)
  {
    string token = tokens[i];
    double& value = getValueReference(i);
    jassert(token != T("_"));
    value = tokens[i].getDoubleValue();
  }
  return ok;
}

size_t DenseDoubleVector::getSizeInBytes(bool recursively) const
{
  size_t res = Object::getSizeInBytes(recursively);
  if (values && ownValues)
    res += sizeof (*values) + values->size() * sizeof (double);
  return res;
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
  if (!values || value == 1.0)
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


double DenseDoubleVector::computeEntropy(double l1norm) const
{
  if (l1norm < 0)
    l1norm = this->l1norm();
  if (!l1norm)
    return 0.0;

  double Z = 1.0 / l1norm;
  double res = 0.0;
  double sumOfP = 0.0;
  for (size_t i = 0; i < values->size(); ++i)
  {
    double p = (*values)[i] * Z;
    if (p)
    {
      res -= p * log2(p);
      sumOfP += p;
    }
  }
  jassert(fabs(sumOfP - 1.0) < 1e-12);
  return res;
}

// DoubleVector
double DenseDoubleVector::entropy() const
  {return defaultEntropy(*this);}

size_t DenseDoubleVector::l0norm() const
  {return values ? defaultL0Norm(*this) : 0;}

double DenseDoubleVector::l1norm() const
  {return values ? defaultL1Norm(*this) : 0.0;}

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

void DenseDoubleVector::computeFeatures(FeatureGeneratorCallback& callback) const
{
  for (size_t i = 0; i < values->size() && !callback.shouldStop(); ++i)
    callback.sense(i, (*values)[i]);
}

double DenseDoubleVector::l2norm(const DoubleVectorPtr& vector) const
{
  size_t numElements = values->size();
  jassert(numElements == vector->getNumElements());
  double distance = 0;

  if (vector.isInstanceOf<SparseDoubleVector> ())
  {
    size_t indexInSparse = 0;
    SparseDoubleVectorPtr tmpVector = vector;
    const std::vector<std::pair<size_t, double> >& sparseValues = tmpVector->getValuesVector();

    for (size_t i = 0; i < numElements; i++)
    {
      if ((indexInSparse < sparseValues.size()) && (i == sparseValues[indexInSparse].first))
      {
        const double diff = (*values)[i] - sparseValues[indexInSparse].second;
        distance += diff * diff;
        indexInSparse++;
      }
      else
        distance += (*values)[i] * (*values)[i];
    }
  }
  else if (vector.isInstanceOf<DenseDoubleVector> ())
  {
    DenseDoubleVectorPtr tmpVector = vector;
    for (size_t i = 0; i < numElements; i++)
    {
      const double diff = (*values)[i] - (*(tmpVector->values))[i];
      distance += diff * diff;
    }
  }
  else
  {
    for (size_t i = 0; i < numElements; i++)
    {
      const double diff = (*values)[i] - Double::get(vector->getElement(i));
      distance += diff * diff;
    }
  }

  return std::sqrt(distance);
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

void DenseDoubleVector::prepend(const ObjectPtr& value)
{
  // this operation is not permitted
  jassert(false);
  //values->insert(values->begin(), value.getDouble());
}

static inline double doubleObjectToDouble(const ObjectPtr& value)
  {jassert(value); return Double::get(value);}

void DenseDoubleVector::append(const ObjectPtr& value)
  {values->push_back(doubleObjectToDouble(value));}

void DenseDoubleVector::remove(size_t index)
{
  // this operation is not permitted
  jassert(false);
}

// Container
size_t DenseDoubleVector::getNumElements() const
  {return values ? values->size() : 0;}

ObjectPtr DenseDoubleVector::getElement(size_t index) const
  {return new Double(values && index < values->size() ? (*values)[index] : 0.0);}

void DenseDoubleVector::setElement(size_t index, const ObjectPtr& value)
  {(*values)[index] = doubleObjectToDouble(value);}

// Object
string DenseDoubleVector::toShortString() const
{
  string res = T("[");
  size_t n = getNumElements();
  bool tooLong = (n > 20);
  if (tooLong)
    n = 20;
  for (size_t i = 0; i < n; ++i)
  {
    res += getElement(i)->toShortString();
    if (i < n - 1)
      res += " ";
  }
  if (tooLong)
    res += T("...");
  res += T("]");
  return res;
}

void DenseDoubleVector::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  const DenseDoubleVectorPtr& target = t.staticCast<DenseDoubleVector>();
  if (values)
  {
    target->values = new std::vector<double>(*values);
    target->ownValues = true;
  }
}

int DenseDoubleVector::__len(LuaState& state) const
{
  state.pushInteger(values ? values->size() : 0);
  return 1;
}

int DenseDoubleVector::__newIndex(LuaState& state)
{
  if (!state.isInteger(1))
    return Object::__newIndex(state);

  int index = state.toInteger(1);
  size_t enumSize = getElementsEnumeration()->getNumElements();
  if (index < 1 || (enumSize && index > (int)enumSize))
    state.error("Invalid index in Container::set()");
  else
    setValue(index - 1, state.toNumber(2));
  return 0;
}

int DenseDoubleVector::__index(LuaState& state) const
{
  if (!state.isInteger(1))
    return Object::__index(state);

  int index = state.toInteger(1);
  size_t enumSize = getElementsEnumeration()->getNumElements();
  if (index < 1 || (enumSize && index > (int)enumSize))
  {
    state.error("Invalid index in Container::get()");
    return 0;
  }
  state.pushNumber(values ? getValue(index - 1) : 0.0);
  return 1;
}

/*
** CompositeDoubleVector
*/
// DoubleVector
double CompositeDoubleVector::entropy() const
  {return defaultEntropy(*this);}

size_t CompositeDoubleVector::l0norm() const
  {return defaultL0Norm(*this);}

double CompositeDoubleVector::l1norm() const
  {return defaultL1Norm(*this);}

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

void CompositeDoubleVector::computeFeatures(FeatureGeneratorCallback& callback) const
{
  for (size_t i = 0; i < vectors.size() && !callback.shouldStop(); ++i)
  {
    const std::pair<size_t, DoubleVectorPtr>& subVector = vectors[i];
    callback.sense(subVector.first, subVector.second, 1.f);
  }
}

// Container
size_t CompositeDoubleVector::getNumElements() const
{
  if (vectors.empty())
    return 0;
  const std::pair<size_t, DoubleVectorPtr>& lastSubVector = vectors.back();
  return lastSubVector.first + lastSubVector.second->getNumElements();
}

ObjectPtr CompositeDoubleVector::getElement(size_t index) const
{
  if (vectors.empty())
    return ObjectPtr();
  
  if (index < vectors[0].first)
    return ObjectPtr();

  int i = 1;
  while (i < (int)vectors.size() && vectors[i].first <= index)
    ++i;
  --i;
  jassert(i >= 0 && i < (int)vectors.size());
  return vectors[i].second->getElement(index - vectors[i].first);
}

void CompositeDoubleVector::setElement(size_t index, const ObjectPtr& value)
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

void CompositeDoubleVector::prepend(const ObjectPtr& value)
  {jassert(false);}

void CompositeDoubleVector::append(const ObjectPtr& value)
  {jassert(false);}

void CompositeDoubleVector::remove(size_t index)
  {jassert(false);}

void CompositeDoubleVector::appendSubVector(size_t shift, const DoubleVectorPtr& subVector)
{
  jassert(subVector);
  jassert(vectors.empty() || shift >= (vectors.back().first + vectors.back().second->getElementsEnumeration()->getNumElements()));
  vectors.push_back(std::make_pair(shift, subVector));
}

void CompositeDoubleVector::appendSubVector(const DoubleVectorPtr& subVector)
{
  jassert(subVector);
  const size_t shift = vectors.empty() ? 0 : vectors.back().first + vectors.back().second->getElementsEnumeration()->getNumElements();
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
    ObjectPtr variable = importer.loadObject(doubleVectorClass());
    if (offset < 0 || !variable)
    {
      importer.getContext().errorCallback(T("Could not read sub vector"));
      return false;
    }
    vectors.push_back(std::make_pair((size_t)offset, variable.staticCast<DoubleVector>()));
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
    exporter.writeObject(vectors[i].second, doubleVectorClass());
    exporter.leave();
  }
}

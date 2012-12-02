/*-----------------------------------------.---------------------------------.
| Filename: DoubleVector.h                 | Base class for Double Vectors   |
| Author  : Francis Maes                   |                                 |
| Started : 03/02/2011 16:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_DOUBLE_VECTOR_H_
# define LBCPP_DATA_DOUBLE_VECTOR_H_

# include "../Core/Vector.h"
# include <lbcpp/Core/XmlSerialisation.h>

namespace lbcpp
{

class FeatureGeneratorCallback;

class DoubleVector;
typedef ReferenceCountedObjectPtr<DoubleVector> DoubleVectorPtr;

class SparseDoubleVector;
typedef ReferenceCountedObjectPtr<SparseDoubleVector> SparseDoubleVectorPtr;

class DenseDoubleVector;
typedef ReferenceCountedObjectPtr<DenseDoubleVector> DenseDoubleVectorPtr;

class CompositeDoubleVector;
typedef ReferenceCountedObjectPtr<CompositeDoubleVector> CompositeDoubleVectorPtr;

class DoubleVector : public Object
{
public:
  DoubleVector(ClassPtr thisClass)
    : Object(thisClass) {}
  DoubleVector() {}

  // tmp
  virtual size_t getNumElements() const = 0;
  virtual ObjectPtr getElement(size_t index) const = 0;
  virtual void setElement(size_t index, const ObjectPtr& value) = 0;
  virtual string getElementName(size_t index) const
    {return getElementsEnumeration()->getElementName(index);}
  virtual void append(const ObjectPtr& object) const {jassertfalse;}

  virtual EnumerationPtr getElementsEnumeration() const
    {return thisClass->getTemplateArgument(0);}

  virtual ClassPtr getElementsType() const
    {return thisClass->getTemplateArgument(1);}

  static EnumerationPtr getElementsEnumeration(ClassPtr doubleVectorType);
  static bool getTemplateParameters(ExecutionContext& context, ClassPtr type, EnumerationPtr& elementsEnumeration, ClassPtr& elementsType);

  // compute - sum v[i] * log2(v[i])
  virtual double entropy() const = 0;

  virtual size_t l0norm() const = 0;
  virtual double l1norm() const = 0;
  virtual double sumOfSquares() const = 0;
  virtual double getExtremumValue(bool lookForMaximum, size_t* index = NULL) const = 0;
  
  virtual void multiplyByScalar(double scalar) = 0;
  virtual void appendTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const = 0;
  virtual void addWeightedTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const = 0;
  virtual void addWeightedTo(const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector, double weight) const = 0;
  virtual double dotProduct(const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector) const = 0;
  virtual void computeFeatures(FeatureGeneratorCallback& callback) const = 0;

  double getMaximumValue() const
    {return getExtremumValue(true);}

  int getIndexOfMaximumValue() const;

  double getMinimumValue() const
    {return getExtremumValue(false);}

  int getIndexOfMinimumValue() const;

  virtual double l2norm() const
    {return sqrt(sumOfSquares());}

  double dotProduct(const DenseDoubleVectorPtr& denseVector)
    {return dotProduct(denseVector, 0);}

  void addTo(const DenseDoubleVectorPtr& denseVector)
    {addWeightedTo(denseVector, 0, 1.0);}
  
  void subtractFrom(const DenseDoubleVectorPtr& denseVector)
    {addWeightedTo(denseVector, 0, -1.0);}

  virtual double l2norm(const DoubleVectorPtr& vector) const;

  virtual SparseDoubleVectorPtr toSparseVector() const;
  virtual DenseDoubleVectorPtr toDenseDoubleVector() const;

  /*
  ** Lua
  */
  static int dot(LuaState& state);
  static int add(LuaState& state);
  static int mul(LuaState& state);
  static int l0norm(LuaState& state);
  static int l1norm(LuaState& state);
  static int l2norm(LuaState& state);
  static int argmin(LuaState& state);
  static int argmax(LuaState& state);

  virtual int __add(LuaState& state);
  virtual int __sub(LuaState& state);
  virtual int __mul(LuaState& state);
  virtual int __div(LuaState& state);

  lbcpp_UseDebuggingNewOperator
};

extern ClassPtr doubleVectorClass(ClassPtr elementsEnumeration = enumValueClass, ClassPtr elementsType = doubleClass);
extern ClassPtr sparseDoubleVectorClass(ClassPtr elementsEnumeration = enumValueClass, ClassPtr elementsType = doubleClass);
extern ClassPtr denseDoubleVectorClass(ClassPtr elementsEnumeration = enumValueClass, ClassPtr elementsType = doubleClass);
extern ClassPtr lazyDoubleVectorClass(ClassPtr elementsEnumeration = enumValueClass, ClassPtr elementsType = doubleClass);
extern ClassPtr compositeDoubleVectorClass(ClassPtr elementsEnumeration = enumValueClass, ClassPtr elementsType = doubleClass);

extern ClassPtr simpleDenseDoubleVectorClass;
extern ClassPtr simpleSparseDoubleVectorClass;

class SparseDoubleVector : public DoubleVector
{
public:
  SparseDoubleVector(EnumerationPtr elementsEnumeration, ClassPtr elementsType);
  SparseDoubleVector(ClassPtr thisClass);
  SparseDoubleVector(size_t initialReservedSize);
  SparseDoubleVector();

  void appendValue(size_t index, double value)
    {jassert((int)index > lastIndex); values.push_back(std::make_pair(index, value)); lastIndex = (int)index;}

  size_t incrementValue(size_t index, double delta);

  size_t getNumValues() const
    {return values.size();}

  void reserveValues(size_t size)
    {values.reserve(size);}

  const std::pair<size_t, double>& getValue(size_t index) const
    {jassert(index < values.size()); return values[index];}

  const std::pair<size_t, double>* getValues() const
    {return &values[0];}

  std::pair<size_t, double>* getValues()
    {return &values[0];}

  std::vector< std::pair<size_t, double> >& getValuesVector()
    {return values;}

  int getLastIndex() const
    {return lastIndex;}

  void updateLastIndex()
    {lastIndex = values.size() ? (int)values.back().first : -1;}

  void pruneValues(double epsilon = 0.0);

  // DoubleVector
  virtual double entropy() const;
  virtual size_t l0norm() const;
  virtual double l1norm() const;
  virtual double sumOfSquares() const;
  virtual double getExtremumValue(bool lookForMaximum, size_t* index = NULL) const;
  virtual void multiplyByScalar(double scalar);
  virtual void appendTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const;
  virtual void addWeightedTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const;
  virtual void addWeightedTo(const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector, double weight) const;
  virtual double dotProduct(const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector) const;
  virtual void computeFeatures(FeatureGeneratorCallback& callback) const;

  virtual double l2norm(const DoubleVectorPtr& vector) const;
  virtual double l2norm() const {return DoubleVector::l2norm();}

  virtual SparseDoubleVectorPtr toSparseVector() const
    {return refCountedPointerFromThis(this);}

  // Vector
  virtual void clear();
  virtual void reserve(size_t size);
  virtual void resize(size_t size);
  virtual void prepend(const ObjectPtr& value);
  virtual void append(const ObjectPtr& value);
  virtual void remove(size_t index);

  // Container
  virtual size_t getNumElements() const;
  virtual ObjectPtr getElement(size_t index) const;
  virtual void setElement(size_t index, const ObjectPtr& value);
  
  // Object
  virtual string toShortString() const;
  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlImporter& importer);
  virtual size_t getSizeInBytes(bool recursively) const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  // Lua
  static int append(LuaState& state);
  static int increment(LuaState& state);
  virtual int __index(LuaState& state) const;
  virtual int __newIndex(LuaState& state);

  lbcpp_UseDebuggingNewOperator

private:
  friend class DenseDoubleVector;

  std::vector< std::pair<size_t, double> > values;
  int lastIndex;
};

class DenseDoubleVector : public DoubleVector
{
public:
  DenseDoubleVector(ClassPtr thisClass, std::vector<double>& values);
  DenseDoubleVector(ClassPtr thisClass, size_t initialSize = (size_t)-1, double initialValue = 0.0);
  DenseDoubleVector(EnumerationPtr enumeration, ClassPtr elementsType, size_t initialSize = (size_t)-1, double initialValue = 0.0);
  DenseDoubleVector(size_t initialSize, double initialValue);
  DenseDoubleVector();
  virtual ~DenseDoubleVector();
  
  const std::vector<double>& getValues() const
    {jassert(values); return *values;}

  std::vector<double>& getValues()
    {jassert(values); return *values;}

  double* getValuePointer(size_t index)
    {jassert(values && index <= values->size()); return &(*values)[0] + index;}

  const double* getValuePointer(size_t index) const
    {jassert(values && index <= values->size()); return &(*values)[0] + index;}

  double& getValueReference(size_t index)
    {ensureSize(index + 1); return *getValuePointer(index);}

  double getValue(size_t index) const
    {jassert(values); return index < values->size() ? (*values)[index] : 0.0;}

  void setValue(size_t index, double value)
    {getValueReference(index) = value;}

  size_t getNumValues() const
    {return values ? values->size() : 0;}

  void incrementValue(size_t index, double value)
    {jassert(values && index < values->size()); (*values)[index] += value;}

  void decrementValue(size_t index, double value)
    {jassert(values && index < values->size()); (*values)[index] -= value;}

  void appendValue(double value)
    {jassert(values); values->push_back(value);}
    
  void removeLastValue()
    {jassert(values && values->size()); values->pop_back();}

  void ensureSize(size_t minimumSize);

  // compute log(sum_i(exp(value[i]))) by avoiding numerical errors
  double computeLogSumOfExponentials() const;

  // compute -sum_i(p_i * log2(p_i)) with p_i = value[i] / l1norm. By default, l1norm is computed automatically
  double computeEntropy(double l1norm = -1.0) const;

  // DoubleVector
  virtual double entropy() const;
  virtual size_t l0norm() const;
  virtual double l1norm() const;
  virtual double sumOfSquares() const;
  virtual double getExtremumValue(bool lookForMaximum, size_t* index = NULL) const;
  virtual void multiplyByScalar(double value);
  virtual void appendTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const;
  virtual void addWeightedTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const;
  virtual void addWeightedTo(const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector, double weight) const;
  virtual double dotProduct(const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector) const;
  virtual void computeFeatures(FeatureGeneratorCallback& callback) const;

  virtual double l2norm(const DoubleVectorPtr& vector) const;
  virtual double l2norm() const {return DoubleVector::l2norm();}

  virtual DenseDoubleVectorPtr toDenseDoubleVector() const
    {return refCountedPointerFromThis(this);}

  // Vector
  virtual void clear();
  virtual void reserve(size_t size);
  virtual void resize(size_t size);
  virtual void prepend(const ObjectPtr& value);
  virtual void append(const ObjectPtr& value);
  virtual void remove(size_t index);

  // Container
  virtual size_t getNumElements() const;
  virtual ObjectPtr getElement(size_t index) const;
  virtual void setElement(size_t index, const ObjectPtr& value);

  // Object
  virtual string toShortString() const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;
  void saveToXml(XmlExporter& exporter) const;
  bool loadFromXml(XmlImporter& importer);
  virtual size_t getSizeInBytes(bool recursively) const;

  // Lua
  virtual int __len(LuaState& state) const;
  virtual int __index(LuaState& state) const;
  virtual int __newIndex(LuaState& state);

  lbcpp_UseDebuggingNewOperator

private:
  std::vector<double>* values;
  bool ownValues;
};

class CompositeDoubleVector : public DoubleVector
{
public:
  CompositeDoubleVector(ClassPtr thisClass)
    : DoubleVector(thisClass) {}
  CompositeDoubleVector() {}
  
  // sub vectors
  size_t getNumSubVectors() const
    {return vectors.size();}

  const DoubleVectorPtr& getSubVector(size_t index) const
    {jassert(index < vectors.size()); return vectors[index].second;}

  size_t getSubVectorOffset(size_t index) const
    {jassert(index < vectors.size()); return vectors[index].first;}

  void reserveSubVectors(size_t count)
    {vectors.reserve(count);}

  void appendSubVector(size_t shift, const DoubleVectorPtr& subVector);
  void appendSubVector(const DoubleVectorPtr& subVector);

  // DoubleVector
  virtual double entropy() const;
  virtual size_t l0norm() const;
  virtual double l1norm() const;
  virtual double sumOfSquares() const;
  virtual double getExtremumValue(bool lookForMaximum, size_t* index = NULL) const;
  virtual void multiplyByScalar(double value);
  virtual void appendTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const;
  virtual void addWeightedTo(const SparseDoubleVectorPtr& sparseVector, size_t offsetInSparseVector, double weight) const;
  virtual void addWeightedTo(const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector, double weight) const;
  virtual double dotProduct(const DenseDoubleVectorPtr& denseVector, size_t offsetInDenseVector) const;
  virtual void computeFeatures(FeatureGeneratorCallback& callback) const;

  virtual double l2norm() const {return DoubleVector::l2norm();}

  // Vector
  virtual void clear();
  virtual void reserve(size_t size);
  virtual void resize(size_t size);
  virtual void prepend(const ObjectPtr& value);
  virtual void append(const ObjectPtr& value);
  virtual void remove(size_t index);

  // Container
  virtual size_t getNumElements() const;
  virtual ObjectPtr getElement(size_t index) const;
  virtual void setElement(size_t index, const ObjectPtr& value);

  // Object
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;
  virtual bool loadFromXml(XmlImporter& importer);
  virtual void saveToXml(XmlExporter& exporter) const;
  
  lbcpp_UseDebuggingNewOperator

private:
  std::vector< std::pair<size_t, DoubleVectorPtr> > vectors;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_DOUBLE_VECTOR_H_

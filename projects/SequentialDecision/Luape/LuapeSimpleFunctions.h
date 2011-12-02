/*-----------------------------------------.---------------------------------.
| Filename: LuapeSimpleFunctions.h         | Luape simple functions          |
| Author  : Francis Maes                   |                                 |
| Started : 18/11/2011 15:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_SIMPLE_FUNCTIONS_H_
# define LBCPP_LUAPE_SIMPLE_FUNCTIONS_H_

# include "LuapeFunction.h"
# include "LuapeNode.h"
# include <algorithm>

namespace lbcpp
{

/*
** Base classes
*/
class HomogeneousUnaryLuapeFunction : public LuapeFunction
{
public:
  HomogeneousUnaryLuapeFunction(TypePtr type = anyType)
    : type(type) {}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type->inheritsFrom(this->type);}

  virtual TypePtr getOutputType(const std::vector<TypePtr>& inputTypes) const
    {return type;}

private:
  TypePtr type;
};

class HomogeneousBinaryLuapeFunction : public LuapeFunction
{
public:
  HomogeneousBinaryLuapeFunction(TypePtr type = anyType)
    : type(type) {}

  virtual size_t getNumInputs() const
    {return 2;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type->inheritsFrom(this->type);}

  virtual TypePtr getOutputType(const std::vector<TypePtr>& inputTypes) const
    {return type;}

private:
  TypePtr type;
};

/*
** Binary Boolean
*/
class BinaryBooleanLuapeFunction : public HomogeneousBinaryLuapeFunction
{
public:
  BinaryBooleanLuapeFunction()
    : HomogeneousBinaryLuapeFunction(booleanType) {}

  virtual bool computeBoolean(bool first, bool second) const = 0;

  virtual Flags getFlags() const
    {return (Flags)(commutativeFlag | allSameArgIrrelevantFlag);}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
    {return computeBoolean(inputs[0].getBoolean(), inputs[1].getBoolean());}

  virtual VectorPtr compute(ExecutionContext& context, const std::vector<VectorPtr>& inputs, TypePtr outputType) const
  {
    const std::vector<bool>& inputs1 = inputs[0].staticCast<BooleanVector>()->getElements();
    const std::vector<bool>& inputs2 = inputs[1].staticCast<BooleanVector>()->getElements();
    jassert(inputs1.size() == inputs2.size());

    BooleanVectorPtr res = new BooleanVector(inputs1.size());
    std::vector<bool>& dest = res->getElements();

    std::vector<bool>::const_iterator it1 = inputs1.begin();
    std::vector<bool>::const_iterator it2 = inputs2.begin();
    std::vector<bool>::iterator itd = dest.begin();
    while (it1 != inputs1.end())
      *itd++ = computeBoolean(*it1++, *it2++);
    return res;
  }
};

class AndBooleanLuapeFunction : public BinaryBooleanLuapeFunction
{
public:
  virtual String toShortString() const
    {return "&&";}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return inputs[0]->toShortString() + " && " + inputs[1]->toShortString();}

  virtual bool computeBoolean(bool first, bool second) const
    {return first && second;}
};

class EqualBooleanLuapeFunction : public BinaryBooleanLuapeFunction
{
public:
  virtual String toShortString() const
    {return "==";}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return inputs[0]->toShortString() + " == " + inputs[1]->toShortString();}

  virtual bool computeBoolean(bool first, bool second) const
    {return first == second;}
};

/*
** Unary Double
*/
class UnaryDoubleLuapeFuntion : public HomogeneousUnaryLuapeFunction
{
public:
  UnaryDoubleLuapeFuntion()
    : HomogeneousUnaryLuapeFunction(doubleType), vectorClass(simpleDenseDoubleVectorClass) {}

  virtual double computeDouble(double value) const = 0;

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
    {return computeDouble(inputs[0].getDouble());}

  virtual VectorPtr compute(ExecutionContext& context, const std::vector<VectorPtr>& in, TypePtr outputType) const
  {
    const DenseDoubleVectorPtr& inputs = in[0].staticCast<DenseDoubleVector>();

    DenseDoubleVectorPtr res = new DenseDoubleVector(vectorClass, inputs->getNumValues(), 0.0);
    const double* ptr = inputs->getValuePointer(0);
    const double* lim = ptr + inputs->getNumValues();
    double* target = res->getValuePointer(0);
    while (ptr != lim)
      *target++ = computeDouble(*ptr++);
    return res;
  }

protected:
  ClassPtr vectorClass;
};

class NormalizerLuapeFunction : public UnaryDoubleLuapeFuntion
{
public:
  NormalizerLuapeFunction()
    {vectorClass = denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, probabilityType);}

  virtual TypePtr getOutputType(const std::vector<TypePtr>& inputTypes) const
    {return probabilityType;}

  void initialize(const DenseDoubleVectorPtr& inputValues, size_t numPercentiles = 10)
    {computePercentiles(inputValues, numPercentiles, percentiles);}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return "normalize(" + inputs[0]->toShortString() + ")";}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
    {return Variable(computeDouble(inputs[0].getDouble()), probabilityType);}

  virtual double computeDouble(double value) const
  {
    size_t n = percentiles.size();
    jassert(n);

    // tmp !
    return juce::jlimit(0.0, 1.0, (value - percentiles.front()) / (percentiles.back() - percentiles.front()));

    if (value <= percentiles[0])
      return 0.0;
    for (size_t i = 1; i < percentiles.size(); ++i)
      if (value < percentiles[i])
      {
        double k = (value - percentiles[i - 1]) / (percentiles[i] - percentiles[i - 1]);
        return ((double)i - 1 + k) / (double)(percentiles.size() - 1.0);
      }
    return 1.0;
  }

  lbcpp_UseDebuggingNewOperator

private:
  std::vector<double> percentiles;

  struct CompareValues
  {
    bool operator ()(const std::pair<size_t, double>& a, const std::pair<size_t, double>& b) const
      {return a.second != b.second ? a.second < b.second : a.first < b.first;}
  };

  void computePercentiles(const DenseDoubleVectorPtr& values, size_t numPercentiles, std::vector<double>& res)
  {
    size_t n = values->getNumValues();
    std::vector< std::pair<size_t, double> > sortedValues(n);
    for (size_t i = 0; i < n; ++i)
      sortedValues[i] = std::make_pair(i, values->getValue(i));
    std::sort(sortedValues.begin(), sortedValues.end(), CompareValues());

    if (numPercentiles > n)
      numPercentiles = n;
    
    res.resize(numPercentiles);

    double samplesPerPercentile = numPercentiles > 1 ? (n - 1.0) / (numPercentiles - 1.0) : 0.0;
    for (size_t i = 0; i < numPercentiles; ++i)
    {
      size_t index = (size_t)(samplesPerPercentile * i);
      jassert(index < sortedValues.size());
      res[i] = sortedValues[index].second;
    }
  }
};

/*
** Binary Double
*/
class BinaryDoubleLuapeFunction : public HomogeneousBinaryLuapeFunction
{
public:
  BinaryDoubleLuapeFunction()
    : HomogeneousBinaryLuapeFunction(doubleType) {}

  virtual double computeDouble(double first, double second) const = 0;

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
    {return computeDouble(inputs[0].getDouble(), inputs[1].getDouble());}

  virtual VectorPtr compute(ExecutionContext& context, const std::vector<VectorPtr>& inputs, TypePtr outputType) const
  {
    const DenseDoubleVectorPtr& inputs1 = inputs[0].staticCast<DenseDoubleVector>();
    const DenseDoubleVectorPtr& inputs2 = inputs[1].staticCast<DenseDoubleVector>();
    jassert(inputs1->getNumValues() == inputs2->getNumValues());

    DenseDoubleVectorPtr res = new DenseDoubleVector(inputs1->getNumValues(), 0.0);
    const double* ptr1 = inputs1->getValuePointer(0);
    const double* lim = ptr1 + inputs1->getNumValues();
    const double* ptr2 = inputs2->getValuePointer(0);
    double* target = res->getValuePointer(0);
    while (ptr1 != lim)
      *target++ = computeDouble(*ptr1++, *ptr2++);
    return res;
  }
};

class AddDoubleLuapeFunction : public BinaryDoubleLuapeFunction
{
public:
  virtual String toShortString() const
    {return "+";}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " + " + inputs[1]->toShortString() + ")";}

  virtual double computeDouble(double first, double second) const
    {return first + second;}

  virtual Flags getFlags() const
    {return (Flags)commutativeFlag;}
};

class SubDoubleLuapeFunction : public BinaryDoubleLuapeFunction
{
public:
  virtual String toShortString() const
    {return "-";}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " - " + inputs[1]->toShortString() + ")";}

  virtual double computeDouble(double first, double second) const
    {return first - second;}

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

class MulDoubleLuapeFunction : public BinaryDoubleLuapeFunction
{
public:
  virtual String toShortString() const
    {return "*";}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " x " + inputs[1]->toShortString() + ")";}

  virtual double computeDouble(double first, double second) const
    {return first * second;}

  virtual Flags getFlags() const
    {return (Flags)commutativeFlag;}
};

class DivDoubleLuapeFunction : public BinaryDoubleLuapeFunction
{
public:
  virtual String toShortString() const
    {return "/";}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return "(" + inputs[0]->toShortString() + " / " + inputs[1]->toShortString() + ")";}

  virtual double computeDouble(double first, double second) const
    {return second ? first / second : 0.0;}

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

/*
** Comparison
*/
class GreaterThanDoubleLuapeFunction : public LuapeFunction
{
public:
  virtual String toShortString() const
    {return ">";}

  virtual size_t getNumInputs() const
    {return 2;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type->inheritsFrom(doubleType);}

  virtual TypePtr getOutputType(const std::vector<TypePtr>& ) const
    {return booleanType;}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return inputs[0]->toShortString() + T(" > ") + inputs[1]->toShortString();}
  
  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
    {return inputs[0].getDouble() > inputs[1].getDouble();}

  virtual Flags getFlags() const
    {return (Flags)allSameArgIrrelevantFlag;}
};

/*
** Enumeration
*/
class EqualsConstantEnumLuapeFunction : public LuapeFunction
{
public:
  EqualsConstantEnumLuapeFunction(const Variable& value = Variable())
    : value(value) {}

  virtual String toShortString() const
    {return "= " + value.toShortString();}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type.isInstanceOf<Enumeration>();}

  virtual TypePtr getOutputType(const std::vector<TypePtr>& ) const
    {return booleanType;}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return inputs[0]->toShortString() + T(" == ") + value.toShortString();}
  
  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
    {return inputs[0] == value;}

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
  {
    const EnumerationPtr& enumeration = inputTypes[0].staticCast<Enumeration>();
    size_t n = enumeration->getNumElements();
    VectorPtr res = vector(enumeration, n);
    for (size_t i = 0; i < n; ++i)
      res->setElement(i, Variable(i, enumeration));
    return res;
  }

protected:
  friend class EqualsConstantEnumLuapeFunctionClass;

  Variable value; 
};

/*
** Object
*/
class GetVariableLuapeFunction : public LuapeFunction
{
public:
  GetVariableLuapeFunction(size_t variableIndex = 0)
    : variableIndex(variableIndex) {}

  virtual String toShortString() const
    {return ".var[" + String((int)variableIndex) + "]";}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type->inheritsFrom(objectClass);}

  virtual TypePtr getOutputType(const std::vector<TypePtr>& inputTypes) const
  {
    jassert(inputTypes.size() == 1);
    return inputTypes[0]->getMemberVariableType(variableIndex);
  }

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
  {
    jassert(inputs.size() == 1);
    return inputs[0]->toShortString() + "." + inputs[0]->getType()->getMemberVariableName(variableIndex);
  }

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    const ObjectPtr& input = inputs[0].getObject();
    jassert(input); // FIXME
    return input->getVariable(variableIndex);
  }

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
  {
    TypePtr objectClass = inputTypes[0];
    size_t n = objectClass->getNumMemberVariables();
    VectorPtr res = vector(positiveIntegerType, n);
    for (size_t i = 0; i < n; ++i)
      res->setElement(i, i);
    return res;
  }

protected:
  friend class GetVariableLuapeFunctionClass;
  size_t variableIndex;
};

/*
** Stump
*/
class StumpLuapeFunction : public LuapeFunction
{
public:
  StumpLuapeFunction(double threshold = 0.0) 
    : threshold(threshold) {}

  virtual String toShortString() const
    {return "> " + String(threshold);}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type->inheritsFrom(doubleType) || type->inheritsFrom(integerType);}
  
  virtual TypePtr getOutputType(const std::vector<TypePtr>& ) const
    {return booleanType;}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {return inputs[0]->toShortString() + " >= " + String(threshold);}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
    {return inputs[0].toDouble() >= threshold;}

  virtual VectorPtr compute(ExecutionContext& context, const std::vector<VectorPtr>& inputs, TypePtr outputType) const
  {
    if (inputs[0].isInstanceOf<DenseDoubleVector>())
    {
      size_t n =  inputs[0]->getNumElements();
      double* scalars = inputs[0].staticCast<DenseDoubleVector>()->getValuePointer(0);
      BooleanVectorPtr res = new BooleanVector(n);
      for (std::vector<bool>::iterator it = res->getElements().begin(); it != res->getElements().end(); ++it)
        *it = ((*scalars++) >= threshold);
      return res;
    }
    else
      return LuapeFunction::compute(context, inputs, outputType);
  }

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
  {
    /*
    DenseDoubleVectorPtr res = new DenseDoubleVector(0, 0.0);

    LuapeNodeCachePtr cache = inputs[0]->getCache();
    jassert(cache->isConvertibleToDouble());
    
    const std::vector< std::pair<size_t, double> >& sortedDoubleValues = cache->getSortedDoubleValues();
    if (sortedDoubleValues.size())
    {
      jassert(sortedDoubleValues.size());
      double previousThreshold = sortedDoubleValues[0].second;
      for (size_t i = 0; i < sortedDoubleValues.size(); ++i)
      {
        double threshold = sortedDoubleValues[i].second;
        jassert(threshold >= previousThreshold);
        if (threshold > previousThreshold)
        {
          res->appendValue((threshold + previousThreshold) / 2.0);
          previousThreshold = threshold;
        }
      }
    }
    else
      jassert(false); // no training data, cannot choose thresholds

    return res;*/
    return ContainerPtr();
  }

protected:
  friend class StumpLuapeFunctionClass;

  double threshold;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_SIMPLE_FUNCTIONS_H_

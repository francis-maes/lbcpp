/*-----------------------------------------.---------------------------------.
| Filename: Function.h                     | Function Base Class             |
| Author  : Francis Maes                   |                                 |
| Started : 18/11/2011 15:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_FUNCTION_H_
# define LBCPP_ML_FUNCTION_H_

# include "predeclarations.h"
# include <lbcpp/Core/Variable.h>
# include <lbcpp/Core/Vector.h>

namespace lbcpp
{

class LuapeSampleVector;
typedef ReferenceCountedObjectPtr<LuapeSampleVector> LuapeSampleVectorPtr;

class Function : public Object
{
public:
  enum Flags
  {
    noFlags = 0x00,
    commutativeFlag = 0x01,  // f(x1..xn) = f(x_p_1 .. x_p_n) for any permutation p
    allSameArgIrrelevantFlag = 0x02, // f(x,...,x) = irrelevant (either equal to x or equal to a constant)
  };

  virtual Flags getFlags() const
    {return noFlags;}

  bool hasFlags(const Flags& flags) const
    {return (getFlags()& flags) == flags;}

  virtual size_t getNumInputs() const = 0;
  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const = 0; 
  virtual TypePtr initialize(const TypePtr* inputTypes) = 0; // returns the output type

  virtual bool acceptInputsStack(const std::vector<ExpressionPtr>& stack) const;

  virtual String toShortString() const
    {return getClassName();}
    
  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const;

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
    {jassert(getNumVariables() == 0); return ContainerPtr();}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const = 0;
  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const std::vector<LuapeSampleVectorPtr>& inputs, TypePtr outputType) const;

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<Function> FunctionPtr;
extern ClassPtr luapeFunctionClass;

// Boolean
extern FunctionPtr notBooleanFunction();

extern FunctionPtr andBooleanFunction();
extern FunctionPtr orBooleanFunction();
extern FunctionPtr nandBooleanFunction();
extern FunctionPtr norBooleanFunction();
extern FunctionPtr equalBooleanFunction();

extern FunctionPtr ifThenElseBooleanFunction();

// Integer
extern FunctionPtr addIntegerFunction();
extern FunctionPtr subIntegerFunction();
extern FunctionPtr mulIntegerFunction();
extern FunctionPtr divIntegerFunction();

// Double
extern FunctionPtr oppositeDoubleFunction();
extern FunctionPtr inverseDoubleFunction();
extern FunctionPtr absDoubleFunction();
extern FunctionPtr logDoubleFunction();
extern FunctionPtr expDoubleFunction();
extern FunctionPtr sqrtDoubleFunction();
extern FunctionPtr cosDoubleFunction();
extern FunctionPtr sinDoubleFunction();

extern FunctionPtr addDoubleFunction();
extern FunctionPtr subDoubleFunction();
extern FunctionPtr mulDoubleFunction();
extern FunctionPtr divDoubleFunction();
extern FunctionPtr powDoubleFunction();

extern FunctionPtr minDoubleFunction();
extern FunctionPtr maxDoubleFunction();

// Enumeration
extern FunctionPtr equalsConstantEnumFunction(EnumerationPtr enumeration = EnumerationPtr(), size_t value = 0);

// Special
extern FunctionPtr normalizerFunction();
extern FunctionPtr stumpFunction(double threshold = 0.0);
extern FunctionPtr greaterThanDoubleFunction();

// Object
extern FunctionPtr getVariableFunction(ClassPtr inputClass = ClassPtr(), size_t variableIndex = 0);
extern FunctionPtr getVariableFunction(ClassPtr inputClass, const String& variableName);
extern FunctionPtr getContainerLengthFunction();

// DoubleVector
extern FunctionPtr getDoubleVectorElementFunction(EnumerationPtr enumeration = EnumerationPtr(), size_t index = 0);
extern FunctionPtr computeDoubleVectorStatisticsFunction();
extern FunctionPtr getDoubleVectorExtremumsFunction(EnumerationPtr enumeration = EnumerationPtr());

// Voting
extern FunctionPtr scalarVoteFunction(double vote);
extern FunctionPtr vectorVoteFunction(const DenseDoubleVectorPtr& vote);

/*
** Base classes
*/
class HomogeneousUnaryFunction : public Function
{
public:
  HomogeneousUnaryFunction(TypePtr type = anyType)
    : type(type) {}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type->inheritsFrom(this->type);}

  virtual TypePtr initialize(const TypePtr* inputTypes)
    {return type;}

private:
  TypePtr type;
};

class HomogeneousBinaryFunction : public Function
{
public:
  HomogeneousBinaryFunction(TypePtr type = anyType)
    : type(type) {}

  virtual size_t getNumInputs() const
    {return 2;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type->inheritsFrom(this->type);}

  virtual TypePtr initialize(const TypePtr* inputTypes)
    {return type;}

private:
  TypePtr type;
};

class HomogeneousTernaryFunction : public Function
{
public:
  HomogeneousTernaryFunction(TypePtr type = anyType)
    : type(type) {}

  virtual size_t getNumInputs() const
    {return 3;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type->inheritsFrom(this->type);}

  virtual TypePtr initialize(const TypePtr* inputTypes)
    {return type;}

private:
  TypePtr type;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_FUNCTION_H_

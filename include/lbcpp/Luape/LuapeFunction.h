/*-----------------------------------------.---------------------------------.
| Filename: LuapeFunction.h                | Luape function                  |
| Author  : Francis Maes                   |                                 |
| Started : 18/11/2011 15:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_FUNCTION_H_
# define LBCPP_LUAPE_FUNCTION_H_

# include "predeclarations.h"
# include "../Core/Variable.h"

namespace lbcpp
{

class LuapeFunction : public Object
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

  virtual bool acceptInputsStack(const std::vector<LuapeNodePtr>& stack) const;

  virtual String toShortString() const
    {return getClassName();}
    
  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const;

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
    {jassert(getNumVariables() == 0); return ContainerPtr();}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const = 0;
  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const std::vector<LuapeSampleVectorPtr>& inputs, TypePtr outputType) const;

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<LuapeFunction> LuapeFunctionPtr;
extern ClassPtr luapeFunctionClass;

// Boolean
extern LuapeFunctionPtr andBooleanLuapeFunction();
extern LuapeFunctionPtr equalBooleanLuapeFunction();

// Integer
extern LuapeFunctionPtr addIntegerLuapeFunction();
extern LuapeFunctionPtr subIntegerLuapeFunction();
extern LuapeFunctionPtr mulIntegerLuapeFunction();
extern LuapeFunctionPtr divIntegerLuapeFunction();

// Double
extern LuapeFunctionPtr logDoubleLuapeFunction();
extern LuapeFunctionPtr sqrtDoubleLuapeFunction();

extern LuapeFunctionPtr addDoubleLuapeFunction();
extern LuapeFunctionPtr subDoubleLuapeFunction();
extern LuapeFunctionPtr mulDoubleLuapeFunction();
extern LuapeFunctionPtr divDoubleLuapeFunction();

// Enumeration
extern LuapeFunctionPtr equalsConstantEnumLuapeFunction(EnumerationPtr enumeration = EnumerationPtr(), size_t value = 0);

// Special
extern LuapeFunctionPtr normalizerLuapeFunction();
extern LuapeFunctionPtr stumpLuapeFunction(double threshold = 0.0);
extern LuapeFunctionPtr greaterThanDoubleLuapeFunction();

// Object
extern LuapeFunctionPtr getVariableLuapeFunction(ClassPtr inputClass = ClassPtr(), size_t variableIndex = 0);
extern LuapeFunctionPtr getVariableLuapeFunction(ClassPtr inputClass, const String& variableName);
extern LuapeFunctionPtr getContainerLengthLuapeFunction();

// DoubleVector
extern LuapeFunctionPtr getDoubleVectorElementLuapeFunction(EnumerationPtr enumeration = EnumerationPtr(), size_t index = 0);
extern LuapeFunctionPtr computeDoubleVectorStatisticsLuapeFunction();
extern LuapeFunctionPtr getDoubleVectorExtremumsLuapeFunction(EnumerationPtr enumeration = EnumerationPtr());

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

  virtual TypePtr initialize(const TypePtr* inputTypes)
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

  virtual TypePtr initialize(const TypePtr* inputTypes)
    {return type;}

private:
  TypePtr type;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FUNCTION_H_

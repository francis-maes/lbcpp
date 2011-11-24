/*-----------------------------------------.---------------------------------.
| Filename: LuapeFunction.h                | Luape function                  |
| Author  : Francis Maes                   |                                 |
| Started : 18/11/2011 15:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_FUNCTION_H_
# define LBCPP_LUAPE_FUNCTION_H_

namespace lbcpp
{

class LuapeNode;
typedef ReferenceCountedObjectPtr<LuapeNode> LuapeNodePtr;

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
  virtual TypePtr getOutputType(const std::vector<TypePtr>& inputTypes) const = 0;

  bool acceptInputsStack(const std::vector<LuapeNodePtr>& stack) const;

  virtual String toShortString() const
    {return getClassName();}
    
  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const;

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
    {jassert(getNumVariables() == 0); return ContainerPtr();}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const = 0;
  virtual Variable compute(ExecutionContext& context, const std::vector<LuapeNodePtr>& inputs, const std::vector<Variable>& state) const;
  virtual VectorPtr compute(ExecutionContext& context, const std::vector<VectorPtr>& inputs, TypePtr outputType) const;

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<LuapeFunction> LuapeFunctionPtr;
extern ClassPtr luapeFunctionClass;

extern LuapeFunctionPtr andBooleanLuapeFunction();
extern LuapeFunctionPtr equalBooleanLuapeFunction();

extern LuapeFunctionPtr addDoubleLuapeFunction();
extern LuapeFunctionPtr subDoubleLuapeFunction();
extern LuapeFunctionPtr mulDoubleLuapeFunction();
extern LuapeFunctionPtr divDoubleLuapeFunction();

extern LuapeFunctionPtr greaterThanDoubleLuapeFunction();
extern LuapeFunctionPtr equalsConstantEnumLuapeFunction(const Variable& value = Variable());
extern LuapeFunctionPtr stumpLuapeFunction(double threshold = 0.0);
extern LuapeFunctionPtr getVariableLuapeFunction(size_t variableIndex = 0);

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FUNCTION_H_

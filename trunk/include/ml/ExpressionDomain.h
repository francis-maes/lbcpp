/*-----------------------------------------.---------------------------------.
| Filename: ExpressionDomain.h             | Expression Domain               |
| Author  : Francis Maes                   |                                 |
| Started : 03/10/2012 13:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_EXPRESSION_DOMAIN_H_
# define LBCPP_ML_EXPRESSION_DOMAIN_H_

# include "Domain.h"
# include "Expression.h"
# include "Problem.h"
# include "Search.h"

namespace lbcpp
{

/*
** Expression Domain
*/
class ExpressionDomain : public Domain
{
public:
  /*
  ** Inputs
  */
  size_t getNumInputs() const
    {return inputs.size();}

  const VariableExpressionPtr& getInput(size_t index) const
    {jassert(index < inputs.size()); return inputs[index];}
  
  const std::vector<VariableExpressionPtr>& getInputs() const
    {return inputs;}

  VariableExpressionPtr addInput(const ClassPtr& type, const string& name);
  void addInputs(const std::vector<VariableExpressionPtr>& inputs);
  
  /*
  ** Available Constants
  */
  size_t getNumConstants() const
    {return constants.size();}

  const ConstantExpressionPtr& getConstant(size_t index) const
    {jassert(index < constants.size()); return constants[index];}

  void addConstant(const ObjectPtr& value)
    {constants.push_back(new ConstantExpression(value));}

  /*
  ** Active variables
  */
  size_t getNumActiveVariables() const
    {return activeVariables.size();}

  ExpressionPtr getActiveVariable(size_t index) const;

  const std::set<ExpressionPtr>& getActiveVariables() const
    {return activeVariables;}

  void addActiveVariable(const ExpressionPtr& node)
    {activeVariables.insert(node);}

  void clearActiveVariables()
    {activeVariables.clear();}

  /*
  ** Supervision variable
  */
  VariableExpressionPtr getSupervision() const
    {return supervision;}

  void setSupervision(const VariableExpressionPtr& supervision)
    {this->supervision = supervision;}

  VariableExpressionPtr createSupervision(const ClassPtr& type, const string& name);

  /*
  ** Table
  */
  TablePtr createTable(size_t numSamples) const;

  /*
  ** Available Functions
  */
  size_t getNumFunctions() const
    {return functions.size();}

  const FunctionPtr& getFunction(size_t index) const
    {jassert(index < functions.size()); return functions[index];}

  const std::vector<FunctionPtr>& getFunctions() const
    {return functions;}

  void addFunction(const FunctionPtr& function)
    {functions.push_back(function);}

  size_t getMaxFunctionArity() const;

  /*
  ** Accepted target types
  */
  bool isTargetTypeAccepted(ClassPtr type);
  void addTargetType(ClassPtr type)
    {targetTypes.insert(type);}
  void clearTargetTypes()
    {targetTypes.clear();}

  /*
  ** Symbol Map
  */
  std::vector<ExpressionPtr> getTerminals() const;
  std::vector<ObjectPtr> getTerminalsAndFunctions() const;

  const std::map<ObjectPtr, size_t>& getSymbolMap() const;
  size_t getSymbolIndex(const ObjectPtr& symbol) const;
  size_t getNumSymbols() const;
  ObjectPtr getSymbol(size_t index) const;
  static size_t getSymbolArity(const ObjectPtr& symbol);

  /*
  ** Object
  */
  virtual string toShortString() const;

  /*
  ** Search space - bof
  */
  PostfixExpressionTypeSpacePtr getSearchSpace(ExecutionContext& context, size_t complexity, bool verbose = false) const; // cached with initialState = vector<ClassPtr>()
  PostfixExpressionTypeSpacePtr createTypeSearchSpace(ExecutionContext& context, const std::vector<ClassPtr>& initialState, size_t complexity, bool verbose) const;

protected:
  friend class ExpressionDomainClass;

  std::vector<VariableExpressionPtr> inputs;
  VariableExpressionPtr supervision;
  std::vector<ConstantExpressionPtr> constants;
  std::vector<FunctionPtr> functions;
  std::set<ClassPtr> targetTypes;
  std::set<ExpressionPtr> activeVariables;

  CriticalSection typeSearchSpacesLock;
  std::vector<PostfixExpressionTypeSpacePtr> typeSearchSpaces;

  std::map<ObjectPtr, size_t> symbolMap;
  std::vector<ObjectPtr> symbols;

  void addSymbol(ObjectPtr symbol);
};

typedef ReferenceCountedObjectPtr<ExpressionDomain> ExpressionDomainPtr;
extern ClassPtr expressionDomainClass;

/*
** Expression Search Spaces
*/
class ExpressionState : public SearchState
{
public:
  ExpressionState(ExpressionDomainPtr domain, size_t maxSize);
  ExpressionState() {}

  const ExpressionDomainPtr& getDomain() const
    {return domain;}

  size_t getMaxSize() const
    {return maxSize;}

  const std::vector<ObjectPtr>& getTrajectory() const
    {return trajectory;}
  
  size_t getTrajectoryLength() const
    {return trajectory.size();}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

protected:
  friend class ExpressionStateClass;

  ExpressionDomainPtr domain;
  size_t maxSize;
  std::vector<ObjectPtr> trajectory;
};

typedef ReferenceCountedObjectPtr<ExpressionState> ExpressionStatePtr;

extern ExpressionStatePtr prefixExpressionState(ExpressionDomainPtr domain, size_t maxSize);
extern ExpressionStatePtr postfixExpressionState(ExpressionDomainPtr domain, size_t maxSize);
extern ExpressionStatePtr typedPostfixExpressionState(ExpressionDomainPtr domain, size_t maxSize);

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_DOMAIN_H_

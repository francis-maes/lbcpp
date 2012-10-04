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
# include "ExpressionUniverse.h"
# include "ExpressionRPN.h"
# include <lbcpp/Luape/LuapeCache.h>

namespace lbcpp
{

class ExpressionDomain : public Domain
{
public:
  ExpressionDomain(ExpressionUniversePtr universe = ExpressionUniversePtr());

  const ExpressionUniversePtr& getUniverse() const
    {return universe;}

  /*
  ** Inputs
  */
  size_t getNumInputs() const
    {return inputs.size();}

  const VariableExpressionPtr& getInput(size_t index) const
    {jassert(index < inputs.size()); return inputs[index];}
  
  const std::vector<VariableExpressionPtr>& getInputs() const
    {return inputs;}

  VariableExpressionPtr addInput(const TypePtr& type, const String& name);

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

  VariableExpressionPtr createSupervision(const TypePtr& type, const String& name);

  /*
  ** Available Functions
  */
  size_t getNumFunctions() const
    {return functions.size();}

  const FunctionPtr& getFunction(size_t index) const
    {jassert(index < functions.size()); return functions[index];}

  void addFunction(const FunctionPtr& function)
    {functions.push_back(function);}

  /*
  ** Available Constants
  */
  size_t getNumConstants() const
    {return constants.size();}

  const ConstantExpressionPtr& getConstant(size_t index) const
    {jassert(index < constants.size()); return constants[index];}

  void addConstant(const Variable& value)
    {constants.push_back(new ConstantExpression(value));}

  /*
  ** Accepted target types
  */
  bool isTargetTypeAccepted(TypePtr type);
  void addTargetType(TypePtr type)
    {targetTypes.insert(type);}
  void clearTargetTypes()
    {targetTypes.clear();}

  /*
  ** Search space
  */
  ExpressionRPNTypeSpacePtr getSearchSpace(ExecutionContext& context, size_t complexity, bool verbose = false) const; // cached with initialState = vector<TypePtr>()

  ExpressionRPNTypeSpacePtr createTypeSearchSpace(ExecutionContext& context, const std::vector<TypePtr>& initialState, size_t complexity, bool verbose) const;
  void enumerateNodesExhaustively(ExecutionContext& context, size_t complexity, std::vector<ExpressionPtr>& res, bool verbose = false, const ExpressionRPNSequencePtr& subSequence = ExpressionRPNSequencePtr()) const;

  /*
  ** Samples cache
  */
  virtual void setSamples(ExecutionContext& context, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData = std::vector<ObjectPtr>());

  const LuapeSamplesCachePtr& getTrainingCache() const
    {return trainingCache;}

  const LuapeSamplesCachePtr& getValidationCache() const
    {return validationCache;}

  std::vector<LuapeSamplesCachePtr> getSamplesCaches() const;

  VectorPtr getTrainingPredictions() const;
  VectorPtr getTrainingSupervisions() const;
  VectorPtr getValidationPredictions() const;
  VectorPtr getValidationSupervisions() const;

  LuapeSamplesCachePtr createCache(size_t size, size_t maxCacheSizeInMb = 512) const;

  /*
  ** Deprecated
  */
  const ExpressionPtr& getRootNode() const
    {return node;}
  void setRootNode(ExecutionContext& context, const ExpressionPtr& node);
  void clearRootNode(ExecutionContext& context);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const
    {jassert(false); return 0.0;}
//void setLearner(const LuapeLearnerPtr& learner, bool verbose = false);

protected:
  friend class ExpressionDomainClass;

  ExpressionUniversePtr universe;
  std::vector<VariableExpressionPtr> inputs;
  VariableExpressionPtr supervision;
  std::vector<ConstantExpressionPtr> constants;
  std::vector<FunctionPtr> functions;
  std::set<TypePtr> targetTypes;
  std::set<ExpressionPtr> activeVariables;
  ExpressionPtr node;
  LuapeSamplesCachePtr trainingCache;
  LuapeSamplesCachePtr validationCache;

  CriticalSection typeSearchSpacesLock;
  std::vector<ExpressionRPNTypeSpacePtr> typeSearchSpaces;

  Variable computeNode(ExecutionContext& context, const ObjectPtr& inputs) const;

  LuapeSamplesCachePtr createSamplesCache(ExecutionContext& context, const std::vector<ObjectPtr>& data) const;
};

typedef ReferenceCountedObjectPtr<ExpressionDomain> ExpressionDomainPtr;
extern ClassPtr expressionDomainClass;

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_DOMAIN_H_

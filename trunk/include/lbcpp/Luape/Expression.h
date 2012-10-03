/*-----------------------------------------.---------------------------------.
| Filename: Expression.h                   | Expression classes              |
| Author  : Francis Maes                   |                                 |
| Started : 18/11/2011 15:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_NODE_H_
# define LBCPP_LUAPE_NODE_H_

# include "Function.h"
# include "../Data/IndexSet.h"

namespace lbcpp
{

class Expression : public Object
{
public:
  Expression(const TypePtr& type = nilType);

  const TypePtr& getType() const
    {return type;}

  void setType(const TypePtr& type)
    {this->type = type;}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const = 0;
  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const = 0;
  Variable compute(ExecutionContext& context) const;

  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache, const IndexSetPtr& indices) const = 0;
  
  virtual size_t getNumSubNodes() const
    {return 0;}
    
  virtual const ExpressionPtr& getSubNode(size_t index) const
    {jassert(false); static ExpressionPtr empty; return empty;} 

  size_t getAllocationIndex() const
    {return allocationIndex;}

  void addImportance(double delta);

  double getImportance() const
    {return importance;}

  void setImportance(double importance)
    {jassert(isNumberValid(importance)); this->importance = importance;}

  size_t getDepth() const;
  size_t getTreeSize() const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeGraph;
  friend class ExpressionClass;

  TypePtr type;
  size_t allocationIndex;
  double importance;
};

extern ClassPtr expressionClass;

/*
** Input
*/
class VariableExpression : public Expression
{
public:
  VariableExpression(const TypePtr& type, const String& name, size_t inputIndex);
  VariableExpression();

  virtual String toShortString() const;
  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const;
  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;
  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache, const IndexSetPtr& indices) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class VariableExpressionClass;

  String name;
  size_t inputIndex;
};

/*
** Constant
*/
class ConstantExpression : public Expression
{
public:
  ConstantExpression(const Variable& value);
  ConstantExpression() {}

  virtual String toShortString() const;
  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const;
  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;
  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache, const IndexSetPtr& indices) const;

  const Variable& getValue() const
    {return value;}

  void setValue(const Variable& value)
    {type = value.getType(); this->value = value;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ConstantExpressionClass;

  Variable value;
};

/*
** Function
*/
class FunctionExpression : public Expression
{
public:
  FunctionExpression(const FunctionPtr& function, const std::vector<ExpressionPtr>& arguments);
  FunctionExpression(const FunctionPtr& function, const ExpressionPtr& argument1, const ExpressionPtr& argument2);
  FunctionExpression(const FunctionPtr& function, const ExpressionPtr& argument);
  FunctionExpression() {}

  virtual String toShortString() const;

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const;
  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;
  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache, const IndexSetPtr& indices) const;

  virtual size_t getNumSubNodes() const
    {return arguments.size();}
  virtual const ExpressionPtr& getSubNode(size_t index) const
    {jassert(index < arguments.size()); return arguments[index];}

  const FunctionPtr& getFunction() const
    {return function;}

  size_t getNumArguments() const
    {return arguments.size();}

  const ExpressionPtr& getArgument(size_t index) const
    {jassert(index < arguments.size()); return arguments[index];}

  const std::vector<ExpressionPtr>& getArguments() const
    {return arguments;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class FunctionExpressionClass;

  FunctionPtr function;
  std::vector<ExpressionPtr> arguments;

  void initialize();
};

extern ClassPtr functionExpressionClass;

/*
** Test
*/
class TestExpression : public Expression
{
public:
  TestExpression(const ExpressionPtr& conditionNode, const ExpressionPtr& failureNode, const ExpressionPtr& successNode, const ExpressionPtr& missingNode);
  TestExpression(const ExpressionPtr& conditionNode, TypePtr outputType);
  TestExpression() {}

  virtual String toShortString() const;
  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const;
  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;
  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache, const IndexSetPtr& indices) const;

  virtual size_t getNumSubNodes() const;
  virtual const ExpressionPtr& getSubNode(size_t index) const;

  static void dispatchIndices(const LuapeSampleVectorPtr& conditionValues, IndexSetPtr& failureIndices, IndexSetPtr& successIndices, IndexSetPtr& missingIndices);

  const ExpressionPtr& getCondition() const
    {return conditionNode;}

  const ExpressionPtr& getFailure() const
    {return failureNode;}

  void setFailure(const ExpressionPtr& node)
    {failureNode = node;}

  const ExpressionPtr& getSuccess() const
    {return successNode;}

  void setSuccess(const ExpressionPtr& node)
    {successNode = node;}

  const ExpressionPtr& getMissing() const
    {return missingNode;}

  void setMissing(const ExpressionPtr& node)
    {missingNode = node;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class TestExpressionClass;

  ExpressionPtr conditionNode;
  ExpressionPtr failureNode;
  ExpressionPtr successNode;
  ExpressionPtr missingNode;

  LuapeSampleVectorPtr getSubSamples(ExecutionContext& context, const ExpressionPtr& subNode, const LuapeSamplesCachePtr& cache, const IndexSetPtr& subIndices) const;
};

/*
** Sequence
*/
class SequenceExpression : public Expression
{
public:
  SequenceExpression(TypePtr type, const std::vector<ExpressionPtr>& nodes);
  SequenceExpression(TypePtr type) : Expression(type) {}
  SequenceExpression() {}

  virtual String toShortString() const;
  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache, const IndexSetPtr& indices) const;

  virtual size_t getNumSubNodes() const
    {return nodes.size();}
    
  virtual const ExpressionPtr& getSubNode(size_t index) const
    {return nodes[index];}
  
  void pushNode(ExecutionContext& context, const ExpressionPtr& node, const std::vector<LuapeSamplesCachePtr>& cachesToUpdate = std::vector<LuapeSamplesCachePtr>());

  void clearNodes()
    {nodes.clear();}

  void reserveNodes(size_t size)
    {nodes.reserve(size);}

  const std::vector<ExpressionPtr>& getNodes() const
    {return nodes;}

  void setNodes(const std::vector<ExpressionPtr>& nodes)
    {this->nodes = nodes;}

  void setNode(size_t index, const ExpressionPtr& node)
    {jassert(index < nodes.size()); nodes[index] = node;}

protected:
  friend class SequenceExpressionClass;

  std::vector<ExpressionPtr> nodes;

  virtual VectorPtr createEmptyOutputs(size_t numSamples) const = 0;
  virtual void updateOutputs(const VectorPtr& outputs, const LuapeSampleVectorPtr& newNodeValues, size_t newNodeIndex) const = 0;
};

typedef ReferenceCountedObjectPtr<SequenceExpression> SequenceExpressionPtr;

/*
** Sum
*/
class ScalarSumExpression : public SequenceExpression
{
public:
  ScalarSumExpression(const std::vector<ExpressionPtr>& nodes, bool convertToProbabilities, bool computeAverage);
  ScalarSumExpression(bool convertToProbabilities = false, bool computeAverage = true);

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const;
  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;

protected:
  friend class ScalarSumExpressionClass;

  bool convertToProbabilities;
  bool computeAverage;

  virtual VectorPtr createEmptyOutputs(size_t numSamples) const;
  virtual void updateOutputs(const VectorPtr& outputs, const LuapeSampleVectorPtr& newNodeValues, size_t newNodeIndex) const;
};

class VectorSumExpression : public SequenceExpression
{
public:
  VectorSumExpression(EnumerationPtr enumeration, bool convertToProbabilities);
  VectorSumExpression() {}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const;
  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;
  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache, const IndexSetPtr& indices) const;

protected:
  friend class VectorSumExpressionClass;

  bool convertToProbabilities;

  virtual VectorPtr createEmptyOutputs(size_t numSamples) const;
  virtual void updateOutputs(const VectorPtr& outputs, const LuapeSampleVectorPtr& newNodeValues, size_t newNodeIndex) const;

  DenseDoubleVectorPtr convertToProbabilitiesUsingSigmoid(const DenseDoubleVectorPtr& activations) const;
};

class CreateSparseVectorExpression : public SequenceExpression
{
public:
  CreateSparseVectorExpression(const std::vector<ExpressionPtr>& nodes);
  CreateSparseVectorExpression();
    
  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const;
  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;

protected:
  virtual VectorPtr createEmptyOutputs(size_t numSamples) const;
  virtual void updateOutputs(const VectorPtr& outputs, const LuapeSampleVectorPtr& newNodeValues, size_t newNodeIndex) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_H_

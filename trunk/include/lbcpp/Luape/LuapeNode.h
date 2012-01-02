/*-----------------------------------------.---------------------------------.
| Filename: LuapeNode.h                    | Luape Graph Node                |
| Author  : Francis Maes                   |                                 |
| Started : 18/11/2011 15:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_NODE_H_
# define LBCPP_LUAPE_NODE_H_

# include "LuapeFunction.h"
# include "../Data/IndexSet.h"

namespace lbcpp
{

class LuapeNode : public Object
{
public:
  LuapeNode(const TypePtr& type = nilType);

  const TypePtr& getType() const
    {return type;}

  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const = 0;
  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache, const IndexSetPtr& indices) const = 0;
  
  virtual size_t getNumSubNodes() const
    {return 0;}
    
  virtual const LuapeNodePtr& getSubNode(size_t index) const
    {jassert(false); static LuapeNodePtr empty; return empty;} 

  size_t getAllocationIndex() const
    {return allocationIndex;}

  void addImportance(double delta);

  double getImportance() const
    {return importance;}

  void setImportance(double importance)
    {jassert(isNumberValid(importance)); this->importance = importance;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeGraph;
  friend class LuapeNodeClass;

  TypePtr type;
  size_t allocationIndex;
  double importance;
};

extern ClassPtr luapeNodeClass;

/*
** Input
*/
class LuapeInputNode : public LuapeNode
{
public:
  LuapeInputNode(const TypePtr& type, const String& name);
  LuapeInputNode() {}

  virtual String toShortString() const;
  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;
  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache, const IndexSetPtr& indices) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeInputNodeClass;

  String name;
};

/*
** Constant
*/
class LuapeConstantNode : public LuapeNode
{
public:
  LuapeConstantNode(const Variable& value);
  LuapeConstantNode() {}

  virtual String toShortString() const;
  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;
  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache, const IndexSetPtr& indices) const;

  const Variable& getValue() const
    {return value;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeConstantNodeClass;

  Variable value;
};

/*
** Function
*/
class LuapeFunctionNode : public LuapeNode
{
public:
  LuapeFunctionNode(const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& arguments);
  LuapeFunctionNode(const LuapeFunctionPtr& function, LuapeNodePtr argument);
  LuapeFunctionNode() {}

  virtual String toShortString() const;

  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;
  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache, const IndexSetPtr& indices) const;

  virtual size_t getNumSubNodes() const
    {return arguments.size();}
  virtual const LuapeNodePtr& getSubNode(size_t index) const
    {jassert(index < arguments.size()); return arguments[index];}

  const LuapeFunctionPtr& getFunction() const
    {return function;}

  size_t getNumArguments() const
    {return arguments.size();}

  const LuapeNodePtr& getArgument(size_t index) const
    {jassert(index < arguments.size()); return arguments[index];}

  const std::vector<LuapeNodePtr>& getArguments() const
    {return arguments;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeFunctionNodeClass;

  LuapeFunctionPtr function;
  std::vector<LuapeNodePtr> arguments;

  void initialize();
};

extern ClassPtr luapeFunctionNodeClass;

/*
** Test
*/
class LuapeTestNode : public LuapeNode
{
public:
  LuapeTestNode(const LuapeNodePtr& conditionNode, const LuapeNodePtr& successNode, const LuapeNodePtr& failureNode, const LuapeNodePtr& missingNode);
  LuapeTestNode() {}

  virtual String toShortString() const;
  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;
  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache, const IndexSetPtr& indices) const;

  virtual size_t getNumSubNodes() const;
  virtual const LuapeNodePtr& getSubNode(size_t index) const;

  void dispatchIndices(const LuapeSampleVectorPtr& conditionValues, IndexSetPtr& failureIndices, IndexSetPtr& successIndices, IndexSetPtr& missingIndices) const;

  const LuapeNodePtr& getCondition() const
    {return conditionNode;}

  const LuapeNodePtr& getFailure() const
    {return failureNode;}

  void setFailure(const LuapeNodePtr& node)
    {failureNode = node;}

  const LuapeNodePtr& getSuccess() const
    {return successNode;}

  void setSuccess(const LuapeNodePtr& node)
    {successNode = node;}

  const LuapeNodePtr& getMissing() const
    {return missingNode;}

  void setMissing(const LuapeNodePtr& node)
    {missingNode = node;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeTestNodeClass;

  LuapeNodePtr conditionNode;
  LuapeNodePtr failureNode;
  LuapeNodePtr successNode;
  LuapeNodePtr missingNode;
};

/*
** Sequence
*/
class LuapeSequenceNode : public LuapeNode
{
public:
  LuapeSequenceNode(TypePtr type, const std::vector<LuapeNodePtr>& nodes);
  LuapeSequenceNode(TypePtr type) : LuapeNode(type) {}
  LuapeSequenceNode() {}

  virtual String toShortString() const;
  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const LuapeSamplesCachePtr& cache, const IndexSetPtr& indices) const;

  virtual size_t getNumSubNodes() const
    {return nodes.size();}
    
  virtual const LuapeNodePtr& getSubNode(size_t index) const
    {return nodes[index];}
  
  void pushNode(ExecutionContext& context, const LuapeNodePtr& node, const std::vector<LuapeSamplesCachePtr>& cachesToUpdate = std::vector<LuapeSamplesCachePtr>());

  void reserveNodes(size_t size)
    {nodes.reserve(size);}

protected:
  friend class LuapeSequenceNodeClass;

  std::vector<LuapeNodePtr> nodes;

  virtual VectorPtr createEmptyOutputs(size_t numSamples) const = 0;
  virtual void updateOutputs(const VectorPtr& outputs, const LuapeSampleVectorPtr& newNodeValues) const = 0;
};

typedef ReferenceCountedObjectPtr<LuapeSequenceNode> LuapeSequenceNodePtr;

/*
** Sum
*/
class LuapeScalarSumNode : public LuapeSequenceNode
{
public:
  LuapeScalarSumNode(const std::vector<LuapeNodePtr>& nodes);
  LuapeScalarSumNode();

  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;

protected:
  virtual VectorPtr createEmptyOutputs(size_t numSamples) const;
  virtual void updateOutputs(const VectorPtr& outputs, const LuapeSampleVectorPtr& newNodeValues) const;
};

class LuapeVectorSumNode : public LuapeSequenceNode
{
public:
  LuapeVectorSumNode(EnumerationPtr enumeration, const std::vector<LuapeNodePtr>& nodes);
  LuapeVectorSumNode(EnumerationPtr enumeration);
  LuapeVectorSumNode() {}

  virtual Variable compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const;

protected:
  virtual VectorPtr createEmptyOutputs(size_t numSamples) const;
  virtual void updateOutputs(const VectorPtr& outputs, const LuapeSampleVectorPtr& newNodeValues) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_H_

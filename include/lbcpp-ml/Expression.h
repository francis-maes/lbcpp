/*-----------------------------------------.---------------------------------.
| Filename: Expression.h                   | Expression classes              |
| Author  : Francis Maes                   |                                 |
| Started : 18/11/2011 15:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_EXPRESSION_H_
# define LBCPP_ML_EXPRESSION_H_

# include "Function.h"
# include "Aggregator.h"
# include <lbcpp/Core/Table.h>

namespace lbcpp
{

/*
** DataVector
*/
class DataVector;
typedef ReferenceCountedObjectPtr<DataVector> DataVectorPtr;

class DataVector : public Object
{
public:
  enum Implementation
  {
    constantValueImpl = 0,
    ownedVectorImpl,
    cachedVectorImpl,
    noImpl
  };

  DataVector(Implementation implementation, const IndexSetPtr& indices, const ClassPtr& elementsType);
  DataVector(const IndexSetPtr& indices, const VectorPtr& ownedVector);
  DataVector();

  static DataVectorPtr createConstant(IndexSetPtr indices, const ObjectPtr& constantValue);
  static DataVectorPtr createCached(IndexSetPtr indices, const VectorPtr& cachedVector);
  
  const ClassPtr& getElementsType() const
    {return elementsType;}

  struct const_iterator
  {
    typedef std::vector<size_t>::const_iterator index_iterator;

    const_iterator(const DataVector* owner, size_t position, index_iterator it)
      : owner(owner), position(position), it(it) {}
    const_iterator(const const_iterator& other)
      : owner(other.owner), position(other.position), it(other.it) {}
    const_iterator() : owner(NULL), position(0) {}

    const_iterator& operator =(const const_iterator& other)
      {owner = other.owner; position = other.position; it = other.it; return *this;}

    const_iterator& operator ++()
    {
      ++position;
      ++it;
      return *this;
    }

    inline unsigned char getRawBoolean() const
    {
      if (owner->implementation == constantValueImpl)
        return owner->constantRawBoolean;
      size_t index = (owner->implementation == ownedVectorImpl ? position : *it);
      if (owner->elementsType == booleanClass)
        return owner->vector.staticCast<BVector>()->get(index);
      else
      {
        ObjectPtr element = owner->vector->getElement(index);
        return element ? (element->toBoolean() ? 1 : 0) : 2;
      }
    }

    inline int getRawInteger() const
    {
      switch (owner->implementation)
      {
      case constantValueImpl: return (int)Integer::get(owner->constantRawObject);
      case ownedVectorImpl: return (int)Integer::get(owner->vector->getElement(position));
      case cachedVectorImpl: return (int)Integer::get(owner->vector->getElement(*it));
      default: jassert(false); return 0;
      }
    }

    inline double getRawDouble() const
    {
      switch (owner->implementation)
      {
      case constantValueImpl: return owner->constantRawDouble;
      case ownedVectorImpl: return owner->vector.staticCast<DVector>()->get(position);
      case cachedVectorImpl: return owner->vector.staticCast<DVector>()->get(*it);
      default: jassert(false); return 0.0;
      }
    }

    inline const ObjectPtr& getRawObject() const
    {
      switch (owner->implementation)
      {
      case constantValueImpl: return owner->constantRawObject;
      case ownedVectorImpl: return owner->vector.staticCast<OVector>()->get(position);
      case cachedVectorImpl: return owner->vector.staticCast<OVector>()->get(*it);
      default: jassert(false); static ObjectPtr empty; return empty;
      }
    }

    bool operator ==(const const_iterator& other) const
      {return owner == other.owner && position == other.position;}

    bool operator !=(const const_iterator& other) const
      {return owner != other.owner || position != other.position;}

    size_t getIndex() const
      {return *it;}

  private:
    friend class DataVector;

    const DataVector* owner;
    size_t position;
    index_iterator it;
  };

  const_iterator begin() const
    {return const_iterator(this, 0, indices->begin());}

  const_iterator end() const
    {return const_iterator(this, indices->size(), indices->end());}

  size_t size() const
    {return indices->size();}

  const IndexSetPtr& getIndices() const
    {return indices;}

  Implementation getImplementation() const
    {return implementation;}

  const VectorPtr& getVector() const
    {return vector;}

  ObjectPtr sampleElement(RandomGeneratorPtr random) const;

protected:
  Implementation implementation;
  IndexSetPtr indices;
  ClassPtr elementsType;

  unsigned char constantRawBoolean;
  double constantRawDouble;
  ObjectPtr constantRawObject;

  VectorPtr vector;       // ownedVectorImpl and cachedVectorImpl
};

typedef ReferenceCountedObjectPtr<DataVector> DataVectorPtr;

class Expression : public Object
{
public:
  Expression(const ClassPtr& type = objectClass);

  const ClassPtr& getType() const
    {return type;}

  void setType(const ClassPtr& type)
    {this->type = type;}

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const = 0;
  
  DataVectorPtr compute(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices = IndexSetPtr()) const;

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
  size_t getNodeDepth(ExpressionPtr node) const;
  size_t getTreeSize() const;
  ExpressionPtr getNodeByTreeIndex(size_t index) const;

  ExpressionPtr cloneAndSubstitute(ExpressionPtr sourceNode, ExpressionPtr targetNode) const;
  void getInternalNodes(std::vector<ExpressionPtr>& res) const;
  void getLeafNodes(std::vector<ExpressionPtr>& res) const;
  ExpressionPtr sampleNode(RandomGeneratorPtr random, double functionSelectionProbability) const;
  ExpressionPtr sampleNode(RandomGeneratorPtr random) const;
  ExpressionPtr sampleSubNode(RandomGeneratorPtr random) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExpressionClass;

  ClassPtr type;
  size_t allocationIndex;
  double importance;

  virtual DataVectorPtr computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const = 0;
};

extern ClassPtr expressionClass;

/*
** Input
*/
class VariableExpression : public Expression
{
public:
  VariableExpression(const ClassPtr& type, const string& name, size_t inputIndex);
  VariableExpression();

  virtual string toShortString() const;
  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const;
  virtual DataVectorPtr computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const;

  size_t getInputIndex() const
    {return inputIndex;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class VariableExpressionClass;

  string name;
  size_t inputIndex;
};

/*
** Constant
*/
class ConstantExpression : public Expression
{
public:
  ConstantExpression(const ObjectPtr& value);
  ConstantExpression() {}

  virtual string toShortString() const;
  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const;
  virtual DataVectorPtr computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const;

  const ObjectPtr& getValue() const
    {return value;}

  void setValue(const ObjectPtr& value)
    {type = value->getClass(); this->value = value;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ConstantExpressionClass;

  ObjectPtr value;
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

  virtual string toShortString() const;

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const;

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

  std::vector<ExpressionPtr>& getArguments()
    {return arguments;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class FunctionExpressionClass;

  FunctionPtr function;
  std::vector<ExpressionPtr> arguments;

  virtual DataVectorPtr computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const;

  void initialize();
};

extern ClassPtr functionExpressionClass;

/*
** Aggregator Expression
*/
class AggregatorExpression : public Expression
{
public:
  AggregatorExpression(AggregatorPtr aggregator, const std::vector<ExpressionPtr>& nodes);
  AggregatorExpression(AggregatorPtr aggregator, ClassPtr type);
  AggregatorExpression() {}

  virtual string toShortString() const;

  virtual size_t getNumSubNodes() const
    {return nodes.size();}
    
  virtual const ExpressionPtr& getSubNode(size_t index) const
    {return nodes[index];}
  
  void clearNodes()
    {nodes.clear();}

  void reserveNodes(size_t size)
    {nodes.reserve(size);}

  void pushNode(const ExpressionPtr& node)
    {nodes.push_back(node);}

  const std::vector<ExpressionPtr>& getNodes() const
    {return nodes;}

  void setNodes(const std::vector<ExpressionPtr>& nodes)
    {this->nodes = nodes;}

  void setNode(size_t index, const ExpressionPtr& node)
    {jassert(index < nodes.size()); nodes[index] = node;}
  
  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const;
  
  lbcpp_UseDebuggingNewOperator

protected:
  friend class AggregatorExpressionClass;

  AggregatorPtr aggregator;
  std::vector<ExpressionPtr> nodes;

  virtual DataVectorPtr computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const;
};

typedef ReferenceCountedObjectPtr<AggregatorExpression> AggregatorExpressionPtr;

/*
** Test
*/
class TestExpression : public Expression
{
public:
  TestExpression(const ExpressionPtr& conditionNode, const ExpressionPtr& failureNode, const ExpressionPtr& successNode, const ExpressionPtr& missingNode);
  TestExpression(const ExpressionPtr& conditionNode, ClassPtr outputType);
  TestExpression() {}

  virtual string toShortString() const;
  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const;

  virtual size_t getNumSubNodes() const;
  virtual const ExpressionPtr& getSubNode(size_t index) const;

  static void dispatchIndices(const DataVectorPtr& conditionValues, IndexSetPtr& failureIndices, IndexSetPtr& successIndices, IndexSetPtr& missingIndices);

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

  virtual DataVectorPtr computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const;

  DataVectorPtr getSubSamples(ExecutionContext& context, const ExpressionPtr& subNode, const TablePtr& data, const IndexSetPtr& subIndices) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_H_

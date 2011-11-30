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

namespace lbcpp
{

class LuapeGraph;
typedef ReferenceCountedObjectPtr<LuapeGraph> LuapeGraphPtr;

class LuapeGraphCallback
{
public:
  virtual ~LuapeGraphCallback() {}

  virtual void valueYielded(const Variable& value) {}
};

typedef LuapeGraphCallback* LuapeGraphCallbackPtr;

class BinaryKey : public Object
{
public:
  BinaryKey(size_t size)
    : values(size, 0), position(0), bitShift(1) {}
  BinaryKey() {}

  virtual int compare(const ObjectPtr& otherObject) const
  {
    const std::vector<unsigned char>& ovalues = otherObject.staticCast<BinaryKey>()->values;
    if (ovalues == values)
      return 0;
    else
      return values < ovalues ? -1 : 1;
  }

  void pushBit(bool value)
  {
    jassert(position < values.size());
    if (value)
      values[position] |= bitShift;
    bitShift <<= 1;
    if (bitShift == 256)
    {
      ++position;
      bitShift = 1;
    }
  }
  void fillBits()
  {
    if (bitShift > 1)
      bitShift = 1, ++position;
  }

  void pushByte(unsigned char c)
    {jassert(position < values.size()); values[position++] = c;}

  void pushBytes(unsigned char* data, size_t length)
  {
    jassert(position + length <= values.size());
    memcpy(&values[position], data, length);
    position += length;
  }

  void push32BitInteger(int value)
  {
    memcpy(&values[position], &value, 4);
    position += 4;
  }

  void pushInteger(juce::int64 value)
  {
    memcpy(&values[position], &value, sizeof (juce::int64));
    position += sizeof (juce::int64);
  }

  void pushPointer(const ObjectPtr& object)
  {
    void* pointer = object.get();
    memcpy(&values[position], &pointer, sizeof (void* ));
    position += sizeof (void* );
  }

  size_t computeHashValue() const
  {
    size_t res = 0;
    const unsigned char* ptr = &values[0];
    const unsigned char* lim = ptr + values.size();
    while (ptr < lim)
      res = 31 * res + *ptr++;
    return res;
  }

protected:
  std::vector<unsigned char> values;
  size_t position;
  size_t bitShift;
};
typedef ReferenceCountedObjectPtr<BinaryKey> BinaryKeyPtr;

//// CACHE

class LuapeNodeCache : public Object
{
public:
  LuapeNodeCache(TypePtr elementsType);
  LuapeNodeCache() {}

  virtual String toShortString() const;
  
  /*
  ** Examples
  */
  void resizeSamples(bool isTrainingSamples, size_t size);
  void resizeSamples(size_t numTrainingSamples, size_t numValidationSamples);
  void setSample(bool isTrainingSample, size_t index, const Variable& value);
  void setSamples(bool isTrainingSamples, const VectorPtr& samples)
    {if (isTrainingSamples) trainingSamples = samples; else validationSamples = samples;}

  size_t getNumTrainingSamples() const
    {return trainingSamples ? trainingSamples->getNumElements() : 0;}

  size_t getNumValidationSamples() const
    {return validationSamples ? validationSamples->getNumElements() : 0;}

  size_t getNumSamples(bool isTrainingSamples) const
  {
    VectorPtr samples = getSamples(isTrainingSamples);
    return samples ? samples->getNumElements() : 0;
  }

  Variable getTrainingSample(size_t index) const
    {jassert(trainingSamples); return trainingSamples->getElement(index);}

  Variable getSample(bool isTrainingSample, size_t index) const
    {return (isTrainingSample ? trainingSamples : validationSamples)->getElement(index);}

  VectorPtr getSamples(bool isTrainingSample) const
    {return isTrainingSample ? trainingSamples : validationSamples;}

  const VectorPtr& getTrainingSamples() const
    {return trainingSamples;}

  const VectorPtr& getValidationSamples() const
    {return validationSamples;}

  void clearSamples(bool clearTrainingSamples = true, bool clearValidationSamples = true);

  BinaryKeyPtr makeKeyFromSamples(bool useTrainingSamples = true) const;

  /*
  ** Double values
  */
  bool isConvertibleToDouble() const
    {return elementsType->isConvertibleToDouble();}

  SparseDoubleVectorPtr getSortedDoubleValues();

  lbcpp_UseDebuggingNewOperator

protected:
  TypePtr elementsType;
  VectorPtr trainingSamples;
  VectorPtr validationSamples;
  SparseDoubleVectorPtr sortedDoubleValues;
};

typedef ReferenceCountedObjectPtr<LuapeNodeCache> LuapeNodeCachePtr;

//////  GRAPH NODES

class LuapeNode;
typedef ReferenceCountedObjectPtr<LuapeNode> LuapeNodePtr;

class LuapeNode : public NameableObject
{
public:
  LuapeNode(const TypePtr& type, const String& name);
  LuapeNode();

  const TypePtr& getType() const
    {return type;}

  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const = 0;
  virtual size_t getDepth() const = 0;

  const LuapeNodeCachePtr& getCache() const
    {return cache;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

  size_t getIndexInGraph() const
    {return indexInGraph;}

  size_t getAllocationIndex() const
    {return allocationIndex;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeGraph;
  friend class LuapeNodeClass;

  TypePtr type;
  LuapeNodeCachePtr cache;
  size_t indexInGraph;
  size_t allocationIndex;
};

extern ClassPtr luapeNodeClass;

class LuapeInputNode : public LuapeNode
{
public:
  LuapeInputNode(const TypePtr& type, const String& name, size_t inputIndex);
  LuapeInputNode() {}

  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const;
  virtual size_t getDepth() const
    {return 0;}

  virtual String toShortString() const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  lbcpp_UseDebuggingNewOperator

protected:
  size_t inputIndex;
};

typedef ReferenceCountedObjectPtr<LuapeInputNode> LuapeInputNodePtr;

class LuapeFunctionNode;
typedef ReferenceCountedObjectPtr<LuapeFunctionNode> LuapeFunctionNodePtr;

class LuapeFunctionNode : public LuapeNode
{
public:
  LuapeFunctionNode(const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& arguments);
  LuapeFunctionNode(const LuapeFunctionPtr& function, LuapeNodePtr argument);
  LuapeFunctionNode() {}

  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const;
  virtual size_t getDepth() const;

  virtual String toShortString() const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

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

class LuapeYieldNode : public LuapeNode
{
public:
  LuapeYieldNode(const LuapeNodePtr& argument);
  LuapeYieldNode();

  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const;
  virtual size_t getDepth() const;

  virtual String toShortString() const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

  const LuapeNodePtr& getArgument() const
    {return argument;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeYieldNodeClass;

  LuapeNodePtr argument;
};

typedef ReferenceCountedObjectPtr<LuapeYieldNode> LuapeYieldNodePtr;
extern ClassPtr luapeYieldNodeClass;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_H_

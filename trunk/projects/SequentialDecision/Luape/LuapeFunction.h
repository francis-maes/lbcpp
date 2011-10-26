/*-----------------------------------------.---------------------------------.
| Filename: LuapeFunction.h                | Lua-evolved function            |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2011 18:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_FUNCTION_H_
# define LBCPP_LUAPE_FUNCTION_H_

# include "LuapeGraph.h"

namespace lbcpp
{

class LuapeFunction : public Function
{
public:
  const LuapeGraphPtr& getGraph() const
    {return graph;}

  void setGraph(const LuapeGraphPtr& graph)
    {this->graph = graph;}

  const VectorPtr& getVotes() const
    {return votes;}

  void setVotes(const VectorPtr& votes)
    {this->votes = votes;}

protected:
  friend class LuapeFunctionClass;

  LuapeGraphPtr graph;
  VectorPtr votes;

  void computeGraph(ExecutionContext& context, const ObjectPtr& inputs, std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
  {
    size_t n;
    state.resize(graph->getNumNodes());
    if (inputs->getClass().isInstanceOf<DynamicClass>())
    {
      n = inputs->getNumVariables();
      jassert(n <= state.size());
      for (size_t i = 0; i < n; ++i)
        state[i] = inputs->getVariable(i);
    }
    else
    {
      ContainerPtr container = inputs.dynamicCast<Container>();
      jassert(container);
      n = container->getNumElements();
      jassert(n <= state.size());
      for (size_t i = 0; i < n; ++i)
        state[i] = container->getElement(i);
    }
    graph->compute(context, state, n, callback);
  }
};

typedef ReferenceCountedObjectPtr<LuapeFunction> LuapeFunctionPtr;
extern ClassPtr luapeFunctionClass;

class LuapeBinaryClassifier : public LuapeFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? booleanType : objectClass;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return booleanType;}

  struct ComputeCallback : public LuapeGraphCallback
  {
    ComputeCallback(VectorPtr votes) 
      : votes(votes), index(0), res(0.0) {}

    VectorPtr votes;
    size_t index;
    double res;

    virtual void valueYielded(const Variable& value)
      {res += votes->getElement(index++).getDouble() * (value.getBoolean() ? 1.0 : -1.0);}
  };

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    // supervision = inputs[1]

    ComputeCallback callback(votes);   
    std::vector<Variable> state;
    computeGraph(context, inputs[0].getObject(), state, &callback);
    return callback.res > 0;
  }

  // votes are scalars (alpha values)
};

extern ClassPtr luapeBinaryClassifierClass;

class LuapeClassifier : public LuapeFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? enumValueType : objectClass;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    EnumerationPtr enumeration = inputVariables[1]->getType().dynamicCast<Enumeration>();
    jassert(enumeration);
    doubleVectorClass = denseDoubleVectorClass(enumeration, doubleType);
    return enumeration;
  }

  struct ComputeCallback : public LuapeGraphCallback
  {
    ComputeCallback(VectorPtr votes, ClassPtr doubleVectorClass)
      : votes(votes), index(0), res(new DenseDoubleVector(doubleVectorClass)) {}

    VectorPtr votes;
    size_t index;
    DenseDoubleVectorPtr res;

    virtual void valueYielded(const Variable& value)
    {
      DenseDoubleVectorPtr vote = votes->getElement(index++).getObjectAndCast<DenseDoubleVector>();
      vote->addWeightedTo(res, 0, value.getBoolean() ? 1.0 : -1.0);
    }
  };

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    // supervision = inputs[1]
    ComputeCallback callback(votes, doubleVectorClass);   
    std::vector<Variable> state;
    computeGraph(context, inputs[0].getObject(), state, &callback);
    return Variable(callback.res->getIndexOfMaximumValue(), getOutputType());
  }

  // votes are vectors (alpha value . vote vector)

  EnumerationPtr getLabels() const
    {return getOutputType().staticCast<Enumeration>();}

  const ClassPtr& getDoubleVectorClass() const
    {return doubleVectorClass;}

protected:
  ClassPtr doubleVectorClass;
};

extern ClassPtr luapeClassifierClass;
typedef ReferenceCountedObjectPtr<LuapeClassifier> LuapeClassifierPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FUNCTION_H_

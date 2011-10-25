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

  void computeGraph(ExecutionContext& context, const ContainerPtr& inputs, std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const
  {
    state.resize(graph->getNumNodes());
    size_t n = inputs->getNumElements();
    jassert(n <= state.size());
    for (size_t i = 0; i < n; ++i)
      state[i] = inputs->getElement(i);
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
    {return index ? booleanType : containerClass();}

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
    ComputeCallback callback(votes);
    // supervision = inputs[1]
    ContainerPtr inputValues = inputs[0].getObjectAndCast<Container>();
    
    std::vector<Variable> state;
    computeGraph(context, inputValues, state, &callback);
    return callback.res > 0;
  }
};

extern ClassPtr luapeBinaryClassifierClass;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FUNCTION_H_

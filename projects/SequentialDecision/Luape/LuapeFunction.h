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

  virtual VectorPtr createVoteVector(size_t initialSize) const = 0;
  virtual void aggregateVote(Variable& targetVote, const Variable& vote, bool positive) const = 0;

  DenseDoubleVectorPtr computeSamplePredictions(ExecutionContext& context, bool isTrainingSamples) const
  {
    jassert(votes.isInstanceOf<DenseDoubleVector>());

    size_t yieldIndex = 0;
    size_t numSamples = graph->getNumSamples(isTrainingSamples);
    DenseDoubleVectorPtr predictions = new DenseDoubleVector(numSamples, 0.0);
    for (size_t i = 0; i < graph->getNumNodes(); ++i)
    {
      LuapeYieldNodePtr yieldNode = graph->getNode(i).dynamicCast<LuapeYieldNode>();
      if (yieldNode)
      {
        LuapeNodePtr weakPredictor = yieldNode->getArgument();
        weakPredictor->updateCache(context, isTrainingSamples);
        BooleanVectorPtr weakPredictions = weakPredictor->getCache()->getSamples(isTrainingSamples);
        double vote = votes.staticCast<DenseDoubleVector>()->getValue(yieldIndex++);
        for (size_t j = 0; j < numSamples; ++j)
          if (weakPredictions->get(j))
            predictions->incrementValue(j, vote);
      }
    }
    return predictions;
  }
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
    {return index ? booleanType : (TypePtr)objectClass;}

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

  virtual VectorPtr createVoteVector(size_t initialSize) const
    {return new DenseDoubleVector(initialSize, 0.0);}

  // targetVote += vote * (positive ? 1 : -1)
  virtual void aggregateVote(Variable& targetVote, const Variable& vote, bool positive) const
    {targetVote = vote.getDouble() * (positive ? 1 : -1.0);}

  // votes are scalars (alpha values)
};

extern ClassPtr luapeBinaryClassifierClass;

class LuapeClassifier : public LuapeFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? enumValueType : (TypePtr)objectClass;}

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

  virtual VectorPtr createVoteVector(size_t initialSize) const
  {
    ObjectVectorPtr res = new ObjectVector(doubleVectorClass, initialSize);
    for (size_t i = 0; i < initialSize; ++i)
      res->set(i, new DenseDoubleVector(doubleVectorClass));
    return res;
  }

  // targetVote += vote * (positive ? 1 : -1)
  virtual void aggregateVote(Variable& targetVote, const Variable& vote, bool positive) const
    {vote.getObjectAndCast<DenseDoubleVector>()->addWeightedTo(targetVote.getObjectAndCast<DenseDoubleVector>(), 0, positive ? 1.0 : -1.0);}

  const EnumerationPtr& getLabels() const
    {return getOutputType().staticCast<Enumeration>();}

  const ClassPtr& getDoubleVectorClass() const
    {return doubleVectorClass;}

protected:
  ClassPtr doubleVectorClass;
};

extern ClassPtr luapeClassifierClass;
typedef ReferenceCountedObjectPtr<LuapeClassifier> LuapeClassifierPtr;

class LuapeRanker : public LuapeFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleType) : vectorClass();}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, doubleType);}

  struct ComputeCallback : public LuapeGraphCallback
  {
    ComputeCallback(DenseDoubleVectorPtr votes) 
      : votes(votes), index(0), res(0.0) {}

    DenseDoubleVectorPtr votes;
    size_t index;
    double res;

    virtual void valueYielded(const Variable& value)
    {
      if (value.getBoolean())
        res += votes->getValue(index++);
    }
  };

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    // supervision = inputs[1]
    ObjectVectorPtr alternatives = inputs[0].getObjectAndCast<ObjectVector>();
    size_t n = alternatives->getNumElements();
    DenseDoubleVectorPtr scores = new DenseDoubleVector(n, 0.0);
    for (size_t i = 0; i < n; ++i)
    {
      ComputeCallback callback(votes);   
      std::vector<Variable> state;
      computeGraph(context, alternatives->get(i), state, &callback);
      scores->setValue(i, callback.res);
    }
    return scores;
  }

  // votes are scalars (alpha values)
  virtual VectorPtr createVoteVector(size_t initialSize) const
    {return new DenseDoubleVector(initialSize, 0.0);}

  // targetVote += vote * (positive ? 1 : -1)
  virtual void aggregateVote(Variable& targetVote, const Variable& vote, bool positive) const
    {targetVote = vote.getDouble() * (positive ? 1 : -1.0);}
};

typedef ReferenceCountedObjectPtr<LuapeRanker> LuapeRankerPtr;


class LuapeRegressor : public LuapeFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? doubleType : objectClass;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}

  struct ComputeCallback : public LuapeGraphCallback
  {
    ComputeCallback(DenseDoubleVectorPtr votes) 
      : votes(votes), index(0), res(0.0) {}

    DenseDoubleVectorPtr votes;
    size_t index;
    double res;

    virtual void valueYielded(const Variable& value)
    {
      if (value.getBoolean())
        res += votes->getValue(index++);
    }
  };

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    ComputeCallback callback(votes);   
    std::vector<Variable> state;
    computeGraph(context, inputs[0].getObject(), state, &callback);
    return callback.res;
  }

  // votes are scalars (alpha values)
  virtual VectorPtr createVoteVector(size_t initialSize) const
    {return new DenseDoubleVector(initialSize, 0.0);}

  // targetVote += vote * (positive ? 1 : -1)
  virtual void aggregateVote(Variable& targetVote, const Variable& vote, bool positive) const
    {targetVote = vote.getDouble() * (positive ? 1 : -1.0);}
};

typedef ReferenceCountedObjectPtr<LuapeRegressor> LuapeRegressorPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FUNCTION_H_

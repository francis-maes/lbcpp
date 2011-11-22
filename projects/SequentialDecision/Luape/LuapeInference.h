/*-----------------------------------------.---------------------------------.
| Filename: LuapeInference.h               | Lua-evolved function            |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2011 18:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_INFERENCE_H_
# define LBCPP_LUAPE_INFERENCE_H_

# include "LuapeGraph.h"

namespace lbcpp
{

class LuapeInference : public Function
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

  virtual VectorPtr createVoteVector() const = 0;
  virtual void updatePredictions(VectorPtr predictions, size_t yieldIndex, const BooleanVectorPtr& yieldOutputs) const = 0;

  virtual TypePtr getPredictionsInternalType() const
    {return getOutputType();}

  VectorPtr makeCachedPredictions(ExecutionContext& context, bool isTrainingSamples) const
  {
    size_t numSamples = graph->getNumSamples(isTrainingSamples);
    VectorPtr predictions = vector(getPredictionsInternalType(), numSamples);

    size_t yieldIndex = 0;
    for (size_t i = 0; i < graph->getNumNodes(); ++i)
    {
      LuapeYieldNodePtr yieldNode = graph->getNode(i).dynamicCast<LuapeYieldNode>();
      if (yieldNode)
      {
        LuapeNodePtr weakNode = yieldNode->getArgument();
        weakNode->updateCache(context, isTrainingSamples);
        updatePredictions(predictions, yieldIndex, weakNode->getCache()->getSamples(isTrainingSamples));
      }
    }
    return predictions;
  }

  virtual void setGraphSamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
  {
    size_t n = data.size();
    graph->resizeSamples(isTrainingData, n);
    for (size_t i = 0; i < n; ++i)
    {
      const PairPtr& example = data[i].staticCast<Pair>();
      graph->setSample(isTrainingData, i, example->getFirst().getObject());
    }
  }

  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const = 0;

protected:
  friend class LuapeInferenceClass;

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

typedef ReferenceCountedObjectPtr<LuapeInference> LuapeInferencePtr;
extern ClassPtr luapeInferenceClass;

class LuapeBinaryClassifier : public LuapeInference
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

  virtual VectorPtr createVoteVector() const
    {return new DenseDoubleVector(0, 0.0);}

  virtual void updatePredictions(VectorPtr predictions, size_t yieldIndex, const BooleanVectorPtr& yieldOutputs) const
  {
    double vote = votes.staticCast<DenseDoubleVector>()->getValue(yieldIndex);
    const DenseDoubleVectorPtr& pred = predictions.staticCast<DenseDoubleVector>();
    size_t n = pred->getNumValues();
    std::vector<bool>::const_iterator it = yieldOutputs->getElements().begin();
    for (size_t i = 0; i < n; ++i)
      pred->incrementValue(i, vote * (*it++ ? 1.0 : -1.0));
  }

  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const
  {
    jassert(false); // not yet implemented
    return 0.0;
  }

};

extern ClassPtr luapeBinaryClassifierClass;

class LuapeClassifier : public LuapeInference
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
      jassert(value.isBoolean());
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

  virtual VectorPtr createVoteVector() const
    {return new ObjectVector(doubleVectorClass, 0);}

  virtual TypePtr getPredictionsInternalType() const
    {return doubleVectorClass;}

  virtual void updatePredictions(VectorPtr predictions, size_t yieldIndex, const BooleanVectorPtr& yieldOutputs) const
  {
    DenseDoubleVectorPtr vote = votes->getElement(yieldIndex).getObjectAndCast<DenseDoubleVector>();
    const ObjectVectorPtr& pred = predictions.staticCast<ObjectVector>();
    std::vector<bool>::const_iterator it = yieldOutputs->getElements().begin();
    size_t n = pred->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      DenseDoubleVectorPtr p = pred->getAndCast<DenseDoubleVector>(i);
      if (!p)
      {
        p = new DenseDoubleVector(doubleVectorClass);
        pred->set(i, p);
      }
      vote->addWeightedTo(p, 0, *it++ ? 1.0 : -1.0);
    }
  }

  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const
  {
    ObjectVectorPtr pred = predictions.staticCast<ObjectVector>();
    size_t numErrors = 0;
    size_t n = pred->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      size_t j = pred->getAndCast<DenseDoubleVector>(i)->getIndexOfMaximumValue();
      if (j != (size_t)data[i].staticCast<Pair>()->getSecond().getInteger())
        ++numErrors;
    }
    return numErrors / (double)n;
  }

  const EnumerationPtr& getLabels() const
    {return getOutputType().staticCast<Enumeration>();}

  const ClassPtr& getDoubleVectorClass() const
    {return doubleVectorClass;}

protected:
  ClassPtr doubleVectorClass;
};

extern ClassPtr luapeClassifierClass;
typedef ReferenceCountedObjectPtr<LuapeClassifier> LuapeClassifierPtr;

class LuapeRanker : public LuapeInference
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
      jassert(value.isBoolean());
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
  virtual VectorPtr createVoteVector() const
    {return new DenseDoubleVector(0, 0.0);}

  virtual void updatePredictions(VectorPtr predictions, size_t yieldIndex, const BooleanVectorPtr& yieldOutputs) const
  {
    jassert(false); // not yet implemented
  }

  virtual void setGraphSamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
  {
    graph->clearSamples(isTrainingData, !isTrainingData);
    for (size_t i = 0; i < data.size(); ++i)
    {
      const PairPtr& rankingExample = data[i].staticCast<Pair>();
      const ContainerPtr& alternatives = rankingExample->getFirst().getObjectAndCast<Container>();
      size_t n = alternatives->getNumElements();

      LuapeNodeCachePtr inputNodeCache = graph->getNode(0)->getCache();
      size_t firstIndex = inputNodeCache->getNumSamples(isTrainingData);
      inputNodeCache->resizeSamples(isTrainingData, firstIndex + n);
      for (size_t i = 0; i < n; ++i)
        inputNodeCache->setSample(true, firstIndex + 1, alternatives->getElement(i));
    }
  }

  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const
  {
    jassert(false); // not yet implemented
    return 0.0;
  }
};

typedef ReferenceCountedObjectPtr<LuapeRanker> LuapeRankerPtr;

class LuapeRegressor : public LuapeInference
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? doubleType : (TypePtr)objectClass;}

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
      jassert(value.isBoolean());
      res += votes->getValue(value.getBoolean() ? index + 1 : index);
    }
  };

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    ComputeCallback callback(votes);   
    std::vector<Variable> state;
    computeGraph(context, inputs[0].getObject(), state, &callback);
    return callback.res;
  }

  virtual VectorPtr createVoteVector() const
    {return new DenseDoubleVector(0, 0.0);}

  virtual void updatePredictions(VectorPtr predictions, size_t yieldIndex, const BooleanVectorPtr& yieldOutputs) const
  {
    double negativeVote = votes.staticCast<DenseDoubleVector>()->getValue(yieldIndex * 2);
    double positiveVote = votes.staticCast<DenseDoubleVector>()->getValue(yieldIndex * 2 + 1);
    const DenseDoubleVectorPtr& pred = predictions.staticCast<DenseDoubleVector>();
    size_t n = pred->getNumValues();
    std::vector<bool>::const_iterator it = yieldOutputs->getElements().begin();
    for (size_t i = 0; i < n; ++i)
      pred->incrementValue(i, *it++ ? positiveVote : negativeVote);
  }

  virtual void setGraphSamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
  {
    graph->clearSamples(isTrainingData, !isTrainingData);
    LuapeNodeCachePtr inputNodeCache = graph->getNode(0)->getCache();
    inputNodeCache->resizeSamples(isTrainingData, data.size());
    DenseDoubleVectorPtr supervisions = new DenseDoubleVector(data.size(), 0.0);
    for (size_t i = 0; i < data.size(); ++i)
    {
      const PairPtr& example = data[i].staticCast<Pair>();
      inputNodeCache->setSample(isTrainingData, i, example->getFirst());
      supervisions->setValue(i, example->getSecond().getDouble());
    }
  }

  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const
  {
    const DenseDoubleVectorPtr& pred = predictions.staticCast<DenseDoubleVector>();
    size_t n = pred->getNumValues();
    jassert(n == data.size());
    double res = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
      double delta = pred->getValue(i) - data[i].staticCast<Pair>()->getSecond().getDouble();
      res += delta * delta;
    }
    return sqrt(res / (double)n);
  }
};

typedef ReferenceCountedObjectPtr<LuapeRegressor> LuapeRegressorPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_INFERENCE_H_

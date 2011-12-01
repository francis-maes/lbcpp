/*-----------------------------------------.---------------------------------.
| Filename: LuapeInference.h               | Lua-evolved function            |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2011 18:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_INFERENCE_H_
# define LBCPP_LUAPE_INFERENCE_H_

# include "LuapeNode.h"
# include "LuapeGraph.h" // for LuapeGraphUniverse

namespace lbcpp
{
/*
class LuapeWeakPredictionVector : public Vector
{
public:
  LuapeWeakPredictionVector(size_t count)
    : weakPredictions(count, std::pair<int, double>(-1, 0.0)) {}

  virtual TypePtr getElementsType() const
    {return pairClass(integerType, doubleType);}

  virtual size_t getNumElements() const
    {return weakPredictions.size();}

  virtual Variable getElement(size_t index) const
    {return new Pair(weakPredictions[index].first, weakPredictions[index].second);}

  virtual void setElement(size_t index, const Variable& value)
    {jassert(false);}
  
  virtual void clear()
    {weakPredictions.clear();}

  virtual void reserve(size_t size)
    {weakPredictions.reserve(size);}

  virtual void resize(size_t size)
    {weakPredictions.resize(size, std::pair<int, double>(-1, 0.0));}

  virtual void prepend(const Variable& value)
    {jassert(false);}

  virtual void append(const Variable& value)
    {jassert(false);}

  virtual void remove(size_t index)
    {jassert(false);}

  void yield(size_t exampleIndex, size_t yieldIndex, double value)
  {
    jassert(weakPredictions[exampleIndex].first < 0);
    weakPredictions[exampleIndex] = std::make_pair(yieldIndex, value);
  }

protected:
  std::vector< std::pair<int, double> > weakPredictions;
};

typedef ReferenceCountedObjectPtr<LuapeWeakPredictionVector> LuapeWeakPredictionVectorPtr;
*/
class LuapeInference : public Function
{
public:
  LuapeInference();

  const LuapeGraphUniversePtr& getUniverse() const
    {return universe;}

  /*
  ** Inputs
  */
  size_t getNumInputs() const
    {return inputs.size();}

  const LuapeInputNodePtr& getInput(size_t index) const
    {jassert(index < inputs.size()); return inputs[index];}
  
  void addInput(const TypePtr& type, const String& name)
    {inputs.push_back(new LuapeInputNode(type, name));}

  /*
  ** Available Functions
  */
  size_t getNumFunctions() const
    {return functions.size();}

  const LuapeFunctionPtr& getFunction(size_t index) const
    {jassert(index < functions.size()); return functions[index];}

  void addFunction(const LuapeFunctionPtr& function)
    {functions.push_back(function);}

  /*
  ** Luape Node 
  */
  const LuapeNodePtr& getRootNode() const
    {return node;}

  // new 
  virtual LuapeSamplesCachePtr createSamplesCache(ExecutionContext& context, const std::vector<ObjectPtr>& data) const;
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const = 0;

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;


/*  virtual VectorPtr createVoteVector() const = 0;
  virtual void updatePredictions(VectorPtr predictions, size_t yieldIndex, const VectorPtr& yieldOutputs) const = 0;

  virtual TypePtr getPredictionsInternalType() const
    {return getOutputType();}
*/
//  virtual void setCachedSamples(ExecutionContext& context, bool isTrainingSamples, const std::vector<ObjectPtr>& data);
//  void setCachedSample(ExecutionContext& context, bool isTrainingSample, size_t index, const ObjectPtr& example);

  //VectorPtr makeCachedPredictions(ExecutionContext& context, bool isTrainingSamples) const;
  //DenseDoubleVectorPtr computeSignedWeakPredictions(ExecutionContext& context, const ObjectPtr& input) const;

protected:
  friend class LuapeInferenceClass;

  LuapeGraphUniversePtr universe;
  std::vector<LuapeInputNodePtr> inputs;
  std::vector<LuapeFunctionPtr> functions;
  LuapeNodePtr node;

  Variable computeNode(ExecutionContext& context, const ObjectPtr& inputs) const;
};

typedef ReferenceCountedObjectPtr<LuapeInference> LuapeInferencePtr;
extern ClassPtr luapeInferenceClass;

// votes = DenseDoubleVector whose size is 2 x numYields (neg,pos,neg,pos ...)
class LuapeRegressor : public LuapeInference
{
public:
  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);

  /*virtual VectorPtr createVoteVector() const;
  virtual void updatePredictions(VectorPtr predictions, size_t yieldIndex, const VectorPtr& yieldOutputs) const;*/
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const;
};

typedef ReferenceCountedObjectPtr<LuapeRegressor> LuapeRegressorPtr;

// votes = DenseDoubleVector of size numYields (symmetric weak predictor)
class LuapeBinaryClassifier : public LuapeInference
{
public:
  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
/*  virtual VectorPtr createVoteVector() const;
  virtual void updatePredictions(VectorPtr predictions, size_t yieldIndex, const VectorPtr& yieldOutputs) const;*/
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const;
};

extern ClassPtr luapeBinaryClassifierClass;

// votes = ObjectVector of size numYields, containing DenseDoubleVector of size numLabels (symmetric weak predictor)
class LuapeClassifier : public LuapeInference
{
public:
  const EnumerationPtr& getLabels() const
    {return getOutputType().staticCast<Enumeration>();}

  const ClassPtr& getDoubleVectorClass() const
    {return doubleVectorClass;}

  DenseDoubleVectorPtr computeActivations(ExecutionContext& context, const ObjectPtr& input) const;

  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
/*  virtual VectorPtr createVoteVector() const;
  virtual TypePtr getPredictionsInternalType() const;
  virtual void updatePredictions(VectorPtr predictions, size_t yieldIndex, const VectorPtr& yieldOutputs) const;*/
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const;

protected:
  ClassPtr doubleVectorClass;
};

extern ClassPtr luapeClassifierClass;
typedef ReferenceCountedObjectPtr<LuapeClassifier> LuapeClassifierPtr;

// votes = DenseDoubleVector whose size is numYields (symmetric weak learners)
class LuapeRanker : public LuapeInference
{
public:
  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

  virtual LuapeSamplesCachePtr createSamplesCache(ExecutionContext& context, const std::vector<ObjectPtr>& data) const;
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const;

  // votes are scalars (alpha values)
/*  virtual VectorPtr createVoteVector() const;
  virtual void updatePredictions(VectorPtr predictions, size_t yieldIndex, const VectorPtr& yieldOutputs) const;
  */
};

typedef ReferenceCountedObjectPtr<LuapeRanker> LuapeRankerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_INFERENCE_H_

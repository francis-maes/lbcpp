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
# include "LuapeUniverse.h"
# include "LuapeCache.h"

namespace lbcpp
{

class LuapeInference : public Object
{
public:
  LuapeInference(LuapeUniversePtr universe = LuapeUniversePtr());

  const LuapeUniversePtr& getUniverse() const
    {return universe;}

  /*
  ** Inputs
  */
  size_t getNumInputs() const
    {return inputs.size();}

  const LuapeInputNodePtr& getInput(size_t index) const
    {jassert(index < inputs.size()); return inputs[index];}
  
  const std::vector<LuapeInputNodePtr>& getInputs() const
    {return inputs;}

  void addInput(const TypePtr& type, const String& name)
    {size_t index = inputs.size(); inputs.push_back(new LuapeInputNode(type, name, index));}

  /*
  ** Active variables
  */
  size_t getNumActiveVariables() const
    {return activeVariables.size();}

  LuapeNodePtr getActiveVariable(size_t index) const;

  const std::set<LuapeNodePtr>& getActiveVariables() const
    {return activeVariables;}

  void addActiveVariable(const LuapeNodePtr& node)
    {activeVariables.insert(node);}

  void clearActiveVariables()
    {activeVariables.clear();}

  /*
  ** Supervision variable
  */
  LuapeInputNodePtr getSupervision() const
    {return supervision;}

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

  const LuapeConstantNodePtr& getConstant(size_t index) const
    {jassert(index < constants.size()); return constants[index];}

  void addConstant(const Variable& value)
    {constants.push_back(new LuapeConstantNode(value));}

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
  LuapeGraphBuilderTypeSearchSpacePtr getSearchSpace(ExecutionContext& context, size_t complexity, bool verbose = false) const; // cached with initialState = vector<TypePtr>()

  LuapeGraphBuilderTypeSearchSpacePtr createTypeSearchSpace(ExecutionContext& context, const std::vector<TypePtr>& initialState, size_t complexity, bool verbose) const;
  void enumerateNodesExhaustively(ExecutionContext& context, size_t complexity, std::vector<LuapeNodePtr>& res, bool verbose = false, const LuapeRPNSequencePtr& subSequence = LuapeRPNSequencePtr()) const;

  /*
  ** Luape Node 
  */
  const LuapeNodePtr& getRootNode() const
    {return node;}

  void setRootNode(ExecutionContext& context, const LuapeNodePtr& node);
  void clearRootNode(ExecutionContext& context);

  /*
  ** Compute
  */
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const
    {jassert(false); return 0.0;}

  /*
  ** Learner
  */
  void setLearner(const LuapeLearnerPtr& learner, bool verbose = false);


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

protected:
  friend class LuapeInferenceClass;

  LuapeUniversePtr universe;
  std::vector<LuapeInputNodePtr> inputs;
  LuapeInputNodePtr supervision;
  std::vector<LuapeConstantNodePtr> constants;
  std::vector<FunctionPtr> functions;
  std::set<TypePtr> targetTypes;
  std::set<LuapeNodePtr> activeVariables;
  LuapeNodePtr node;
  LuapeSamplesCachePtr trainingCache;
  LuapeSamplesCachePtr validationCache;

  CriticalSection typeSearchSpacesLock;
  std::vector<LuapeGraphBuilderTypeSearchSpacePtr> typeSearchSpaces;

  Variable computeNode(ExecutionContext& context, const ObjectPtr& inputs) const;

  LuapeSamplesCachePtr createCache(size_t size, size_t maxCacheSizeInMb) const;
  LuapeSamplesCachePtr createSamplesCache(ExecutionContext& context, const std::vector<ObjectPtr>& data) const;
};

typedef ReferenceCountedObjectPtr<LuapeInference> LuapeInferencePtr;
extern ClassPtr luapeInferenceClass;

// votes = DenseDoubleVector whose size is 2 x numYields (neg,pos,neg,pos ...)
class LuapeRegressor : public LuapeInference
{
public:
  LuapeRegressor(LuapeUniversePtr universe = LuapeUniversePtr())
    : LuapeInference(universe) {}

  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const;
};

typedef ReferenceCountedObjectPtr<LuapeRegressor> LuapeRegressorPtr;
extern ClassPtr luapeRegressorClass;

// votes = DenseDoubleVector of size numYields (symmetric weak predictor)
class LuapeBinaryClassifier : public LuapeInference
{
public:
  LuapeBinaryClassifier(LuapeUniversePtr universe = LuapeUniversePtr())
    : LuapeInference(universe) {}

  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const;
};

extern ClassPtr luapeBinaryClassifierClass;
typedef ReferenceCountedObjectPtr<LuapeBinaryClassifier> LuapeBinaryClassifierPtr;

// votes = ObjectVector of size numYields, containing DenseDoubleVector of size numLabels (symmetric weak predictor)
class LuapeClassifier : public LuapeInference
{
public:
  LuapeClassifier(LuapeUniversePtr universe = LuapeUniversePtr())
    : LuapeInference(universe) {}

  static EnumerationPtr getLabelsFromSupervision(TypePtr supervisionType);

  const EnumerationPtr& getLabels() const
    {return labels;}

  const ClassPtr& getDoubleVectorClass() const
    {return doubleVectorClass;}

  DenseDoubleVectorPtr computeActivations(ExecutionContext& context, const ObjectPtr& input) const;

  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const;

protected:
  ClassPtr doubleVectorClass;
  EnumerationPtr labels;
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

  virtual void setSamples(ExecutionContext& context, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData);
  virtual double evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const;

  const std::vector<size_t>& getTrainingExampleSizes() const
    {return trainingExampleSizes;}

  const std::vector<size_t>& getValidationExampleSizes() const
    {return trainingExampleSizes;}

protected:
  std::vector<size_t> trainingExampleSizes;
  std::vector<size_t> validationExampleSizes;

  LuapeSamplesCachePtr createSamplesCache(ExecutionContext& context, const std::vector<ObjectPtr>& data, std::vector<size_t>& exampleSizes) const;
};

typedef ReferenceCountedObjectPtr<LuapeRanker> LuapeRankerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_INFERENCE_H_

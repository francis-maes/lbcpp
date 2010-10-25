/*-----------------------------------------.---------------------------------.
| Filename: DisorderRegionsInference.h     | Disorder Regions Inference      |
| Author  : Francis Maes                   |                                 |
| Started : 25/10/2010 18:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_INFERENCE_DISORDER_REGIONS_H_
# define LBCPP_PROTEINS_INFERENCE_DISORDER_REGIONS_H_

# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Inference/SequentialInference.h>
# include "../Data/Protein.h"

namespace lbcpp
{

class RankingBasedExtractionInference : public VectorSequentialInference
{
public:
  RankingBasedExtractionInference(const String& name, InferencePtr rankingInference, InferencePtr cutoffInference)
    : VectorSequentialInference(name)
  {
    appendInference(rankingInference);
    appendInference(cutoffInference);
  }
  RankingBasedExtractionInference() {}

  virtual TypePtr getInputType() const
    {return subInferences[0]->getInputType();}

  virtual TypePtr getSupervisionType() const
    {return subInferences[0]->getSupervisionType();}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return subInferences[0]->getOutputType(inputType);}

  virtual ContainerPtr createRankingInputs(const Variable& input) const = 0;
  virtual ContainerPtr createRankingCosts(const Variable& supervision) const = 0;
  virtual Variable createCutoffInput(const Variable& input) const
    {return input;}
  virtual Variable computeBestCutoff(const ContainerPtr& scores, const ContainerPtr& costs) const = 0;

  virtual Variable computeOutput(const ContainerPtr& scores, double cutoff) const = 0;

  struct State : public SequentialInferenceState
  {
    State(const Variable& input, const Variable& supervision)
      : SequentialInferenceState(input, supervision) {}

    ContainerPtr scores;
    Variable bestCutoff;
  };

  typedef ReferenceCountedObjectPtr<State> StatePtr;

  virtual SequentialInferenceStatePtr prepareInference(const InferenceContextPtr& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    SequentialInferenceStatePtr state = new State(input, supervision);
    state->setSubInference(subInferences[0], createRankingInputs(input), supervision.exists() ? createRankingCosts(supervision) : ContainerPtr());
    return state;
  }

  virtual void prepareSubInference(InferenceContextPtr context, SequentialInferenceStatePtr s, size_t index, ReturnCode& returnCode)
  {
    jassert(index == 1);
    const StatePtr& state = s.staticCast<State>();
    state->scores = state->getSubOutput().getObjectAndCast<Container>();

    Variable supervision;
    if (state->scores && state->getSubSupervision().exists())
      supervision = state->bestCutoff = computeBestCutoff(state->scores, state->getSubSupervision().getObjectAndCast<Container>());
    state->setSubInference(subInferences[1], createCutoffInput(state->getInput()), supervision);
  }

  virtual Variable finalizeInference(const InferenceContextPtr& context, SequentialInferenceStatePtr finalState, ReturnCode& returnCode)
  {
    const StatePtr& state = finalState.staticCast<State>();
    if (!state->scores)
      return Variable();
    Variable predictedCutoff = state->getSubOutput();

    /* Use optimistic cutoff
    predictedCutoff = state->bestCutoff.exists() ? state->bestCutoff : Variable(0.0);
    */

    // tmp
    double cutoff = 0.0;//predictedCutoff.exists() ? predictedCutoff.getDouble() : 0.0;
    return computeOutput(state->scores, cutoff);
  }
};

class DisorderedRegionInference : public RankingBasedExtractionInference
{
public:
  DisorderedRegionInference(const String& name, InferencePtr rankingInference, InferencePtr cutoffInference)
    : RankingBasedExtractionInference(name, rankingInference, cutoffInference)
  {
    checkInheritance(rankingInference->getInputType(), containerClass(pairClass(proteinClass, positiveIntegerType)));
    checkInheritance(cutoffInference->getInputType(), proteinClass);
  }
  DisorderedRegionInference() {}

  virtual TypePtr getInputType() const
    {return proteinClass;}

  virtual TypePtr getSupervisionType() const
    {return containerClass(probabilityType);}

  virtual TypePtr getOutputType() const
    {return containerClass(probabilityType);}

  virtual ContainerPtr createRankingInputs(const Variable& input) const
  {
    const ProteinPtr& protein = input.getObjectAndCast<Protein>();
    size_t n = protein->getLength();

    TypePtr elementsType = pairClass(proteinClass, positiveIntegerType);
    ContainerPtr res = objectVector(elementsType, n + 1);
    for (size_t i = 0; i < n; ++i)
      res->setElement(i, Variable::pair(input, i, elementsType));
    res->setElement(n, Variable::missingValue(elementsType));
    return res;
  }

  virtual ContainerPtr createRankingCosts(const Variable& sup) const
  {
    const ContainerPtr& supervision = sup.getObjectAndCast<Container>();
    size_t n = supervision->getNumElements();
    ContainerPtr res = vector(doubleType, n + 1);
    for (size_t i = 0; i < n; ++i)
      res->setElement(i, supervision->getElement(i).getDouble() < 0.5 ? 1.0 : -1.0);
    res->setElement(n, 0.99);
    return res;
  }

  virtual Variable createCutoffInput(const Variable& input) const
    {return input;}

  virtual Variable computeBestCutoff(const ContainerPtr& scores, const ContainerPtr& costs) const
  {
    ROCAnalyse roc;
    size_t n = scores->getNumElements();
    if (!n)
      return Variable();

    jassert(n == costs->getNumElements());
    for (size_t i = 0; i < n; ++i)
      roc.addPrediction(scores->getElement(i).getDouble(), costs->getElement(i).getDouble() == 0.0);
    double bestMcc;
    double res = roc.findBestThreshold(&BinaryClassificationConfusionMatrix::computeMatthewsCorrelation, bestMcc);
    //MessageCallback::info(T("computeBestCutoff: ") + String(res) + T(" (MCC = ") + String(bestMcc) + T(")"));
    return res;
  }

  virtual Variable computeOutput(const ContainerPtr& scores, double cutoff) const
  {
    size_t n = scores->getNumElements() - 1;
    VectorPtr res = vector(probabilityType, n);
    for (size_t i = 0; i < n; ++i)
    {
      static const double temperature = 1.0;
      double score = scores->getElement(i).getDouble() - cutoff;
      double probability = 1.0 / (1.0 + exp(-score * temperature));
      res->setElement(i, Variable(probability, probabilityType));
    }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_INFERENCE_DISORDER_REGIONS_H_

/*-----------------------------------------.---------------------------------.
| Filename: DisulfideBondsInference.h      | Disulfide Bonds Inference       |
| Author  : Francis Maes                   |                                 |
| Started : 31/10/2010 21:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_DISULFIDE_BONDS_H_
# define LBCPP_PROTEIN_INFERENCE_DISULFIDE_BONDS_H_

# include <lbcpp/Data/SymmetricMatrix.h>
# include <lbcpp/Inference/ParallelInference.h>
# include "../Data/Protein.h"

namespace lbcpp
{

class SymmetricProbabilityMatrixInference : public SharedParallelInference
{
public:
  SymmetricProbabilityMatrixInference(const String& name, InferencePtr probabilityInference)
    : SharedParallelInference(name, probabilityInference) {}
  SymmetricProbabilityMatrixInference() {}
  
  virtual TypePtr getSupervisionType() const
    {return symmetricMatrixClass(probabilityType);}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return symmetricMatrixClass(probabilityType);}

  virtual Variable finalizeInference(const InferenceContextPtr& context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    const ProteinPtr& inputProtein = state->getInput().getObjectAndCast<Protein>();    
    size_t n = state.staticCast<State>()->dimension;
    size_t minDistance = state.staticCast<State>()->minDistance;

    SymmetricMatrixPtr res = new SymmetricMatrix(probabilityType, n);
    bool atLeastOnePrediction = false;
    size_t index = 0;
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + minDistance; j < n; ++j)
      {
        Variable result = state->getSubOutput(index++);
        if (result.exists())
        {
          atLeastOnePrediction = true;
          res->setElement(i, j, result);
        }
      }

    return atLeastOnePrediction ? res : Variable::missingValue(res->getClass());
  }

protected:
  struct State : public ParallelInferenceState
  {
    State(const Variable& input, const Variable& supervision, size_t dimension, size_t minDistance)
      : ParallelInferenceState(input, supervision), dimension(dimension), minDistance(minDistance)
    {
      reserve(dimension * (dimension - minDistance) / 2);
    }

    size_t dimension;
    size_t minDistance;
  };

  typedef ReferenceCountedObjectPtr<State> StatePtr;
};

class DisulfideBondsInference : public SymmetricProbabilityMatrixInference
{
public:
  DisulfideBondsInference(const String& name, InferencePtr contactInference)
    : SymmetricProbabilityMatrixInference(name, contactInference) {}
  DisulfideBondsInference() {}

  virtual TypePtr getInputType() const
    {return proteinClass;}

  virtual ParallelInferenceStatePtr prepareInference(const InferenceContextPtr& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    const ProteinPtr& inputProtein = input.getObjectAndCast<Protein>();
    const SymmetricMatrixPtr& supervisionMap = supervision.getObjectAndCast<SymmetricMatrix>();
    jassert(inputProtein && (!supervision.exists() || supervisionMap));

    std::vector<size_t> cysteines;
    inputProtein->getCysteineIndices(cysteines);
    size_t n = cysteines.size();

    const int minDistance = 1;
    StatePtr res = new State(input, supervision, n, minDistance);
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + minDistance; j < n; ++j)
      {
        Variable elementSupervision;
        if (supervisionMap)
          elementSupervision = supervisionMap->getElement(i, j);
        res->addSubInference(subInference, Variable::pair(input, Variable::pair(cysteines[i], cysteines[j])), elementSupervision);
      }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_DISULFIDE_BONDS_H_

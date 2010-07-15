/*-----------------------------------------.---------------------------------.
| Filename: ContactMapInference.h          | Contact Map Inference           |
| Author  : Francis Maes                   |                                 |
| Started : 15/07/2010 16:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_H_
# define LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_H_

# include <lbcpp/Data/Perception.h>
# include <lbcpp/Data/SymmetricMatrix.h>
# include <lbcpp/Inference/ParallelInference.h>
# include "../Data/Protein.h"

namespace lbcpp
{

class ContactMapInference : public SharedParallelInference
{
public:
  ContactMapInference(const String& name, PerceptionPtr perception, InferencePtr contactInference)
    : SharedParallelInference(name, contactInference), perception(perception) {}
  ContactMapInference() {}

  virtual TypePtr getInputType() const
    {return proteinClass();}

  virtual TypePtr getSupervisionType() const
    {return symmetricMatrixClass(probabilityType());}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return symmetricMatrixClass(probabilityType());}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ProteinPtr inputProtein = input.getObjectAndCast<Protein>();
    SymmetricMatrixPtr supervisionMap = supervision.getObjectAndCast<SymmetricMatrix>();
    jassert(inputProtein && (!supervision || supervisionMap));

    size_t n = inputProtein->getLength();

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve((n * (n - 6)) / 2);
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + 6; j < n; ++j)
      {
        Variable elementSupervision;
        if (supervisionMap)
          elementSupervision = supervisionMap->getElement(i, j);
        res->addSubInference(subInference, perception->compute(Variable::pair(input, Variable::pair(i, j))), elementSupervision);
      }
    return res;
  }

  virtual Variable finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    ProteinPtr inputProtein = state->getInput().getObjectAndCast<Protein>();    
    size_t n = inputProtein->getLength();

    SymmetricMatrixPtr res = new SymmetricMatrix(probabilityType(), n);
    bool atLeastOnePrediction = false;
    size_t index = 0;
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + 6; j < n; ++j)
      {
        Variable result = state->getSubOutput(index++);
        if (result)
        {
          atLeastOnePrediction = true;
          res->setElement(i, j, result);
        }
      }

    return atLeastOnePrediction ? res : Variable::missingValue(res->getClass());
  }

protected:
  PerceptionPtr perception;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_H_
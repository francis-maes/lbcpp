/*-----------------------------------------.---------------------------------.
| Filename: ProteinInference.h             | Protein Top Level Inferences    |
| Author  : Francis Maes                   |                                 |
| Started : 14/07/2010 01:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_H_
# define LBCPP_PROTEIN_INFERENCE_H_

# include "../Data/Protein.h"
# include <lbcpp/Inference/SequentialInference.h>
# include <lbcpp/Inference/DecoratorInference.h>

namespace lbcpp
{

// Prototype:
//   Input: ProteinObject
//   Supervision: ProteinObject
//   Output: ProteinObject

// Sub-inferences prototype:
//   Input: ProteinObject
//   Supervision: ProteinObject
//   Output: ProteinObject Object or ProteinObject

class ProteinInferenceHelper
{
public:
  void setPDBDebugDirectory(const File& directory);
  void setProteinDebugDirectory(const File& directory);

protected:
  static ProteinPtr prepareInputProtein(const Variable& input);

  void prepareSupervisionProtein(ProteinPtr protein);
  void saveDebugFiles(ProteinPtr protein, size_t stepNumber);

private:
  File pdbDebugDirectory;
  File proteinDebugDirectory;
};

class ProteinSequentialInference : public VectorSequentialInference, public ProteinInferenceHelper
{
public:
  ProteinSequentialInference();

  virtual SequentialInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  virtual void prepareSubInference(InferenceContextPtr context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode);
  virtual void finalizeSubInference(InferenceContextPtr context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode);
  virtual Variable finalizeInference(InferenceContextPtr context, SequentialInferenceStatePtr finalState, ReturnCode& returnCode);
};

typedef ReferenceCountedObjectPtr<ProteinSequentialInference> ProteinSequentialInferencePtr;

// todo: ProteinParallelInference

// Transforms a "Protein -> Target (Target)" Inference into a "Protein -> Protein (Protein)" Inference
class ProteinInferenceStep : public StaticDecoratorInference
{
public:
  ProteinInferenceStep(const String& targetName, InferencePtr targetInference);
  ProteinInferenceStep() : targetIndex(0) {}

  // Accessors
  String getTargetName() const
    {return proteinClass()->getStaticVariableName(targetIndex);}

  TypePtr getTargetType() const
    {return proteinClass()->getStaticVariableType(targetIndex);}

  InferencePtr getTargetInference() const
    {return decorated;}

  // DecoratorInference
  virtual DecoratorInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  virtual Variable finalizeInference(InferenceContextPtr context, DecoratorInferenceStatePtr finalState, ReturnCode& returnCode);

  // Inference
  virtual TypePtr getInputType() const
    {return proteinClass();}
  virtual TypePtr getSupervisionType() const
    {return proteinClass();}
  virtual TypePtr getOutputType(TypePtr ) const
    {return proteinClass();}

protected:
  friend class ProteinInferenceStepClass;

  size_t targetIndex;
};

typedef ReferenceCountedObjectPtr<ProteinInferenceStep> ProteinInferenceStepPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_H_


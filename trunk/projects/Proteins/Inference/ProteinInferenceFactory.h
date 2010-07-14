/*-----------------------------------------.---------------------------------.
| Filename: ProteinInferenceFactory.h      | Protein Inference Factory       |
| Author  : Francis Maes                   |                                 |
| Started : 13/07/2010 23:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_FACTORY_H_
# define LBCPP_PROTEIN_INFERENCE_FACTORY_H_

# include <lbcpp/Data/Perception.h>
# include <lbcpp/Inference/Inference.h>
# include "../Data/Protein.h"

namespace lbcpp
{

class ProteinInferenceFactory : public Object
{
public:
  ProteinInferenceFactory();

  /*
  ** High level inferences
  */
  virtual InferencePtr createInference(const String& targetName) const;
  virtual InferencePtr createTargetInference(const String& targetName) const;
  virtual InferencePtr createSequenceLabelingInference(const String& targetName) const;

  /*
  ** Perceptions
  */
  virtual PerceptionPtr createPerception(const String& targetName, bool is1DTarget, bool is2DTarget) const;
  virtual void createPrimaryStructureResiduePerception(CompositePerceptionPtr res) const;
  virtual void createPositionSpecificScoringMatrixResiduePerception(CompositePerceptionPtr res) const;

  /*
  ** Low level inferences
  */
  virtual InferencePtr createBinaryClassifier(const String& targetName, TypePtr inputType) const = 0;
  virtual InferencePtr createMultiClassClassifier(const String& targetName, TypePtr inputType, EnumerationPtr classes) const = 0;

protected:
  ClassPtr proteinClass;

  size_t getTargetIndex(const String& targetName) const;
  TypePtr getTargetType(const String& targetName) const;
  InferencePtr addToProteinInference(InferencePtr targetInference, const String& targetName) const;

  PerceptionPtr applyPerceptionOnProteinVariable(const String& variableName, PerceptionPtr variablePerception) const;
};

typedef ReferenceCountedObjectPtr<ProteinInferenceFactory> ProteinInferenceFactoryPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_FACTORY_H_

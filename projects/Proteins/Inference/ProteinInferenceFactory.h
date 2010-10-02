/*-----------------------------------------.---------------------------------.
| Filename: ProteinInferenceFactory.h      | Protein Inference Factory       |
| Author  : Francis Maes                   |                                 |
| Started : 13/07/2010 23:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_FACTORY_H_
# define LBCPP_PROTEIN_INFERENCE_FACTORY_H_

# include <lbcpp/Function/Perception.h>
# include <lbcpp/Function/PerceptionRewriter.h>
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
  virtual InferencePtr createInferenceStep(const String& targetName) const;
  virtual InferencePtr createTargetInference(const String& targetName) const;
  
  virtual InferencePtr createLabelSequenceInference(const String& targetName) const;
  virtual InferencePtr createProbabilitySequenceInference(const String& targetName) const;
  virtual InferencePtr createContactMapInference(const String& targetName) const;

  /*
  ** Perceptions
  */
  virtual void getPerceptionRewriteRules(PerceptionRewriterPtr rewriter) const
    {}

  virtual PerceptionPtr createPerception(const String& targetName, bool is1DTarget, bool is2DTarget) const;

  virtual PerceptionPtr createResiduePerception(const String& targetName) const;
  virtual PerceptionPtr createResiduePairPerception(const String& targetName) const;

  virtual PerceptionPtr createProteinPerception() const;
  virtual PerceptionPtr createLabelSequencePerception(const String& targetName) const;
  virtual PerceptionPtr createProbabilitySequencePerception(const String& targetName) const;
  virtual PerceptionPtr createPositionSpecificScoringMatrixPerception() const;
  virtual PerceptionPtr createPairsSequencesPerception() const;

  /*
  ** Low level inferences
  */
  virtual InferencePtr createBinaryClassifier(const String& targetName, PerceptionPtr perception) const
    {jassert(false); return InferencePtr();}

  virtual InferencePtr createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const
    {jassert(false); return InferencePtr();}

protected:
  ClassPtr proteinClass;
  PerceptionRewriterPtr perceptionRewriter;

  size_t getTargetIndex(const String& targetName) const;
  TypePtr getTargetType(const String& targetName) const;
  InferencePtr addToProteinInference(InferencePtr targetInference, const String& targetName) const;

  PerceptionPtr applyPerceptionOnProteinVariable(const String& variableName, PerceptionPtr variablePerception) const;
  PerceptionPtr applyWindowOnPerception(const String& variableName, size_t windowSize, PerceptionPtr perception) const;
  PerceptionPtr applyPerceptionOnEntireProteinVariable(const String& variableName, PerceptionPtr perception) const;
  PerceptionPtr createHistogramPerception(const String& targetName) const;
  PerceptionPtr createPairSequencesPerception(const String& firstTargetName, const String& secondTargetName, size_t windowSize) const;
};

typedef ReferenceCountedObjectPtr<ProteinInferenceFactory> ProteinInferenceFactoryPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_FACTORY_H_

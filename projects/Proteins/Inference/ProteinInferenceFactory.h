/*-----------------------------------------.---------------------------------.
| Filename: ProteinInferenceFactory.h      | Protein Inference Factory       |
| Author  : Francis Maes                   |                                 |
| Started : 13/07/2010 23:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_FACTORY_H_
# define LBCPP_PROTEIN_INFERENCE_FACTORY_H_

# include <lbcpp/Perception/Perception.h>
# include <lbcpp/Perception/PerceptionRewriter.h>
# include <lbcpp/Inference/Inference.h>
# include "../Data/Protein.h"   

namespace lbcpp
{

class ProteinInferenceFactory : public Object
{
public:
  ProteinInferenceFactory(ExecutionContext& context);
  virtual ~ProteinInferenceFactory();

  /*
  ** High level inferences
  */
  virtual InferencePtr createInferenceStep(const String& targetName) const;
  virtual InferencePtr createTargetInference(const String& targetName) const;
  
  virtual InferencePtr createLabelSequenceInference(const String& targetName) const;
  virtual InferencePtr createProbabilitySequenceInference(const String& targetName) const;
  virtual InferencePtr createContactMapInference(const String& targetName) const;
  virtual InferencePtr createDisulfideBondsInference(const String& targetName) const;

  /*
  ** Perceptions
  */
  virtual void getPerceptionRewriteRules(PerceptionRewriterPtr rewriter) const
    {}

  enum PerceptionType
  {
    proteinPerception = 0,
    residuePerception,
    residuePairPerception
  };

  virtual PerceptionPtr createPerception(const String& targetName, PerceptionType type) const;

  virtual PerceptionPtr createProteinPerception(const String& targetName) const;
  virtual PerceptionPtr createResiduePerception(const String& targetName) const;
  virtual PerceptionPtr createResiduePairPerception(const String& targetName) const;

  virtual PerceptionPtr createLabelSequencePerception(const String& targetName) const;
  virtual PerceptionPtr createProbabilitySequencePerception(const String& targetName) const;
  virtual PerceptionPtr createPositionSpecificScoringMatrixPerception() const;

  /*
  ** Low level inferences
  */
  virtual InferencePtr createBinaryClassifier(const String& targetName, PerceptionPtr perception) const
    {jassert(false); return InferencePtr();}

  virtual InferencePtr createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const
    {jassert(false); return InferencePtr();}

protected:
  ExecutionContext& context;

  PerceptionRewriterPtr perceptionRewriter;

  size_t getTargetIndex(const String& targetName) const;
  TypePtr getTargetType(const String& targetName) const;
  InferencePtr addToProteinInference(InferencePtr targetInference, const String& targetName) const;

  void addPerception(CompositePerceptionPtr composite, const String& name, const String& targetName, PerceptionPtr perception) const;
};

typedef ReferenceCountedObjectPtr<ProteinInferenceFactory> ProteinInferenceFactoryPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_FACTORY_H_

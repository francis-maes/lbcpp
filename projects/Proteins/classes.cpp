/*-----------------------------------------.---------------------------------.
| Filename: classes.cpp                    | Declaration of serializable     |
| Author  : Francis Maes                   |   classes                       |
| Started : 27/03/2010 12:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include <lbcpp/impl/impl.h>
using namespace lbcpp;

#define LBCPP_DECLARE_DICTIONARY(ClassName) \
  lbcpp::FeatureDictionaryManager::getInstance().addDictionary(ClassName::getInstance())



#include "InferenceData/LabelSequence.h"
#include "InferenceData/ScoreVectorSequence.h"

#include "InferenceStep/ClassificationInferenceStep.h"
#include "InferenceStep/DecoratorInferenceStep.h"
#include "InferenceStep/ParallelSequenceLabelingInferenceStep.h"

#include "InferenceContext/CancelAfterStepCallback.h"

void declareInferenceClasses()
{
  // Data
  LBCPP_DECLARE_CLASS(LabelSequence);
  LBCPP_DECLARE_CLASS(ScoreVectorSequence);

  // InferenceStep
  LBCPP_DECLARE_CLASS(ClassificationInferenceStep);
  LBCPP_DECLARE_CLASS(CallbackBasedDecoratorInferenceStep);

  // InferenceCallback
  LBCPP_DECLARE_CLASS(CancelAfterStepCallback);
}

#include "ProteinInference/Protein.h"
#include "ProteinInference/ProteinInference.h"
#include "ProteinInference/AminoAcidDictionary.h"
#include "ProteinInference/SecondaryStructureDictionary.h"

void declareProteinClasses()
{
  declareInferenceClasses();

  LBCPP_DECLARE_CLASS(Protein);
  LBCPP_DECLARE_CLASS(ProteinInference);

  LBCPP_DECLARE_DICTIONARY(AminoAcidDictionary);
  LBCPP_DECLARE_DICTIONARY(SecondaryStructureDictionary);
  LBCPP_DECLARE_DICTIONARY(DSSPSecondaryStructureDictionary);
  LBCPP_DECLARE_DICTIONARY(SolventAccesibility2StateDictionary);
}

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

#include "InferenceData/LabelSequence.h"
#include "InferenceData/ScalarSequence.h"
#include "InferenceData/ScoreVectorSequence.h"
#include "InferenceData/ScoreSymmetricMatrix.h"

#include "InferenceStep/ClassificationInferenceStep.h"
#include "InferenceStep/RegressionInferenceStep.h"
#include "InferenceStep/DecoratorInferenceStep.h"

#include "InferenceContext/CancelAfterStepCallback.h"

void declareInferenceClasses()
{
  // Data
  LBCPP_DECLARE_CLASS(LabelSequence);
  LBCPP_DECLARE_CLASS(ScalarSequence);
  LBCPP_DECLARE_CLASS(ScoreVectorSequence);
  LBCPP_DECLARE_CLASS(ScoreSymmetricMatrix);

  // InferenceStep
  LBCPP_DECLARE_CLASS(ClassificationInferenceStep);
  LBCPP_DECLARE_CLASS(RegressionInferenceStep);
  LBCPP_DECLARE_CLASS(TransferRegressionInferenceStep);
  LBCPP_DECLARE_CLASS(CallbackBasedDecoratorInferenceStep);

  // InferenceCallback
  LBCPP_DECLARE_CLASS(CancelAfterStepCallback);
}

#include "Protein/ProteinTertiaryStructure.h"
#include "Protein/Protein.h"
#include "Protein/AminoAcidDictionary.h"
#include "Protein/SecondaryStructureDictionary.h"

#include "Protein/Inference/Protein1DInferenceStep.h"
#include "Protein/Inference/ProteinBackboneBondSequenceInferenceStep.h"
#include "Protein/Inference/ProteinInference.h"

extern void declareProteinResidueFeaturesClasses();
extern void declareProteinResiduePairFeaturesClasses();

void declareProteinClasses()
{
  declareInferenceClasses();
  declareProteinResidueFeaturesClasses();
  declareProteinResiduePairFeaturesClasses();

  LBCPP_DECLARE_CLASS(ProteinBackboneBondSequence);
  LBCPP_DECLARE_CLASS(CartesianCoordinatesSequence);
  LBCPP_DECLARE_CLASS(ProteinAtom);
  LBCPP_DECLARE_CLASS(ProteinResidue);
  LBCPP_DECLARE_CLASS(ProteinTertiaryStructure);

  LBCPP_DECLARE_CLASS(Protein);

  LBCPP_DECLARE_CLASS(ProteinInference);
    LBCPP_DECLARE_CLASS(ProteinSequenceLabelingInferenceStep);
    
    LBCPP_DECLARE_CLASS(PSSMPredictionInferenceStep);
      LBCPP_DECLARE_CLASS(PSSMRowPredictionInferenceStep);

    LBCPP_DECLARE_CLASS(ProteinBackboneBondSequenceInferenceStep);
      LBCPP_DECLARE_CLASS(ProteinBackboneBondInferenceStep);
      LBCPP_DECLARE_CLASS(AngleDifferenceScalarFunction);
      LBCPP_DECLARE_CLASS(ScaledSigmoidScalarFunction);


  LBCPP_DECLARE_DICTIONARY(AminoAcidDictionary);
  LBCPP_DECLARE_DICTIONARY(SecondaryStructureDictionary);
  LBCPP_DECLARE_DICTIONARY(DSSPSecondaryStructureDictionary);
  LBCPP_DECLARE_DICTIONARY(AminoAcidPropertyDictionary);
}

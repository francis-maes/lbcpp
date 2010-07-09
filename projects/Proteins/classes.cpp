/*-----------------------------------------.---------------------------------.
| Filename: classes.cpp                    | Declaration of serializable     |
| Author  : Francis Maes                   |   classes                       |
| Started : 27/03/2010 12:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
using namespace lbcpp;

#include "InferenceData/LabelSequence.h"
#include "InferenceData/ScalarSequence.h"
#include "InferenceData/ScoreVectorSequence.h"
#include "InferenceData/ScoreSymmetricMatrix.h"
#include "InferenceData/BondCoordinatesSequence.h"

#include "Protein/ProteinTertiaryStructure.h"
#include "Protein/ProteinObject.h"
#include "Protein/AminoAcidDictionary.h"
#include "Protein/SecondaryStructureDictionary.h"

#include "Protein/Inference/Protein1DTargetInference.h"
#include "Protein/Inference/ProteinCAlphaBondSequenceInference.h"
#include "Protein/Inference/ProteinBackboneBondSequenceInference.h"
#include "Protein/Inference/Protein2DTargetInference.h"
#include "Protein/Inference/ProteinContactMapInference.h"
#include "Protein/Inference/ProteinTertiaryStructureInference.h"
#include "Protein/Inference/ProteinInference.h"

extern void declareProteinGlobalFeaturesClasses();
extern void declareProteinResidueFeaturesClasses();
extern void declareProteinResiduePairFeaturesClasses();

// new:
extern void declareVector3Classes();
extern void declareCartesianPositionVectorClasses();

extern void declareAminoAcidClasses();
extern void declareSecondaryStructureClasses();
extern void declareProteinClass();

void declareProteinClasses()
{
  // Data
  LBCPP_DECLARE_CLASS_LEGACY(LabelSequence);
  LBCPP_DECLARE_CLASS_LEGACY(ScalarSequence);
  LBCPP_DECLARE_CLASS_LEGACY(ScoreVectorSequence);
  LBCPP_DECLARE_CLASS_LEGACY(ScoreSymmetricMatrix);
  LBCPP_DECLARE_CLASS_LEGACY(BondCoordinatesSequence);
  LBCPP_DECLARE_CLASS_LEGACY(CartesianCoordinatesSequence);

  declareProteinGlobalFeaturesClasses();
  declareProteinResidueFeaturesClasses();
  declareProteinResiduePairFeaturesClasses();
  
  LBCPP_DECLARE_CLASS_LEGACY(ProteinBackboneBond);
  LBCPP_DECLARE_CLASS_LEGACY(ProteinBackboneBondSequence);
  LBCPP_DECLARE_CLASS_LEGACY(ProteinAtom);
  LBCPP_DECLARE_CLASS_LEGACY(ProteinResidueAtoms);
  LBCPP_DECLARE_CLASS_LEGACY(ProteinTertiaryStructure);

  LBCPP_DECLARE_CLASS_LEGACY(ProteinObject);

  LBCPP_DECLARE_CLASS_LEGACY(ProteinToInputOutputPairFunction);

  LBCPP_DECLARE_CLASS_LEGACY(ProteinInference);
    LBCPP_DECLARE_CLASS_LEGACY(ProteinSequenceLabelingInferenceStep);
    
//    LBCPP_DECLARE_CLASS_LEGACY(PSSMPredictionInferenceStep);
//      LBCPP_DECLARE_CLASS_LEGACY(PSSMRowPredictionInferenceStep);

    LBCPP_DECLARE_CLASS_LEGACY(ProteinCAlphaBondSequenceInferenceStep);
      LBCPP_DECLARE_CLASS_LEGACY(ProteinCAlphaBondInferenceStep);

    LBCPP_DECLARE_CLASS_LEGACY(ProteinBackboneBondSequenceInferenceStep);
      LBCPP_DECLARE_CLASS_LEGACY(ScaledSigmoidScalarFunction);
      LBCPP_DECLARE_CLASS_LEGACY(ProteinBackboneBondInferenceStep);

    LBCPP_DECLARE_CLASS_LEGACY(ProteinContactMapInference);
      LBCPP_DECLARE_CLASS_LEGACY(ContactMapScoresInference);
      LBCPP_DECLARE_CLASS_LEGACY(ContactMapScoresToProbabilitiesInference);

    LBCPP_DECLARE_CLASS_LEGACY(ProteinTertiaryStructureRefinementInferenceStep);
      LBCPP_DECLARE_CLASS_LEGACY(ProteinResidueRefinementInferenceStep);

  LBCPP_DECLARE_DICTIONARY(AminoAcidDictionary);
  LBCPP_DECLARE_DICTIONARY(SecondaryStructureDictionary);
  LBCPP_DECLARE_DICTIONARY(DSSPSecondaryStructureDictionary);
  LBCPP_DECLARE_DICTIONARY(AminoAcidPropertyDictionary);
  LBCPP_DECLARE_DICTIONARY(StructuralAlphabetDictionary);

  // new:
  declareVector3Classes();
  declareCartesianPositionVectorClasses();

  declareAminoAcidClasses();
  declareSecondaryStructureClasses();
  declareProteinClass();
  // -
}

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
#include "InferenceData/BondCoordinatesSequence.h"

#include "Protein/ProteinTertiaryStructure.h"
#include "Protein/Protein.h"
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

void declareProteinClasses()
{
  // Data
  LBCPP_DECLARE_CLASS(LabelSequence);
  LBCPP_DECLARE_CLASS(ScalarSequence);
  LBCPP_DECLARE_CLASS(ScoreVectorSequence);
  LBCPP_DECLARE_CLASS(ScoreSymmetricMatrix);
  LBCPP_DECLARE_CLASS(BondCoordinatesSequence);
  LBCPP_DECLARE_CLASS(CartesianCoordinatesSequence);

  declareProteinGlobalFeaturesClasses();
  declareProteinResidueFeaturesClasses();
  declareProteinResiduePairFeaturesClasses();
  
  LBCPP_DECLARE_CLASS(ProteinBackboneBond);
  LBCPP_DECLARE_CLASS(ProteinBackboneBondSequence);
  LBCPP_DECLARE_CLASS(ProteinAtom);
  LBCPP_DECLARE_CLASS(ProteinResidue);
  LBCPP_DECLARE_CLASS(ProteinTertiaryStructure);

  LBCPP_DECLARE_CLASS(Protein);

  LBCPP_DECLARE_CLASS(ProteinToInputOutputPairFunction);

  LBCPP_DECLARE_CLASS(ProteinInference);
    LBCPP_DECLARE_CLASS(ProteinSequenceLabelingInferenceStep);
    
//    LBCPP_DECLARE_CLASS(PSSMPredictionInferenceStep);
//      LBCPP_DECLARE_CLASS(PSSMRowPredictionInferenceStep);

    LBCPP_DECLARE_CLASS(ProteinCAlphaBondSequenceInferenceStep);
      LBCPP_DECLARE_CLASS(ProteinCAlphaBondInferenceStep);

    LBCPP_DECLARE_CLASS(ProteinBackboneBondSequenceInferenceStep);
      LBCPP_DECLARE_CLASS(ScaledSigmoidScalarFunction);
      LBCPP_DECLARE_CLASS(ProteinBackboneBondInferenceStep);

    LBCPP_DECLARE_CLASS(ProteinContactMapInference);
      LBCPP_DECLARE_CLASS(ContactMapScoresInference);
      LBCPP_DECLARE_CLASS(ContactMapScoresToProbabilitiesInference);

    LBCPP_DECLARE_CLASS(ProteinTertiaryStructureRefinementInferenceStep);
      LBCPP_DECLARE_CLASS(ProteinResidueRefinementInferenceStep);

  LBCPP_DECLARE_DICTIONARY(AminoAcidDictionary);
  LBCPP_DECLARE_DICTIONARY(SecondaryStructureDictionary);
  LBCPP_DECLARE_DICTIONARY(DSSPSecondaryStructureDictionary);
  LBCPP_DECLARE_DICTIONARY(AminoAcidPropertyDictionary);
  LBCPP_DECLARE_DICTIONARY(StructuralAlphabetDictionary);
}
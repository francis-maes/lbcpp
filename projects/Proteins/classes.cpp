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

#include "Geometry/Matrix4.h" // todo: move
const Matrix4 Matrix4::identity = Matrix4(
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      0.0, 0.0, 0.0, 1.0);


#include "InferenceData/LabelSequence.h"
#include "InferenceData/ScoreVectorSequence.h"
#include "InferenceData/ScoreSymmetricMatrix.h"

#include "InferenceStep/ClassificationInferenceStep.h"
#include "InferenceStep/RegressionInferenceStep.h"
#include "InferenceStep/DecoratorInferenceStep.h"
#include "InferenceStep/ParallelSequenceLabelingInferenceStep.h"

#include "InferenceContext/CancelAfterStepCallback.h"

void declareInferenceClasses()
{
  // Data
  LBCPP_DECLARE_CLASS(LabelSequence);
  LBCPP_DECLARE_CLASS(ScoreVectorSequence);
  LBCPP_DECLARE_CLASS(ScoreSymmetricMatrix);

  // InferenceStep
  LBCPP_DECLARE_CLASS(ClassificationInferenceStep);
  LBCPP_DECLARE_CLASS(RegressionInferenceStep);
  LBCPP_DECLARE_CLASS(CallbackBasedDecoratorInferenceStep);

  // InferenceCallback
  LBCPP_DECLARE_CLASS(CancelAfterStepCallback);
}

#include "Protein/ProteinTertiaryStructure.h"
#include "Protein/Protein.h"
#include "Protein/InferenceStep/ProteinInference.h"
#include "Protein/AminoAcidDictionary.h"
#include "Protein/SecondaryStructureDictionary.h"

void declareProteinClasses()
{
  declareInferenceClasses();

  LBCPP_DECLARE_CLASS(ProteinDihedralAngles);
  LBCPP_DECLARE_CLASS(CartesianCoordinatesSequence);
  LBCPP_DECLARE_CLASS(ProteinAtom);
  LBCPP_DECLARE_CLASS(ProteinResidue);
  LBCPP_DECLARE_CLASS(ProteinTertiaryStructure);

  LBCPP_DECLARE_CLASS(Protein);
  LBCPP_DECLARE_CLASS(ProteinInference);


  LBCPP_DECLARE_DICTIONARY(AminoAcidDictionary);
  LBCPP_DECLARE_DICTIONARY(SecondaryStructureDictionary);
  LBCPP_DECLARE_DICTIONARY(DSSPSecondaryStructureDictionary);
  LBCPP_DECLARE_DICTIONARY(SolventAccesibility2StateDictionary);
  LBCPP_DECLARE_DICTIONARY(OrderDisorderDictionary);
  LBCPP_DECLARE_DICTIONARY(AminoAcidPropertyDictionary);
}

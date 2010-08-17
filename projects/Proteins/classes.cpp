/*-----------------------------------------.---------------------------------.
| Filename: classes.cpp                    | Declaration of serializable     |
| Author  : Francis Maes                   |   classes                       |
| Started : 27/03/2010 12:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
using namespace lbcpp;

// geometry
extern void declareVector3Classes();
extern void declareCartesianPositionVectorClasses();

// data
extern void declareAminoAcidClasses();
extern void declareSecondaryStructureClasses();
extern void declareAtomClasses();
extern void declareResidueClasses();
extern void declareTertiaryStructureClasses();
extern void declareProteinClass();

// perception
extern void declareProteinPerceptionClasses();
extern void declareResiduePerceptionClasses();
extern void declareResiduePairPerceptionClasses();

// inference
extern void declareProteinInferenceClasses();

void declareProteinClasses()
{
  // geometry
  declareVector3Classes();
  declareCartesianPositionVectorClasses();

  // data
  declareAminoAcidClasses();
  declareSecondaryStructureClasses();
  declareAtomClasses();
  declareResidueClasses();
  declareTertiaryStructureClasses();
  declareProteinClass();

  // perception
  declareProteinPerceptionClasses();
  declareResiduePerceptionClasses();
  declareResiduePairPerceptionClasses();

  // inference
  declareProteinInferenceClasses();
}

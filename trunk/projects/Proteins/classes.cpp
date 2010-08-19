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
extern void declareProteinMiscTypes();

// inference
extern void declareProteinInferenceClassesOld();

// generated
extern void declareProteinDataClasses();
extern void declareProteinPerceptionClasses();

void declareProteinLibrary()
{
  // geometry
  declareVector3Classes();
  declareCartesianPositionVectorClasses();

  // data
  declareAminoAcidClasses();
  declareSecondaryStructureClasses();
  declareProteinMiscTypes();
   

  // inference
  declareProteinInferenceClassesOld();

  // generated
  declareProteinDataClasses();
  declareProteinPerceptionClasses();
}

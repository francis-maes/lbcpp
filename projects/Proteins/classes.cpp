/*-----------------------------------------.---------------------------------.
| Filename: classes.cpp                    | Declaration of serializable     |
| Author  : Francis Maes                   |   classes                       |
| Started : 27/03/2010 12:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
using namespace lbcpp;

// data
extern void declareAminoAcidClasses();
extern void declareSecondaryStructureClasses();
extern void declareProteinMiscTypes();

// generated
extern void declareGeometryClasses();
extern void declareProteinDataClasses();
extern void declareProteinPerceptionClasses();
extern void declareProteinInferenceClasses();

void declareProteinLibrary()
{
  // data
  declareAminoAcidClasses();
  declareSecondaryStructureClasses();
  declareProteinMiscTypes();

  // generated
  declareGeometryClasses();
  declareProteinDataClasses();
  declareProteinPerceptionClasses();
  declareProteinInferenceClasses();
}

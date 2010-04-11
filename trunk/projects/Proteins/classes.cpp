/*-----------------------------------------.---------------------------------.
| Filename: classes.cpp                    | Declaration of serializable     |
| Author  : Francis Maes                   |   classes                       |
| Started : 27/03/2010 12:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "ProteinInference/Protein.h"
#include "ProteinInference/AminoAcidDictionary.h"
#include "ProteinInference/SecondaryStructureDictionary.h"
#include <lbcpp/impl/impl.h>
using namespace lbcpp;

#define LBCPP_DECLARE_DICTIONARY(ClassName) \
  lbcpp::FeatureDictionaryManager::getInstance().addDictionary(ClassName::getInstance())

void declareProteinClasses()
{
  LBCPP_DECLARE_CLASS(Protein);
  LBCPP_DECLARE_CLASS(LabelSequence);
  LBCPP_DECLARE_CLASS(ScoreVectorSequence);

  LBCPP_DECLARE_DICTIONARY(AminoAcidDictionary);
  LBCPP_DECLARE_DICTIONARY(SecondaryStructureDictionary);
  LBCPP_DECLARE_DICTIONARY(DSSPSecondaryStructureDictionary);
  LBCPP_DECLARE_DICTIONARY(SolventAccesibility2StateDictionary);
}

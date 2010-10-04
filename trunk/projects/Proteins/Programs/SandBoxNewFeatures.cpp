/*-----------------------------------------.---------------------------------.
| Filename: SandBoxNewFeatures.cpp         | New Feature Generators          |
| Author  : Francis Maes                   |                                 |
| Started : 20/08/2010 19:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
using namespace lbcpp;

extern void declareProteinClasses();

void testPerception(const String& name, PerceptionPtr perception, const Variable& input)
{
  std::cout << "=========== " << name << " ==============" << std::endl;
  Variable output = perception->compute(input);
  output.printRecursively(std::cout);
  std::cout << std::endl;
}

PerceptionPtr positiveIntegerFeatures()
{
  return softDiscretizedLogNumberFeatures(positiveIntegerType(), 0.0, 10.0, 20, true);
}

PerceptionPtr doubleFeatures()
{
  return signedNumberFeatures(softDiscretizedLogNumberFeatures(doubleType(), -10.0, 10.0, 20, true));
}

int main(int argc, char** argv)
{
  lbcpp::initialize();
  Variable myProb(0.5, probabilityType());
  Variable myBoolean(true);
  Variable myInteger(1664, positiveIntegerType());

  // int => softDiscretizedLogNumberFeatures
  // positive int => softDiscretizedLogPositiveNumberFeatures
  // double => softDiscretizedLogNumberFeatures
  // positive double => softDiscretizedLogPositiveNumberFeatures
  // probability => softDiscretizedNumberFeatures
  
  /*testPerception(T("hardDiscretizedNumberFeatures"), hardDiscretizedNumberFeatures(probabilityType(), 0.0, 1.0, 10, false), myProb);
  testPerception(T("softDiscretizedNumberFeatures"), softDiscretizedNumberFeatures(probabilityType(), 0.0, 1.0, 10, false, false), myProb);
  testPerception(T("softDiscretizedNumberFeatures cyclic"), softDiscretizedNumberFeatures(probabilityType(), 0.0, 1.0, 10, false, true), myProb);
  testPerception(T("softDiscretizedNumberFeatures oob"), softDiscretizedNumberFeatures(probabilityType(), 0.0, 1.0, 10, true, false), myProb);*/
  testPerception(T("positiveIntegerFeatures"), positiveIntegerFeatures(), myInteger);

  testPerception(T("doubleFeatures"), doubleFeatures(), -0.000000000005);

  testPerception(T("boolean"), booleanFeatures(), myBoolean);

  return 0;
}

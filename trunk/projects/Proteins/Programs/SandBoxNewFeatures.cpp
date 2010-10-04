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

int main(int argc, char** argv)
{
  lbcpp::initialize();
  Variable myProb(0.1664, probabilityType());
  Variable myBoolean(true);

  // int => softDiscretizedLogNumberFeatures
  // positive int => softDiscretizedLogPositiveNumberFeatures
  // double => softDiscretizedLogNumberFeatures
  // positive double => softDiscretizedLogPositiveNumberFeatures
  // probability => softDiscretizedNumberFeatures
  
  testPerception(T("hardDiscretizedNumberFeatures"), hardDiscretizedNumberFeatures(probabilityType(), 0.0, 1.0, 10, false), myProb);
  testPerception(T("boolean"), booleanFeatures(), myBoolean);

  return 0;
}

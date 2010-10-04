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
  output.printRecursively(std::cout, -1, false, false);
  std::cout << std::endl;
}
namespace lbcpp
{
  extern FunctionPtr multiplyDoubleFunction();
};

class IdentityFunction : public Function
{
public:
  IdentityFunction(TypePtr type) : type(type) {}

  virtual TypePtr getInputType() const
    {return type;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return inputType;}

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
    {return input;}

protected:
  TypePtr type;
};

int main(int argc, char** argv)
{
  lbcpp::initialize();
  Variable myProb(0.5, probabilityType());
  Variable myBoolean(true);
  Variable myInteger1(51, positiveIntegerType());
  Variable myInteger2(2, positiveIntegerType());

  // int => softDiscretizedLogNumberFeatures
  // positive int => softDiscretizedLogPositiveNumberFeatures
  // double => softDiscretizedLogNumberFeatures
  // positive double => softDiscretizedLogPositiveNumberFeatures
  // probability => softDiscretizedNumberFeatures
  
  /*testPerception(T("hardDiscretizedNumberFeatures"), hardDiscretizedNumberFeatures(probabilityType(), 0.0, 1.0, 10, false), myProb);
  testPerception(T("softDiscretizedNumberFeatures"), softDiscretizedNumberFeatures(probabilityType(), 0.0, 1.0, 10, false, false), myProb);
  testPerception(T("softDiscretizedNumberFeatures cyclic"), softDiscretizedNumberFeatures(probabilityType(), 0.0, 1.0, 10, false, true), myProb);
  testPerception(T("softDiscretizedNumberFeatures oob"), softDiscretizedNumberFeatures(probabilityType(), 0.0, 1.0, 10, true, false), myProb);*/

  CompositePerceptionPtr composite = compositePerception(positiveIntegerType(), T("combo"));
  composite->addPerception(T("base20"), defaultPositiveIntegerFeatures(20));
  composite->addPerception(T("base2"), defaultPositiveIntegerFeatures(2));

  FunctionPtr makePairFunction = new IdentityFunction(pairType(anyType(), anyType()));


  testPerception(T("F"), defaultPositiveIntegerFeatures(), myInteger1);
  testPerception(T("C"), composite, myInteger2);
  //testPerception(T("int 51 x 10.0"), productPerception(makePairFunction, composite, doubleType()), Variable::pair(myInteger1, 10.0));
  //testPerception(T("10.0 x int 51"), productPerception(makePairFunction, doubleType(), composite), Variable::pair(10.0, myInteger1));

  //testPerception(T("int 1664"), defaultPositiveIntegerFeatures(), myInteger2);
  
  //testPerception(T("pair F,F"), conjunctionFeatures(defaultPositiveIntegerFeatures(), defaultPositiveIntegerFeatures()), Variable::pair(myInteger1, myInteger2));
  //testPerception(T("pair C,F"), productPerception(makePairFunction, false, composite, defaultPositiveIntegerFeatures()), Variable::pair(myInteger1, myInteger2));
  testPerception(T("pair F,C"), productPerception(makePairFunction, false, defaultPositiveIntegerFeatures(), composite), Variable::pair(myInteger1, myInteger2));
  //testPerception(T("pair C,C"), conjunctionFeatures(composite, composite), Variable::pair(myInteger1, myInteger2));

//  testPerception(T("doubleFeatures"), doubleFeatures(), -0.000000000005);

//  testPerception(T("boolean"), booleanFeatures(), myBoolean);

  return 0;
}

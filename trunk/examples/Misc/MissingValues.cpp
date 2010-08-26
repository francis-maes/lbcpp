/*-----------------------------------------.---------------------------------.
| Filename: MissingValues.cpp              | Illustrates missing values      |
| Author  : Francis Maes                   |                                 |
| Started : 08/07/2010 12:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
using namespace lbcpp;

class A : public Object
{
public:
  virtual String toString() const
    {return T("aaa");}
};

int main(int argc, char** argv)
{
  lbcpp::initialize();
  Type::declare(new DefaultClass(T("A")));  

  Variable myBoolean(true);
  Variable myMissingBoolean = Variable::missingValue(booleanType());
  Variable myInt(51);
  Variable myMissingInt = Variable::missingValue(integerType());
  Variable myDouble(8.6);
  Variable myMissingDouble = Variable::missingValue(doubleType());
  Variable myString(T("Hello"));
  Variable myMissingString = Variable::missingValue(stringType());
  Variable myObject(new A());
  Variable myMissingObject = Variable::missingValue(Class::get(T("A")));
  Variable myPair = Variable::pair(myBoolean, myObject);
  Variable myMissingPair = Variable::missingValue(pairType(booleanType(), Class::get(T("A"))));
  
  std::cout << myBoolean << " " << myMissingBoolean << std::endl
            << myInt << " " << myMissingInt << std::endl
            << myDouble << " " << myMissingDouble << std::endl
            << myString << " " << myMissingString << std::endl
            << myObject << " " << myMissingObject << std::endl
            << myPair << " " << myMissingPair << std::endl;
      
  std::cout << myBoolean.isMissingValue() << " " << myMissingBoolean.isMissingValue() << std::endl
            << myInt.isMissingValue() << " " << myMissingInt.isMissingValue() << std::endl
            << myDouble.isMissingValue() << " " << myMissingDouble.isMissingValue() << std::endl
            << myString.isMissingValue() << " " << myMissingString.isMissingValue() << std::endl
            << myObject.isMissingValue() << " " << myMissingObject.isMissingValue() << std::endl
            << myPair.isMissingValue() << " " << myMissingPair.isMissingValue() << std::endl;
            
  return 0;
}

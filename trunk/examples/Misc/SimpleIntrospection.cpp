/*-----------------------------------------.---------------------------------.
| Filename: SimpleIntrospection.cpp        | This example illustrates simple |
| Author  : Francis Maes                   |  use of introspection           |
| Started : 09/07/2010 17:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
using namespace lbcpp;

class A : public Object
{
public:
  A(bool b) : myBool(true), myInt(1664), myDouble(8.6), myString(T("yo")), myVariable(Variable::pair(51, 33)) {}
  A() {}

private:
  friend class AClass;

  bool myBool;
  int myInt;
  double myDouble;
  String myString;
  ObjectPtr myObject;
  Variable myVariable;
};

typedef ReferenceCountedObjectPtr<A> APtr;

class AClass : public DynamicClass
{
public:
  AClass() : DynamicClass(T("A"), objectClass())
  {
    addVariable(booleanType(), T("myBool"));
    addVariable(integerType(), T("myInt"));
    addVariable(doubleType(), T("myDouble"));
    addVariable(stringType(), T("myString"));
    addVariable(objectClass(), T("myObject"));
    addVariable(topLevelType(), T("myVariable"));
  }

  virtual VariableValue create() const
    {return new A();}

  LBCPP_DECLARE_VARIABLE_BEGIN(A)
    LBCPP_DECLARE_VARIABLE(myBool);
    LBCPP_DECLARE_VARIABLE(myInt);
    LBCPP_DECLARE_VARIABLE(myDouble);
    LBCPP_DECLARE_VARIABLE(myString);
    LBCPP_DECLARE_VARIABLE(myObject);
    LBCPP_DECLARE_VARIABLE(myVariable);
  LBCPP_DECLARE_VARIABLE_END();
};

int main(int argc, char** argv)
{
  lbcpp::initialize();

  Class::declare(new AClass());
  
  APtr a = new A(true);
  APtr emptyA = new A();
  std::cout << "A = " << std::endl << a->toString() << std::endl;
  std::cout << "EmptyA = " << std::endl << emptyA->toString() << std::endl;
  File file = File::createTempFile(T("a"));
  Variable(a).saveToFile(file);

  APtr loadedA = Variable::createFromFile(file).getObjectAndCast<A>();
  std::cout << "LoadedA = " << std::endl << loadedA->toString() << std::endl;
  return 0;
}

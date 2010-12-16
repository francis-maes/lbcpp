/*-----------------------------------------.---------------------------------.
| Filename: SimpleIntrospectionExample.h   | Illustrates Simple Introspection|
| Author  : Francis Maes                   |  operations                     |
| Started : 16/12/2010 14:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXAMPLES_SIMPLE_INTROSPECTION_H_
# define LBCPP_EXAMPLES_SIMPLE_INTROSPECTION_H_

# include <lbcpp/Core/Variable.h>

namespace lbcpp
{

class SimpleClass : public Object
{
public:
  SimpleClass(bool b)
    : myBool(true), myInt(1664), myDouble(8.6), myString(T("yo")), myVariable(Variable::pair(51, 33)) {}
  SimpleClass() {}

private:
  friend class SimpleClassClass;

  bool myBool;
  int myInt;
  double myDouble;
  String myString;
  ObjectPtr myObject;
  Variable myVariable;
};

typedef ReferenceCountedObjectPtr<SimpleClass> SimpleClassPtr;

class SimpleIntrospectionExample : public WorkUnit
{
public:
  virtual bool run(ExecutionContext& context)
  {
    SimpleClassPtr defaultObject = new SimpleClass();
    SimpleClassPtr someObject = new SimpleClass(true);
    context.resultCallback(T("Default Object"), defaultObject);
    context.resultCallback(T("Default Object as string"), defaultObject->toString());
    context.resultCallback(T("Some Object"), someObject);
    
    File file = File::createTempFile(T("object"));
    Variable(someObject).saveToFile(context, file);

    SimpleClassPtr loadedObject = Variable::createFromFile(context, file).getObjectAndCast<SimpleClass>(context);
    context.resultCallback(T("Loaded Object"), loadedObject);
    file.deleteFile();
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_SIMPLE_INTROSPECTION_H_

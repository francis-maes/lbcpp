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
    : myBool(true), myInt(1664), myDouble(8.6), myString(T("yo")) {}
  SimpleClass() {}

private:
  friend class SimpleClassClass;

  bool myBool;
  int myInt;
  double myDouble;
  String myString;
  ObjectPtr myObject;
};

typedef ReferenceCountedObjectPtr<SimpleClass> SimpleClassPtr;

class SimpleIntrospectionExample : public WorkUnit
{
public:
  virtual ObjectPtr run(ExecutionContext& context)
  {
    SimpleClassPtr defaultObject = new SimpleClass();
    SimpleClassPtr someObject = new SimpleClass(true);
    context.resultCallback(T("Default Object"), defaultObject);
    context.resultCallback(T("Default Object as string"), defaultObject->toString());
    context.resultCallback(T("Some Object"), someObject);
    
    File file = File::createTempFile(T("object"));
    //Variable(someObject).saveToFile(context, file); // FIXME

    //SimpleClassPtr loadedObject = Variable::createFromFile(context, file).getObjectAndCast<SimpleClass>(context);
    //context.resultCallback(T("Loaded Object"), loadedObject); // FIXME
    file.deleteFile();

    return ObjectPtr();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_SIMPLE_INTROSPECTION_H_

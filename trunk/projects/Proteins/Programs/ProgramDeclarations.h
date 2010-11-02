

#include "Programs/Program.h"

namespace lbcpp {

class HelloWorldProgram : public Program
{
public:
  HelloWorldProgram()
    {myInt = 4;}
  
  virtual String toString() const
    {return T("It's just a test program ! Don't worry about it !");}
  
  virtual int runProgram(MessageCallback& callback)
    {std::cout << "Hello World : " << myInt << std::endl; return 0;}

protected:
  friend class HelloWorldProgramClass;
  
  int myInt;
};
  
class SaveObjectProgram : public Program
{
  virtual String toString() const
    {return T("SaveObjectProgram is able to serialize a object.");}
  
  virtual int runProgram(MessageCallback& callback)
  {
    ObjectPtr obj = Object::create(Type::get(className, callback));
    if (!obj)
      return -1;

    obj->saveToFile(outputFile, callback);
    return 0;
  }

protected:
  String className;
  File outputFile;
};

class SmartBox : public Program
{
public:
  virtual String toString() const
  {
    return T("SmartBox is ... a smart box ! It's a flexible program allowing"    \
              " you to learn, save, resume models. Just by giving the targets"   \
              " to learn and a training dataset. You can specified the learning" \
              " based-model, the learning rate for each target and each passes," \
              " a specific testing set or a cross-validation protocol.");
  }
  
  virtual int runProgram(MessageCallback& callback);

protected:
  friend class SmartBoxClass;
  
  File trainingDirectory;
  File testingDirectory;
  
  size_t numberOfFolds;
  size_t currentFold;
};

};
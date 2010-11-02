

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
    if (className == String::empty)
    {
      callback.warningMessage(T("SaveObjectProgram::runProgram"), T("No class name specified"));
      return 0;
    }

    if (outputFile == File::nonexistent)
      outputFile = File::getCurrentWorkingDirectory().getChildFile(className + T(".xml"));
    
    std::cout << "Loading class " << className.quoted() << " ... ";
    std::flush(std::cout);

    TypePtr type = Type::get(className, callback);
    if (!type)
    {
      std::cout << "Fail" << std::endl;
      return -1;
    }

    ObjectPtr obj = Object::create(type);
    if (!obj)
    {
      std::cout << "Fail" << std::endl;
      return -1;
    }

    std::cout << "OK" << std::endl;
    std::cout << "Saving class to " << outputFile.getFileName().quoted() << " ... ";
    std::flush(std::cout);

    obj->saveToFile(outputFile, callback);

    std::cout << "OK" << std::endl;
    return 0;
  }

protected:
  friend class SaveObjectProgramClass;
  
  String className;
  File outputFile;
};

class SmartBox : public Program
{
public:
  SmartBox() : numberOfFolds(7), currentFold(0) {}
  
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
  
  File learningDirectory;
  File testingDirectory;
  File validationDirectory;
  
  size_t numberOfFolds;
  size_t currentFold;
};

};
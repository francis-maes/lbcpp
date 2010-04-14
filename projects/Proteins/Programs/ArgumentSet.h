#include <lbcpp/lbcpp.h>
#include <iostream>
#include <map>
#include <vector>

class Argument
{
public:
  String& name;
  
  Argument(String& name): name(name)
  {
  }
  
  virtual String toString()
  {
    return name;
  }
  
  virtual String& getName()
  {
    return name;
  }

  virtual size_t parse(char** str, size_t startIndex, size_t stopIndex) 
  {
    std::cout << T("Parse method not yet implemented ! (")  << name.toUTF8() << T(")") << std::endl;
    return 0;
  }
  
  virtual String getStringValue() {
    return "Method not yet implemented !";
  }
};

class IntegerArgument : public Argument
{
public:
  IntegerArgument(String name, int* destination): Argument(name), destination(destination)
  {
  }
  
  virtual String toString()
  {
    return name + " (int)";
  }
  
  virtual size_t parse(char** str, size_t startIndex, size_t stopIndex)
  {
    if (stopIndex - startIndex < 1)
      return 0;
    
    *destination = (int) strtol(str[++startIndex], (char**) NULL, 10);
    
    return 2;
  }
  
  virtual String getStringValue()
  {
    return String("") << (*destination);
  }
  
private:
  int* destination;
};

class BooleanArgument : public Argument
{
public:
  BooleanArgument(String name, bool* destination): Argument(name), destination(destination)
  {
    *destination = false;
  }
  
  virtual String toString()
  {
    return name + " (bool)";
  }
  
  virtual size_t parse(char** str, size_t startIndex, size_t stopIndex)
  {
    *destination = true;
    
    if (stopIndex - startIndex == 0)
      return 1;
    
    startIndex++;
    if (strcmp(str[startIndex], "false") || strcmp(str[startIndex], "true"))
    {
      *destination = strcmp(str[startIndex], "true") == 0 ? true : false;
      return 2;
    }
    
    return 1;
  }
  
  String getStringValue()
  {
    return (*destination) ? T("True") : T("False");
  }
   
private:
  bool* destination;
};

class StringArgument : public Argument
{
public:
  StringArgument(String name, String* destination): Argument(name), destination(destination)
  {
  }
  
  virtual String toString()
  {
    return name + " (string)";
  }
  
  virtual size_t parse(char** str, size_t startIndex, size_t stopIndex)
  {
    if (stopIndex - startIndex == 0)
      return 0;
    
    startIndex++;
    *destination = String(str[startIndex]);
    
    return 2;
  }
  
  String getStringValue()
  {
    return *destination;
  }

private:
  String* destination;
};

class DoubleArgument : public Argument
{
public:
  DoubleArgument(String name, double* destination): Argument(name), destination(destination)
  {
  }
  
  virtual String toString()
  {
    return name + " (double)";
  }
  
  virtual size_t parse(char** str, size_t startIndex, size_t stopIndex)
  {
    if (stopIndex - startIndex < 1)
      return 0;
    
    *destination = (double) strtod(str[++startIndex], (char**) NULL);
    
    return 2;
  }
  
  virtual String getStringValue()
  {
    return String("") << (*destination);
  }
  
private:
  double* destination;
};


class ArgumentSet
{
public:
  bool insert(Argument* arg)
  {
    if (nameToArgument[arg->getName()])
      return false;
    
    nameToArgument[arg->getName()] = arg;
    arguments.push_back(arg);
    return true;
  }
  
  bool parse(char** str, size_t startIndex, size_t nbStr)
  {
    for (size_t i = startIndex; i < startIndex + nbStr; )
    {
      Argument* arg = nameToArgument[str[i]];
      if (!arg)
      {
        std::cout << "Unknown argument: " << str[i] << std::endl;
        return false;
      }

      size_t argRead = arg->parse(str, i, startIndex + nbStr - 1);
      if (!argRead)
      {
        std::cout << "Error in argument: " << str[i] << std::endl;
        return false;
      }

      i += argRead;
    }
    
    return true;
  }
  
  String toString()
  {
    String toReturn;
    for (size_t i = 0; i < arguments.size(); ++i)
    {
      toReturn += arguments[i]->toString() + T(" ");
    }
    
    return toReturn;
  }
  
  friend std::ostream& operator<<(std::ostream& o, ArgumentSet a);
  
private:
  std::map<String, Argument*> nameToArgument;
  std::vector<Argument*> arguments;
};


std::ostream& operator<<(std::ostream& o, ArgumentSet args)
{
  for (size_t i = 0; i < args.arguments.size(); ++i)
  {
    o << "|> " << args.arguments[i]->name.toUTF8();
    o << ": " << args.arguments[i]->getStringValue().toUTF8() << std::endl;
  }
  
  return o;
}

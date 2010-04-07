/*-----------------------------------------.---------------------------------.
| Filename: ErrorManager.cpp               | A simple manager for compilation|
| Author  : Francis Maes                   |   errors and warnings           |
| Started : 11/02/2009 14:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ErrorManager.h"

struct StringError : public Error
{
  StringError(const std::string& str) : str(str) {}
  
  std::string str;
  
  virtual void write(std::ostream& ostr) const
    {ostr << str;}
};

struct LocalizedError : public Error
{
  LocalizedError(const std::string& errorMessage, const std::string& filename, size_t lineNumber = (size_t)-1, bool isWarning = false)
    : errorMessage(errorMessage), filename(filename), lineNumber(lineNumber), isWarning(isWarning) {}
  
  std::string errorMessage;
  std::string filename;
  size_t lineNumber;
  bool isWarning;
  
  virtual void write(std::ostream& ostr) const
  {
    ostr << filename;
    if (lineNumber != (size_t)-1)
    {
#ifdef WIN32
      ostr << '(' << lineNumber << ") ";
#else
      ostr << ':' << lineNumber;
#endif
    }
    ostr << ": " << (isWarning ? "warning" : "error") << ": " << errorMessage;
  }
};

/*
** ErrorManager
*/
ErrorManager& ErrorManager::getInstance()
  {static ErrorManager e; return e;}

void ErrorManager::clear()
{
  for (size_t i = 0; i < errors.size(); ++i)
    delete errors[i];
  errors.clear();
}

void ErrorManager::write(std::ostream& ostr)
{
  for (size_t i = 0; i < errors.size(); ++i)
  {
    errors[i]->write(ostr);
    ostr << std::endl;
  }
}

void ErrorManager::addError(const std::string& message)
  {failure = true; errors.push_back(new StringError(message));}
  
void ErrorManager::addError(const std::string& message, const std::string& filename, size_t lineNumber)
  {failure = true; errors.push_back(new LocalizedError(message, filename, lineNumber));}

void ErrorManager::addError(const std::string& message, PTree::Node* node, bool isWarning)
{
  failure |= !isWarning;
  assert(currentSourceBuffer);
  try
  {
    std::string filename;
    unsigned long lineNumber = currentSourceBuffer->origin(node->begin(), filename);
    errors.push_back(new LocalizedError(message, filename, lineNumber, isWarning));
  }
  catch (std::exception& )
    {errors.push_back(new StringError(message));}
}

void ErrorManager::addError(Error* newError)
  {failure = true; errors.push_back(newError);}
  
void ErrorManager::addErrors(const std::vector<Error* >& errors)
{
  for (size_t i = 0; i < errors.size(); ++i)
  {
    failure = true;
    // clone the error
    std::ostringstream ostr;
    errors[i]->write(ostr);
    this->errors.push_back(new StringError(ostr.str()));
  }
}

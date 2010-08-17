/*-----------------------------------------.---------------------------------.
| Filename: ErrorManager.h                 | A simple manager for compilation|
| Author  : Francis Maes                   |   errors and warnings           |
| Started : 11/02/2009 14:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_TOOLS_ERROR_MANAGER_H_
# define CRALGORITHM_TOOLS_ERROR_MANAGER_H_

# include "../common.h"

typedef Parser::Error Error;

class ErrorManager
{
public:
  ~ErrorManager() {clear();}
  
  static ErrorManager& getInstance();
  
  void clear();
  void write(std::ostream& ostr);
  
  void addError(const std::string& message);
  void addError(const std::string& message, const std::string& filename, size_t lineNumber = (size_t)-1);
  void addError(const std::string& message, PTree::Node* node, bool isWarning = false);
  
  void addError(Error* newError);
  void addErrors(const std::vector<Error* >& errors);
  
  void addWarning(const std::string& message, PTree::Node* node)
    {addError(message, node, true);}
  
  void setCurrentSourceBuffer(Buffer& buffer)
    {currentSourceBuffer = &buffer;}

  bool containFatalErrors() const
    {return failure;}
    
  size_t getNumErrors() const
    {return errors.size();}
    
private:
  ErrorManager() : currentSourceBuffer(NULL), failure(false) {}
  
  std::vector<Error* > errors;
  Buffer* currentSourceBuffer;
  bool failure;
};


#endif // !CRALGORITHM_TOOLS_ERROR_MANAGER_H_

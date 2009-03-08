/*-----------------------------------------.---------------------------------.
| Filename: ErrorHandler.cpp               | Error handling                  |
| Author  : Francis Maes                   |                                 |
| Started : 06/03/2009 17:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/ErrorHandler.h>
#include <iostream>
using namespace cralgo;

class DefaultErrorHandler : public ErrorHandler
{
public:
  virtual void errorMessage(const std::string& where, const std::string& what)
    {std::cerr << "Error in '" << where << "': " << what << "." << std::endl;}  
};

static DefaultErrorHandler defaultErrorHandler;

ErrorHandler* ErrorHandler::instance = &defaultErrorHandler;

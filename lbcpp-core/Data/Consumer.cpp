/*-----------------------------------------.---------------------------------.
| Filename: Consumer.cpp                   | Consumer Classes                |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2010 15:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <lbcpp/Data/Consumer.h>
#include <lbcpp/Data/Stream.h>
#include <lbcpp/Core/Container.h>
#include <fstream>
using namespace lbcpp;

/*
** Consumer
*/
void Consumer::consumeStream(ExecutionContext& context, StreamPtr stream, size_t maximumCount)
{
  for (size_t i = 0; !maximumCount || i < maximumCount; ++i)
  {
    Variable variable = stream->next();
    if (!variable.exists())
      break;
    consume(context, variable);
  }
}

void Consumer::consumeContainer(ExecutionContext& context, ContainerPtr container)
{
  size_t s = container->getNumElements();
  for (size_t i = 0; i < s; ++i)
    consume(context, container->getElement(i));
}

/*
** TextPrinter
*/
TextPrinter::TextPrinter(OutputStream* newOutputStream)
  : ostr(newOutputStream) {}

TextPrinter::TextPrinter(ExecutionContext& context, const File& file)
  : ostr(NULL)
{
  if (file == File::nonexistent)
  {
    context.errorCallback(T("TextPrinter::TextPrinter"), T("No filename specified"));
    return;
  }
  if (file.existsAsFile())
    file.deleteFile();
  OutputStream* outputStream = file.createOutputStream();
  if (!outputStream)
  {
    context.errorCallback(T("TextPrinter::TextPrinter"), T("Could not open file ") + file.getFullPathName());
    return;
  }
  this->ostr = outputStream;
}

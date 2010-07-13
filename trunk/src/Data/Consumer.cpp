
/*-----------------------------------------.---------------------------------.
 | Filename: Consumer.cpp                   | Consumer Classes                |
 | Author  : Francis Maes                   |                                 |
 | Started : 22/03/2010 15:52               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#include <lbcpp/Data/Consumer.h>
#include <lbcpp/Data/Stream.h>
#include <lbcpp/Data/Container.h>
#include <fstream>
using namespace lbcpp;

/*
 ** Consumer
 */
void Consumer::consumeStream(StreamPtr stream, size_t maximumCount)
{
  for (size_t i = 0; !maximumCount || i < maximumCount; ++i)
  {
    Variable variable = stream->next();
    if (!variable)
      break;
    consume(variable);
  }
}

void Consumer::consumeContainer(ContainerPtr container)
{
  size_t s = container->size();
  for (size_t i = 0; i < s; ++i)
    consume(container->getVariable(i));
}

/*
 ** TextPrinter
 */
TextPrinter::TextPrinter(OutputStream* newOutputStream)
  : ostr(newOutputStream) {}

TextPrinter::TextPrinter(const File& file, ErrorHandler& callback)
  : ostr(NULL)
{
  if (file == File::nonexistent)
  {
    callback.errorMessage(T("TextPrinter::TextPrinter"), T("No filename specified"));
    return;
  }
  if (file.existsAsFile())
    file.deleteFile();
  OutputStream* outputStream = file.createOutputStream();
  if (!outputStream)
  {
    callback.errorMessage(T("TextPrinter::TextPrinter"), T("Could not open file ") + file.getFullPathName());
    return;
  }
  this->ostr = outputStream;
}

void declareConsumerClasses()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(Consumer, Function);
    LBCPP_DECLARE_ABSTRACT_CLASS(TextPrinter, Consumer);
}

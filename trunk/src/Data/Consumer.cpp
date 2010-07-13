
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
void Consumer::consumeStream(ObjectStreamPtr stream, size_t maximumCount)
{
  for (size_t i = 0; !maximumCount || i < maximumCount; ++i)
  {
    ObjectPtr object = stream->next();
    if (!object)
      break;
    consume(object);
  }
}

void Consumer::consumeContainer(ContainerPtr container)
{
  size_t s = container->size();
  for (size_t i = 0; i < s; ++i)
    consume(container->get(i));
}

/*
 ** TextObjectPrinter
 */
TextPrinter::TextPrinter(OutputStream* newOutputStream)
: ostr(newOutputStream) {}

TextPrinter::TextPrinter(const File& file)
: ostr(NULL)
{
  if (file == File::nonexistent)
  {
    Object::error(T("TextPrinter::TextPrinter"), T("No filename specified"));
    return;
  }
  if (file.existsAsFile())
    file.deleteFile();
  OutputStream* outputStream = file.createOutputStream();
  if (!outputStream)
  {
    Object::error(T("TextPrinter::TextPrinter"), T("Could not open file ") + file.getFullPathName());
    return;
  }
  this->ostr = outputStream;
}

void declareConsumerClasses()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(Consumer, Function);
    LBCPP_DECLARE_CLASS(TextPrinter, Consumer);
}

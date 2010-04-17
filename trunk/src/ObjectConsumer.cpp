/*-----------------------------------------.---------------------------------.
| Filename: ObjectConsumer.cpp             | Object Consumer Classes         |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2010 15:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/ObjectConsumer.h>
#include <lbcpp/ObjectStream.h>
#include <lbcpp/ObjectContainer.h>
#include <lbcpp/FeatureVisitor.h>
#include <lbcpp/LearningExample.h>
#include <fstream>
using namespace lbcpp;

/*
** ObjectConsumer
*/
void ObjectConsumer::consumeStream(ObjectStreamPtr stream, size_t maximumCount)
{
  for (size_t i = 0; !maximumCount || i < maximumCount; ++i)
  {
    ObjectPtr object = stream->next();
    if (!object)
      break;
    consume(object);
  }
}

void ObjectConsumer::consumeContainer(ObjectContainerPtr container)
{
  size_t s = container->size();
  for (size_t i = 0; i < s; ++i)
    consume(container->get(i));
}

/*
** VectorObjectContainerFiller
*/
class VectorObjectContainerFiller : public ObjectConsumer
{
public:
  VectorObjectContainerFiller(VectorObjectContainerPtr container)
    : container(container) {}

  virtual void consume(ObjectPtr object)
    {container->append(object);}

private:
  VectorObjectContainerPtr container;
};

ObjectConsumerPtr lbcpp::vectorObjectContainerFiller(VectorObjectContainerPtr container)
  {return new VectorObjectContainerFiller(container);}

/*
** TextObjectPrinter
*/
TextObjectPrinter::TextObjectPrinter(OutputStream* newOutputStream)
  : ostr(newOutputStream) {}
    
TextObjectPrinter::TextObjectPrinter(const File& file)
  : ostr(NULL)
{
  if (file == File::nonexistent)
  {
    Object::error(T("TextObjectPrinter::TextObjectPrinter"), T("No filename specified"));
    return;
  }
  if (file.existsAsFile())
    file.deleteFile();
  OutputStream* outputStream = file.createOutputStream();
  if (!outputStream)
  {
    Object::error(T("TextObjectPrinter::TextObjectPrinter"), T("Could not open file ") + file.getFullPathName());
    return;
  }
  this->ostr = outputStream;
}

/*
** LearningDataObjectPrinter
*/
struct WriteFeatureListVisitor : public PathBasedFeatureVisitor
{
  WriteFeatureListVisitor(OutputStream& ostr) : ostr(ostr), isFirst(true) {}
  
  OutputStream& ostr;
  bool isFirst;
  
  virtual void featureSense(const std::vector<size_t>& path, const String& name, double value)
  {
    if (value)
    {
      if (isFirst)
        isFirst = false;
      else
        ostr << " ";
      ostr << (const char* )name << ":" << value;
    }
  }
};

void LearningDataObjectPrinter::printFeatureList(FeatureGeneratorPtr features)
  {features->accept(new WriteFeatureListVisitor(*ostr));}

/*
** ClassificationExamplesPrinter
*/
class ClassificationExamplesPrinter : public LearningDataObjectPrinter
{
public:
  ClassificationExamplesPrinter(const File& file, StringDictionaryPtr labels)
    : LearningDataObjectPrinter(file), labels(labels) {}
  
  virtual void consume(ObjectPtr object)
  {
    ClassificationExamplePtr example = object.dynamicCast<ClassificationExample>();
    jassert(example);
    print(/*labels->getString*/lbcpp::toString(example->getOutput()) + " ");
    printFeatureList(example->getInput());
    printNewLine();
  }
  
private:
  StringDictionaryPtr labels;
};

ObjectConsumerPtr lbcpp::classificationExamplesPrinter(const File& file, StringDictionaryPtr labels)
{
  jassert(labels);
  return new ClassificationExamplesPrinter(file, labels);
}

/*
** ObjectSaveToFileConsumer
*/
class ObjectSaveToFileConsumer : public ObjectConsumer
{
public:
  ObjectSaveToFileConsumer(const File& directory, const String& extension)
    : directory(directory), extension(extension) {}
  
  virtual void consume(ObjectPtr object)
    {object->saveToFile(directory.getChildFile(object->getName() + T(".") + extension));}

private:
  File directory;
  String extension;
};
  
ObjectConsumerPtr lbcpp::objectSaveToFileConsumer(const File& directory, const String& extension)
  {return new ObjectSaveToFileConsumer(directory, extension);}

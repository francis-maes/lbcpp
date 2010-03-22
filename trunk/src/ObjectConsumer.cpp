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
void ObjectConsumer::consume(ObjectStreamPtr stream, size_t maximumCount)
{
  for (size_t i = 0; !maximumCount || i < maximumCount; ++i)
  {
    ObjectPtr object = stream->next();
    if (!object)
      break;
    consume(object);
  }
}

void ObjectConsumer::consume(ObjectContainerPtr container)
{
  size_t s = container->size();
  for (size_t i = 0; i < s; ++i)
    consume(container->get(i));
}

/*
** TextObjectPrinter
*/
TextObjectPrinter::TextObjectPrinter(std::ostream* newOutputStream)
  : ostr(newOutputStream) {}
    
TextObjectPrinter::TextObjectPrinter(const std::string& filename)
  : ostr(NULL)
{
  if (filename.empty())
  {
    Object::error("TextObjectPrinter::TextObjectPrinter", "No filename specified");
    return;
  }
  std::ofstream* ostr = new std::ofstream(filename.c_str());
  if (ostr->is_open())
    this->ostr = ostr;
  else
  {
    Object::error("TextObjectPrinter::TextObjectPrinter", "Could not open file '" + filename + "'");
    delete ostr;
  }
}

/*
** LearningDataObjectPrinter
*/
struct WriteFeatureListVisitor : public PathBasedFeatureVisitor
{
  WriteFeatureListVisitor(std::ostream& ostr) : ostr(ostr), isFirst(true) {}
  
  std::ostream& ostr;
  bool isFirst;
  
  virtual void featureSense(const std::vector<size_t>& path, const std::string& name, double value)
  {
    if (isFirst)
      isFirst = false;
    else
      ostr << " ";
    if (value)
      ostr << name << ":" << value;
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
  ClassificationExamplesPrinter(const std::string& filename, StringDictionaryPtr labels)
    : LearningDataObjectPrinter(filename), labels(labels) {}
  
  virtual void consume(ObjectPtr object)
  {
    ClassificationExamplePtr example = object.dynamicCast<ClassificationExample>();
    assert(example);
    print(/*labels->getString*/lbcpp::toString(example->getOutput()) + " ");
    printFeatureList(example->getInput());
    printNewLine();
  }
  
private:
  StringDictionaryPtr labels;
};

ObjectConsumerPtr lbcpp::classificationExamplesPrinter(const std::string& filename, StringDictionaryPtr labels)
{
  assert(labels);
  return new ClassificationExamplesPrinter(filename, labels);
}

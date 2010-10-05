/*-----------------------------------------.---------------------------------.
| Filename: DynamicObject.cpp              | Dynamic Object                  |
| Author  : Francis Maes                   |                                 |
| Started : 26/08/2010 14:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/DynamicObject.h>
#include <lbcpp/Data/XmlSerialisation.h>
#include "Object/SparseGenericObject.h"
#include "Object/DenseGenericObject.h"
using namespace lbcpp;

VariableValue DynamicClass::create() const
{
  TypePtr pthis = refCountedPointerFromThis(this);
  if (true) // TEST !! (isSparse)
    return new SparseGenericObject(pthis);
  else
    return new DenseGenericObject(pthis);
}

Variable DynamicClass::getObjectVariable(const VariableValue& value, size_t index) const
{
  DynamicObjectPtr object = value.getObjectAndCast<DynamicObject>();
  jassert(object);
  return object->getVariableImpl(index);
}

void DynamicClass::setObjectVariable(const VariableValue& value, size_t index, const Variable& subValue) const
{
  if (checkInheritance(subValue.getType(), getObjectVariableType(index)))
  {
    DynamicObjectPtr object = value.getObjectAndCast<DynamicObject>();
    jassert(object);
    object->setVariableImpl(index, subValue);
  }
}

void DynamicClass::saveToXml(XmlExporter& exporter) const
{
  exporter.enter(T("class"));

  exporter.setAttribute(T("name"), getName());
  exporter.setAttribute(T("base"), getBaseType()->getName().replaceCharacters(T("<>"), T("[]")));
  for (size_t i = 0; i < variables.size(); ++i)
  {
    TypePtr type = variables[i].first;
    exporter.enter(T("variable"));
    exporter.setAttribute(T("name"), variables[i].second);
    exporter.writeType(type);
    exporter.leave();
  }
  exporter.leave();
}

bool DynamicClass::loadFromXml(XmlImporter& importer)
{
  variables.clear();
  if (!importer.enter(T("class")))
    return false;
  setName(importer.getStringAttribute(T("name")));
  baseType = Type::get(importer.getStringAttribute(T("base"), T("???")), importer.getCallback());
  if (!baseType)
    return false;

  bool res = true;
  forEachXmlChildElementWithTagName(*importer.getCurrentElement(), elt, T("variable"))
  {
    importer.enter(elt);
    TypePtr type = importer.loadType();
    if (type)
    {
      String name = elt->getStringAttribute(T("name"), T("???"));
      variables.push_back(std::make_pair(type, name));
    }
    else
      res = false;
    importer.leave();
  }

  importer.leave();
  return res;
}

///////////////////////////////////////////////////////////////////////////////////////

class SparseDynamicObject : public Object
{
public:
  SparseDynamicObject(TypePtr thisType)
    : Object(thisType), first(NULL), last(NULL), numElements(0)  {}
  
  ~SparseDynamicObject()
  {
    Node* nextNode;
    for (Node* node = first; node; node = nextNode)
    {
      nextNode = node->next;
      thisClass->getObjectVariableType(node->index)->destroy(node->value);
      delete node;
    }
    first = NULL;
    last = NULL;
  }

  size_t getNumElements() const
    {return numElements;}

  void append(size_t index, const Variable& value)
  {
    jassert(!last || last->index < index);
    if (checkInheritance(value, thisClass->getObjectVariableType(index)))
    {
      Node* node = new Node(index, value);
      if (last)
        last->next = node;
      else
        first = node;
      last = node;
      ++numElements;
    }
  }

private:
  struct Node
  {
    Node(size_t index, const Variable& value, Node* next = NULL)
      : index(index), next(next) {value.copyTo(this->value);}

    size_t index;
    VariableValue value;
    Node* next;
  };

  TypePtr elementsType;
  Node* first;
  Node* last;
  size_t numElements;
};

typedef ReferenceCountedObjectPtr<SparseDynamicObject> SparseDynamicObjectPtr;

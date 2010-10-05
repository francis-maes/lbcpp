/*-----------------------------------------.---------------------------------.
| Filename: SparseGenericObject.h          | Sparse Generic Object           |
| Author  : Francis Maes                   |                                 |
| Started : 05/10/2010 23:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_OBJECT_SPARSE_GENERIC_H_
# define LBCPP_DATA_OBJECT_SPARSE_GENERIC_H_

# include <lbcpp/Data/DynamicObject.h>
# include <lbcpp/Data/Stream.h>

namespace lbcpp
{

class SparseGenericObject : public DynamicObject
{
public:
  SparseGenericObject(TypePtr thisType)
    : DynamicObject(thisType), first(NULL), last(NULL), numElements(0)  {}
  virtual ~SparseGenericObject()
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
  
  virtual Variable getVariableImpl(size_t index) const
  {
    const Node* node = const_cast<SparseGenericObject* >(this)->findLastNodeBefore(index);
    TypePtr type = thisClass->getObjectVariableType(index);
    if (node && node->index == index)
      return Variable::copyFrom(type, node->value);
    else
      return Variable::missingValue(type);
  }

  virtual void setVariableImpl(size_t index, const Variable& value)
  {
    if (!last || last->index < index)
      appendNode(index, value);
    else
    {
      Node* lastBefore = findLastNodeBefore(index);
      if (!lastBefore)
        prependNode(index, value);
      else if (lastBefore->index == index)
        setNodeValue(lastBefore, value);
      else
        insertNodeAfter(index, value, lastBefore);
    }
  }

  virtual StreamPtr getVariablesStream() const;

  size_t getNumElements() const
    {return numElements;}

private:
  friend class SparseGenericObjectVariablesStream;

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

  void appendNode(size_t index, const Variable& value)
  {
    jassert(!last || last->index < index);
    Node* node = new Node(index, value);
    if (last)
      last->next = node;
    else
      first = node;
    last = node;
    ++numElements;
  }

  void prependNode(size_t index, const Variable& value)
  {
    jassert(first);
    Node* node = new Node(index, value, first);
    first = node;
    ++numElements;
  }

  void insertNodeAfter(size_t index, const Variable& value, Node* nodeBefore)
  {
    Node* newNode = new Node(index, value, nodeBefore->next);
    nodeBefore->next = newNode;
    if (last == nodeBefore)
      last = newNode;
    ++numElements;
  }

  void setNodeValue(Node* node, const Variable& value)
    {value.copyTo(node->value);}

  Node* findLastNodeBefore(size_t index)
  {
    Node* node = first;
    Node* res = NULL;
    while (node && node->index <= index)
    {
      res = node;
      node = node->next;
    }
    return res;
  }
};

typedef ReferenceCountedObjectPtr<SparseGenericObject> SparseGenericObjectPtr;

class SparseGenericObjectVariablesStream : public Stream
{
public:
  SparseGenericObjectVariablesStream(SparseGenericObjectPtr object)
    : object(object), current(object->first)
    {}

  virtual TypePtr getElementsType() const
    {return pairType(variableIndexType(), anyType());}

  virtual bool rewind()
    {current = object->first; return true;}

  virtual bool isExhausted() const
    {return !current;}

  virtual Variable next()
  {
    if (!current)
      return Variable();
    TypePtr type = object->thisClass->getObjectVariableType(current->index);
    Variable res = Variable::pair(Variable(current->index, variableIndexType()), Variable::copyFrom(type, current->value));
    current = current->next;
    return res;
  }

private:
  SparseGenericObjectPtr object;
  SparseGenericObject::Node* current;
};

inline StreamPtr SparseGenericObject::getVariablesStream() const
  {return new SparseGenericObjectVariablesStream(refCountedPointerFromThis(this));}

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_OBJECT_SPARSE_GENERIC_H_

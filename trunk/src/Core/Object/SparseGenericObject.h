/*-----------------------------------------.---------------------------------.
| Filename: SparseGenericObject.h          | Sparse Generic Object           |
| Author  : Francis Maes                   |                                 |
| Started : 05/10/2010 23:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_OBJECT_SPARSE_GENERIC_H_
# define LBCPP_DATA_OBJECT_SPARSE_GENERIC_H_

# include <lbcpp/Core/DynamicObject.h>
# include <lbcpp/Data/Stream.h>

namespace lbcpp
{

class SparseGenericObject : public Object
{
public:
  SparseGenericObject(DynamicClassPtr thisClass)
    : Object(thisClass), first(NULL), last(NULL), numElements(0)  {}

  virtual ~SparseGenericObject()
  {
    Node* nextNode;
    for (Node* node = first; node; node = nextNode)
    {
      nextNode = node->next;
      thisClass->getMemberVariableType(node->index)->destroy(node->value);
      delete node;
    }
    first = NULL;
    last = NULL;
  }
  
  virtual Variable getVariable(size_t index) const
  {
    const Node* node = const_cast<SparseGenericObject* >(this)->findLastNodeBefore(index);
    TypePtr type = thisClass->getMemberVariableType(index);
    if (node && node->index == index)
      return Variable::copyFrom(type, node->value);
    else
      return Variable::missingValue(type);
  }

  virtual void setVariable(ExecutionContext& context, size_t index, const Variable& value)
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

  VariableIterator* createVariablesIterator() const;

  size_t getNumElements() const
    {return numElements;}

  lbcpp_UseDebuggingNewOperator

private:
  friend class SparseGenericObjectVariableIterator;

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

class SparseGenericObjectVariableIterator : public Object::VariableIterator
{
public:
  SparseGenericObjectVariableIterator(SparseGenericObjectPtr object)
    : object(object), current(object->first)
    {}

  virtual bool exists() const
    {return current != NULL;}
  
  virtual Variable getCurrentVariable(size_t& index) const
  {
    jassert(current);
    index = current->index;
    return Variable::copyFrom(object->getVariableType(current->index), current->value);
  }

  virtual void next()
  {
    jassert(current);
    current = current->next;
  }

private:
  SparseGenericObjectPtr object;
  SparseGenericObject::Node* current;
};

inline Object::VariableIterator* SparseGenericObject::createVariablesIterator() const
  {return new SparseGenericObjectVariableIterator(refCountedPointerFromThis(this));}

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_OBJECT_SPARSE_GENERIC_H_

/*-----------------------------------------.---------------------------------.
| Filename: CollapsePerception.h           | A decorator to collapse a       |
| Author  : Francis Maes                   | Perception                      |
| Started : 05/10/2010 19:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_COLLAPSE_H_
# define LBCPP_FUNCTION_PERCEPTION_COLLAPSE_H_

# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class CollapsePerception : public Perception
{
public:
  CollapsePerception(PerceptionPtr decorated = PerceptionPtr())
    : decorated(decorated)
    {computeOutputType();}

  virtual TypePtr getInputType() const
    {return decorated->getInputType();}

  virtual String toString() const
    {return decorated->toString() + T(" collapsed");}
  
  virtual void computeOutputType()
  {
    precompute(decorated, String::empty, rootNode);
    Perception::computeOutputType();
  }

  struct Node
  {
    Node() : variableNumber(-1), parent(NULL) {}

    int variableNumber;
    std::vector<Node> childrens;
    Node* parent;
  };

  struct Callback : public PerceptionCallback
  {
    Callback(const CollapsePerception* owner, PerceptionCallbackPtr targetCallback)
      : owner(owner), targetCallback(targetCallback), currentNode(&owner->rootNode) {}

    virtual void sense(size_t variableNumber, double value)
      {}

    virtual void sense(size_t variableNumber, const ObjectPtr& value)
      {}

    virtual void sense(size_t variableNumber, const Variable& value)
      {}

    virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& input)
    {
      currentNode = &(currentNode->childrens[variableNumber]);
      jassert(currentNode);
      if (currentNode->variableNumber >= 0)
        targetCallback->sense(currentNode->variableNumber, subPerception, input);
      else
        subPerception->computePerception(input, PerceptionCallbackPtr(this));
      currentNode = currentNode->parent;
      jassert(currentNode);
    }

    const CollapsePerception* owner;
    PerceptionCallbackPtr targetCallback;
    const Node* currentNode;
  };

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr targetCallback) const
  {
    Callback callback(this, targetCallback);
    decorated->computePerception(input, &callback);
  }

  juce_UseDebuggingNewOperator

private:
  friend class CollapsePerceptionClass;

  PerceptionPtr decorated;
  Node rootNode;

  bool isLeafPerception(PerceptionPtr perception) const
  {
    size_t n = perception->getNumOutputVariables();
    for (size_t i = 0; i < n; ++i)
      if (perception->getOutputVariableSubPerception(i))
        return false;
    return true;
  }

  void precompute(PerceptionPtr perception, const String& fullName, Node& currentNode)
  {
    if (isLeafPerception(perception))
    {
      currentNode.variableNumber = (int)outputVariables.size();
      addOutputVariable(fullName, perception);
    }
    else
    {
      size_t n = perception->getNumOutputVariables();
      currentNode.childrens.resize(n);
      
      for (size_t i = 0; i < n; ++i)
      {
        PerceptionPtr subPerception = perception->getOutputVariableSubPerception(i);
        if (subPerception)
        {
          String newFullName = fullName;
          if (newFullName.isNotEmpty())
            newFullName += '.';
          newFullName += perception->getOutputVariableName(i);

          Node& childNode = currentNode.childrens[i];
          precompute(subPerception, newFullName, childNode);
          childNode.parent = &currentNode;
        }
      }
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_COLLAPSE_H_

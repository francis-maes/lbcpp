/*-----------------------------------------.---------------------------------.
| Filename: DecisionTreeExampleVector.h    | Decision Tree Example Vector    |
| Author  : Julien Becker                  |                                 |
| Started : 18/01/2011 09:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DECISION_TREE_EXAMPLE_VECTOR_H_
# define LBCPP_DECISION_TREE_EXAMPLE_VECTOR_H_

# include <lbcpp/Core/Variable.h>

namespace lbcpp
{

class DecisionTreeExampleVector
{
public:
  DecisionTreeExampleVector(std::vector< std::vector<Variable> >& attributes,
                            std::vector<Variable>& labels,
                            const std::vector<size_t>& indices = std::vector<size_t>())
    : attributes(attributes), labels(labels), indices(indices) {}
  
  DecisionTreeExampleVector subset(const std::vector<size_t>& newIndices) const
    {return DecisionTreeExampleVector(attributes, labels, newIndices);}
  
  size_t getNumExamples() const
    {return indices.size();}
  
  const Variable& getLabel(size_t index) const
    {jassert(index < indices.size()); return labels[indices[index]];}
  
  const Variable& getAttribute(size_t exampleIndex, size_t variableIndex) const
    {jassert(exampleIndex < indices.size()); return attributes[indices[exampleIndex]][variableIndex];}
  
  bool isAttributeConstant(size_t variableIndex) const;
  
  bool isLabelConstant(Variable& constantValue) const;
  
  const std::vector<size_t>& getIndices() const
    {return indices;}
  
private:
  std::vector< std::vector<Variable> >& attributes;
  std::vector<Variable>& labels;
  std::vector<size_t> indices;
};

}; 

#endif // !LBCPP_DECISION_TREE_EXAMPLE_VECTOR_H_

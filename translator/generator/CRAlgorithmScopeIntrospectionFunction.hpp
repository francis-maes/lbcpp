/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmScopeIntrospect..hpp| CR-algorithm introspection      |
| Author  : Francis Maes                   |   functions                     |
| Started : 02/02/2009 00:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_GENERATOR_SCOPE_INTROSPECTION_FUNCTION_H_
# define CRALGORITHM_GENERATOR_SCOPE_INTROSPECTION_FUNCTION_H_

class CRAlgorithmScopeIntrospectionFunction : public FunctionPTreeGenerator
{
public:
  void addCase(PTree::Node* caseImpl, const PTree::Node* sourceIdentifier)
  {
    BlockPTreeGenerator block;
    block.add(list(atom("/* variable "), const_cast<PTree::Node* >(sourceIdentifier), atom(" */ "), caseImpl));
    switchGenerator.addCase(literal(switchGenerator.getNumCases()), block.createContent());
  }
  
  void setDefault(PTree::Node* defaultImpl)
    {switchGenerator.setDefault(defaultImpl);}
  
  void prepare(const std::string& returnType, const std::string& name, bool isConst = true)
  {
    setReturnType(identifier(returnType));
    setName(name);
    PTree::Identifier* id = identifier("__index__");
    addParameter(identifier("size_t"), id);
    switchGenerator.setCondition(id);
    body.add(atom("assert(__index__ < " + size2str(switchGenerator.getNumCases()) + ");"));
    body.add(switchGenerator.createStatement());
    setConst(isConst);
  }
    
private:
  SwitchPTreeGenerator switchGenerator;
};

#endif // !CRALGORITHM_GENERATOR_SCOPE_INTROSPECTION_FUNCTION_H_

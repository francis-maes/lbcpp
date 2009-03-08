/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmScopeDestructor.hpp | CR-algorithm destructor         |
| Author  : Francis Maes                   |                                 |
| Started : 05/02/2009 20:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_GENERATOR_SCOPE_DESTRUCTOR_H_
# define CRALGORITHM_GENERATOR_SCOPE_DESTRUCTOR_H_

class CRAlgorithmScopeDestructor : public FunctionPTreeGenerator
{
public:
  CRAlgorithmScopeDestructor(PTree::Identifier* classIdentifier, const std::vector<CRAlgorithmLocalScope>& localScopes)
  {
    setReturnType(atom("~"));
    setIdentifier(classIdentifier);
    
    for (size_t i = 0; i < localScopes.size(); ++i)
    {
      const CRAlgorithmLocalScope& localScope = localScopes[i];
      
      // if (__localScope0__) delete __localScope__
      std::string name = localScope.getVariableName();
      body.addExpressionStatement(atom("if (" + name + ") delete " + name));
    }
    body.add(NULL);
  }  
};

#endif // !CRALGORITHM_GENERATOR_SCOPE_DESTRUCTOR_H_

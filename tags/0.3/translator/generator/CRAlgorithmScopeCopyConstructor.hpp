/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmScopeCopyConstr..hpp| CR-algorithm copy constructor   |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2009 00:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_GENERATOR_SCOPE_COPY_CONSTRUCTOR_H_
# define CRALGORITHM_GENERATOR_SCOPE_COPY_CONSTRUCTOR_H_

class CRAlgorithmScopeCopyConstructor : public ConstructorPTreeGenerator
{
public:
  CRAlgorithmScopeCopyConstructor(PTree::Identifier* classIdentifier,
                                  const std::vector<ParameterPTreeAnalyser>& parameters,
                                  const std::vector<ParameterPTreeAnalyser>& variables,
                                  const std::vector<CRAlgorithmLocalScope>& localScopes)
  {
    setIdentifier(classIdentifier);
    PTree::Identifier* other = identifier("other");
    addParameter(list(constKeyword(), classIdentifier, atom("&")), other);
    
    for (size_t i = 0; i < localScopes.size(); ++i)
    {
      const CRAlgorithmLocalScope& localScope = localScopes[i];

     // __localScope0__(other.__localScope0__ ? new __LocalScope0__(*other.__localScope0__) : NULL)
      PTree::Identifier* id = identifier(localScope.getVariableName());
      PTree::Node* otherDotId = dotMemberExpr(other, id);
      addInitializer(id, condExpr(otherDotId,
        list(newKeyword(), atom(localScope.getClassName() + "(*"), otherDotId, atom(")")),
        atom("NULL")), true);
    }
    
    for (size_t i = 0; i < parameters.size(); ++i)
     addCopyInitializer(parameters[i].getIdentifier());
    for (size_t i = 0; i < variables.size(); ++i)
     addCopyInitializer(variables[i].getIdentifier());
    addCopyInitializer(identifier("__state__"));
    
    body.add(NULL);
  }
  
private:
  void addCopyInitializer(PTree::Identifier* id, bool onANewLine = false)
    {addInitializer(id, dotMemberExpr(identifier("other"), id), onANewLine);}
};

#endif // !CRALGORITHM_GENERATOR_SCOPE_COPY_CONSTRUCTOR_H_

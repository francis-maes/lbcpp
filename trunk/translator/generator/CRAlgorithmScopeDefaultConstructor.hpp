/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmScopeDefaultCon..hpp| CR-algorithm default            |
| Author  : Francis Maes                   |   constructor                   |
| Started : 02/02/2009 00:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_GENERATOR_SCOPE_DEFAULT_CONSTRUCTOR_H_
# define CRALGORITHM_GENERATOR_SCOPE_DEFAULT_CONSTRUCTOR_H_

class CRAlgorithmScopeDefaultConstructor : public ConstructorPTreeGenerator
{
public:
  CRAlgorithmScopeDefaultConstructor(PTree::Identifier* classIdentifier,
                                      const std::vector<ParameterPTreeAnalyser>& parameters,
                                      const std::vector<ParameterPTreeAnalyser>& variables,
                                      const std::set<std::string>& variablesInitializedByAssignment,
                                      const std::vector<CRAlgorithmLocalScope>& localScopes,
                                      CRAlgorithmVariableTranslator& translator,
                                      SymbolLookup::Scope* currentScope)
  {
    setIdentifier(classIdentifier);
    
    PTree::Node* nullAtom = atom("NULL");
    for (size_t i = 0; i < localScopes.size(); ++i)
     // __localScope0__(NULL)
      addInitializer(identifier(localScopes[i].getVariableName()), nullAtom);

    if (currentScope != translator.getCRAlgorithmScope())
      addParameter(atom(translator.getCRAlgorithmClassName()), atom("&__crAlgorithm__"));

    for (size_t i = 0; i < parameters.size(); ++i)
    {
      const ParameterPTreeAnalyser& param = parameters[i];
      if (variablesInitializedByAssignment.find(param.getIdentifierString()) == variablesInitializedByAssignment.end())
        addInitializer(param.getIdentifier(), param.getIdentifier(), true);
      addParameter(param.getType(), param.getDeclarator());
    }
    
    for (size_t i = 0; i < variables.size(); ++i)
    {
      const ParameterPTreeAnalyser& var = variables[i];
      PTree::Node* initialValue = var.getInitializationArguments(true);
      if (variablesInitializedByAssignment.find(var.getIdentifierString()) == variablesInitializedByAssignment.end() && initialValue)
      {
        if (currentScope != translator.getCRAlgorithmScope())
        {
          CRAlgorithmVariableTranslatorVisitor translatorVisitor(translator);
          initialValue = translatorVisitor.translateVariables(initialValue, currentScope, true);
        }
        addInitializer(var.getIdentifier(), initialValue, true);
      }
      else
        addInitializer(var.getIdentifier(), NULL, true);
    } 
         
    // __state__(-1)
    addInitializer(identifier("__state__"), atom("-1"));
    body.add(NULL);
  }
};

#endif // !CRALGORITHM_GENERATOR_SCOPE_DEFAULT_CONSTRUCTOR_H_

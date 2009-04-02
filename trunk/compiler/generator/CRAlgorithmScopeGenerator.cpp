/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmScopeGenerator.cpp  | CR-algorithm scope generator    |
| Author  : Francis Maes                   |                                 |
| Started : 03/02/2009 18:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "CRAlgorithmScopeGenerator.h"

#include "ChooseClassGenerator.h"

#include "CRAlgorithmScopeCopyConstructor.hpp"
#include "CRAlgorithmScopeDefaultConstructor.hpp"
#include "CRAlgorithmScopeDestructor.hpp"
#include "CRAlgorithmScopeIntrospectionFunction.hpp"
#include "CRAlgorithmStateFunctionClass.hpp"
#include "CRAlgorithmScopeRunFunction.hpp"
#include "CRAlgorithmScopeStepFunction.hpp"
#include "CRAlgorithmScopeEnumerator.hpp"

#include "../tools/RecursiveVisitor.h"
#include <algorithm>


/*
** lcpp::CRAlgorithmScopePtr getCurrentInnerScope() {...}
*/
class GetCurrentInnerScopeGenerator : public FunctionPTreeGenerator
{
public:
  void prepare(const std::vector<CRAlgorithmLocalScope>& localScopes)
  {
    setReturnType(atom("lcpp::CRAlgorithmScopePtr"));
    setName("getCurrentInnerScope");
    SwitchPTreeGenerator switchgen;
    switchgen.setCondition(atom("__state__"));
    for (size_t i = 0; i < localScopes.size(); ++i)
      switchgen.addCase(literal(i), returnStatement(atom("lcpp::staticToDynamicCRAlgorithmScope(" + localScopes[i].getVariableName() + ")")));
    switchgen.setDefault(returnStatement(atom("lcpp::CRAlgorithmScopePtr()")));
    body.add(switchgen.createStatement());
  }
};

void CRAlgorithmScopeGenerator::prepare(CRAlgorithmVariableTranslator& translator, PTree::Node* block, SymbolLookup::Scope* scope, const std::string& className)
{
  this->scope = scope;
  setName(className);

  /*
  ** Analyse scopes
  */
  for (SymbolLookup::Scope::scope_iterator it = scope->scopes_begin(); it != scope->scopes_end(); ++it)
  {
    SymbolLookup::LocalScope* localScope = dynamic_cast<SymbolLookup::LocalScope* >(it->second);
    if (localScope)
      localScopes.push_back(CRAlgorithmLocalScope(const_cast<PTree::Node* >(it->first), localScope));
  }
  std::sort(localScopes.begin(), localScopes.end());
  
  /*
  ** List chooses, typedefs and variables
  */
  std::vector<PTree::Node* > typedefs;
  CRAlgorithmScopeEnumerator enumerator;
  enumerator.enumerate(block, scope, chooses, typedefs, variables, variablesInitializedByAssignment);

  /*
  ** Sort and merge variables and parameters
  */
  std::sort(parameters.begin(), parameters.end());
  std::sort(variables.begin(), variables.end());
  for (size_t i = 0; i < parameters.size(); ++i)
    parametersAndVariables.push_back(parameters[i]);
  for (size_t i = 0; i < variables.size(); ++i)
    parametersAndVariables.push_back(variables[i]);

  /*
  ** Typedefs
  */
  if (typedefs.size())
  {
    beginPart("Typedefs", publicKeyword());
    body.add(typedefs);
    endPart();
  }
  
  /*
  ** Generate classes for sub-scopes
  */
  for (size_t i = 0; i < localScopes.size(); ++i)
  {
    static size_t subscopeNumber = 0;
    CRAlgorithmLocalScope& subScope = localScopes[i];
    subScope.setNumber(i);
    subScope.setName(size2str(subscopeNumber++));
    
    translator.addLocalScopeVariable(subScope.getScope(), subScope.getVariableName());
    if (body.isEmpty())
      beginPart("Local Scopes", publicKeyword());
    CRAlgorithmScopeGenerator subScopeGenerator;
    subScopeGenerator.prepare(translator, subScope);
    body.add(subScopeGenerator.createDeclaration());
  }  
 
  /*
  ** Generate constructors / destructor
  */
  beginPart("Constructors / Destructor", publicKeyword());
  CRAlgorithmScopeCopyConstructor copyConstructor(getIdentifier(), parameters, variables, localScopes);
  customizeCopyConstructor(copyConstructor);
  body.add(copyConstructor.createDeclaration());
  CRAlgorithmScopeDefaultConstructor defaultConstructor(getIdentifier(), parameters, variables, variablesInitializedByAssignment, localScopes, translator, scope);
  customizeDefaultConstructor(defaultConstructor);
  body.add(defaultConstructor.createDeclaration());
  
  CRAlgorithmScopeDestructor destructor(getIdentifier(), localScopes);
  customizeDestructor(destructor);
  body.add(destructor.createDeclaration());
  endPart();
  
  /*
  ** Generate State Functions
  */
  beginPart("State Functions", publicKeyword());
  for (SymbolLookup::Scope::scope_iterator it = scope->scopes_begin(); it != scope->scopes_end(); ++it)
  {
    SymbolLookup::UserScope* userScope = dynamic_cast<SymbolLookup::UserScope* >(it->second);
    if (userScope)
    {
      if (*PTree::first(it->first) == CRAlgoToken::chooseFunction)
      {
        CRAlgo::StateFundefStatement* fundef = dynamic_cast<CRAlgo::StateFundefStatement* >(const_cast<PTree::Node* >(it->first));
        assert(fundef);
        CRAlgorithmStateFunctionClass traitsClass(translator, fundef, userScope);
        traitsClass.createCode(body);
      }
    }
  }
      
  /*
  ** Generate choose traits
  */
  if (chooses.size())
  {
    beginPart("Chooses", publicKeyword());
    for (size_t i = 0; i < chooses.size(); ++i)
    {
      ChooseClassGenerator chooseClassGenerator(chooses[i], scope, translator.getCRAlgorithmClassName());
      body.add(chooseClassGenerator.createDeclaration());
    }
    endPart();
  }
 

  /*
  ** Generate introspection functions
  */
  beginPart("Introspection", publicKeyword());
  addIntrospectionMembers();
  GetCurrentInnerScopeGenerator g;
  g.prepare(localScopes);
  body.add(g.createDeclaration());
  endPart();
  
  /*
  ** Generate step() function
  */
  CRAlgorithmScopeStepFunction stepFunction(translator);
  stepFunction.prepare(block, scope, className, localScopes, chooses, variablesInitializedByAssignment);
  body.add(stepFunction.createDeclaration());

  /*
  ** Generate run() function
  */
//  CRAlgorithmScopeRunFunction runFunction(block, scope);
//  body.add(runFunction.createDeclaration());

  /*
  ** Generate local members
  */
  addLocalScopeMembers();
  addLocalVariableMembers();
}

void CRAlgorithmScopeGenerator::addIntrospectionMembers()
{
  // getNumVariables()
  {
    std::ostringstream ostr;
    ostr << "size_t getNumVariables() const  {return " << parametersAndVariables.size() << ";}" << std::endl;
    body.add(atom(ostr.str()));
  }

  // getVariableType()
  {
    CRAlgorithmScopeIntrospectionFunction getVariableType;
    for (size_t i = 0; i < parametersAndVariables.size(); ++i)
      getVariableType.addCase(returnStatement(atom(quote(parametersAndVariables[i].getTypeString()))), parametersAndVariables[i].getIdentifier());
    getVariableType.setDefault(returnStatement(atom(quote(""))));
    getVariableType.prepare("std::string", "getVariableType");
    body.add(getVariableType.createDeclaration());
  }

  // getVariableName()
  {
    CRAlgorithmScopeIntrospectionFunction getVariableName;
    for (size_t i = 0; i < parametersAndVariables.size(); ++i)
      getVariableName.addCase(returnStatement(atom(quote(parametersAndVariables[i].getIdentifierString()))), parametersAndVariables[i].getIdentifier());
    getVariableName.setDefault(returnStatement(atom(quote(""))));
    getVariableName.prepare("std::string", "getVariableName");
    body.add(getVariableName.createDeclaration());
  }

  // getVariableValue()
  {
    CRAlgorithmScopeIntrospectionFunction getVariableValue;
    for (size_t i = 0; i < parametersAndVariables.size(); ++i)
    {
      ParameterPTreeAnalyser& v = parametersAndVariables[i];
      getVariableValue.addCase(returnStatement(funcallExpr(atom("lcpp::toString"), v.getIdentifier())), v.getIdentifier());
    }
    getVariableValue.setDefault(returnStatement(atom(quote(""))));
    getVariableValue.prepare("std::string", "getVariableValue");
    body.add(getVariableValue.createDeclaration());
  }

  // getVariablePointer()
  {
    CRAlgorithmScopeIntrospectionFunction getVariablePointer;
    for (size_t i = 0; i < parametersAndVariables.size(); ++i)
      getVariablePointer.addCase(returnStatement(atom("&" + parametersAndVariables[i].getIdentifierString())), parametersAndVariables[i].getIdentifier());
    getVariablePointer.setDefault(returnStatement(atom("NULL")));
    getVariablePointer.prepare("const void*", "getVariablePointer");
    body.add(getVariablePointer.createDeclaration());
  }  
}

void CRAlgorithmScopeGenerator::addLocalScopeMembers()
{
  if (localScopes.size())
  {
    beginPart("Local Scopes", publicKeyword());
    for (size_t i = 0; i < localScopes.size(); ++i)
    {
      CRAlgorithmLocalScope& s = localScopes[i];
      body.addVariableDeclaration(identifier(s.getClassName()),
        identifier("*" + s.getVariableName()));
    }
    endPart();
  }
}

void CRAlgorithmScopeGenerator::addLocalVariableMembers()
{
  beginPart("Local Variables", publicKeyword());
  for (size_t i = 0; i < parametersAndVariables.size(); ++i)
    body.addVariableDeclaration(parametersAndVariables[i].getType(), parametersAndVariables[i].getDeclaratorWithoutInitialValue());
  body.addVariableDeclaration(intKeyword(), identifier("__state__"));
  endPart();
}

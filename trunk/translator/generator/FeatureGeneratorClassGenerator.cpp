/*-----------------------------------------.---------------------------------.
| Filename: FeatureGeneratorGenerator.cpp  | Feature generator generator     |
| Author  : Francis Maes                   |                                 |
| Started : 18/02/2009 13:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "FeatureGeneratorClassGenerator.h"
#include "../tools/ErrorManager.h"
#include "../tools/ScopeBasedRewriteVisitor.h"
#include "../language/CRAlgoPTree.h"
#include "../language/CRAlgoLexer.h"

class FeatureGeneratorRewriteVisitor : public ScopeBasedRewriteVisitor, public PTreeGenerator
{
public:
  FeatureGeneratorRewriteVisitor(FeatureGeneratorClassGenerator::Location location, SymbolLookup::Class* classScope, SymbolLookup::Scope* scope, PTree::Identifier* dictionaryVariable)
    : ScopeBasedRewriteVisitor(scope), location(location), classScope(classScope), functionScope(scope), enabled(true) {dictionaryStack.push_back(dictionaryVariable);}
  
  // featureScope, featureCall
  virtual void visit(PTree::UserStatement* node)
  {
    pushScope(node);
    CRAlgo::FeatureScopeStatement* featureScope = dynamic_cast<CRAlgo::FeatureScopeStatement* >(node);
    if (featureScope)
    {
      setResult(createFeatureScopeCode(featureScope->getArguments(), featureScope->getBody()));
      popScope();
      return;
    }
    
    CRAlgo::FeatureGeneratorCallStatement* featureGeneratorCall = dynamic_cast<CRAlgo::FeatureGeneratorCallStatement* >(node);
    if (featureGeneratorCall)
    {
      setResult(createFeatureCallCode(featureGeneratorCall));
      popScope();
      return;
    }
    RewriteVisitor::visit(node);
    popScope();
  }
  
  // featureSense
  virtual void visit(PTree::FuncallExpr* node)
  {
    PTree::Node* identifier = PTree::first(node);
    if (CRAlgo::isFeatureSense(identifier))
      setResult(createFeatureSenseCode(PTree::third(node)));
    else
      RewriteVisitor::visit(node);
  }
  
  
  /*
  ** Class member identifier translations
  */
  // [postfix].[name] => rewrite([postfix]).[name]
  virtual void visit(PTree::DotMemberExpr* expr)
  {
    setResult(new PTree::DotMemberExpr(rewrite(PTree::first(expr)),
        PTree::list(rewriteWithoutTranslation(PTree::second(expr)),
                    rewriteWithoutTranslation(PTree::third(expr)))));
  }

  // [postfix]->[name] => rewrite([postfix])->[name]
  virtual void visit(PTree::ArrowMemberExpr* expr)
  {
    setResult(new PTree::ArrowMemberExpr(rewrite(PTree::first(expr)),
        PTree::list(rewriteWithoutTranslation(PTree::second(expr)),
                    rewriteWithoutTranslation(PTree::third(expr)))));
  }
  
  virtual void visit(PTree::Declaration* decl)
  {
    setResult(new PTree::Declaration(PTree::first(decl), PTree::list(
      PTree::second(decl), rewrite(PTree::third(decl)), PTree::nth(decl, 3))));
  }

  virtual void visit(PTree::CastExpr* expr)
  {
    setResult(new PTree::CastExpr(PTree::first(expr), PTree::list(
      PTree::second(expr), PTree::third(expr), rewrite(PTree::nth(expr, 3)))));
  }

  bool shouldAddThisAccess(PTree::Identifier* node)
  {
    if (!enabled || !classScope)
      return false;

    const SymbolLookup::Symbol* symbol = simpleSymbolLookup(getCurrentScope(), node);

    if (!symbol)
      return false;

    bool isVariable = dynamic_cast<const SymbolLookup::VariableName* >(symbol) != NULL;
    bool isFunction = dynamic_cast<const SymbolLookup::FunctionName* >(symbol) != NULL;
    if (!isVariable && !isFunction)
      return false;

    SymbolLookup::Class* symbolClass = dynamic_cast<SymbolLookup::Class* >(symbol->scope());
    return symbolClass && isBaseClass(symbolClass, classScope);
  }

  virtual void visit(PTree::Identifier* node)
  {
    if (shouldAddThisAccess(node))
      setResult(arrowMemberExpr(identifier("__this__"), node));
    else
      setResult(node);
  }

  virtual void visit(PTree::Kwd::This* node)
  {
    if (location == FeatureGeneratorClassGenerator::inClassDefinition ||
        location == FeatureGeneratorClassGenerator::inClassDefinitionStatic ||
        location == FeatureGeneratorClassGenerator::inClassImplementation)
      setResult(identifier("__this__"));
    else
    {
      ErrorManager::getInstance().addError("Cannot use 'this' outside classes", node);
      setResult(node);
    }
  }

protected:
  PTree::Node* rewriteWithoutTranslation(PTree::Node* node)
  {
    bool oEnabled = enabled;
    enabled = false;
    PTree::Node* res = rewrite(node);
    enabled = oEnabled;
    return res;
  }
  
private:
  FeatureGeneratorClassGenerator::Location location;

  SymbolLookup::Class* classScope;
  SymbolLookup::Scope* functionScope;
  bool enabled;
  std::vector<PTree::Identifier* > dictionaryStack;

  PTree::Node* rewriteScopeOrFeatureIdentifier(PTree::Node* identifier)
  {
    if (dynamic_cast<PTree::Literal* >(identifier))
    {
      std::string lit = PTree::reify(identifier);
      if (lit.find_first_not_of("0123456789") == std::string::npos)
        return list(atom("(size_t)"), identifier);
    }
    return rewrite(identifier);
  }

  PTree::Node* createFeatureSenseCode(PTree::Node* arguments)
  {
    FuncallPTreeGenerator output;
    output.setName("__featureVisitor__.featureSense_");
    output.addArgument(list(dictionaryStack.back()));
    bool isFirst = true;
    for (PTree::Iterator it(arguments); !it.empty(); ++it)
      if (**it != ",")
      {
        if (isFirst)
        {
          output.addArgument(rewriteScopeOrFeatureIdentifier(*it));
          isFirst = false;
        }
        else
          output.addArgument(rewrite(*it));
      }
    return output.createExpression();
  }
  
  PTree::Node* createFeatureScopeCode(PTree::Node* scopeArguments, PTree::Node* scopeBody)
  {
    PTree::Node* rewritedScopeArgument = rewriteScopeOrFeatureIdentifier(scopeArguments);

    BlockPTreeGenerator scopeBlock;
    static size_t uniqueId = 0;
    // lbcpp::FeatureDictionaryPtr __currentFeatureDictionaryXX__ = previousDictionary->getSubDictionary(...);
    PTree::Identifier* newDictionaryIdentifier = identifier("__currentFeatureDictionary" + size2str(++uniqueId) + "__");
    scopeBlock.addVariableDeclaration(atom("lbcpp::FeatureDictionaryPtr"), newDictionaryIdentifier, 
      list(dictionaryStack.back(), atom("->getSubDictionary("), rewritedScopeArgument, atom(")")));

    // __featureVisitor__.featureEnter(dictionary, [argument])
    FuncallPTreeGenerator featureEnterCall;
    featureEnterCall.setName("__featureVisitor__.featureEnter_");
    featureEnterCall.addArgument(dictionaryStack.back());
    featureEnterCall.addArgument(rewritedScopeArgument);
    featureEnterCall.addArgument(newDictionaryIdentifier);
    PTree::Node* condition = featureEnterCall.createExpression();
  
    BlockPTreeGenerator block;

    // [body];
    dictionaryStack.push_back(newDictionaryIdentifier);
    block.add(BlockPTreeAnalyser(rewrite(scopeBody)).getContent());
    dictionaryStack.pop_back();
    
    // __feature_visitor__.featureLeave();
    block.addExpressionStatement(funcallExpr(atom(" __featureVisitor__.featureLeave_")));
        
    // if (...) {[body] ...}
    BlockPTreeGenerator ifblock;
    ifblock.add(ifStatement(condition, block.createBlock()));

    scopeBlock.add(ifblock.createBlock());
    return scopeBlock.createBlock();
  }
  
  PTree::Node* createFeatureCallCode(CRAlgo::FeatureGeneratorCallStatement* featureGeneratorCall)
  {
    PTree::Node* rewritedIdentifier = rewriteScopeOrFeatureIdentifier(featureGeneratorCall->getIdentifier());

    PTree::Node* expression = featureGeneratorCall->getExpression();

    // FIXME: inline calls are not supported anymore, since new featureCall(expr) syntax 
   /* if (featureGeneratorCall->isInlineCall())
    {
      PTree::Node* id = PTree::first(expression);
      PTree::Node* args = PTree::third(expression);
      
      std::string identifier = PTree::reify(rewrite(id)) + "InlineCall";
      FuncallPTreeGenerator funcall;
      funcall.setName(identifier);
      funcall.addArgument(atom("__featureVisitor__"));
      funcall.addArgument(dictionaryStack.back());
      funcall.addArguments(rewrite(args));
      return exprStatement(funcall.createExpression());
    }
    else*/
    {
      FuncallPTreeGenerator funcall;
      funcall.setName("__featureVisitor__.featureCall_");
      funcall.addArgument(rewrite(expression));
      funcall.addArgument(dictionaryStack.back());
      funcall.addArgument(rewritedIdentifier);
      return exprStatement(funcall.createExpression());
    }
  }
};

/*
** FeatureGeneratorClassGenerator
*/
FeatureGeneratorClassGenerator::FeatureGeneratorClassGenerator(PTree::FunctionDefinition* node, SymbolLookup::Scope* scope, bool isInCRAlgorithm)
  : input(node), parameters(input.getParameters()), isInCRAlgorithm(isInCRAlgorithm)
{
  classScope = const_cast<SymbolLookup::Class* >(dynamic_cast<const SymbolLookup::Class* >(scope->outer_scope()));

  inputFunctionIdentifier = input.getIdentifierString();

  if (input.hasBody())
  {
    if (classScope)
    {
      inputClassIdentifier = classScope->name();
      location = input.getModifiers().contains("static") ? inClassDefinitionStatic : inClassDefinition;
    }
    else
    {
      if (input.isPartOfClassImplementation(inputClassIdentifier, inputFunctionIdentifier))
        location = inClassImplementation;
      else
        location = input.getModifiers().contains("static") ? outOfClassStatic : outOfClass;
    }
  }
  else
    location = prototypeOnly;

  //static const char* strings[] = {"prototypeOnly", "outOfClass", "outOfClassStatic", "inClassDefinition", "inClassDefinitionStatic", "inClassImplementation"};
  //std::cout << "Name: " << input.getIdentifierString() << " location = " << strings[location] << " classId = " << inputClassIdentifier << " functionId = " << inputFunctionIdentifier << std::endl;

  bool addThisParameter = (location == inClassDefinition || location == inClassImplementation);
  if (addThisParameter)
  {
    PTree::Identifier* classId = identifier(inputClassIdentifier);
    PTree::Node* type;
    if (input.isConst())
      type = list(constKeyword(), classId);
    else
      type = classId;
    parameters.pushFront(parameterDeclaration(type, list(atom("*"), identifier("__this__"))));
  }

   
  // struct <name>FeatureGenerator 
  generatedClassIdentifier = inputFunctionIdentifier + "FeatureGenerator";
  if (inputClassIdentifier.size())
    generatedClassIdentifier = inputClassIdentifier + "__" + generatedClassIdentifier;
  setKeyword(structKeyword());
  setName(generatedClassIdentifier);
  
  // constructor
  ConstructorPTreeGenerator ctor;
  ctor.setName(generatedClassIdentifier);
  for (size_t i = 0; i < parameters.size(); ++i)
  {
    ctor.addParameter(parameters[i].getPTree());
    PTree::Node* id = parameters[i].getIdentifier();
    ctor.addInitializer(id, id);
  }
  ctor.body.add(NULL); // empty implementation
  body.add(ctor.createDeclaration());
  
  // members
  for (size_t i = 0; i < parameters.size(); ++i)
  {
    body.add(parameters[i].getPTree());
    body.add(atom(";\n"));
  }
  
  std::string featureGeneratorName = inputFunctionIdentifier;
  if (inputClassIdentifier.size())
    featureGeneratorName = inputClassIdentifier + "::" + featureGeneratorName;

  body.add(atom("static const char* getName() {return " + quote(featureGeneratorName) + ";}\n"));
  body.add(atom("static lbcpp::FeatureDictionaryPtr getDictionary()\n"
      "  {static lbcpp::FeatureDictionaryPtr dictionary = lbcpp::FeatureDictionaryManager::get(" + quote(featureGeneratorName) + "); return dictionary;}\n"));
  
  // static feature generator
  FunctionPTreeGenerator staticFunction;
  staticFunction.addModifier(atom("template<class __FeatureVisitor__>"));
  staticFunction.addModifier(staticKeyword());
  staticFunction.setReturnType(voidKeyword());
  staticFunction.setIdentifier(atom("staticFeatureGenerator"));
  staticFunction.addParameter(atom("__FeatureVisitor__"), atom("&__featureVisitor__"));
  for (size_t i = 0; i < parameters.size(); ++i)
    staticFunction.addParameter(parameters[i].getPTree());
  staticFunction.body.add(atom("lbcpp::FeatureDictionaryPtr __featureDictionary__ = getDictionary();\n"));
  staticFunction.body.add(FeatureGeneratorRewriteVisitor(location, addThisParameter ? classScope : NULL, scope, identifier("__featureDictionary__")).rewrite(input.getBody().getPTree()));
  body.add(staticFunction.createDeclaration());
      
  // virtualisable feature generator
  FunctionPTreeGenerator normalizedFunction;
  normalizedFunction.addModifier(atom("template<class __FeatureVisitor__>"));
  normalizedFunction.setReturnType(voidKeyword());
  normalizedFunction.setIdentifier(atom("featureGenerator"));
  normalizedFunction.addParameter(atom("__FeatureVisitor__"), atom("&__featureVisitor__"));
  FuncallPTreeGenerator funcall;
  funcall.setName("staticFeatureGenerator");
  funcall.addArgument(identifier("__featureVisitor__"));
  for (size_t i = 0; i < parameters.size(); ++i)
    funcall.addArgument(parameters[i].getIdentifier());
  normalizedFunction.body.addExpressionStatement(funcall.createExpression());
  body.add(normalizedFunction.createDeclaration());

  if (addThisParameter)
    parameters.popFront();
}
  
PTree::Node* FeatureGeneratorClassGenerator::createCode(PTree::FunctionDefinition** staticToDynamicFunctionDefinition)
{
  bool addThisParameter = (location == inClassDefinition || location == inClassImplementation);

  BlockPTreeGenerator block;

  /*
  ** Static to dynamic function predeclaration (only for out-of-class featureGenerators)
  **   -> necessary to support recursive featureGenerator that use a non-inlined featureCall
  */
  FunctionPTreeGenerator staticToDynamicFunction;
  if (input.getModifiers().contains("static"))
    staticToDynamicFunction.addModifier(staticKeyword());
  if (input.getModifiers().contains("virtual"))
    staticToDynamicFunction.addModifier(virtualKeyword());
 // else if (location != inClassImplementation)
 //   staticToDynamicFunction.addModifier(inlineKeyword());
  staticToDynamicFunction.setReturnType(atom("lbcpp::FeatureGeneratorPtr"));
  staticToDynamicFunction.setName(input.getIdentifierString());
  staticToDynamicFunction.setConst(input.isConst());
  for (size_t i = 0; i < parameters.size(); ++i)
    staticToDynamicFunction.addParameter(parameters[i].getPTree());

  if (!isInCRAlgorithm && (location == outOfClass || location == outOfClassStatic))
    block.add(staticToDynamicFunction.createDeclaration());
  
  /*
  ** Class Declaration
  */
  block.add(PTree::list(atom("// "), PTree::first(input.getPTree()), PTree::second(input.getPTree()), atom("\n")));
  block.add(createDeclaration()); // Class declaration

  /*
  ** Static to dynamic function implementation
  */
  FuncallPTreeGenerator newCall;
  newCall.setName(generatedClassIdentifier);
  if (addThisParameter)
    newCall.addArgument(thisKeyword());
  for (size_t i = 0; i < parameters.size(); ++i)
    newCall.addArgument(parameters[i].getIdentifier());
  staticToDynamicFunction.body.add(returnStatement(funcallExpr(atom("lbcpp::staticToDynamicFeatureGenerator"), newCall.createExpression())));
  PTree::FunctionDefinition* staticToDynamicFundef = staticToDynamicFunction.createDeclaration();
  if (staticToDynamicFunctionDefinition)
    *staticToDynamicFunctionDefinition = staticToDynamicFundef;
  block.add(staticToDynamicFundef);

  /*
  ** inline 'featureCall' function
  */
  if (location == outOfClass || location == outOfClassStatic || location == inClassDefinition || location == inClassDefinitionStatic)
  {
    FunctionPTreeGenerator featureCallFunction;
    featureCallFunction.addModifier(atom("template<class __FeatureVisitor__>"));
    if (input.getModifiers().contains("static"))
      featureCallFunction.addModifier(staticKeyword());
    featureCallFunction.addModifier(inlineKeyword());
    
    featureCallFunction.setReturnType(voidKeyword());
    featureCallFunction.setName(input.getIdentifierString() + "InlineCall");
    featureCallFunction.addParameter(atom("__FeatureVisitor__"), atom("&__featureVisitor__"));
    featureCallFunction.addParameter(atom("lbcpp::FeatureDictionaryPtr"), atom("__featureDictionary__"));

    featureCallFunction.setConst(input.isConst());
    for (size_t i = 0; i < parameters.size(); ++i)
      featureCallFunction.addParameter(parameters[i].getPTree());
    
      FuncallPTreeGenerator funcall;
      funcall.setName(generatedClassIdentifier + "::staticFeatureGenerator");
      funcall.addArgument(atom("__featureVisitor__"));
      funcall.addArgument(atom("__featureDictionary__"));
      if (addThisParameter)
        funcall.addArgument(thisKeyword());
      for (size_t i = 0; i < parameters.size(); ++i)
        funcall.addArgument(parameters[i].getIdentifier());
      
      featureCallFunction.body.addExpressionStatement(funcall.createExpression());
    block.add(featureCallFunction.createDeclaration());
  }

  return block.createContent();
}

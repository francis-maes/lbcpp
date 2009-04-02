/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmStateFunctionC...hpp| State dependent functions       |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2009 00:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_GENERATOR_STATE_ACTION_PAIR_FUNCTION_H_
# define CRALGORITHM_GENERATOR_STATE_ACTION_PAIR_FUNCTION_H_

# include "FeatureGeneratorClassGenerator.h"
# include "../tools/ErrorManager.h"
# include "../tools/PTreeAnalyser.h"

class CRAlgorithmStateFunctionClass : public ClassPTreeGenerator
{
public:
  CRAlgorithmStateFunctionClass(CRAlgorithmVariableTranslator& translator, CRAlgo::StateFundefStatement* stateFundef, SymbolLookup::UserScope* scope)
    : additionalCode(NULL)
  {
    assert(stateFundef);
    FunctionPTreeAnalyser input(translateFunctionDefinition(translator, stateFundef, scope));
    ParameterListPTreeAnalyser parameters(input.getParameters());
    std::string identifier = input.getIdentifierString();
    
    setKeyword(structKeyword());
    setName("__" + identifier + "Function__");
    std::string kind = stateFundef->getKind();
    std::string baseClass = "lcpp::impl::";
    if (kind == "ActionValue") // todo: ActionFeatures et ActionDescription
      baseClass += "Typed";    
    baseClass += stateFundef->getKind() + "Function<__" + identifier + "Function__";
    
    if (kind != "Choose")
      for (size_t i = 0; i < parameters.size(); ++i)
        baseClass += ", " + PTree::reify(parameters[i].getRecomposedType(false));
    baseClass += ">";
    
    
    addBaseClass(publicKeyword(), ClassPTreeGenerator::identifier(baseClass));
    
    // returnType, parameters
    body.add(list(typedefKeyword(), getReturnType(input), atom("__ReturnType__;\n")));
    
    if (kind != "Choose" && parameters.size() == 1)
      body.add(list(typedefKeyword(), parameters[(size_t)0].getRecomposedType(false), atom("ChoiceType;\n")));      

//            output.addParameter(atom(translator.getCRAlgorithmClassName()), list(atom("&"), identifier("__crAlgorithm__")));

    // ctor, getName(), setChoose()
    body.add(atom("__" + identifier + "Function__() : __crAlgorithm__(NULL) {}\n"));
    body.add(atom("inline std::string getName() const {return " + quote(identifier) + ";}\n"));
    body.add(atom("inline void setChoose(lcpp::ChoosePtr choose) {\n"
        "__crAlgorithm__ = &lcpp::dynamicToStaticCRAlgorithm<" + translator.getCRAlgorithmClassName() + ">(choose->getCRAlgorithm()); }\n"));
    body.add(atom(translator.getCRAlgorithmClassName() + "* __crAlgorithm__;\n"));

    // function body
    FunctionPTreeGenerator function;
    function.addModifiers(input.getModifiers());
//    function.addModifier(staticKeyword());
    function.setReturnType(input.getReturnType());
    function.setName("compute");
    function.addParameters(parameters.getPTreeVector());
    function.body.add(input.getBody());
    function.setConst(true);
    
    body.add(function.createDeclaration());
    
    if (input.getReturnType().getString() == "featureGenerator")
      body.add(atom("typedef " + identifier + "FeatureGenerator FeatureGenerator;\n"));
  }
  
  void createCode(BlockPTreeGenerator& block)
  {
    if (additionalCode)
      block.add(additionalCode);
    block.add(createDeclaration());
  }
  
private:
  PTree::Node* additionalCode;

  PTree::Node* getReturnType(FunctionPTreeAnalyser& input)
    {return input.getReturnTypeString() == "featureGenerator" ? identifier("lcpp::FeatureGeneratorPtr") : input.getReturnType();}

  PTree::FunctionDefinition* translateFunctionDefinition(CRAlgorithmVariableTranslator& translator, CRAlgo::StateFundefStatement* stateFundef, SymbolLookup::UserScope* scope)
  {
    PTree::FunctionDefinition* fundef = stateFundef->getFunctionDefinition();
    FunctionPTreeAnalyser input(fundef);
    FunctionPTreeGenerator output;
    
    bool isFeatureGenerator = CRAlgo::isFeatureGenerator(fundef);
    
    ParameterListPTreeAnalyser parameters = input.getParameters();
    
    // add parameter __crAlgorithm__ and translate variables
    output.addModifiers(input.getModifiers());
    output.setReturnType(input.getReturnType());
    output.setIdentifier(input.getIdentifier());
    if (isFeatureGenerator)
    {
      output.addModifier(staticKeyword());
      output.addParameter(identifier(translator.getCRAlgorithmClassName()), list(atom("*"), identifier("__crAlgorithm__")));
    }
    output.addParameters(parameters);

    CRAlgorithmVariableTranslatorVisitor visitor(translator);
    output.body.add(visitor.translateVariables(input.getBody(), scope, false, "__crAlgorithm__->"));
    fundef = output.createDeclaration();

    if (isFeatureGenerator)
    {
      // replace fundef by the staticToDynamic feature generator function
      FeatureGeneratorClassGenerator featureGeneratorGenerator(fundef, scope, true);
      additionalCode = featureGeneratorGenerator.createCode();
      
      FunctionPTreeGenerator output;
      output.addModifiers(input.getModifiers());
      output.setReturnType(identifier("lcpp::FeatureGeneratorPtr"));
      output.setIdentifier(input.getIdentifier());
      output.addParameters(parameters);
      
      FuncallPTreeGenerator call;
      call.setIdentifier(input.getIdentifier());
      call.addArgument(identifier("__crAlgorithm__"));
      for (size_t i = 0; i < parameters.size(); ++i)
        call.addArgument(parameters[i].getIdentifier());
      output.body.add(returnStatement(call.createExpression()));
      fundef = output.createDeclaration();
    }
    
    // feature generators
    return fundef;
  }
};

#endif // !CRALGORITHM_GENERATOR_STATE_ACTION_PAIR_FUNCTION_H_

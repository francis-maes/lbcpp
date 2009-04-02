/*-----------------------------------------.---------------------------------.
| Filename: CRAlgoCompiler.cpp             | CR-Algorithm Compiler           |
| Author  : Francis Maes                   |                                 |
| Started : 05/01/2009 17:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
# include "common.h"
# include "tools/PlatformUtilities.h"
# include "tools/ErrorManager.h"
# include "tools/PrettyPrintVisitor.h"
# include "compiler/CRAlgoCompilerRewriteVisitor.h"
# include "compiler/CRInputFile.h"
# include "compiler/CppOutputFile.h"

class CRAlgorithmCompiler
{
public:
  CRAlgorithmCompiler() : isDebugMode(false), isVerboseMode(false)
  {
    PTree::Encoding::do_init_static();
  }
  
  void setVerboseMode(bool enabled = true)
  {
    if (enabled)
      Trace::enable(Trace::ALL);
    isVerboseMode = enabled;
  }
  
  void setDebugMode(bool enabled = true)
    {isDebugMode = enabled;}
  
  bool compile(const std::string& inputFile, const std::string& outputFile)
  {
    /*
    ** Parse
    */
    if (isDebugMode)
      std::cout << "Opening file '" << inputFile << "'." << std::endl;
    CRInputFile crAlgoFile(inputFile);
    if (!crAlgoFile.parse(isDebugMode))
      return false;
    if (isDebugMode)
      crAlgoFile.writeIntermediaryRepresentations(inputFile);
    if (isVerboseMode)
      std::cout << prettyPrint(crAlgoFile.getRootNode()) << std::endl;
    
    /*
    ** Check Syntax
    */
    if (isDebugMode)
      std::cout << "Checking syntax..." << std::endl;
    if (!crAlgoFile.checkSyntax())
      return false;
      
    /*
    ** Source-to-source transformation
    */
    if (isDebugMode)
      std::cout << "Rewriting program..." << std::endl;
    
    PTree::Node* node = crAlgoFile.getRootNode();
    CRAlgoCompilerRewriteVisitor compiler(crAlgoFile.getSymbols().current_scope(), isDebugMode);
    node = compiler.rewrite(node);

    CRAlgoCompilerFirstPassRewriteVisitor precompiler;
    node = precompiler.rewrite(node);        
          
    /*
    ** Save file
    */
    if (isDebugMode)
      std::cout << "Saving file '" << outputFile << "'." << std::endl;
    std::string outputDirectory = PlatformUtilities::getParentDirectory(outputFile);
    PlatformUtilities::createMissingDirectories(outputDirectory);    
    CppOutputFile cppOutputFile(outputFile);
    return cppOutputFile.write(node, crAlgoFile, false); // <--- the flag for #line creation //
  }

  bool compileAndFlushErrors(const std::string& inputFile, const std::string& outputFile)
  {
    bool res = compile(inputFile, outputFile);
    ErrorManager::getInstance().write(std::cerr);
    if (isVerboseMode && ErrorManager::getInstance().containFatalErrors())
      std::cerr << "There are some fatal errors, aborting." << std::endl;
    return res;
  }

private:
  bool isDebugMode;
  bool isVerboseMode;
};

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    std::cerr << "Usage: " << argv[0] << " inputfile outputfile [debug/verbose]" << std::endl;
    return 1;
  }
  CRAlgorithmCompiler compiler;
  if (argc > 3)
  {
    if (!strcmp(argv[3], "debug"))
      compiler.setDebugMode();
    if (!strcmp(argv[3], "verbose"))
      compiler.setDebugMode(), compiler.setVerboseMode();
  }
  std::string inputFile = PlatformUtilities::convertToFullPath(argv[1]);
  std::string outputFile = PlatformUtilities::convertToFullPath(argv[2]);
  return compiler.compileAndFlushErrors(inputFile, outputFile) ? 0 : 1;
}

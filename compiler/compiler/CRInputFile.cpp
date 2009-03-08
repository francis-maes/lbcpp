/*-----------------------------------------.---------------------------------.
| Filename: CRInputFile.cpp                | CR-Algorithm input file         |
| Author  : Francis Maes                   |                                 |
| Started : 10/02/2009 19:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "CRInputFile.h"
#include "../tools/PrettyPrintVisitor.h"
#include "../tools/PreprocessorAndCommentsChunker.h"
#include "../tools/ErrorManager.h"
#include "../language/CRAlgoPTree.h"
#include "../language/CRAlgoLexer.h"
#include "../language/CRAlgoParser.h"
#include "CheckCRAlgoSyntaxVisitor.h"

CRInputFile::CRInputFile(const std::string& filename)
  : filename(filename), buffer(NULL), rootNode(NULL) {}

CRInputFile::~CRInputFile()
{
  if (buffer)
    delete buffer;
  if (rootNode)
    delete rootNode;
}

void CRInputFile::prettyPrint(PTree::Node* node, std::ostream& target, bool addSourceLineNumbers)
{
  assert(buffer);
  CodePrinter printer(target);
  printer.setSourceBuffer(*buffer);
  if (addSourceLineNumbers)
    printer.doSourceLineNumbers();
  printer.doPrintChunks(chunks);
  ::prettyPrint(node, printer);
}

bool CRInputFile::parse(bool verbose)
{
  ErrorManager& errors = ErrorManager::getInstance();
  
  /*
  ** Chunking
  */
  std::istream* istr = openFile();
  if (!istr)
    return false;
  PreprocessorAndCommentsChunker chunker;
  chunker.chunk(*istr, chunks);
  delete istr;
  
  /*
  ** Parsing
  */
  istr = openFile();
  if (!istr)
    return false;
  assert(!buffer);  
  buffer = new Buffer(istr->rdbuf(), filename);
  errors.setCurrentSourceBuffer(*buffer);  

  CRAlgoLexer lexer(buffer);
  CRAlgoParser parser(lexer, symbols);
  
  if (verbose)
    std::cout << "Parsing file '" << filename << "'." << std::endl;
  rootNode = parser.parse();
  delete istr;

  /*
  ** Display parse errors
  */
  errors.addErrors(parser.errors());
  if (verbose)
    std::cout << parser.errors().size() << " Parsing errors." << std::endl;
  if (!rootNode)
  {
    if (parser.errors().size() == 0)
      errors.addError("No source code in file", filename, 0);
    else
      errors.addError("Parsing failed", filename, 0);
    return false;
  }
  return true;
}

unsigned long CRInputFile::getFirstLineOfCode() const
{
  std::string filename;
  return buffer->origin(rootNode->begin(), filename);
}

unsigned long CRInputFile::getLastLineOfCode() const
{
  std::string filename;
  unsigned long res = buffer->origin(rootNode->end(), filename) - 1;
  unsigned long first = getFirstLineOfCode();
  if (res < first)
    res = first;
  return res;
}

void CRInputFile::addChunk(size_t lineNumber, const std::string& content)
{
  size_t i;
  for (i = 0; i < chunks.size() && chunks[i].position < lineNumber; ++i)
    ;
  chunks.insert(chunks.begin() + i, CodeChunk(lineNumber, content));
}

bool CRInputFile::checkSyntax()
{
  assert(rootNode);
  CheckCRAlgoSyntaxVisitor v;
  return v.checkSyntax(rootNode);
}

void CRInputFile::writeIntermediaryRepresentations(const std::string& basename)
{
  // lexer
  std::ofstream lexerOstr((basename + ".lexer").c_str());
  {
    std::ifstream istr(filename.c_str());
    Buffer buffer(istr.rdbuf(), filename);
    CRAlgoLexer lexer(&buffer);
    while (true)
    {
      Token token;
      Token::Type type = lexer.get_token(token);
      if (!type || type == Token::BadToken)
        break;
      lexerOstr << "[" << type << "] " << std::string(token.ptr, token.length) << " ";
    }
  }

  // chunks
  std::ofstream chunksOstr((basename + ".chunks").c_str());
  chunksOstr << chunks.size() << " chunks." << std::endl;
  for (size_t i = 0; i < chunks.size(); ++i)
    chunksOstr << "Chunk " << i + 1 << " line = " << chunks[i].position << " content = " << std::endl << chunks[i].content << std::endl << "-" << std::endl;

  // symbols
  std::ofstream symbolOstr((basename + ".symbols").c_str());
  display(symbols.current_scope(), symbolOstr);

  // parse tree
  std::ofstream parseOstr((basename + ".parse").c_str());
  PTree::display(rootNode, parseOstr, false, true);
  
  // pretty-print
  std::ofstream prettyOstr((basename + ".pretty").c_str());
  CodePrinter codePrinter(prettyOstr);
  ::prettyPrint(rootNode, codePrinter);
  prettyOstr << std::endl;
}

std::istream* CRInputFile::openFile() const
{
  std::ifstream* res = new std::ifstream(filename.c_str());
  if (!res->is_open())
  {
    ErrorManager::getInstance().addError("Could not open file", filename);
    delete res;
    return NULL;
  }
  return res;
}


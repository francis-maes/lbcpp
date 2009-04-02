/*-----------------------------------------.---------------------------------.
| Filename: CodePrinter.h                  | Code Printer Tool               |
| Author  : Francis Maes                   |                                 |
| Started : 10/02/2009 18:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LCPP_TOOLS_CODE_PRINTER_H_
# define LCPP_TOOLS_CODE_PRINTER_H_

# include "PTreeGenerator.h"
# include "PreprocessorAndCommentsChunker.h"

class CodePrinter
{
public:
  CodePrinter(std::ostream& ostr) 
    : ostr(ostr), inputBuffer(NULL), chunks(NULL), printSourceLineNumbers(false), chunksPosition(0),
      indentation(0), lineNumberHasChanged(true) {}
  
  void setSourceBuffer(Buffer& inputBuffer)
    {this->inputBuffer = &inputBuffer;}
  
  void doSourceLineNumbers(bool enabled = true)
    {printSourceLineNumbers = enabled;}
  
  void doPrintChunks(CodeChunkVector& chunks)
    {this->chunks = &chunks;}

  void write(const std::string& str, int offset = 0)
    {write(str.c_str(), str.length(), offset);}
    
    
  std::string createSourceLineCommand(unsigned long line) const
  {
    std::ostringstream ostr;
    ostr << "#line " << line << " " << PTreeGenerator::quote(sourceFilename) << std::endl;
    return ostr.str();
  }
  
  void write(const char* ptr, size_t len, int offset = 0)
  {
    if (inputBuffer && lineNumberHasChanged)
    {
      try
      {
        unsigned long line = inputBuffer->origin(ptr, this->sourceFilename);
        // we enter here as soon as we find the origin of ptr 
        
        // update chunks
        flushChunks((size_t)line);

        // put a #line 
        if (printSourceLineNumbers)
          currentLine = createSourceLineCommand(line) + currentLine;
        
        lineNumberHasChanged = false;
      }
      catch (std::exception&) {}
    }
      
    if (offset < 0)
      currentLine = currentLine.substr(0, std::max(0, (int)currentLine.size() - offset));
    
    for (size_t i = 0; i < len; ++i)
    {
      char c = ptr[i];
      if (c == '\n')
        newLine();
      else
        currentLine += c;
    }
  }    

  void flushLine()
  {
    if (currentLine.size())
      ostr << currentLine << std::endl;
    resetLine();
  }
  
  void newLine()
    {flushLine(); lineNumberHasChanged = true;}
  
  void resetLine()
  {
    currentLine = "";
    for (size_t i = 0; i < indentation; ++i)
      currentLine += "  ";
  }
  
  void indent()
    {++indentation;}
    
  void dedent()
    {assert(indentation > 0); --indentation;}
  
  void flush()
  {
    flushLine();
    flushChunks();
    flushLine();
  }
  
private:
  std::ostream& ostr;
  Buffer* inputBuffer;
  CodeChunkVector* chunks;
  bool printSourceLineNumbers;

  size_t chunksPosition;
  size_t indentation;
  std::string currentLine;
  bool lineNumberHasChanged;
  
  std::string sourceFilename;
  
  void flushChunks(size_t currentLineNumber = (size_t)-1)
  {
    if (chunks)
    {
      std::string str;
      while (chunksPosition < chunks->size() && 
              (currentLineNumber == (size_t)-1 || (*chunks)[chunksPosition].position < currentLineNumber))
      {
        if (printSourceLineNumbers && lineNumberHasChanged)
          str += createSourceLineCommand((*chunks)[chunksPosition].position + 1);
        str += (*chunks)[chunksPosition].content + "\n";
        ++chunksPosition;
        lineNumberHasChanged = true;
      }
      if (str.size())
      {
        currentLine = str + currentLine;
        lineNumberHasChanged = false;
      }
    }
  }
};

#endif // !LCPP_TOOLS_CODE_PRINTER_H_

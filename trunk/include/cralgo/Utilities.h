/*-----------------------------------------.---------------------------------.
| Filename: Utilities.h                    | Miscelaneous Utilities          |
| Author  : Francis Maes                   |                                 |
| Started : 29/03/2009 18:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_UTILITIES_H_
# define CRALGO_UTILITIES_H_

# include <string>
# include <cassert>
# include <sstream>
# include <vector>

namespace cralgo
{

class ErrorHandler
{
public:
  virtual ~ErrorHandler() {}
  virtual void errorMessage(const std::string& where, const std::string& what) = 0;
  virtual void warningMessage(const std::string& where, const std::string& what) = 0;

  static void setInstance(ErrorHandler& handler);
  static ErrorHandler& getInstance() {assert(instance); return *instance;}
  
  static void error(const std::string& where, const std::string& what)
    {getInstance().errorMessage(where, what);}

  static void warning(const std::string& where, const std::string& what)
    {getInstance().warningMessage(where, what);}
  
private:
  static ErrorHandler* instance;
};

class ProgressCallback
{
public:
  static ProgressCallback& getConsoleProgressCallback();

public:
  virtual ~ProgressCallback() {}
  
  virtual void progressStart(const std::string& description)
    {}
    
  // return false to stop the task
  virtual bool progressStep(const std::string& description, double iteration, double totalIterations = 0)
    {return true;}
    
  virtual void progressEnd()
    {}
};

class TextFileParser
{
public:
  virtual ~TextFileParser() {}

  virtual void parseBegin()
    {}
  virtual bool parseEmptyLine()
    {return true;}
  virtual bool parseDataLine(const std::vector<std::string>& columns)
    {return false;}
  virtual bool parseCommentLine(const std::string& comment)
    {return true;}
  virtual bool parseEnd()
    {return parseEmptyLine();}
  
  bool parseLine(const std::string& line);
  bool parseStream(std::istream& istr);
  bool parseFile(const std::string& filename);
  
protected:
  void breakParsing()
    {parsingBreaked = true;}
  
  static void tokenize(const std::string& line, std::vector< std::string >& columns, const char* separators = " \t");
  
  template<class T>
  static bool parse(std::istream& istr, T& res)
    {return !(istr >> res).fail();}

  template<class T>
  static bool parse(const std::string& str, T& res)
    {std::istringstream istr(str); return parse(istr, res);}
    
private:
  bool parsingBreaked;
};

}; /* namespace cralgo */

#endif // !CRALGO_UTILITIES_H_

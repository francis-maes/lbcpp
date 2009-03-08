/*-----------------------------------------.---------------------------------.
| Filename: PreprocesserAndCommentsChunker.h| A simple chunker of            |
| Author  : Francis Maes                   |   preprocessor calls and        |
| Started : 05/01/2009 17:46               |         comments                |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_TOOLS_PREPROCESSOR_AND_COMMENTS_CHUNKER_H_
# define CRALGORITHM_TOOLS_PREPROCESSOR_AND_COMMENTS_CHUNKER_H_

class CodeChunk
{
public:
  CodeChunk(size_t position, std::string content)
    : position(position), content(content) {}
  
  size_t position;
  std::string content;
};
  
typedef std::vector<CodeChunk> CodeChunkVector;

class PreprocessorAndCommentsChunker
{
public:
  void chunk(std::istream& istr, CodeChunkVector& res)
  {
    std::string currentLongComment;
    
    std::string line;
    size_t lineNumber = 0;
    while (!istr.eof())
    {
      getline(istr, line);
      
      if (line.empty() && currentLongComment.empty())
        res.push_back(CodeChunk(lineNumber, "")); // empty line
      
      while (line.size() > 0)
      {
        if (currentLongComment.empty())
        {
          /*
          ** Not inside long comment
          */
          size_t i = line.find_first_not_of(" \t\r\n");
          if (i == std::string::npos)
          {
            res.push_back(CodeChunk(lineNumber, "")); // empty line
            break;
          }
            
          if (line[i] == '#') // preprocessor line
          {
            res.push_back(CodeChunk(lineNumber, line.substr(i)));
            break;
          }
          
          size_t singleComment = line.find("//");
          if (singleComment != std::string::npos)
          {
            res.push_back(CodeChunk(lineNumber, line.substr(singleComment)));
            break;
          }
          
          size_t longCommentStart = line.find("/*");
          if (longCommentStart != std::string::npos)
          {
            currentLongComment += "/*";
            line = line.substr(longCommentStart + 2);
            continue;
          }
          
          break; // nothing interesting in this line
        }
        else
        {
          size_t i = line.find("*/");
          if (i == std::string::npos)
          {
            currentLongComment += line + "\n";
            break;
          }
          else
          {
            currentLongComment += line.substr(0, i + 2);
            res.push_back(CodeChunk(lineNumber, currentLongComment));
            currentLongComment = "";
            line = line.substr(i + 2);
            continue;
          }
        }
      }
      ++lineNumber;
    }
  }
};

#endif // !CRALGORITHM_TOOLS_PREPROCESSOR_AND_COMMENTS_CHUNKER_H_

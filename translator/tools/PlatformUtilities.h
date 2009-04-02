/*-----------------------------------------.---------------------------------.
| Filename: PlatformUtilities.h            | Platform-dependent code         |
| Author  : Francis Maes                   |                                 |
| Started : 05/01/2009 19:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGORITHM_PLATFORM_UTILITIES_H_
# define CRALGORITHM_PLATFORM_UTILITIES_H_

# include <string>
# include <iostream>
# include <vector>
# include <assert.h>

namespace PlatformUtilities
{
  // platform dependent functions
  bool isPathRoot(const std::string& path);
  bool isPathSeparator(const char c);
  std::string getCurrentWorkingDirectory();
  bool isFullPath(const std::string& filename);
  bool isDirectory(const std::string& filename);
  bool createDirectory(const std::string& filename);

  // generic functions
  inline std::string concatenatePath(const std::vector<std::string>& pathElements)
  {
    std::string res;
    for (size_t i = 0; i < pathElements.size(); ++i)
      res += pathElements[i] + "/";
    return res.substr(0, res.size() - 1);
  }
  
  inline std::string simplifyFullPath(const std::string& fullpath)
  {
    assert(isFullPath(fullpath));
    std::vector<std::string> res;
    size_t first = fullpath.find_first_of("/\\");
    assert(first != std::string::npos);
    
    size_t next;
    for (size_t n = first; n != std::string::npos; n = next)
    {
      next = fullpath.find_first_of("/\\", n + 1);
      std::string part;
      if (next == std::string::npos)
        part = fullpath.substr(n + 1);
      else
        part = fullpath.substr(n + 1, next - (n + 1));
      if (part == ".")
        continue;
      if (part == "..")
      {
        assert(res.size());
        res.pop_back();
      }
      else
        res.push_back(part);
    }
    return fullpath.substr(0, first + 1) + concatenatePath(res);
  }
    
  inline std::string makeFullPath(const std::string& basename, const std::string& filename)
  {
    std::string b = basename;
    if (basename.size() && isPathSeparator(basename[basename.size() - 1]))
      b = basename.substr(0, basename.size() - 1);
    std::string f = filename;
    if (filename.size() && isPathSeparator(filename[0]))
      f = basename.substr(1);
    assert(f.size());
    return simplifyFullPath(b + "/" + f);
  }

  inline std::string convertToFullPath(const std::string& filename)
  {
    if (isFullPath(filename))
      return filename;
    else
      return makeFullPath(getCurrentWorkingDirectory(), filename);
  }
  
  inline std::string getParentDirectory(const std::string& fullpath)
  {
    if (!isFullPath(fullpath))
    {
      std::cerr << "Invalid directory: " << fullpath << std::endl;
      assert(false);
    }
    if (isPathRoot(fullpath))
      return "";
    size_t n = fullpath.find_last_of("/\\");
    assert(n != std::string::npos);
    return fullpath.substr(0, n);
  }
  
  inline std::string getFileName(const std::string& fullpath)
  {
    assert(isFullPath(fullpath));
    size_t n = fullpath.find_last_of("/\\");
    if (n == std::string::npos || n == 0)
      return "";
    return fullpath.substr(n + 1);
  }
  
  inline bool createMissingDirectories(const std::string& directoryFullPath)
  {
    std::string parentDirectory = getParentDirectory(directoryFullPath);
    if (parentDirectory.size() && !createMissingDirectories(parentDirectory))
      return false;
    if (!isDirectory(directoryFullPath) && !createDirectory(directoryFullPath))
    {
      std::cerr << "Could not create directory " << directoryFullPath << std::endl;
      return false;
    }
    return true;
  }
};

#endif // !CRALGORITHM_PLATFORM_UTILITIES_H_

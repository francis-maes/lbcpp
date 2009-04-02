/*-----------------------------------------.---------------------------------.
| Filename: PlatformUtilities.cpp          | Platform-dependent code         |
| Author  : Francis Maes                   |                                 |
| Started : 20/02/2009 13:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "PlatformUtilities.h"

#ifdef WIN32

# include <windows.h>
# define MAX_PATH_CHARS (MAX_PATH + 256)

namespace PlatformUtilities
{
  bool isPathRoot(const std::string& path)
    {return path.size() == 2 && path[1] == ':';}

  bool isPathSeparator(const char c)
    {return c == '/' || c == '\\';}

  std::string getCurrentWorkingDirectory()
  {
    char dest[MAX_PATH_CHARS];
    dest[0] = 0;
    GetCurrentDirectoryA(MAX_PATH_CHARS, dest);
    return std::string(dest);
  }    

  bool isFullPath(const std::string& filename)
    {return filename.size() >= 2 && filename[1] == ':';}

  bool isDirectory(const std::string& filename)
  {    
    const DWORD attr = GetFileAttributesA(filename.c_str());
    return (attr != 0xffffffff)
             && ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0);
  }
    
  bool createDirectory(const std::string& filename)
  {
    assert(isFullPath(filename));
    if (isDirectory(filename))
      return true;
    CreateDirectoryA(filename.c_str(), 0);
    return true;
  }

}; /* namespace PlatformUtilities */

#else // POSIX

# include <sys/stat.h>

namespace PlatformUtilities
{
  bool isPathRoot(const std::string& path)
    {return path == "/";}

  bool isPathSeparator(const char c)
    {return c == '/';}

  std::string getCurrentWorkingDirectory()
  {
    char buffer[2048];
    getcwd(buffer, sizeof (buffer));
    return buffer;
  }
  
  bool isFullPath(const std::string& filename)
    {return filename.size() && filename[0] == '/';}
    
  bool isDirectory(const std::string& fullpath)
  {
    assert(isFullPath(fullpath));
    if (fullpath.size() == 0)
      return false;
    struct stat info;
    return (stat(fullpath.c_str(), &info) == 0) && ((info.st_mode & S_IFDIR) != 0);
  }
  
  bool createDirectory(const std::string& fullpath)
  {
    assert(isFullPath(fullpath));
    if (fullpath.size() == 0)
      return false;
    mkdir(fullpath.c_str(), 0777);
    return true;
  }

}; /* namespace PlatformUtilities */

#endif // POSIX

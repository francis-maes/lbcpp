/* -------------------------------------- . ---------------------------------- .
| Filename : PosixConsoleProcess.h        | Child console process class.       |
| Author   : Alexandre Buge               | Manage process hosting and IO      |
| Started  : 14/02/2010 10:34             | redirection (POSIX Implementation) |
` --------------------------------------- . --------------------------------- */

#ifndef SMODE_PROJECT_REPOSITORY_POSIX_CONSOLE_PROCESS_H_
# define SMODE_PROJECT_REPOSITORY_POSIX_CONSOLE_PROCESS_H_

#include "ConsoleProcess.h"

#include <sys/wait.h> /** for waitpid */
#include <string.h>   /** for strlen */
#include <errno.h>    /** for errno */
#include <unistd.h>   /** needed by standard */
#include <stdlib.h>   /** for select FD_SET... */

/* DRAFT VERSION TODO CHECK ON MACOS-X * DRAFT VERSION TODO CHECK ON MACOS-X */
#include <stdio.h>    /** todo remove this and all printf */
#include <assert.h>   /** todo remplace this by jassert */
#include <string>     /** todo remplace this by Juce::String */

namespace juce
{

class FileDescriptorHelper
{
public:
  static void close(int* fileDescriptor)
  {
    assert(*fileDescriptor != -1);
    if (::close(*fileDescriptor) == -1)
    {
      assert(false);
    }      
		*fileDescriptor = -1;
  }
  
private:
  FileDescriptorHelper();
};

class StandardIORedirector
{
public:
  StandardIORedirector()
   : standardInputRead(-1), standardInputWrite(-1), standardOutputWrite(-1), standardOutputRead(-1) {}
  
  ~StandardIORedirector()
  {
    if (standardInputRead != -1)
    {    
      assert(false);
      FileDescriptorHelper::close(&standardInputRead);
    }   
    if (standardInputWrite != -1)
    {    
      assert(false);
      FileDescriptorHelper::close(&standardInputWrite);
    }
    if (standardOutputWrite != -1)
    {    
      assert(false);
      FileDescriptorHelper::close(&standardInputWrite);
    }
    if (standardOutputRead != -1)
    {    
      assert(false);
      FileDescriptorHelper::close(&standardInputWrite);
    }    
  }
  
  bool initializeChildProcess()
  {
    assert(standardInputRead   == -1
        && standardInputWrite  == -1
        && standardOutputWrite == -1
        && standardOutputRead  == -1);

    int filedes[2];
    if (pipe(filedes) == -1)
    {
      assert(false);
      return false;
    }
    standardInputRead = filedes[0];
    standardInputWrite = filedes[1];

    if (pipe(filedes) == -1)
    {
      int lasterror = errno;
      assert(false);
      FileDescriptorHelper::close(&standardInputRead);
      FileDescriptorHelper::close(&standardInputWrite);
      errno = lasterror;
      return false;
    }
    standardOutputRead = filedes[0];
    standardOutputWrite = filedes[1];
    return true;     
  }
  
  bool completeChildProcessInitialize(bool callerIsParentProcess = false)
  {
    assert(standardInputRead   != -1
        && standardInputWrite  != -1
        && standardOutputWrite != -1
        && standardOutputRead  != -1);
  
    if (callerIsParentProcess) 
    {
      FileDescriptorHelper::close(&standardInputRead); 
      FileDescriptorHelper::close(&standardOutputWrite);       
    }
    else 
    {  
      if (dup2(standardInputRead, STDIN_FILENO) == -1)
      {       
        assert(false);
        return false;
      }
      if (dup2(standardOutputWrite, STDOUT_FILENO) == -1)
      { 
        int lasterror = errno;      
        // todo undo previous dup
        errno = lasterror;
        assert(false);
        return false;
      }        
      FileDescriptorHelper::close(&standardInputRead);      
      FileDescriptorHelper::close(&standardInputWrite); 
      FileDescriptorHelper::close(&standardOutputRead);      
      FileDescriptorHelper::close(&standardOutputWrite); 
    }
    return true;
  }
  
  void release()
  {
    assert(standardInputWrite  != -1
        && standardInputRead   == -1
        && standardOutputWrite == -1
        && standardOutputRead  != -1);
    FileDescriptorHelper::close(&standardInputWrite); 
    FileDescriptorHelper::close(&standardOutputRead);     
  }       
  
  bool writeStandardInput(const char* in) const
  {
    assert(standardInputWrite != -1);

    const size_t totalToWrite = strlen(in);    
    size_t totalNumberOfBytesWritten = 0;
    while (totalNumberOfBytesWritten < totalToWrite)
    {    
      ssize_t numberOfBytesWritten = write(standardInputWrite, 
                                          in + totalNumberOfBytesWritten, 
                                          totalToWrite - totalNumberOfBytesWritten);
      if (numberOfBytesWritten == -1)
        return false;
     totalNumberOfBytesWritten += numberOfBytesWritten;
    }
    return true;
  }
  
  bool readStandardOutput(String& out) const
  {      
    assert(standardOutputRead != -1);
    
    out = String::empty;
    char buffer[256];
    fd_set selectFileDescriptorSet;
    ssize_t dataReaded = 0;
    struct timeval noWait = {0, 0};
    do
    { 
      FD_ZERO(&selectFileDescriptorSet);
      FD_SET(standardOutputRead, &selectFileDescriptorSet);    
      int selectResult = select(standardOutputRead + 1, &selectFileDescriptorSet, NULL, NULL, &noWait);
      if (selectResult == -1)
        return out.isNotEmpty(); // error, the child process might have ended
      else if (selectResult == 0)
        break; // nothing to read, exit
        
      dataReaded = read(standardOutputRead, buffer, sizeof(buffer) - 1);
      if (dataReaded == -1)
        return out.isNotEmpty();
      buffer[dataReaded] = 0;
      out += buffer;
    }
    while (dataReaded != 0);
    return true;
  }
  
  // todo juce_UseDebuggingNewOperator

private:  
  int standardInputRead;    // written by parent
  int standardInputWrite;   // readed by child
  int standardOutputWrite;  // written by child
  int standardOutputRead;   // readed by parent
};

class PosixConsoleProcess : public ConsoleProcess
{
public:
  PosixConsoleProcess(const std::string& executable, const std::string& parameters = "", const std::string& workingDirectory = "")
    : executable(executable), parameters(parameters), workingDirectory(workingDirectory), processID(-1), returnCode(0)
  {}
  virtual ~PosixConsoleProcess() 
  {
    int notUsed;
    if (isRunning(notUsed))
    {
      assert(false); // class deletion but the process is still running
      // force process detach and free resources.
      standardIO.release();
      // todo: find a POSIX detach method avoiding zombie childs
    }
  }

  virtual bool readStandardOutput(String& result) const
    {return standardIO.readStandardOutput(result);}  
    
  virtual bool writeStandardInput(const String& input) const
    {return standardIO.writeStandardInput(input);}  
  
  virtual bool kill()
    {jassert(false); return false;} // not implemented yet

  virtual bool isRunning(int& returnCode)
  {
    // process previously dead... or never be run???
    if (processID == -1)
    {    
      returnCode = this->returnCode;
      return false;
    }
    int childStatus;
    int waitStatus = waitpid(processID, &childStatus, WNOHANG);
    assert(waitStatus != -1);
    if (waitStatus == 0)
      return true;
      
    // process have just to die: set return code and free resources
    if (WIFEXITED(childStatus)) 
      returnCode = this->returnCode = WEXITSTATUS(childStatus);
    else if (WIFSIGNALED(childStatus))
    {      
      returnCode = this->returnCode = WTERMSIG(childStatus);
      assert(false); // child process interupted: returnCode is set to a TermSig code.
    }    
    else 
    { 
      assert(false); // unexpected child status
    }   
    standardIO.release();
    processID = -1;
    return false;
  }

  // todo juce_UseDebuggingNewOperator

//todo protected:
  virtual bool createdCallback()
  {
    assert(processID == -1);
     
    if (!standardIO.initializeChildProcess())
    {  
      assert(false);
      return false;
    }      
    processID = fork();
    if (processID == -1)
    {
      int lastError = errno;
      standardIO.completeChildProcessInitialize(true);
      standardIO.release();
      errno = lastError;    
      assert(false);
      return false;
    }    
    if (processID == 0)
      _exit(childProcessMain());
    /* else parent */
    if (!standardIO.completeChildProcessInitialize(true))
    {  
      assert(false);
    }   
    return true;
  }

private:
  int childProcessMain()
  {
    if (!standardIO.completeChildProcessInitialize())
      return 101; // unable to initialize child io redirection, see errno
    if (workingDirectory.size())
    {    
      if (chdir(workingDirectory.c_str()) == -1)
        return 102; // working directory not found, see errno
    }
    // hardcoded parameters : todo construct argv from class::parameters
    char* argv[] = {strdup(executable.c_str()), strdup("-help"), NULL};
    char** environ = NULL; // FIXME
    if (execve(argv[0], argv, environ) == -1)
      return 103; // process not found ? see errno
    return 104; // theoricaly never called  
  }

private:
  std::string executable;       // executable file name, requiered
  std::string parameters;       // process parameters, can be empty.
  std::string workingDirectory; // process working directory, can be empty to use calling process one.
  pid_t processID;
  StandardIORedirector standardIO;
  int returnCode;
};

}; /* namespace juce */

/*
// todo remove this test function:
int main()
{
  PosixConsoleProcess process("/usr/bin/unison", strdup("-help"));
  if (!process.createdCallback())
  {
    assert(false);
    return 1;
  }
  int returnCode;
  do
  {  
    std::string output;
    if (process.readStandardOutput(output))
    {
      printf("%s", output.c_str());
    }    
  }
  while (process.isRunning(returnCode));
  return 0;
} */

#endif // !SMODE_PROJECT_REPOSITORY_POSIX_CONSOLE_PROCESS_H_

/* -------------------------------------- . ---------------------------------- .
| Filename : Win32ConsoleProcess.h        | Child console process class.       |
| Author   : Alexandre Buge               | Manage process hosting and IO      |
| Started  : 29/01/2010 12:34             | redirection (Win32 Implementation) |
` --------------------------------------- . --------------------------------- */
                               
#ifndef SMODE_PROJECT_REPOSITORY_WIN32_CONSOLE_PROCESS_H_
# define SMODE_PROJECT_REPOSITORY_WIN32_CONSOLE_PROCESS_H_

# include "ConsoleProcess.h"

namespace juce
{

class HandleHelper
{
public:
  static void close(HANDLE* handle)
  {
    jassert(*handle != INVALID_HANDLE_VALUE)
    const BOOL ok = CloseHandle(*handle);
	  jassert(ok);
		*handle = INVALID_HANDLE_VALUE;
  }
private:
  HandleHelper();
};

// This class just handles opening/closing/holding the pipes for IO redirection
class StandardIORedirector
{
public:
	StandardIORedirector()
    : standardInputWriteHandle(INVALID_HANDLE_VALUE) 
    , standardInputReadHandle(INVALID_HANDLE_VALUE)
    , standardOutputWriteHandle(INVALID_HANDLE_VALUE)
    , standardOutputReadHandle(INVALID_HANDLE_VALUE)
	  {}

	~StandardIORedirector()
	 {
     if (standardInputWriteHandle != INVALID_HANDLE_VALUE)
     {
       jassertfalse;
       HandleHelper::close(&standardInputWriteHandle);
     }
     if (standardInputReadHandle != INVALID_HANDLE_VALUE)
     {
       jassertfalse;
       HandleHelper::close(&standardInputReadHandle);
     }
     if (standardOutputWriteHandle != INVALID_HANDLE_VALUE)
     {
       jassertfalse;
       HandleHelper::close(&standardOutputWriteHandle);
     }
     if (standardOutputReadHandle != INVALID_HANDLE_VALUE)
     {
       jassertfalse;
       HandleHelper::close(&standardOutputReadHandle);
     }
   }

	// initialize child process IO redirection...
	bool initializeChildProcess(LPSTARTUPINFO startupInfo)
	{
		jassert(standardInputWriteHandle	== INVALID_HANDLE_VALUE 
				&&	standardInputReadHandle	  == INVALID_HANDLE_VALUE
				&&	standardOutputWriteHandle == INVALID_HANDLE_VALUE
				&&	standardOutputReadHandle	== INVALID_HANDLE_VALUE
        &&  startupInfo);

		SECURITY_ATTRIBUTES securityAttributes;
		securityAttributes.nLength				      = sizeof(SECURITY_ATTRIBUTES);
		securityAttributes.lpSecurityDescriptor	= NULL;
		securityAttributes.bInheritHandle			  = TRUE;

		if (!CreatePipe(&standardOutputReadHandle, &standardOutputWriteHandle, &securityAttributes, 0))
		{
			jassertfalse; // Failed to create output pipe
			return false;
		}
    if (!SetHandleInformation(standardOutputReadHandle, HANDLE_FLAG_INHERIT, 0))
		{
      jassertfalse; // Failed to convert output read handle
      DWORD dwError = GetLastError();
      HandleHelper::close(&standardOutputReadHandle);
		  HandleHelper::close(&standardOutputWriteHandle);
      SetLastError(dwError);
			return false;
		}

		if (!CreatePipe(&standardInputReadHandle, &standardInputWriteHandle, &securityAttributes, 0))
		{
      jassertfalse; // Failed to create output pipe
      DWORD dwError = GetLastError();
      HandleHelper::close(&standardOutputReadHandle);
		  HandleHelper::close(&standardOutputWriteHandle);
      SetLastError(dwError);
			return false;
		}    
    if (!SetHandleInformation(standardInputWriteHandle, HANDLE_FLAG_INHERIT, 0))
		{
      jassertfalse; // Failed to convert input write handle
      DWORD dwError = GetLastError();
      completeChildProcessInitialize();
      release();
      SetLastError(dwError);
			return false;
		}
    // Set startup info for child process IO redirection and windows hidding.
    ZeroMemory(startupInfo, sizeof(STARTUPINFO));
		startupInfo->cb			      = sizeof(STARTUPINFO);
		startupInfo->dwFlags		  = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    startupInfo->hStdInput	  = standardInputReadHandle;
    startupInfo->hStdOutput   = standardOutputWriteHandle;
    startupInfo->hStdError    = standardOutputWriteHandle;		
		startupInfo->wShowWindow  = SW_HIDE;
		return true;
	}

  // Release temporary resources of the currend process inherited by child process.
	void completeChildProcessInitialize()
	{
		jassert(standardInputWriteHandle	!= INVALID_HANDLE_VALUE 
				&&	standardInputReadHandle		!= INVALID_HANDLE_VALUE
				&&	standardOutputWriteHandle	!= INVALID_HANDLE_VALUE
				&&	standardOutputReadHandle	!= INVALID_HANDLE_VALUE);
		// [all handles must be set at this point]

		HandleHelper::close(&standardInputReadHandle);
		HandleHelper::close(&standardOutputWriteHandle);
	}

  // Release the IO redirection
	void release()
	{
		jassert(standardInputWriteHandle	!= INVALID_HANDLE_VALUE 
				&&	standardInputReadHandle		== INVALID_HANDLE_VALUE
				&&	standardOutputWriteHandle	== INVALID_HANDLE_VALUE
				&&	standardOutputReadHandle	!= INVALID_HANDLE_VALUE);
    // prepareForProcess and completeChildProcessInitialize must be called at this point

		HandleHelper::close(&standardInputWriteHandle);
		HandleHelper::close(&standardOutputReadHandle);
	}

  // Read child process standard output
	bool readStandardOutput(String& out) const
	{
		jassert(standardOutputReadHandle != INVALID_HANDLE_VALUE); // output read handle must be valid

    out = String::empty;
		for (;;)
		{
			DWORD dataSize = 0;
			if (!PeekNamedPipe(standardOutputReadHandle, NULL, 0, NULL, &dataSize, NULL))
        return out.isNotEmpty();	// error, the child process might have ended
			if (dataSize == 0) 
        break;
			char szOutput[256];
			DWORD dataRead = 0;
			if (!ReadFile(standardOutputReadHandle, szOutput, min(sizeof(szOutput) - 1, dataSize), &dataRead, NULL) || !dataRead)
				return  out.isNotEmpty();	// error, the child process might have ended
			szOutput[dataRead] = 0;
			out += String(szOutput);
		}
		return true;	
	}

  // Write on child process standard output
  bool writeStandardInput(const String& in) const
  {
    jassert(standardInputWriteHandle != INVALID_HANDLE_VALUE); // output read handle must be valid

    DWORD totalNumberOfBytesWritten = 0;
    const DWORD totalToWrite = (DWORD)in.length();
    while (totalNumberOfBytesWritten < totalToWrite)
    {
      String toWrite(in.substring(totalNumberOfBytesWritten));
      DWORD numberOfBytesWritten;
      if (!WriteFile(standardInputWriteHandle, (const char*)toWrite, toWrite.length(), &numberOfBytesWritten, NULL) != 0)
      {
        jassertfalse;
        return false;
      }
      totalNumberOfBytesWritten += numberOfBytesWritten;
    }
    return true;
  }

  juce_UseDebuggingNewOperator

private:
	HANDLE standardInputWriteHandle;  // written by us...
	HANDLE standardInputReadHandle;	  // read by child process...
	HANDLE standardOutputWriteHandle; // written by child process...
	HANDLE standardOutputReadHandle;	// read by us...
};

class Win32ConsoleProcess : public ConsoleProcess
{
public:
  Win32ConsoleProcess(const String& executable, const String& parameters = String::empty, const String& workingDirectory = String::empty)
    : executable(executable), parameters(parameters), workingDirectory(workingDirectory), processHandle(INVALID_HANDLE_VALUE), returnCode(0)
    {jassert(executable.isNotEmpty());}
 
  virtual ~Win32ConsoleProcess() 
  {
    int notUsed;
    if (isRunning(notUsed))
    {
      //jassertfalse; // class deletion but the process is still running.
      // force process detach and free resources.
      standardIO.release(); 
      HandleHelper::close(&processHandle);
    }
  }

  virtual bool readStandardOutput(String& result) const
    {return standardIO.readStandardOutput(result);}

  virtual bool writeStandardInput(const String& input) const
    {return standardIO.writeStandardInput(input);}

  virtual bool isRunning(int& returnCode)
  {
    // process previously dead... or never be run???
    if (processHandle == INVALID_HANDLE_VALUE)
    {
      returnCode = this->returnCode;
      return false;
    }
    DWORD dwReturnCode;
    BOOL ok = GetExitCodeProcess(processHandle, &dwReturnCode);
    jassert(ok);
    if (dwReturnCode == STILL_ACTIVE)
      return true;
    // process have just to die: set return code and free resources
    returnCode = this->returnCode = (int)dwReturnCode;
    standardIO.release();
    HandleHelper::close(&processHandle);
    return false;
  }

  virtual bool killProcess()
  {
    if (processHandle == INVALID_HANDLE_VALUE)
      return false;
    return TerminateProcess(processHandle, 1) == TRUE;
  }

  juce_UseDebuggingNewOperator

protected:
  virtual bool createdCallback()
  {
		jassert(processHandle == INVALID_HANDLE_VALUE);

    // Set startup info for IO redirection and hidding windows.
		STARTUPINFO startupInfo;
    const bool ok = standardIO.initializeChildProcess(&startupInfo);
    if (!ok)
    {
      jassertfalse;
      return false;
    }
    // Command line must include executable name and parameters.
    const String commandLine = (parameters.isEmpty() ? T("EXE") : T("EXE ") + parameters);

    // Command can be modified by CreateProcess so convert String content into POD.
    LPTSTR commandLineBuffer = new TCHAR[commandLine.length() + 1];
    commandLine.copyToBuffer(commandLineBuffer, commandLine.length());
    commandLineBuffer[commandLine.length()] = 0;

    // If working directory is empty use parent process working directory.
    const LPCTSTR lpCurrentDirectory = workingDirectory.isEmpty() ? NULL : (LPCTSTR)workingDirectory;

    // Child process have normal priority.
    // Child process do not need to create console window.
    // On unicode build, hosting process environment variable can include potential unicode charactere.
#ifdef UNICODE 
    const DWORD dwCreationFlags = /*CREATE_NO_WINDOW |*/ NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT;
#else /** !UNICODE */
    const DWORD dwCreationFlags = /*CREATE_NO_WINDOW |*/ NORMAL_PRIORITY_CLASS;
#endif /** !UNICODE */

		// Attempt to start the process...
    PROCESS_INFORMATION processInfo;
		if (!CreateProcess((LPCTSTR)executable, 
      commandLineBuffer,      // read / write command line buffer.
			NULL,                   // Child process handle security: default and cannot be inherited.
			NULL,                   // Child main thread handle security: default and cannot be inherited.
			TRUE,	                  // Calling process handle inheritance set true for standard Input/Output redirection.
			dwCreationFlags,        // Child process creation flags.
			NULL,                   // Child process use calling process environment variables.
			lpCurrentDirectory,     // Set child process working directory.
			&startupInfo,           // Input startup info.
			&processInfo))          // Output process info.
    {
      DWORD dwError = GetLastError();
      delete [] commandLineBuffer;
		  standardIO.completeChildProcessInitialize();
			standardIO.release();
      SetLastError(dwError);
			return false;
    }
    delete [] commandLineBuffer;
    standardIO.completeChildProcessInitialize();
    HandleHelper::close(&processInfo.hThread);  // Close unused child process main thread handle.
		processHandle = processInfo.hProcess;       // Backup child process handle.
		return true;
  }

private:
  String executable;        // executable file name, required.
  String parameters;        // process parameters, can be empty.
  String workingDirectory;  // process working directory, can be empty to use calling process one.
  HANDLE processHandle;
  StandardIORedirector standardIO;
  int returnCode;
};

}; /* namespace juce */

#endif // !SMODE_PROJECT_REPOSITORY_WIN32_CONSOLE_PROCESS_H_

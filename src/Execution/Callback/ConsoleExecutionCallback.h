/*-----------------------------------------.---------------------------------.
| Filename: ConsoleExecutionCallback.h     | Console Execution Callback      |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 17:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CALLBACK_CONSOLE_H_
# define LBCPP_EXECUTION_CALLBACK_CONSOLE_H_

# include <lbcpp/Execution/ExecutionCallback.h>
# include <lbcpp/Execution/ExecutionStack.h>

namespace lbcpp
{

class ConsoleOutput
{
public:
  void print(size_t threadNumber, size_t depth, const String& type, const String& text, bool isError)
  {
    String prefix = T("T") + makeFixedSizeNumber(threadNumber, 5) + T(" ");
    prefix += makeFixedSizeString(type, 8) + T(" ");
    for (size_t i = 0; i < depth; ++i)
      prefix += T("  ");

    String res = prefix + text;
    if (res.length() % numColumns)
      res += makeFixedSizeString(String::empty, numColumns - (res.length() % numColumns) - 1);
    print(res, isError);
/*
    int remainingLength = numColumns - prefix.length() - 1;
    if (remainingLength > 0)
    {
      StringArray lines;
      lines.addTokens(text, T("\n"), NULL);

      ScopedLock _(lock);
      for (int i = 0; i < lines.size(); ++i)
      {
        String line;
        if (i == 0)
          line = prefix;
        else
          for (int j = 0; j < prefix.length(); ++j)
            line += ' ';
        line += makeFixedSizeString(lines[i], remainingLength);
        print(line, isError);
      }
    }*/
  }

  void print(const String& line, bool isError)
  {
    ScopedLock _(lock);
    std::cout << line << std::endl;
    if (isError)
      std::cerr << line << std::endl;
  }

private:
  enum {numColumns = 80};

  CriticalSection lock;
 
  static String makeFixedSizeNumber(size_t number, int requiredLength)
  {
    String res((int)number);
    while (res.length() < requiredLength)
      res = T("0") + res;
    if (res.length() > requiredLength)
      res = T("..") + res.substring(res.length() - (requiredLength - 2));
    return res;
  }

  static String makeFixedSizeString(const String& str, int requiredLength)
  {
    String res = str;
    if (res.length() <= requiredLength)
    {
      while (res.length() < requiredLength)
        res += ' ';
      return res;
    }
    else
    {
      if (requiredLength > 3)
        return res.dropLastCharacters(res.length() - requiredLength + 3) + T("...");
      else
        return res.dropLastCharacters(res.length() - requiredLength);
    }
  }
};

class ConsoleThreadExecutionCallback : public ExecutionCallback
{
public:
  ConsoleThreadExecutionCallback(ConsoleOutput& output, size_t threadNumber, size_t depth)
    : output(output), depth(depth), threadNumber(threadNumber), lastMessageTime(0) {} 

  virtual void informationCallback(const String& where, const String& what)
  {
    String text = what;
    if (where.isNotEmpty())
      text += T(" (in ") + where + T(")");
    print(T("info"), text, false);
  }

  virtual void warningCallback(const String& where, const String& what)
  {
    String text = what;
    if (where.isNotEmpty())
      text += T(" (in ") + where + T(")");
    print(T("warning"), text, true);
  }

  virtual void errorCallback(const String& where, const String& what)
  {
    String text = what;
    if (where.isNotEmpty())
      text += T(" (in ") + where + T(")");
    print(T("error"), text, false);
    jassertfalse;
  }

  virtual void progressCallback(const ProgressionStatePtr& progression)
  {
    juce::uint32 time = Time::getApproximateMillisecondCounter();
    if (!lastMessageTime || time > lastMessageTime + 250)
      print(T("progress"), progression->toString(), false);
  }

  virtual void resultCallback(const String& name, const Variable& value)
  {
    if (false) // todo: verboseResults flag
      print(T("result"), name + T(" = ") + value.toShortString(), false);
  }

  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit)
    {print(T("start"), description, false); ++depth;}

  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit, const Variable& result)
  {
    if (result.exists())
      print(T("result"), description + T(" ") + result.toShortString(), false);
    jassert(depth);
    --depth;
  }

private:
  ConsoleOutput& output;
  size_t depth;
  size_t threadNumber;
  juce::uint32 lastMessageTime;

  void print(const String& type, const String& text, bool isError)
  {
    lastMessageTime = Time::getApproximateMillisecondCounter();
    output.print(threadNumber, depth, type, text, isError);
  }
};

class ConsoleExecutionCallback : public DispatchByThreadExecutionCallback
{
public:
  ConsoleExecutionCallback() : counter(0) {}

  virtual ExecutionCallbackPtr createCallbackForThread(const ExecutionStackPtr& stack, Thread::ThreadID threadId)
    {return new ConsoleThreadExecutionCallback(output, counter++, stack->getDepth());}

private:
  ConsoleOutput output;
  size_t counter;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CALLBACK_CONSOLE_H_

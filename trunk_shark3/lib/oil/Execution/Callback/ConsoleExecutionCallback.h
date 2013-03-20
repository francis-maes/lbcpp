/*-----------------------------------------.---------------------------------.
| Filename: ConsoleExecutionCallback.h     | Console Execution Callback      |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 17:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef OIL_EXECUTION_CALLBACK_CONSOLE_H_
# define OIL_EXECUTION_CALLBACK_CONSOLE_H_

# include <oil/Execution/ExecutionCallback.h>
# include <oil/Execution/ExecutionStack.h>

namespace lbcpp
{

class ConsoleOutput
{
public:
  void print(size_t threadNumber, size_t depth, const string& type, const string& text, bool isError)
  {
    string prefix = JUCE_T("T") + makeFixedSizeNumber(threadNumber, 5) + JUCE_T(" ");
    prefix += makeFixedSizeString(type, 8) + JUCE_T(" ");
    for (size_t i = 0; i < depth; ++i)
      prefix += JUCE_T("  ");

    string res = prefix + text;
    if (res.length() % numColumns)
      res += makeFixedSizeString(string::empty, numColumns - (res.length() % numColumns) - 1);
    print(res, isError);
/*
    int remainingLength = numColumns - prefix.length() - 1;
    if (remainingLength > 0)
    {
      StringArray lines;
      lines.addTokens(text, JUCE_T("\n"), NULL);

      ScopedLock _(lock);
      for (int i = 0; i < lines.size(); ++i)
      {
        string line;
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

  void print(const string& line, bool isError)
  {
    ScopedLock _(lock);
    std::cout << line << std::endl;
    if (isError)
      std::cerr << line << std::endl;
  }

private:
  enum {numColumns = 80};

  CriticalSection lock;
 
  static string makeFixedSizeNumber(size_t number, int requiredLength)
  {
    string res((int)number);
    while (res.length() < requiredLength)
      res = JUCE_T("0") + res;
    if (res.length() > requiredLength)
      res = JUCE_T("..") + res.substring(res.length() - (requiredLength - 2));
    return res;
  }

  static string makeFixedSizeString(const string& str, int requiredLength)
  {
    string res = str;
    if (res.length() <= requiredLength)
    {
      while (res.length() < requiredLength)
        res += ' ';
      return res;
    }
    else
    {
      if (requiredLength > 3)
        return res.dropLastCharacters(res.length() - requiredLength + 3) + JUCE_T("...");
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
  ConsoleThreadExecutionCallback() : output(*(ConsoleOutput* )0), depth(0), threadNumber(0), lastMessageTime(0) {}

  virtual void informationCallback(const string& where, const string& what)
  {
    string text = what;
    if (where.isNotEmpty())
      text += JUCE_T(" (in ") + where + JUCE_T(")");
    print(JUCE_T("info"), text, false);
  }

  virtual void warningCallback(const string& where, const string& what)
  {
    string text = what;
    if (where.isNotEmpty())
      text += JUCE_T(" (in ") + where + JUCE_T(")");
    print(JUCE_T("warning"), text, false);
  }

  virtual void errorCallback(const string& where, const string& what)
  {
    string text = what;
    if (where.isNotEmpty())
      text += JUCE_T(" (in ") + where + JUCE_T(")");
    print(JUCE_T("error"), text, false);
    jassertfalse;
  }

  virtual void progressCallback(const ProgressionStatePtr& progression)
  {
    juce::uint32 time = juce::Time::getApproximateMillisecondCounter();
    if (!lastMessageTime || time > lastMessageTime + 250)
      print(JUCE_T("progress"), progression->toString(), false);
  }

  virtual void resultCallback(const string& name, const ObjectPtr& value)
  {
    if (false) // todo: verboseResults flag
      print(JUCE_T("result"), name + JUCE_T(" = ") + value->toShortString(), false);
  }

  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const string& description, const WorkUnitPtr& workUnit)
    {print(JUCE_T("start"), description, false); ++depth;}

  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const string& description, const WorkUnitPtr& workUnit, const ObjectPtr& result)
  {
    if (result)
      print(JUCE_T("result"), description + JUCE_T(" ") + result->toShortString(), false);
    jassert(depth);
    --depth;
  }

private:
  ConsoleOutput& output;
  size_t depth;
  size_t threadNumber;
  juce::uint32 lastMessageTime;

  void print(const string& type, const string& text, bool isError)
  {
    lastMessageTime = juce::Time::getApproximateMillisecondCounter();
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

#endif //!OIL_EXECUTION_CALLBACK_CONSOLE_H_

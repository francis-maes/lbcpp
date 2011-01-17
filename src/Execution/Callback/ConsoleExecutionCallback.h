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
    String line = makeFixedSizeString(T("T") + String(threadNumber), 3) + T(" ");
    line += makeFixedSizeString(type, 8) + T(" ");
    for (size_t i = 0; i < depth; ++i)
      line += T("  ");
    int remainingLength = numColumns - line.length() - 1;
    if (remainingLength > 0)
      line += makeFixedSizeString(text, remainingLength);
    print(line, isError);
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
  ConsoleThreadExecutionCallback(ConsoleOutput& output, size_t threadNumber)
    : output(output), threadNumber(threadNumber), lastMessageTime(0) {} 

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
  }

  virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit)
  {
    juce::uint32 time = Time::getApproximateMillisecondCounter();
    if (!lastMessageTime || time > lastMessageTime + 250)
    {
      String text = String(progression);
      if (progressionTotal)
        text += T(" / ") + String(progressionTotal);
      if (progressionUnit.isNotEmpty())
        text += T(" ") + progressionUnit;
      print(T("progress"), text, false);
    }
  }

  virtual void resultCallback(const String& name, const Variable& value)
    {print(T("result"), name + T(" = ") + value.toShortString(), false);}

  virtual void preExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit)
  {
    depthStack.push_back(stack->getDepth());
    print(T("start"), description, false, 1);
  }

  virtual void postExecutionCallback(const ExecutionStackPtr& stack, const String& description, const WorkUnitPtr& workUnit, bool result)
    {depthStack.pop_back();}

private:
  ConsoleOutput& output;
  std::vector<size_t> depthStack;
  size_t threadNumber;
  juce::uint32 lastMessageTime;

  void print(const String& type, const String& text, bool isError, size_t depthOffset = 0)
  {
    size_t depth = depthStack.size() ? depthStack.back() : 0;
    if (depth >= depthOffset)
      depth -= depthOffset;
    lastMessageTime = Time::getApproximateMillisecondCounter();
    output.print(threadNumber, depth, type, text, isError);
  }
};

class ConsoleExecutionCallback : public DispatchByThreadExecutionCallback
{
public:
  ConsoleExecutionCallback() : counter(0) {}

  virtual ExecutionCallbackPtr createCallbackForThread(Thread::ThreadID threadId)
    {return new ConsoleThreadExecutionCallback(output, counter++);}

private:
  ConsoleOutput output;
  size_t counter;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CALLBACK_CONSOLE_H_

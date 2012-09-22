/*-----------------------------------------.---------------------------------.
| Filename: ApplyFunctionStream.h          | Application of a Function       |
| Author  : Francis Maes                   |  on a Stream                    |
| Started : 06/05/2010 12:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_STREAM_APPLY_FUNCTION_H_
# define LBCPP_DATA_STREAM_APPLY_FUNCTION_H_

# include <lbcpp/Data/Stream.h>
# include <lbcpp/Core/Function.h>

namespace lbcpp
{

class ApplyFunctionStream : public Stream
{
public:
  ApplyFunctionStream(ExecutionContext& context, StreamPtr stream, FunctionPtr function)
    : Stream(context), stream(stream), function(function), outputType(function->getOutputType()) {jassert(outputType);}
  ApplyFunctionStream() {}

  virtual TypePtr getElementsType() const
    {return outputType;}

  virtual bool rewind()
    {return stream->rewind();}

  virtual bool isExhausted() const
    {return stream->isExhausted();}

  virtual Variable next()
  {
    Variable v = stream->next();
    return v.isNil() ? Variable() : function->compute(context, v);
  }

  virtual ProgressionStatePtr getCurrentPosition() const
    {return stream->getCurrentPosition();}

private:
  friend class ApplyFunctionStreamClass;

  StreamPtr stream;
  FunctionPtr function;
  TypePtr outputType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_STREAM_APPLY_FUNCTION_H_

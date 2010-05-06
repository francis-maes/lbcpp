/*-----------------------------------------.---------------------------------.
| Filename: ApplyFunctionObjectStream.h    | Application of an ObjectFunction|
| Author  : Francis Maes                   |  on an ObjectStream             |
| Started : 06/05/2010 12:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_STREAM_APPLY_FUNCTION_H_
# define LBCPP_OBJECT_STREAM_APPLY_FUNCTION_H_

# include <lbcpp/Object/ObjectStream.h>

namespace lbcpp
{

class ApplyFunctionObjectStream : public ObjectStream
{
public:
  ApplyFunctionObjectStream(ObjectStreamPtr stream, ObjectFunctionPtr function)
    : ObjectStream(function->getName() + T("(") + stream->getName() + T(")")), stream(stream), function(function) {}
    
  virtual String getContentClassName() const
    {return function->getOutputClassName(stream->getContentClassName());}

  virtual bool rewind()
    {return stream->rewind();}

  virtual bool isExhausted() const
    {return stream->isExhausted();}
    
  virtual ObjectPtr next()
  {
    ObjectPtr object = stream->next();
    return object ? function->function(object) : ObjectPtr();
  }

private:
  ObjectStreamPtr stream;
  ObjectFunctionPtr function;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_STREAM_APPLY_FUNCTION_H_

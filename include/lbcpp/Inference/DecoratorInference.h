/*-----------------------------------------.---------------------------------.
| Filename: DecoratorInference.h           | Decorator Inference             |
| Author  : Francis Maes                   |                                 |
| Started : 27/05/2010 21:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_DECORATOR_H_
# define LBCPP_INFERENCE_DECORATOR_H_

# include "Inference.h"
# include "InferenceVisitor.h"
# include "InferenceContext.h"
# include "InferenceCallback.h"

namespace lbcpp
{

class DecoratorInference : public Inference
{
public:
  DecoratorInference(const String& name, InferencePtr decorated)
    : Inference(name), decorated(decorated) {}
  DecoratorInference() {}
 
  /*
  ** Object
  */
  virtual String toString() const;
  virtual bool loadFromFile(const File& file);
  virtual bool saveToFile(const File& file) const;
  virtual ObjectPtr clone() const;

  /*
  ** Inference
  */
  virtual void accept(InferenceVisitorPtr visitor);
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);

  InferencePtr getDecoratedInference() const
    {return decorated;}

protected:
  InferencePtr decorated;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DECORATOR_H_

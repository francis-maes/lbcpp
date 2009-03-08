/*-----------------------------------------.---------------------------------.
| Filename: Callback.h                     | CR-algorithm callback           |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2009 18:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_CALLBACK_H_
# define CRALGO_CALLBACK_H_

# include <string>

namespace cralgo
{

class Choose;
typedef boost::shared_ptr<Choose> ChoosePtr;

class Callback
{
public:
  virtual ~Callback() {}
  
  virtual void choose(ChoosePtr choose) {}
  virtual void reward(double reward) {}
};

}; /* namespace cralgo */

#endif // !CRALGO_CALLBACK_H_

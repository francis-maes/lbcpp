/*-----------------------------------------.---------------------------------.
| Filename: Callback.h                     | CR-algorithm callback           |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2009 18:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LCPP_CALLBACK_H_
# define LCPP_CALLBACK_H_

# include <string>

namespace lcpp
{

class Callback
{
public:
  virtual ~Callback() {}
  
  virtual void choose(ChoosePtr choose) {}
  virtual void reward(double reward) {}
};

}; /* namespace lcpp */

#endif // !LCPP_CALLBACK_H_

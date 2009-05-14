/*-----------------------------------------.---------------------------------.
| Filename: Callback.h                     | CR-algorithm callback           |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2009 18:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_CALLBACK_H_
# define LBCPP_CALLBACK_H_

# include <string>

namespace lbcpp
{

class Callback
{
public:
  virtual ~Callback() {}
  
  virtual void choose(ChoosePtr choose) {}
  virtual void reward(double reward) {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_CALLBACK_H_

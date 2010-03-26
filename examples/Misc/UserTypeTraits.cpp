/*-----------------------------------------.---------------------------------.
| Filename: UserTypeTraits.cpp             | An example that illustrates     |
| Author  : Francis Maes                   |   definition of Type Traits     |
| Started : 16/06/2009 19:20               |     for a User type             |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#include <lbcpp/lbcpp.h>
#include <fstream>
using namespace lbcpp;

/*
** A user-defined class containing a single member
*/
class MyClass
{
public:
  MyClass(int value = 0)
    : value(value) {}
    
  int getValue() const
    {return value;}
    
  void setValue(int value)
    {this->value = value;}
  
private:
  int value;
};

/*
** The Traits for MyClass. Note that, in order to be valid, the traits must be defined inside the lbcpp namespace.
*/
namespace lbcpp {

  template<>
  struct Traits<MyClass>
  {
    static inline String toString(const MyClass& c)
      {return "value = " + lbcpp::toString(c.getValue());}

    static inline void write(std::ostream& ostr, const MyClass& c)
      {lbcpp::write(ostr, c.getValue());}
      
    static inline bool read(std::istream& istr, MyClass& result)
    {
      int val;
      if (lbcpp::read(istr, val))
      {
        result.setValue(val);
        return true;
      }
      return false;
    }
  };

};  // namespace lbcpp 


int main(int argc, char* argv[])
{
  /*
  ** Instantiate some stuff related to MyClass
  */
  MyClass toto1(1);
  MyClass toto2(2);
  MyClass* totoptr = &toto1;
  MyClass** totoptrptr = &totoptr;
  std::vector<MyClass> totos(5);
  for (size_t i = 0; i < totos.size(); ++i) totos[i].setValue(i);
  std::vector<MyClass* > totoptrs(5);
  for (size_t i = 0; i < totoptrs.size(); ++i) {totoptrs[i] = new MyClass(i);}
  
  /*
  ** Illustrates the lbcpp::toString() function which is implemented for various 
  **  types related to MyClass
  */
  std::cout << lbcpp::toString(toto1) << std::endl
            << lbcpp::toString(toto2) << std::endl
            << lbcpp::toString(&toto1) << std::endl
            << lbcpp::toString(&toto2) << std::endl
            << lbcpp::toString(totoptr) << std::endl
            << lbcpp::toString(totoptrptr) << std::endl
            << lbcpp::toString(totos) << std::endl
            << lbcpp::toString(totoptrs) << std::endl
            << lbcpp::toString(&totoptrs) << std::endl;
  
  /*
  ** Illustrates serialization
  */
  {
    std::ofstream ostr("tmpfile");
    lbcpp::write(ostr, toto1);
  }
  
  {
    std::ifstream istr("tmpfile");
    lbcpp::read(istr, toto2);
    std::cout << lbcpp::toString(toto2) << std::endl;
  }
  return 0;
}

/*-----------------------------------------.---------------------------------.
| Filename: Vector3.cpp                    | Vector in a 3D-space            |
| Author  : Francis Maes                   |                                 |
| Started : 09/07/2010 13:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "Vector3.h"
using namespace lbcpp;

namespace lbcpp {
namespace impl {

/*
** Vector3
*/
Vector3 Vector3::fromString(const String& str, ErrorHandler& callback)
{
  StringArray tokens;
  tokens.addTokens(str, T(" "), NULL);
  if (tokens.size() != 3)
  {
    callback.errorMessage(T("Vector3::fromString"), T("Invalid format: ") + str.quoted());
    return Vector3();
  }
  return Vector3(tokens[0].getDoubleValue(), tokens[1].getDoubleValue(), tokens[2].getDoubleValue());
}

}; /* namespace impl */

class Vector3Class : public DynamicClass
{
public:
  Vector3Class()
    : DynamicClass(T("Vector3"), objectClass())
  {
    addVariable(doubleType(), T("x"));
    addVariable(doubleType(), T("y"));
    addVariable(doubleType(), T("z"));
  }

  virtual VariableValue create() const
    {return new Vector3();}

  LBCPP_DECLARE_VARIABLE_BEGIN(Vector3)
    LBCPP_DECLARE_VARIABLE(value.x);
    LBCPP_DECLARE_VARIABLE(value.y);
    LBCPP_DECLARE_VARIABLE(value.z);
  LBCPP_DECLARE_VARIABLE_END();
};

ClassPtr vector3Class()
  {static TypeCache cache(T("Vector3")); return cache();}

}; /* namespace lbcpp */

/*
** Vector3KDTree
*/
#include "kdtree_lib.h"

Vector3KDTree::Vector3KDTree()
  {tree = kd_create(3);}

Vector3KDTree::~Vector3KDTree()
{
  if (tree)
    kd_free(tree);
}

void Vector3KDTree::insert(size_t index, const impl::Vector3& position)
{
  jassert(position.exists());
  kd_insert3(tree, position.getX(), position.getY(), position.getZ(), (void*)index);
}

void Vector3KDTree::findPointsInSphere(const impl::Vector3& center, double radius, std::vector<size_t>& results)
{
  jassert(center.exists());
  struct kdres* res = kd_nearest_range3(tree, center.getX(), center.getY(), center.getZ(), radius);
  results.resize(kd_res_size(res));
  for (size_t i = 0; i < results.size(); ++i)
  {
    jassert(!kd_res_end(res));
    results[i] = (size_t)kd_res_item_data(res);
    kd_res_next(res);
  }
  kd_res_free(res);
}


void declareVector3Classes()
{
  Class::declare(new Vector3Class());
}

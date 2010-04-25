/*-----------------------------------------.---------------------------------.
| Filename: Matrix4.cpp                    | 4x4 Matrix                      |
| Author  : Francis Maes                   | Most of this code comes from    |
| Started : 25/04/2010 14:12               | the Ogre open source 3D engine  |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "Matrix4.h"
using namespace lbcpp;

const Matrix4 Matrix4::zero = Matrix4(
      0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0);

const Matrix4 Matrix4::identity = Matrix4(
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      0.0, 0.0, 0.0, 1.0);

String Matrix4::toString() const
{
  String res = T("[[");
  for (size_t i = 0; i < 4; ++i)
    for (size_t j = 0; j < 4; ++j)
    {
      res += String(m[i][j]);
      if (j < 3)
        res += T(",");
      else
      {
        res += T("]");
        if (i < 3)
          res += T("\n");
        else
          res += T("]\n");
      }
    }
  return res;
}

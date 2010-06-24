/*-----------------------------------------.---------------------------------.
| Filename: Matrix4.h                      | 4x4 Matrix                      |
| Author  : Francis Maes                   | Most of this code comes from    |
| Started : 20/04/2010 19:25               | the Ogre open source 3D engine  |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_GEOMETRY_MATRIX4_H_
# define LBCPP_PROTEIN_GEOMETRY_MATRIX4_H_

# include "Vector3.h"
# include "Matrix3.h"

namespace lbcpp
{

class Matrix4
{
public:
  Matrix4() {}

  Matrix4(double m00, double m01, double m02, double m03,
          double m10, double m11, double m12, double m13,
          double m20, double m21, double m22, double m23,
          double m30, double m31, double m32, double m33)
  {
    m[0][0] = m00;
    m[0][1] = m01;
    m[0][2] = m02;
    m[0][3] = m03;
    m[1][0] = m10;
    m[1][1] = m11;
    m[1][2] = m12;
    m[1][3] = m13;
    m[2][0] = m20;
    m[2][1] = m21;
    m[2][2] = m22;
    m[2][3] = m23;
    m[3][0] = m30;
    m[3][1] = m31;
    m[3][2] = m32;
    m[3][3] = m33;
  }

  Matrix4(const Matrix3& matrix3, const Vector3& translation = Vector3(0.0))
  {
    m[0][0] = matrix3[0][0]; m[0][1] = matrix3[0][1]; m[0][2] = matrix3[0][2]; m[0][3] = translation.getX();
    m[1][0] = matrix3[1][0]; m[1][1] = matrix3[1][1]; m[1][2] = matrix3[1][2]; m[1][3] = translation.getY();
    m[2][0] = matrix3[2][0]; m[2][1] = matrix3[2][1]; m[2][2] = matrix3[2][2]; m[2][3] = translation.getZ();
    m[3][0] = m[3][1] = m[3][2] = 0.0; m[3][3] = 1.0;
  }

  static const Matrix4 zero;
  static const Matrix4 identity;

  // returns the matrix (rotation + translation) to transform points1 into points2
  static Matrix4 findAffineTransformToSuperposePoints(const std::vector< std::pair<Vector3, Vector3> >& pointPairs, bool* succeeded = NULL);

  String toString() const;

  double* operator [](size_t row)
    {jassert(row < 4); return m[row];}

  const double* const operator [](size_t row) const
    {jassert(row < 4); return m[row];}

  Vector3 getTranslation() const
    {return Vector3(m[0][3], m[1][3], m[2][3]);}

  void setTranslation(const Vector3& translation)
    {m[0][3] = translation.getX(); m[1][3] = translation.getY(); m[2][3] = translation.getZ();}

  Matrix4& translate(const Vector3& translation)
    {jassert(isAffine()); setTranslation(transformAffine(translation)); return *this;}

  Matrix4& rotateAroundXAxis(double angle)
  {
    jassert(isAffine());
	  double cc = cos(angle);
	  double ss = sin(angle);
	  double temp = (m[0][1] * cc + m[0][2] * ss);
	  m[0][2] = (m[0][2] * cc - m[0][1] * ss);
	  m[0][1] = temp;
	  temp = m[1][1] * cc + m[1][2] * ss;
	  m[1][2] = (m[1][2] * cc - m[1][1] * ss);
	  m[1][1] = temp;
	  temp = m[2][1] * cc + m[2][2] * ss;
	  m[2][2] = (m[2][2] * cc - m[2][1] * ss);
	  m[2][1] = temp;
    return *this;
  }

  Matrix4& rotateAroundYAxis(double angle)
  {
    jassert(isAffine());
	  double cc = cos(angle);
	  double ss = sin(angle);
	  double temp = m[0][0] * cc - m[0][2] * ss;
	  m[0][2] = (m[0][0] * ss + m[0][2] * cc);
	  m[0][0] = temp;
	  temp = m[1][0] * cc - m[1][2] * ss;
	  m[1][2] = (m[1][0] * ss + m[1][2] * cc);
	  m[1][0] = temp;
	  temp = m[2][0] * cc - m[2][2] * ss;
	  m[2][2] = (m[2][0] * ss + m[2][2] * cc);
	  m[2][0] = temp;
    return *this;
  }

  Matrix4& rotateAroundZAxis(double angle)
  {
    jassert(isAffine());
	  double cc = cos(angle);
	  double ss = sin(angle);
	  double temp = m[0][0] * cc + m[0][1] * ss;
	  m[0][1] = (m[0][1] * cc - m[0][0] * ss);
	  m[0][0] = temp;
	  temp = m[1][0] * cc + m[1][1] * ss;
	  m[1][1] = (m[1][1] * cc - m[1][0] * ss);
	  m[1][0] = temp;
	  temp = m[2][0] * cc + m[2][1] * ss;
	  m[2][1] = (m[2][1] * cc - m[2][0] * ss);
	  m[2][0] = temp;
    return *this;
  }

  inline bool operator ==(const Matrix4& m2) const
  {
    if (m[0][0] != m2.m[0][0] || m[0][1] != m2.m[0][1] || m[0][2] != m2.m[0][2] || m[0][3] != m2.m[0][3] ||
        m[1][0] != m2.m[1][0] || m[1][1] != m2.m[1][1] || m[1][2] != m2.m[1][2] || m[1][3] != m2.m[1][3] ||
        m[2][0] != m2.m[2][0] || m[2][1] != m2.m[2][1] || m[2][2] != m2.m[2][2] || m[2][3] != m2.m[2][3] ||
        m[3][0] != m2.m[3][0] || m[3][1] != m2.m[3][1] || m[3][2] != m2.m[3][2] || m[3][3] != m2.m[3][3])
        return false;
    return true;
  }

  inline bool operator !=(const Matrix4& m2) const
  {
    if (m[0][0] != m2.m[0][0] || m[0][1] != m2.m[0][1] || m[0][2] != m2.m[0][2] || m[0][3] != m2.m[0][3] ||
        m[1][0] != m2.m[1][0] || m[1][1] != m2.m[1][1] || m[1][2] != m2.m[1][2] || m[1][3] != m2.m[1][3] ||
        m[2][0] != m2.m[2][0] || m[2][1] != m2.m[2][1] || m[2][2] != m2.m[2][2] || m[2][3] != m2.m[2][3] ||
        m[3][0] != m2.m[3][0] || m[3][1] != m2.m[3][1] || m[3][2] != m2.m[3][2] || m[3][3] != m2.m[3][3])
      return true;
    return false;
  }

  bool isAffine() const
    {return m[3][0] == 0 && m[3][1] == 0 && m[3][2] == 0 && m[3][3] == 1;}
  
  bool isSymmetric() const
  {
    return m[0][1] == m[1][0] && m[0][2] == m[2][0] && m[0][3] == m[3][0] &&
      m[1][2] == m[2][1] && m[1][3] == m[3][1] && m[2][3] == m[3][2];
  }

  Vector3 transformAffine(const Vector3& v) const
  {
    jassert(isAffine());
    return Vector3(m[0][0] * v.getX() + m[0][1] * v.getY() + m[0][2] * v.getZ() + m[0][3],
                    m[1][0] * v.getX() + m[1][1] * v.getY() + m[1][2] * v.getZ() + m[1][3],
                    m[2][0] * v.getX() + m[2][1] * v.getY() + m[2][2] * v.getZ() + m[2][3]);
  }

private:
  union
  {
    double m[4][4];
    double _m[16];
  };

  friend struct Traits<Matrix4>;
};

inline Vector3 operator *(const Vector3& v, const Matrix4& mat)
{
  double w = v.getX() * mat[0][3] + v.getY() * mat[1][3] + v.getZ() * mat[2][3] + mat[3][3];
  Vector3 res(v.getX() * mat[0][0] + v.getY() * mat[1][0] + v.getZ() * mat[2][0] + mat[3][0],
              v.getX() * mat[0][1] + v.getY() * mat[1][1] + v.getZ() * mat[2][1] + mat[3][1],
              v.getX() * mat[0][2] + v.getY() * mat[1][2] + v.getZ() * mat[2][2] + mat[3][2]);
  if (w != 1.0 && w != 0.0)
    res /= w;
  return res;
}

template<>
struct Traits<Matrix4>
{
  typedef Matrix4 Type;

  static inline String toString(const Matrix4& value)
    {return value.toString();}

  static inline void write(OutputStream& ostr, const Matrix4& value)
  {
    for (size_t i = 0; i < 16; ++i)
      lbcpp::write(ostr, value._m[i]);
  }

  static inline bool read(InputStream& istr, Matrix4& res)
  {
    for (size_t i = 0; i < 16; ++i)
    if (!lbcpp::read(istr, res._m[i]))
      return false;
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_GEOMETRY_MATRIX4_H_

/*-----------------------------------------.---------------------------------.
| Filename: Matrix3.h                      | 3x3 Matrix                      |
| Author  : Francis Maes                   |                                 |
| Started : 25/04/2010 15:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_GEOMETRY_MATRIX3_H_
# define LBCPP_PROTEIN_GEOMETRY_MATRIX3_H_

# include "Vector3.h"

namespace lbcpp
{

namespace impl
{

class Matrix3
{
public:
  Matrix3() {}

  Matrix3(double m00, double m01, double m02,
          double m10, double m11, double m12,
          double m20, double m21, double m22)
  {
    m[0][0] = m00;
    m[0][1] = m01;
    m[0][2] = m02;
    m[1][0] = m10;
    m[1][1] = m11;
    m[1][2] = m12;
    m[2][0] = m20;
    m[2][1] = m21;
    m[2][2] = m22;
  }

  static const Matrix3 zero;
  static const Matrix3 identity;

  static Matrix3 diagonal(const Vector3& diag)
    {return Matrix3(diag.getX(), 0.0, 0.0, 0.0, diag.getY(), 0.0, 0.0, 0.0, diag.getZ());}

  String toString() const;

  double* operator [](size_t row)
    {jassert(row < 3); return m[row];}

  const double* const operator [](size_t row) const
    {jassert(row < 3); return m[row];}

  double sumOfSquares() const
  {
    return  m[0][0] * m[0][0] + m[0][1] * m[0][1] + m[0][2] * m[0][2] +
            m[1][0] * m[1][0] + m[1][1] * m[1][1] + m[1][2] * m[1][2] +
            m[2][0] * m[2][0] + m[2][1] * m[2][1] + m[2][2] * m[2][2];
  }

  double l2norm() const
    {return sqrt(sumOfSquares());}

  Matrix3 transposed() const
  {
    return Matrix3( m[0][0], m[1][0], m[2][0],
                    m[0][1], m[1][1], m[2][1],
                    m[0][2], m[1][2], m[2][2]);
  }

  Matrix3 operator -(const Matrix3& otherMatrix) const
  {
    return Matrix3(
      m[0][0] - otherMatrix.m[0][0], m[0][1] - otherMatrix.m[0][1], m[0][2] - otherMatrix.m[0][2],
      m[1][0] - otherMatrix.m[1][0], m[1][1] - otherMatrix.m[1][1], m[1][2] - otherMatrix.m[1][2],
      m[2][0] - otherMatrix.m[2][0], m[2][1] - otherMatrix.m[2][1], m[2][2] - otherMatrix.m[2][2]);
  }

  inline bool operator ==(const Matrix3& m2) const
  {
    if (m[0][0] != m2.m[0][0] || m[0][1] != m2.m[0][1] || m[0][2] != m2.m[0][2] ||
        m[1][0] != m2.m[1][0] || m[1][1] != m2.m[1][1] || m[1][2] != m2.m[1][2] ||
        m[2][0] != m2.m[2][0] || m[2][1] != m2.m[2][1] || m[2][2] != m2.m[2][2])
        return false;
    return true;
  }

  inline bool operator !=(const Matrix3& m2) const
  {
    if (m[0][0] != m2.m[0][0] || m[0][1] != m2.m[0][1] || m[0][2] != m2.m[0][2] ||
        m[1][0] != m2.m[1][0] || m[1][1] != m2.m[1][1] || m[1][2] != m2.m[1][2] ||
        m[2][0] != m2.m[2][0] || m[2][1] != m2.m[2][1] || m[2][2] != m2.m[2][2])
      return true;
    return false;
  }

  bool isSymmetric() const
    {return m[0][1] == m[1][0] && m[0][2] == m[2][0] && m[1][2] == m[2][1];}

  Vector3 transformAffine(const Vector3& v) const
  {
    return Vector3( m[0][0] * v.getX() + m[0][1] * v.getY() + m[0][2] * v.getZ(),
                    m[1][0] * v.getX() + m[1][1] * v.getY() + m[1][2] * v.getZ(),
                    m[2][0] * v.getX() + m[2][1] * v.getY() + m[2][2] * v.getZ());
  }

  void addCorrelation(const Vector3& v1, const Vector3& v2)
  {
    m[0][0] += v1.getX() * v2.getX(); m[0][1] += v1.getX() * v2.getY(); m[0][2] += v1.getX() * v2.getZ();
    m[1][0] += v1.getY() * v2.getX(); m[1][1] += v1.getY() * v2.getY(); m[1][2] += v1.getY() * v2.getZ();
    m[2][0] += v1.getZ() * v2.getX(); m[2][1] += v1.getZ() * v2.getY(); m[2][2] += v1.getZ() * v2.getZ();
 }

 bool makeSVDDecomposition(Matrix3& u, Vector3& d, Matrix3& v) const;

 Matrix3 operator *(const Matrix3& otherMatrix) const;

private:
  union
  {
    double m[3][3];
    double _m[9];
  };

  friend struct Traits<Matrix3>;
};

inline Vector3 operator *(const Vector3& v, const Matrix3& mat)
  {return mat.transformAffine(v);}

}; /* namespace impl */

template<>
struct Traits<impl::Matrix3>
{
  typedef impl::Matrix3 Type;

  static inline String toString(const impl::Matrix3& value)
    {return value.toString();}

  static inline void write(OutputStream& ostr, const impl::Matrix3& value)
  {
    for (size_t i = 0; i < 9; ++i)
      lbcpp::write(ostr, value._m[i]);
  }

  static inline bool read(InputStream& istr, impl::Matrix3& res)
  {
    for (size_t i = 0; i < 9; ++i)
    if (!lbcpp::read(istr, res._m[i]))
      return false;
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_GEOMETRY_MATRIX3_H_

/*-----------------------------------------.---------------------------------.
| Filename: Vector3.h                      | Vector in a 3D-space            |
| Author  : Francis Maes                   |                                 |
| Started : 20/04/2010 18:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_GEOMETRY_VECTOR3_H_
# define LBCPP_PROTEIN_GEOMETRY_VECTOR3_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class Vector3
{
public:
  Vector3(double x, double y, double z) : x(x), y(y), z(z) {}
  Vector3(double value) : x(value), y(value), z(value) {}
  Vector3() : x(DBL_MAX), y(DBL_MAX), z(DBL_MAX) {}

  bool exists() const
    {return x != DBL_MAX || y != DBL_MAX || z != DBL_MAX;}

  String toString() const
    {return T("(") + lbcpp::toString(x) + T(", ") + lbcpp::toString(y) + T(", ") + lbcpp::toString(z) + T(")");}

  double getX() const
    {return x;}

  double getY() const
    {return y;}

  double getZ() const
    {return z;}

  void set(double x, double y, double z)
    {this->x = x; this->y = y; this->z = z;}

  Vector3 operator +(const Vector3& otherVector) const
    {return Vector3(x + otherVector.x, y + otherVector.y, z + otherVector.z);}

  Vector3 operator -(const Vector3& otherVector) const
    {return Vector3(x - otherVector.x, y - otherVector.y, z - otherVector.z);}

  Vector3 operator *(const double value) const
    {return Vector3(x * value, y * value, z * value);}

  Vector3 operator /(const double value) const
  {
    jassert(value);
    double inverseValue = 1.0 / value;
    return (*this) * inverseValue;
  }

  Vector3& operator +=(const Vector3& otherVector)
  {
    x += otherVector.x;
    y += otherVector.y;
    z += otherVector.z;
    return *this;
  }

  Vector3& operator -=(const Vector3& otherVector)
  {
    x -= otherVector.x;
    y -= otherVector.y;
    z -= otherVector.z;
    return *this;
  }

  Vector3& operator *=(const double value)
    {x *= value; y *= value; z *= value; return *this;}

  Vector3& operator /=(const double value)
  {
    jassert(value);
    double inverseValue = 1.0 / value;
    return (*this) *= inverseValue;
  }

  double dotProduct(const Vector3& otherVector) const
    {return x * otherVector.x + y * otherVector.y + z * otherVector.z;}

  double angle(const Vector3& otherVector) const
  {
    double n1 = sumOfSquares();
    double n2 = otherVector.sumOfSquares();
    if (!n1 || !n2)
      return 0.0;
    return acos(dotProduct(otherVector) / sqrt(n1 * n2));
  }

  Vector3 crossProduct(const Vector3& otherVector) const
  {
    return Vector3(y * otherVector.z - otherVector.y * z,
                    z * otherVector.x - otherVector.z * x,
                    x * otherVector.y - otherVector.x * y);
  }

  double sumOfSquares() const
    {return x * x + y *y + z * z;}

  double l2norm() const
    {return sqrt(sumOfSquares());}

  Vector3 normalized() const
  {
    double s = sumOfSquares();
    if (s == 1)
      return *this;
    else
    {
      s = 1.0 / sqrt(s);
      return (*this) * s;
    }
  }

  bool operator ==(const Vector3& otherVector) const
    {return x == otherVector.x && y == otherVector.y && z == otherVector.z;}

  bool operator !=(const Vector3& otherVector) const
    {return x != otherVector.x || y != otherVector.y || z != otherVector.z;}

private:
  double x, y, z;
};

template<>
struct Traits<Vector3>
{
  typedef Vector3 Type;

  static inline String toString(const Vector3& value)
    {return value.toString();}

  static inline void write(OutputStream& ostr, const Vector3& value)
  {
    lbcpp::write(ostr, value.getX());
    lbcpp::write(ostr, value.getY());
    lbcpp::write(ostr, value.getZ());
  }

  static inline bool read(InputStream& istr, Vector3& res)
  {
    double x, y, z;
    if (!lbcpp::read(istr, x) || !lbcpp::read(istr, y) || !lbcpp::read(istr, z))
      return false;
    res.set(x, y, z);
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_GEOMETRY_VECTOR3_H_

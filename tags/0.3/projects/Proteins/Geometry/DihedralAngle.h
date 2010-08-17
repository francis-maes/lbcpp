/*-----------------------------------------.---------------------------------.
| Filename: DihedralAngle.h                | Dihedral angle (in [-PI, PI])   |
| Author  : Francis Maes                   |                                 |
| Started : 20/04/2010 18:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_GEOMETRY_DIHEDRAL_ANGLE_H_
# define LBCPP_PROTEIN_GEOMETRY_DIHEDRAL_ANGLE_H_

# include "Vector3.h"

namespace lbcpp
{

namespace impl
{

class Angle
{
public:
  Angle(Vector3& a, Vector3& b, Vector3& c)
    : angle((a - b).angle(c - b)) {}

  Angle(double angle = M_2_TIMES_PI)
    : angle(angle) {}

  operator double () const
    {return angle;}

  operator double& ()
    {return angle;}

  bool exists() const
    {return angle != M_2_TIMES_PI;}

private:
  double angle;
};

class DihedralAngle
{
public:
  DihedralAngle(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d)
    : angle(compute(a, b, c, d)) {}
  DihedralAngle(double angle = M_2_TIMES_PI)
    : angle(angle) {}

  bool exists() const
    {return angle != M_2_TIMES_PI;}

  static double normalize(double angle)
    {return normalizeAngle(angle);}

  static double compute(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d)
  {    
    Vector3 ba = a - b;
    Vector3 bc = c - b;
    Vector3 dc = c - d;
    Vector3 normalB = ba.crossProduct(bc);
    Vector3 normalC = bc.crossProduct(dc);
    double res = normalB.angle(normalC);
    return bc.dotProduct(normalB.crossProduct(normalC)) >= 0 ? res : -res;
  }

  operator double() const
    {return angle;}

  operator double& ()
    {return angle;}

  DihedralAngle& operator =(double angle)
    {this->angle = angle; return *this;}

  DihedralAngle normalized() const
    {return exists() ? DihedralAngle(normalize(angle)) : DihedralAngle();}

private:
  double angle;
};

typedef std::pair<DihedralAngle, DihedralAngle> DihedralAnglesPair;

}; /* namespace impl */


template<>
struct Traits<impl::Angle>
{
  typedef impl::Angle Type;

  static inline String toString(const impl::Angle& value)
    {return String((double)value * 180 / M_PI, 1);}

  static inline void write(OutputStream& ostr, const impl::Angle& value)
    {lbcpp::write(ostr, (double)value);}

  static inline bool read(InputStream& istr, impl::Angle& res)
  {
    double value;
    if (!lbcpp::read(istr, value))
      return false;
    res = value;
    return true;
  }
};

template<>
struct Traits<impl::DihedralAngle>
{
  typedef impl::DihedralAngle Type;

  static inline String toString(const impl::DihedralAngle& value)
    {return String((double)value * 180 / M_PI, 1);}

  static inline void write(OutputStream& ostr, const impl::DihedralAngle& value)
    {lbcpp::write(ostr, (double)value);}

  static inline bool read(InputStream& istr, impl::DihedralAngle& res)
  {
    double value;
    if (!lbcpp::read(istr, value))
      return false;
    res = value;
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_GEOMETRY_DIHEDRAL_ANGLE_H_

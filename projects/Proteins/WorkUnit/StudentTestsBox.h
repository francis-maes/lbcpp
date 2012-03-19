
#define ae_bool bool
#define ae_true true
#define ae_false false

#define DYN_BOTTOM ((void*)1)
#define DYN_FRAME  ((void*)2)

#define AE_LITTLE_ENDIAN 1
#define AE_BIG_ENDIAN 2
#define AE_MIXED_ENDIAN 3

#define ae_machineepsilon 5E-16
#define ae_maxrealnumber  1E300
#define ae_minrealnumber  1E-300
#define ae_pi 3.1415926535897932384626433832795

typedef ptrdiff_t ae_int_t;
typedef juce::int32 AE_INT32_T; // FIX
typedef AE_INT32_T ae_int32_t;
typedef void(*ae_deallocator)(void*);

typedef enum
{
  ERR_OK = 0,
  ERR_OUT_OF_MEMORY = 1,
  ERR_XARRAY_TOO_LARGE = 2,
  ERR_ASSERTION_FAILED = 3
} ae_error_type;

/************************************************************************
dynamic block which may be automatically deallocated during stack unwinding

p_next          next block in the stack unwinding list.
                NULL means that this block is not in the list
deallocator     deallocator function which should be used to deallocate block.
                NULL for "special" blocks (frame/stack boundaries)
ptr             pointer which should be passed to the deallocator.
                may be null (for zero-size block), DYN_BOTTOM or DYN_FRAME
                for "special" blocks (frame/stack boundaries).

************************************************************************/
typedef struct ae_dyn_block
{
  struct ae_dyn_block * volatile p_next;
  /* void *deallocator; */
  void (*deallocator)(void*);
  void * volatile ptr;
} ae_dyn_block;


/************************************************************************
ALGLIB environment state
************************************************************************/
typedef struct
{
  ae_int_t endianness;
  double v_nan;
  double v_posinf;
  double v_neginf;
  
  ae_dyn_block * volatile p_top_block;
  ae_dyn_block last_block;
  
  ae_error_type volatile last_error;
  const char* volatile error_msg;
} ae_state;

/************************************************************************
 Real math functions
 ************************************************************************/
ae_bool ae_fp_eq(double v1, double v2)
{
  /* IEEE-strict floating point comparison */
  volatile double x = v1;
  volatile double y = v2;
  return x==y;
}

ae_bool ae_fp_less(double v1, double v2)
{
  /* IEEE-strict floating point comparison */
  volatile double x = v1;
  volatile double y = v2;
  return x<y;
}

double ae_sqrt(double x, ae_state *state)
{
  return sqrt(x);
}

double ae_atan(double x, ae_state *state)
{
  return atan(x);
}

ae_bool ae_fp_greater(double v1, double v2)
{
  /* IEEE-strict floating point comparison */
  volatile double x = v1;
  volatile double y = v2;
  return x>y;
}

ae_bool ae_fp_greater_eq(double v1, double v2)
{
  /* IEEE-strict floating point comparison */
  volatile double x = v1;
  volatile double y = v2;
  return x>=y;
}

double ae_fabs(double x,  ae_state *state)
{
  return fabs(x);
}

double ae_log(double x, ae_state *state)
{
  return log(x);
}

double ae_exp(double x, ae_state *state)
{
  return exp(x);
}

ae_int_t ae_ifloor(double x, ae_state *state)
{
  return (ae_int_t)(floor(x));
}

ae_int_t ae_round(double x, ae_state *state)
{
  return (ae_int_t)(ae_ifloor(x+0.5,state));
}

double ae_sin(double x, ae_state *state)
{
  return sin(x);
}

double ae_pow(double x, double y, ae_state *state)
{
  return pow(x,y);
}

ae_bool ae_fp_neq(double v1, double v2)
{
  /* IEEE-strict floating point comparison */
  return !ae_fp_eq(v1,v2);
}

ae_bool ae_fp_less_eq(double v1, double v2)
{
  /* IEEE-strict floating point comparison */
  volatile double x = v1;
  volatile double y = v2;
  return x<=y;
}

ae_int_t ae_get_endianness()
{
  union
  {
    double a;
    ae_int32_t p[2];
  } u;
  
  /*
   * determine endianness
   * two types are supported: big-endian and little-endian.
   * mixed-endian hardware is NOT supported.
   *
   * 1983 is used as magic number because its non-periodic double 
   * representation allow us to easily distinguish between upper 
   * and lower halfs and to detect mixed endian hardware.
   *
   */
  u.a = 1.0/1983.0; 
  if( u.p[1]==(ae_int32_t)0x3f408642 )
    return AE_LITTLE_ENDIAN;
  if( u.p[0]==(ae_int32_t)0x3f408642 )
    return AE_BIG_ENDIAN;
  return AE_MIXED_ENDIAN;
}

/************************************************************************
 This function initializes ALGLIB environment state.
 
 NOTES:
 * stacks contain no frames, so ae_make_frame() must be called before
 attaching dynamic blocks. Without it ae_leave_frame() will cycle
 forever (which is intended behavior).
 ************************************************************************/
void ae_state_init(ae_state *state)
{
  ae_int32_t *vp;
  
  /*
   * p_next points to itself because:
   * * correct program should be able to detect end of the list
   *   by looking at the ptr field.
   * * NULL p_next may be used to distinguish automatic blocks
   *   (in the list) from non-automatic (not in the list)
   */
  state->last_block.p_next = &(state->last_block);
  state->last_block.deallocator = NULL;
  state->last_block.ptr = DYN_BOTTOM;
  state->p_top_block = &(state->last_block);
  state->error_msg = "";
  
  /*
   * determine endianness and initialize precomputed IEEE special quantities.
   */
  state->endianness = ae_get_endianness();
  if( state->endianness==AE_LITTLE_ENDIAN )
  {
    vp = (ae_int32_t*)(&state->v_nan);
    vp[0] = 0;
    vp[1] = (ae_int32_t)0x7FF80000;
    vp = (ae_int32_t*)(&state->v_posinf);
    vp[0] = 0;
    vp[1] = (ae_int32_t)0x7FF00000;
    vp = (ae_int32_t*)(&state->v_neginf);
    vp[0] = 0;
    vp[1] = (ae_int32_t)0xFFF00000;
  }
  else if( state->endianness==AE_BIG_ENDIAN )
  {
    vp = (ae_int32_t*)(&state->v_nan);
    vp[1] = 0;
    vp[0] = (ae_int32_t)0x7FF80000;
    vp = (ae_int32_t*)(&state->v_posinf);
    vp[1] = 0;
    vp[0] = (ae_int32_t)0x7FF00000;
    vp = (ae_int32_t*)(&state->v_neginf);
    vp[1] = 0;
    vp[0] = (ae_int32_t)0xFFF00000;
  }
  else
    jassertfalse;
}

/************************************************************************
 This function leaves current stack frame and deallocates all automatic
 dynamic blocks which were attached to this frame.
 ************************************************************************/
void ae_frame_leave(ae_state *state)
{
  while( state->p_top_block->ptr!=DYN_FRAME && state->p_top_block->ptr!=DYN_BOTTOM)
  {
    if( state->p_top_block->ptr!=NULL && state->p_top_block->deallocator!=NULL)
      ((ae_deallocator)(state->p_top_block->deallocator))(state->p_top_block->ptr);
    state->p_top_block = state->p_top_block->p_next;
  }
  state->p_top_block = state->p_top_block->p_next;
}

/************************************************************************
 This function clears ALGLIB environment state.
 All dynamic data controlled by state are freed.
 ************************************************************************/
void ae_state_clear(ae_state *state)
{
  while( state->p_top_block->ptr!=DYN_BOTTOM )
    ae_frame_leave(state);
}

static double gammafunc_gammastirf(double x, ae_state *_state)
{
  double y;
  double w;
  double v;
  double stir;
  double result;
  
  
  w = 1/x;
  stir = 7.87311395793093628397E-4;
  stir = -2.29549961613378126380E-4+w*stir;
  stir = -2.68132617805781232825E-3+w*stir;
  stir = 3.47222221605458667310E-3+w*stir;
  stir = 8.33333333333482257126E-2+w*stir;
  w = 1+w*stir;
  y = ae_exp(x, _state);
  if( ae_fp_greater(x,143.01608) )
  {
    v = ae_pow(x, 0.5*x-0.25, _state);
    y = v*(v/y);
  }
  else
  {
    y = ae_pow(x, x-0.5, _state)/y;
  }
  result = 2.50662827463100050242*y*w;
  return result;
}

/*************************************************************************
 Gamma function
 
 Input parameters:
 X   -   argument
 
 Domain:
 0 < X < 171.6
 -170 < X < 0, X is not an integer.
 
 Relative error:
 arithmetic   domain     # trials      peak         rms
 IEEE    -170,-33      20000       2.3e-15     3.3e-16
 IEEE     -33,  33     20000       9.4e-16     2.2e-16
 IEEE      33, 171.6   20000       2.3e-15     3.2e-16
 
 Cephes Math Library Release 2.8:  June, 2000
 Original copyright 1984, 1987, 1989, 1992, 2000 by Stephen L. Moshier
 Translated to AlgoPascal by Bochkanov Sergey (2005, 2006, 2007).
 *************************************************************************/
double gammafunction(double x, ae_state *_state)
{
  double p;
  double pp;
  double q;
  double qq;
  double z;
  ae_int_t i;
  double sgngam;
  double result;
  
  
  sgngam = 1;
  q = ae_fabs(x, _state);
  if( ae_fp_greater(q,33.0) )
  {
    if( ae_fp_less(x,0.0) )
    {
      p = ae_ifloor(q, _state);
      i = ae_round(p, _state);
      if( i%2==0 )
      {
        sgngam = -1;
      }
      z = q-p;
      if( ae_fp_greater(z,0.5) )
      {
        p = p+1;
        z = q-p;
      }
      z = q*ae_sin(ae_pi*z, _state);
      z = ae_fabs(z, _state);
      z = ae_pi/(z*gammafunc_gammastirf(q, _state));
    }
    else
    {
      z = gammafunc_gammastirf(x, _state);
    }
    result = sgngam*z;
    return result;
  }
  z = 1;
  while(ae_fp_greater_eq(x,3))
  {
    x = x-1;
    z = z*x;
  }
  while(ae_fp_less(x,0))
  {
    if( ae_fp_greater(x,-0.000000001) )
    {
      result = z/((1+0.5772156649015329*x)*x);
      return result;
    }
    z = z/x;
    x = x+1;
  }
  while(ae_fp_less(x,2))
  {
    if( ae_fp_less(x,0.000000001) )
    {
      result = z/((1+0.5772156649015329*x)*x);
      return result;
    }
    z = z/x;
    x = x+1.0;
  }
  if( ae_fp_eq(x,2) )
  {
    result = z;
    return result;
  }
  x = x-2.0;
  pp = 1.60119522476751861407E-4;
  pp = 1.19135147006586384913E-3+x*pp;
  pp = 1.04213797561761569935E-2+x*pp;
  pp = 4.76367800457137231464E-2+x*pp;
  pp = 2.07448227648435975150E-1+x*pp;
  pp = 4.94214826801497100753E-1+x*pp;
  pp = 9.99999999999999996796E-1+x*pp;
  qq = -2.31581873324120129819E-5;
  qq = 5.39605580493303397842E-4+x*qq;
  qq = -4.45641913851797240494E-3+x*qq;
  qq = 1.18139785222060435552E-2+x*qq;
  qq = 3.58236398605498653373E-2+x*qq;
  qq = -2.34591795718243348568E-1+x*qq;
  qq = 7.14304917030273074085E-2+x*qq;
  qq = 1.00000000000000000320+x*qq;
  result = z*pp/qq;
  return result;
}


/*************************************************************************
 Natural logarithm of gamma function
 
 Input parameters:
 X       -   argument
 
 Result:
 logarithm of the absolute value of the Gamma(X).
 
 Output parameters:
 SgnGam  -   sign(Gamma(X))
 
 Domain:
 0 < X < 2.55e305
 -2.55e305 < X < 0, X is not an integer.
 
 ACCURACY:
 arithmetic      domain        # trials     peak         rms
 IEEE    0, 3                 28000     5.4e-16     1.1e-16
 IEEE    2.718, 2.556e305     40000     3.5e-16     8.3e-17
 The error criterion was relative when the function magnitude
 was greater than one but absolute when it was less than one.
 
 The following test used the relative error criterion, though
 at certain points the relative error could be much higher than
 indicated.
 IEEE    -200, -4             10000     4.8e-16     1.3e-16
 
 Cephes Math Library Release 2.8:  June, 2000
 Copyright 1984, 1987, 1989, 1992, 2000 by Stephen L. Moshier
 Translated to AlgoPascal by Bochkanov Sergey (2005, 2006, 2007).
 *************************************************************************/
double lngamma(double x, double* sgngam, ae_state *_state)
{
  double a;
  double b;
  double c;
  double p;
  double q;
  double u;
  double w;
  double z;
  ae_int_t i;
  double logpi;
  double ls2pi;
  double tmp;
  double result;
  
  *sgngam = 0;
  
  *sgngam = 1;
  logpi = 1.14472988584940017414;
  ls2pi = 0.91893853320467274178;
  if( ae_fp_less(x,-34.0) )
  {
    q = -x;
    w = lngamma(q, &tmp, _state);
    p = ae_ifloor(q, _state);
    i = ae_round(p, _state);
    if( i%2==0 )
    {
      *sgngam = -1;
    }
    else
    {
      *sgngam = 1;
    }
    z = q-p;
    if( ae_fp_greater(z,0.5) )
    {
      p = p+1;
      z = p-q;
    }
    z = q*ae_sin(ae_pi*z, _state);
    result = logpi-ae_log(z, _state)-w;
    return result;
  }
  if( ae_fp_less(x,13) )
  {
    z = 1;
    p = 0;
    u = x;
    while(ae_fp_greater_eq(u,3))
    {
      p = p-1;
      u = x+p;
      z = z*u;
    }
    while(ae_fp_less(u,2))
    {
      z = z/u;
      p = p+1;
      u = x+p;
    }
    if( ae_fp_less(z,0) )
    {
      *sgngam = -1;
      z = -z;
    }
    else
    {
      *sgngam = 1;
    }
    if( ae_fp_eq(u,2) )
    {
      result = ae_log(z, _state);
      return result;
    }
    p = p-2;
    x = x+p;
    b = -1378.25152569120859100;
    b = -38801.6315134637840924+x*b;
    b = -331612.992738871184744+x*b;
    b = -1162370.97492762307383+x*b;
    b = -1721737.00820839662146+x*b;
    b = -853555.664245765465627+x*b;
    c = 1;
    c = -351.815701436523470549+x*c;
    c = -17064.2106651881159223+x*c;
    c = -220528.590553854454839+x*c;
    c = -1139334.44367982507207+x*c;
    c = -2532523.07177582951285+x*c;
    c = -2018891.41433532773231+x*c;
    p = x*b/c;
    result = ae_log(z, _state)+p;
    return result;
  }
  q = (x-0.5)*ae_log(x, _state)-x+ls2pi;
  if( ae_fp_greater(x,100000000) )
  {
    result = q;
    return result;
  }
  p = 1/(x*x);
  if( ae_fp_greater_eq(x,1000.0) )
  {
    q = q+((7.9365079365079365079365*0.0001*p-2.7777777777777777777778*0.001)*p+0.0833333333333333333333)/x;
  }
  else
  {
    a = 8.11614167470508450300*0.0001;
    a = -5.95061904284301438324*0.0001+p*a;
    a = 7.93650340457716943945*0.0001+p*a;
    a = -2.77777777730099687205*0.001+p*a;
    a = 8.33333333333331927722*0.01+p*a;
    q = q+a/x;
  }
  result = q;
  return result;
}

/*************************************************************************
 Continued fraction expansion #1 for incomplete beta integral
 
 Cephes Math Library, Release 2.8:  June, 2000
 Copyright 1984, 1995, 2000 by Stephen L. Moshier
 *************************************************************************/
static double ibetaf_incompletebetafe(double a,
                                      double b,
                                      double x,
                                      double big,
                                      double biginv,
                                      ae_state *_state)
{
  double xk;
  double pk;
  double pkm1;
  double pkm2;
  double qk;
  double qkm1;
  double qkm2;
  double k1;
  double k2;
  double k3;
  double k4;
  double k5;
  double k6;
  double k7;
  double k8;
  double r;
  double t;
  double ans;
  double thresh;
  ae_int_t n;
  double result;
  
  
  k1 = a;
  k2 = a+b;
  k3 = a;
  k4 = a+1.0;
  k5 = 1.0;
  k6 = b-1.0;
  k7 = k4;
  k8 = a+2.0;
  pkm2 = 0.0;
  qkm2 = 1.0;
  pkm1 = 1.0;
  qkm1 = 1.0;
  ans = 1.0;
  r = 1.0;
  n = 0;
  thresh = 3.0*ae_machineepsilon;
  do
  {
    xk = -x*k1*k2/(k3*k4);
    pk = pkm1+pkm2*xk;
    qk = qkm1+qkm2*xk;
    pkm2 = pkm1;
    pkm1 = pk;
    qkm2 = qkm1;
    qkm1 = qk;
    xk = x*k5*k6/(k7*k8);
    pk = pkm1+pkm2*xk;
    qk = qkm1+qkm2*xk;
    pkm2 = pkm1;
    pkm1 = pk;
    qkm2 = qkm1;
    qkm1 = qk;
    if( ae_fp_neq(qk,0) )
    {
      r = pk/qk;
    }
    if( ae_fp_neq(r,0) )
    {
      t = ae_fabs((ans-r)/r, _state);
      ans = r;
    }
    else
    {
      t = 1.0;
    }
    if( ae_fp_less(t,thresh) )
    {
      break;
    }
    k1 = k1+1.0;
    k2 = k2+1.0;
    k3 = k3+2.0;
    k4 = k4+2.0;
    k5 = k5+1.0;
    k6 = k6-1.0;
    k7 = k7+2.0;
    k8 = k8+2.0;
    if( ae_fp_greater(ae_fabs(qk, _state)+ae_fabs(pk, _state),big) )
    {
      pkm2 = pkm2*biginv;
      pkm1 = pkm1*biginv;
      qkm2 = qkm2*biginv;
      qkm1 = qkm1*biginv;
    }
    if( ae_fp_less(ae_fabs(qk, _state),biginv)||ae_fp_less(ae_fabs(pk, _state),biginv) )
    {
      pkm2 = pkm2*big;
      pkm1 = pkm1*big;
      qkm2 = qkm2*big;
      qkm1 = qkm1*big;
    }
    n = n+1;
  }
  while(n!=300);
  result = ans;
  return result;
}


/*************************************************************************
 Continued fraction expansion #2
 for incomplete beta integral
 
 Cephes Math Library, Release 2.8:  June, 2000
 Copyright 1984, 1995, 2000 by Stephen L. Moshier
 *************************************************************************/
static double ibetaf_incompletebetafe2(double a,
                                       double b,
                                       double x,
                                       double big,
                                       double biginv,
                                       ae_state *_state)
{
  double xk;
  double pk;
  double pkm1;
  double pkm2;
  double qk;
  double qkm1;
  double qkm2;
  double k1;
  double k2;
  double k3;
  double k4;
  double k5;
  double k6;
  double k7;
  double k8;
  double r;
  double t;
  double ans;
  double z;
  double thresh;
  ae_int_t n;
  double result;
  
  
  k1 = a;
  k2 = b-1.0;
  k3 = a;
  k4 = a+1.0;
  k5 = 1.0;
  k6 = a+b;
  k7 = a+1.0;
  k8 = a+2.0;
  pkm2 = 0.0;
  qkm2 = 1.0;
  pkm1 = 1.0;
  qkm1 = 1.0;
  z = x/(1.0-x);
  ans = 1.0;
  r = 1.0;
  n = 0;
  thresh = 3.0*ae_machineepsilon;
  do
  {
    xk = -z*k1*k2/(k3*k4);
    pk = pkm1+pkm2*xk;
    qk = qkm1+qkm2*xk;
    pkm2 = pkm1;
    pkm1 = pk;
    qkm2 = qkm1;
    qkm1 = qk;
    xk = z*k5*k6/(k7*k8);
    pk = pkm1+pkm2*xk;
    qk = qkm1+qkm2*xk;
    pkm2 = pkm1;
    pkm1 = pk;
    qkm2 = qkm1;
    qkm1 = qk;
    if( ae_fp_neq(qk,0) )
    {
      r = pk/qk;
    }
    if( ae_fp_neq(r,0) )
    {
      t = ae_fabs((ans-r)/r, _state);
      ans = r;
    }
    else
    {
      t = 1.0;
    }
    if( ae_fp_less(t,thresh) )
    {
      break;
    }
    k1 = k1+1.0;
    k2 = k2-1.0;
    k3 = k3+2.0;
    k4 = k4+2.0;
    k5 = k5+1.0;
    k6 = k6+1.0;
    k7 = k7+2.0;
    k8 = k8+2.0;
    if( ae_fp_greater(ae_fabs(qk, _state)+ae_fabs(pk, _state),big) )
    {
      pkm2 = pkm2*biginv;
      pkm1 = pkm1*biginv;
      qkm2 = qkm2*biginv;
      qkm1 = qkm1*biginv;
    }
    if( ae_fp_less(ae_fabs(qk, _state),biginv)||ae_fp_less(ae_fabs(pk, _state),biginv) )
    {
      pkm2 = pkm2*big;
      pkm1 = pkm1*big;
      qkm2 = qkm2*big;
      qkm1 = qkm1*big;
    }
    n = n+1;
  }
  while(n!=300);
  result = ans;
  return result;
}


/*************************************************************************
 Power series for incomplete beta integral.
 Use when b*x is small and x not too close to 1.
 
 Cephes Math Library, Release 2.8:  June, 2000
 Copyright 1984, 1995, 2000 by Stephen L. Moshier
 *************************************************************************/
static double ibetaf_incompletebetaps(double a,
                                      double b,
                                      double x,
                                      double maxgam,
                                      ae_state *_state)
{
  double s;
  double t;
  double u;
  double v;
  double n;
  double t1;
  double z;
  double ai;
  double sg;
  double result;
  
  
  ai = 1.0/a;
  u = (1.0-b)*x;
  v = u/(a+1.0);
  t1 = v;
  t = u;
  n = 2.0;
  s = 0.0;
  z = ae_machineepsilon*ai;
  while(ae_fp_greater(ae_fabs(v, _state),z))
  {
    u = (n-b)*x/n;
    t = t*u;
    v = t/(a+n);
    s = s+v;
    n = n+1.0;
  }
  s = s+t1;
  s = s+ai;
  u = a*ae_log(x, _state);
  if( ae_fp_less(a+b,maxgam)&&ae_fp_less(ae_fabs(u, _state),ae_log(ae_maxrealnumber, _state)) )
  {
    t = gammafunction(a+b, _state)/(gammafunction(a, _state)*gammafunction(b, _state));
    s = s*t*ae_pow(x, a, _state);
  }
  else
  {
    t = lngamma(a+b, &sg, _state)-lngamma(a, &sg, _state)-lngamma(b, &sg, _state)+u+ae_log(s, _state);
    if( ae_fp_less(t,ae_log(ae_minrealnumber, _state)) )
    {
      s = 0.0;
    }
    else
    {
      s = ae_exp(t, _state);
    }
  }
  result = s;
  return result;
}

/*************************************************************************
 Incomplete beta integral
 
 Returns incomplete beta integral of the arguments, evaluated
 from zero to x.  The function is defined as
 
 x
 -            -
 | (a+b)      | |  a-1     b-1
 -----------    |   t   (1-t)   dt.
 -     -     | |
 | (a) | (b)   -
 0
 
 The domain of definition is 0 <= x <= 1.  In this
 implementation a and b are restricted to positive values.
 The integral from x to 1 may be obtained by the symmetry
 relation
 
 1 - incbet( a, b, x )  =  incbet( b, a, 1-x ).
 
 The integral is evaluated by a continued fraction expansion
 or, when b*x is small, by a power series.
 
 ACCURACY:
 
 Tested at uniformly distributed random points (a,b,x) with a and b
 in "domain" and x between 0 and 1.
 Relative error
 arithmetic   domain     # trials      peak         rms
 IEEE      0,5         10000       6.9e-15     4.5e-16
 IEEE      0,85       250000       2.2e-13     1.7e-14
 IEEE      0,1000      30000       5.3e-12     6.3e-13
 IEEE      0,10000    250000       9.3e-11     7.1e-12
 IEEE      0,100000    10000       8.7e-10     4.8e-11
 Outputs smaller than the IEEE gradual underflow threshold
 were excluded from these statistics.
 
 Cephes Math Library, Release 2.8:  June, 2000
 Copyright 1984, 1995, 2000 by Stephen L. Moshier
 *************************************************************************/
double incompletebeta(lbcpp::ExecutionContext& context, double a, double b, double x, ae_state *_state)
{
  double t;
  double xc;
  double w;
  double y;
  ae_int_t flag;
  double sg;
  double big;
  double biginv;
  double maxgam;
  double minlog;
  double maxlog;
  double result;
  
  
  big = 4.503599627370496e15;
  biginv = 2.22044604925031308085e-16;
  maxgam = 171.624376956302725;
  minlog = ae_log(ae_minrealnumber, _state);
  maxlog = ae_log(ae_maxrealnumber, _state);
  if (!(ae_fp_greater(a,0) && ae_fp_greater(b,0) && ae_fp_greater_eq(x,0) && ae_fp_less_eq(x,1)))
  {
    context.errorCallback(T("StudentDirstribution::incompletebeta"), T("Domain error in IncompleteBeta"));
    return -DBL_MAX;
  }
  //ae_assert(ae_fp_greater(a,0)&&ae_fp_greater(b,0), "Domain error in IncompleteBeta", _state);
  //ae_assert(ae_fp_greater_eq(x,0)&&ae_fp_less_eq(x,1), "Domain error in IncompleteBeta", _state);
  if( ae_fp_eq(x,0) )
  {
    result = 0;
    return result;
  }
  if( ae_fp_eq(x,1) )
  {
    result = 1;
    return result;
  }
  flag = 0;
  if( ae_fp_less_eq(b*x,1.0)&&ae_fp_less_eq(x,0.95) )
  {
    result = ibetaf_incompletebetaps(a, b, x, maxgam, _state);
    return result;
  }
  w = 1.0-x;
  if( ae_fp_greater(x,a/(a+b)) )
  {
    flag = 1;
    t = a;
    a = b;
    b = t;
    xc = x;
    x = w;
  }
  else
  {
    xc = w;
  }
  if( (flag==1&&ae_fp_less_eq(b*x,1.0))&&ae_fp_less_eq(x,0.95) )
  {
    t = ibetaf_incompletebetaps(a, b, x, maxgam, _state);
    if( ae_fp_less_eq(t,ae_machineepsilon) )
    {
      result = 1.0-ae_machineepsilon;
    }
    else
    {
      result = 1.0-t;
    }
    return result;
  }
  y = x*(a+b-2.0)-(a-1.0);
  if( ae_fp_less(y,0.0) )
  {
    w = ibetaf_incompletebetafe(a, b, x, big, biginv, _state);
  }
  else
  {
    w = ibetaf_incompletebetafe2(a, b, x, big, biginv, _state)/xc;
  }
  y = a*ae_log(x, _state);
  t = b*ae_log(xc, _state);
  if( (ae_fp_less(a+b,maxgam)&&ae_fp_less(ae_fabs(y, _state),maxlog))&&ae_fp_less(ae_fabs(t, _state),maxlog) )
  {
    t = ae_pow(xc, b, _state);
    t = t*ae_pow(x, a, _state);
    t = t/a;
    t = t*w;
    t = t*(gammafunction(a+b, _state)/(gammafunction(a, _state)*gammafunction(b, _state)));
    if( flag==1 )
    {
      if( ae_fp_less_eq(t,ae_machineepsilon) )
      {
        result = 1.0-ae_machineepsilon;
      }
      else
      {
        result = 1.0-t;
      }
    }
    else
    {
      result = t;
    }
    return result;
  }
  y = y+t+lngamma(a+b, &sg, _state)-lngamma(a, &sg, _state)-lngamma(b, &sg, _state);
  y = y+ae_log(w/a, _state);
  if( ae_fp_less(y,minlog) )
  {
    t = 0.0;
  }
  else
  {
    t = ae_exp(y, _state);
  }
  if( flag==1 )
  {
    if( ae_fp_less_eq(t,ae_machineepsilon) )
    {
      t = 1.0-ae_machineepsilon;
    }
    else
    {
      t = 1.0-t;
    }
  }
  result = t;
  return result;
}

/*************************************************************************
 Incomplete beta integral
 
 Returns incomplete beta integral of the arguments, evaluated
 from zero to x.  The function is defined as
 
 x
 -            -
 | (a+b)      | |  a-1     b-1
 -----------    |   t   (1-t)   dt.
 -     -     | |
 | (a) | (b)   -
 0
 
 The domain of definition is 0 <= x <= 1.  In this
 implementation a and b are restricted to positive values.
 The integral from x to 1 may be obtained by the symmetry
 relation
 
 1 - incbet( a, b, x )  =  incbet( b, a, 1-x ).
 
 The integral is evaluated by a continued fraction expansion
 or, when b*x is small, by a power series.
 
 ACCURACY:
 
 Tested at uniformly distributed random points (a,b,x) with a and b
 in "domain" and x between 0 and 1.
 Relative error
 arithmetic   domain     # trials      peak         rms
 IEEE      0,5         10000       6.9e-15     4.5e-16
 IEEE      0,85       250000       2.2e-13     1.7e-14
 IEEE      0,1000      30000       5.3e-12     6.3e-13
 IEEE      0,10000    250000       9.3e-11     7.1e-12
 IEEE      0,100000    10000       8.7e-10     4.8e-11
 Outputs smaller than the IEEE gradual underflow threshold
 were excluded from these statistics.
 
 Cephes Math Library, Release 2.8:  June, 2000
 Copyright 1984, 1995, 2000 by Stephen L. Moshier
 *************************************************************************/
double incompletebeta(lbcpp::ExecutionContext& context, const double a, const double b, const double x)
{
  ae_state _alglib_env_state;
  ae_state_init(&_alglib_env_state);
  try
  {
    double result = incompletebeta(context, a, b, x, &_alglib_env_state);
    ae_state_clear(&_alglib_env_state);
    return *(reinterpret_cast<double*>(&result));
  }
  catch(ae_error_type)
  {
    context.errorCallback(T("StudentDistribution::incompletebeta"), _alglib_env_state.error_msg);
    return -DBL_MAX;
  }
  catch(...)
  {
    context.errorCallback(T("StudentDistribution::incompletebeta"), T("Exception"));
    return -DBL_MAX;
  }
}

/*************************************************************************
Student's t distribution

Computes the integral from minus infinity to t of the Student
t distribution with integer k > 0 degrees of freedom:

                                     t
                                     -
                                    | |
             -                      |         2   -(k+1)/2
            | ( (k+1)/2 )           |  (     x   )
      ----------------------        |  ( 1 + --- )        dx
                    -               |  (      k  )
      sqrt( k pi ) | ( k/2 )        |
                                  | |
                                   -
                                  -inf.

Relation to incomplete beta integral:

       1 - stdtr(k,t) = 0.5 * incbet( k/2, 1/2, z )
where
       z = k/(k + t**2).

For t < -2, this is the method of computation.  For higher t,
a direct method is derived from integration by parts.
Since the function is symmetric about t=0, the area under the
right tail of the density is found by calling the function
with -t instead of t.

ACCURACY:

Tested at random 1 <= k <= 25.  The "domain" refers to t.
                     Relative error:
arithmetic   domain     # trials      peak         rms
   IEEE     -100,-2      50000       5.9e-15     1.4e-15
   IEEE     -2,100      500000       2.7e-15     4.9e-17

Cephes Math Library Release 2.8:  June, 2000
Copyright 1984, 1987, 1995, 2000 by Stephen L. Moshier
*************************************************************************/
double studenttdistribution(lbcpp::ExecutionContext& context, ae_int_t k, double t, ae_state *_state)
{
  double x;
  double rk;
  double z;
  double f;
  double tz;
  double p;
  double xsqk;
  ae_int_t j;
  double result;
  
  
  //ae_assert(k>0, "Domain error in StudentTDistribution", _state);
  if (!(k > 0))
  {
    context.errorCallback(T("studenttdistribution"), T("Domain error in StudentTDistribution"));
    return -DBL_MAX;
  }

  if( ae_fp_eq(t,0) )
  {
    result = 0.5;
    return result;
  }
  if( ae_fp_less(t,-2.0) )
  {
    rk = k;
    z = rk/(rk+t*t);
    result = 0.5*incompletebeta(context, 0.5*rk, 0.5, z, _state);
    return result;
  }
  if( ae_fp_less(t,0) )
  {
    x = -t;
  }
  else
  {
    x = t;
  }
  rk = k;
  z = 1.0+x*x/rk;
  if( k%2!=0 )
  {
    xsqk = x/ae_sqrt(rk, _state);
    p = ae_atan(xsqk, _state);
    if( k>1 )
    {
      f = 1.0;
      tz = 1.0;
      j = 3;
      while(j<=k-2&&ae_fp_greater(tz/f,ae_machineepsilon))
      {
        tz = tz*((j-1)/(z*j));
        f = f+tz;
        j = j+2;
      }
      p = p+f*xsqk/z;
    }
    p = p*2.0/ae_pi;
  }
  else
  {
    f = 1.0;
    tz = 1.0;
    j = 2;
    while(j<=k-2&&ae_fp_greater(tz/f,ae_machineepsilon))
    {
      tz = tz*((j-1)/(z*j));
      f = f+tz;
      j = j+2;
    }
    p = f*x/ae_sqrt(z*rk, _state);
  }
  if( ae_fp_less(t,0) )
  {
    p = -p;
  }
  result = 0.5+0.5*p;
  return result;
}

namespace lbcpp
{

extern double tDistributionOfStudent(ExecutionContext& context, size_t degreeOfFreedom, double t)
{
  ae_state _alglib_env_state;
  ae_state_init(&_alglib_env_state);
  double res = DBL_MAX;
  try
  {
    res = studenttdistribution(context, degreeOfFreedom, t, &_alglib_env_state);
    ae_state_clear(&_alglib_env_state);
  }
  catch(ae_error_type)
  {
    context.errorCallback(T("StudentDistribution::studentttest2"), _alglib_env_state.error_msg);
  }
  catch(...)
  {
    context.errorCallback(T("StudentDistribution::studentttest2"), T("Exception"));
  }
  return res;
}

class TestStudentDistributionWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    for (size_t df = 1; df < 20; df += 3)
    {
      context.enterScope(T("df = ") + String((int)df));
      context.resultCallback(T("df"), df);
      for (double t = -4.f; t < 4.f; t += 0.2)
      {
        context.enterScope(T("t = ") + String(t));
        context.resultCallback(T("t"), t);
        context.resultCallback(T("p"), tDistributionOfStudent(context, df, t));
        context.leaveScope();
      }
      context.leaveScope();
    }
    return true;
  }
};

};

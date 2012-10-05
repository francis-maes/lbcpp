
/*!         LIMITED MEMORY BFGS METHOD FOR LARGE SCALE OPTIMIZATION
                          JORGE NOCEDAL
                        *** July 1990 ***
     This subroutine solves the unconstrained minimization problem
                      min F(x),    x= (x1,x2,...,xN),
      using the limited memory BFGS method. The routine is especially
      effective on problems involving a large number of variables. In
      a typical iteration of this method an approximation Hk to the
      inverse of the Hessian is obtained by applying M BFGS updates to
      a diagonal matrix Hk0, using information from the previous M steps.
      The user specifies the number M, which determines the amount of
      storage required by the routine. The user may also provide the
      diagonal matrices Hk0 if not satisfied with the default choice.
      The algorithm is described in "On the limited memory BFGS method
      for large scale optimization", by D. Liu and J. Nocedal,
      Mathematical Programming B 45 (1989) 503-528.
      The user is required to calculate the function value F and its
      gradient G. In order to allow the user complete control over
      these computations, reverse  communication is used. The routine
      must be called repeatedly under the control of the parameter
      IFLAG.
      The steplength is determined at each iteration by means of the
      line search routine MCVSRCH, which is a slight modification of
      the routine CSRCH written by More' and Thuente.

      The calling statement is
          CALL LBFGS(N,M,X,F,G,DIAGCO,DIAG,IPRINT,EPS,XTOL,W,IFLAG)
      where

     N       is an  int variable that must be set by the user to the
             number of variables. It is not altered by the routine.
             Restriction: N>0.

     M       is an  int variable that must be set by the user to
             the number of corrections used in the BFGS update. It
             is not altered by the routine. Values of M less than 3 are
             not recommended; large values of M will result in excessive
             computing time. 3<= M <=7 is recommended. Restriction: M>0.


     X       is a DOUBLE PRECISION array of length N. On initial entry
             it must be set by the user to the values of the initial
             estimate of the solution vector. On exit with IFLAG=0, it
             contains the values of the variables at the best point
             found (usually a solution).

     F       is a DOUBLE PRECISION variable. Before initial entry and on
             a re-entry with IFLAG=1, it must be set by the user to
             contain the value of the function F at the point X.

     G       is a DOUBLE PRECISION array of length N. Before initial
             entry and on a re-entry with IFLAG=1, it must be set by
             the user to contain the components of the gradient G at
             the point X.

     DIAGCO  is a  int variable that must be set to .TRUE. if the
             user  wishes to provide the diagonal matrix Hk0 at each
             iteration. Otherwise it should be set to .FALSE., in which
             case  LBFGS will use a default value described below. If
             DIAGCO is set to .TRUE. the routine will return at each
             iteration of the algorithm with IFLAG=2, and the diagonal
              matrix Hk0  must be provided in the array DIAG.

     DIAG    is a DOUBLE PRECISION array of length N. If DIAGCO=.TRUE.,
             then on initial entry or on re-entry with IFLAG=2, DIAG
             it must be set by the user to contain the values of the
             diagonal matrix Hk0.  Restriction: all elements of DIAG
             must be positive.

     IPRINT  is an  int array of length two which must be set by the
             user.
             IPRINT(1) specifies the frequency of the output:
                IPRINT(1) < 0 : no output is generated,
                IPRINT(1) = 0 : output only at first and last iteration,
                IPRINT(1) > 0 : output every IPRINT(1) iterations.
                IPRINT(2) specifies the type of output generated:
                IPRINT(2) = 0 : iteration count, number of function
                                evaluations, function value, norm of the
                                gradient, and steplength,
                IPRINT(2) = 1 : same as IPRINT(2)=0, plus vector of
                                variables and  gradient vector at the
                                initial point,
                IPRINT(2) = 2 : same as IPRINT(2)=1, plus vector of
                                variables,
                IPRINT(2) = 3 : same as IPRINT(2)=2, plus gradient vector

     EPS     is a positive DOUBLE PRECISION variable that must be set by
             the user, and determines the accuracy with which the solution
             is to be found. The subroutine terminates when
                         ||G|| < EPS max(1,||X||),
             where ||.|| denotes the Euclidean norm.

    XTOL    is a  positive DOUBLE PRECISION variable that must be set by

            the user to an estimate of the machine precision (e.g.
            10**(-16) on a SUN station 3/60). The line search routine will
            terminate if the relative width of the interval of uncertainty
             is less than XTOL.

     W       is a DOUBLE PRECISION array of length N(2M+1)+2M used as
             workspace for LBFGS. This array must not be altered by the
             user.

    IFLAG   is an  int variable that must be set to 0 on initial entry
            to the subroutine. A return with IFLAG<0 indicates an error,
            and IFLAG=0 indicates that the routine has terminated without
             detecting errors. On a return with IFLAG=1, the user must
             evaluate the function F and gradient G. On a return with
             IFLAG=2, the user must provide the diagonal matrix Hk0.

             The following negative values of IFLAG, detecting an error,
             are possible:

              IFLAG=-1  The line search routine MCSRCH failed. The
                       parameter INFO provides more detailed information
                       (see also the documentation of MCSRCH):
                       INFO = 0  IMPROPER INPUT PARAMETERS.
                       INFO = 2  RELATIVE WIDTH OF THE INTERVAL OF
                                 UNCERTAINTY IS AT MOST XTOL.

                       INFO = 3  MORE THAN 20 FUNCTION EVALUATIONS WERE

                                 REQUIRED AT THE PRESENT ITERATION.
                       INFO = 4  THE STEP IS TOO SMALL.

                       INFO = 5  THE STEP IS TOO LARGE.

                       INFO = 6  ROUNDING ERRORS PREVENT FURTHER PROGRESS
                                 THERE MAY NOT BE A STEP WHICH SATISFIES
                                 THE SUFFICIENT DECREASE AND CURVATURE
                                 CONDITIONS. TOLERANCES MAY BE TOO SMALL.
            IFLAG=-2  The i-th diagonal element of the diagonal inverse
                       Hessian approximation, given in DIAG, is not
                       positive.
            IFLAG=-3  Improper input parameters for LBFGS (N or M are
                      not positive).



    ON THE DRIVER:

    The program that calls LBFGS must contain the declaration:

                       EXTERNAL LB2

    LB2 is a BLOCK DATA that defines the default values of several
    parameters described in the COMMON section.

    COMMON:

     The subroutine contains one common area, which the user may wish to
     reference:

 awf added stpawf

    MP  is an  int variable with default value 6. It is used as the
        unit number for the printing of the monitoring information
        controlled by IPRINT.

    LP  is an  int variable with default value 6. It is used as the
        unit number for the printing of error messages. This printing
        may be suppressed by setting LP to a non-positive value.

    GTOL is a DOUBLE PRECISION variable with default value 0.9, which
        controls the accuracy of the line search routine MCSRCH. If the
        function and gradient evaluations are inexpensive with respect
        to the cost of the iteration (which is sometimes the case when
        solving very large problems) it may be advantageous to set GTOL
        to a small value. A typical small value is 0.1.  Restriction:
        GTOL should be greater than 1.D-04.
    STPMIN and STPMAX are non-negative DOUBLE PRECISION variables which
        specify lower and uper bounds for the step in the line search.
        Their default values are 1.D-20 and 1.D+20, respectively. These
        values need not be modified unless the exponents are too large
        for the machine being used, or unless the problem is extremely
        badly scaled (in which case the exponents should be increased).

  MACHINE DEPENDENCIES
        The only variables that are machine-dependent are XTOL,
        STPMIN and STPMAX.

  GENERAL INFORMATION
    Other routines called directly:  DAXPY, DDOT, LB1, MCSRCH
    Input/Output  :  No input; diagnostic messages on unit MP and
                     error messages on unit LP.
*/

#include "precompiled.h"
#include <math.h>

typedef int integer;
typedef unsigned int uinteger;
typedef char *address;
typedef short int shortint;
typedef float real;
typedef double doublereal;
typedef int logical;
typedef short int shortlogical;
typedef char logical1;
typedef char integer1;

#define TRUE_ (1)
#define FALSE_ (0)
#define abs(x) ((x) >= 0 ? (x) : -(x))
#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))

/* Common Block Declarations */
struct lb3_1_ {
  integer mp, lp;
  doublereal gtol, stpmin, stpmax;
};

/* Table of constant values */
static struct lb3_1_ lb3_1;
static integer c__1 = 1;

static doublereal ddot_(integer* n, doublereal* dx, integer* incx, doublereal* dy, integer* incy);
static int daxpy_(integer* n, doublereal* da, doublereal* dx, integer* incx, doublereal* dy, integer* incy);
static int mcsrch_(integer* n, doublereal* x, doublereal* f, doublereal* g, doublereal* s, doublereal* stp, doublereal* ftol, doublereal* xtol, integer* maxfev, integer* info, integer* nfev, doublereal* wa);
static int mcstep_(doublereal* stx, doublereal* fx, doublereal* dx, doublereal* sty, doublereal* fy, doublereal* dy, 
          doublereal* stp, doublereal* fp, doublereal* dp, logical* brackt, 
			     doublereal* stpmin, doublereal* stpmax, integer* info);
/*     **************** */
/*     LBFGS SUBROUTINE */
/*     **************** */

/* Subroutine */ int lbfgs(integer* n, integer* m, doublereal* x, doublereal* f, doublereal* g, integer* diagco, doublereal* diag, integer* iprint, doublereal* xtol, doublereal* w, integer* iflag)
/*      *n, *m;
      *x, *f, *g;
      *diagco;
      *diag;
      *iprint;
      *xtol, *w;
      *iflag;*/
{
  /* Initialized data */
  static doublereal one = 1.0;
  static doublereal zero = 0.0;
   
  /* System generated locals */
  integer i__1;
  doublereal d__1;

  /* Builtin functions */
//  double sqrt();

  /* Local variables */
  static doublereal beta;
  static integer inmc;
  static integer info, iscn, nfev, iycn, iter;
  static doublereal ftol;
  static integer nfun, ispt, iypt, i__, bound;
  static doublereal gnorm;

  static integer point;
  static integer cp;
  static doublereal sq, yr, ys;
  static logical finish;
  static doublereal yy;
  static integer maxfev;

  static integer npt;
  static doublereal stp, stp1;
   
  lb3_1.mp = 6;
  lb3_1.lp = 6;
  lb3_1.gtol = .9;
  lb3_1.stpmin = 1e-20;
  lb3_1.stpmax = 1e20;

  /* Parameter adjustments */
  --diag;
  --g;
  --x;
  --w;
  --iprint;

  /* Function Body */

  /*     INITIALIZE */
  /*     ---------- */

  if (*iflag == 0) {
    goto L10;
  }
  switch ((int)*iflag) {
  case 1:  goto L172;
  case 2:  goto L100;
  }
 L10:
  iter = 0;
  if (*n <= 0 || *m <= 0) {
    goto L196;
  }
  if (lb3_1.gtol <= 1e-4) {
    if (lb3_1.lp > 0) {}
    lb3_1.gtol = .9;
  }
  nfun = 1;
  point = 0;
  finish = FALSE_;
  if (*diagco != 0) {
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
      /* L30: */
      if (diag[i__] <= zero) {
	goto L195;
      }
    }
  } else {
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
      /* L40: */
      diag[i__] = 1.;
    }
  }

  ispt = *n + (*m << 1);
  iypt = ispt + *n * *m;
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    /* L50: */
    w[ispt + i__] = -g[i__] * diag[i__];
  }
  gnorm = sqrt(ddot_(n, &g[1], &c__1, &g[1], &c__1));
  stp1 = one / gnorm;

  /*     PARAMETERS FOR LINE SEARCH ROUTINE */

  ftol = 1e-4;
  maxfev = 20;

  /*    if (iprint[1] >= 0) {
	lb1_(&iprint[1], &iter, &nfun, &gnorm, n, m, &x[1], f, &g[1], &stp, &
	finish);
	} */

  /*    -------------------- */
  /*     MAIN ITERATION LOOP */
  /*    -------------------- */

 L80:
  ++iter;
  info = 0;
  bound = iter - 1;
  if (iter == 1) {
    goto L165;
  }
  if (iter > *m) {
    bound = *m;
  }

  ys = ddot_(n, &w[iypt + npt + 1], &c__1, &w[ispt + npt + 1], &c__1);
  if (*diagco == 0) {
    yy = ddot_(n, &w[iypt + npt + 1], &c__1, &w[iypt + npt + 1], &c__1);
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
      /* L90: */
      diag[i__] = ys / yy;
    }
  } else {
    *iflag = 2;
    return 0;
  }
 L100:
  if (*diagco != 0) {
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
      /* L110: */
      if (diag[i__] <= zero) {
	goto L195;
      }
    }
  }

  /*     COMPUTE -H*G USING THE FORMULA GIVEN IN: Nocedal, J. 1980, */
  /*     "Updating quasi-Newton matrices with limited storage", */
  /*     Mathematics of Computation, Vol.24, No.151, pp. 773-782. */
  /*     --------------------------------------------------------- */

  cp = point;
  if (point == 0) {
    cp = *m;
  }
  w[*n + cp] = one / ys;
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    /* L112: */
    w[i__] = -g[i__];
  }
  cp = point;
  i__1 = bound;
  for (i__ = 1; i__ <= i__1; ++i__) {
    --cp;
    if (cp == -1) {
      cp = *m - 1;
    }
    sq = ddot_(n, &w[ispt + cp * *n + 1], &c__1, &w[1], &c__1);
    inmc = *n + *m + cp + 1;
    iycn = iypt + cp * *n;
    w[inmc] = w[*n + cp + 1] * sq;
    d__1 = -w[inmc];
    daxpy_(n, &d__1, &w[iycn + 1], &c__1, &w[1], &c__1);
    /* L125: */
  }

  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    /* L130: */
    w[i__] = diag[i__] * w[i__];
  }

  i__1 = bound;
  for (i__ = 1; i__ <= i__1; ++i__) {
    yr = ddot_(n, &w[iypt + cp * *n + 1], &c__1, &w[1], &c__1);
    beta = w[*n + cp + 1] * yr;
    inmc = *n + *m + cp + 1;
    beta = w[inmc] - beta;
    iscn = ispt + cp * *n;
    daxpy_(n, &beta, &w[iscn + 1], &c__1, &w[1], &c__1);
    ++cp;
    if (cp == *m) {
      cp = 0;
    }
    /* L145: */
  }

  /*     STORE THE NEW SEARCH DIRECTION */
  /*     ------------------------------ */

  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    /* L160: */
    w[ispt + point * *n + i__] = w[i__];
  }

  /*     OBTAIN THE ONE-DIMENSIONAL MINIMIZER OF THE FUNCTION */
  /*     BY USING THE LINE SEARCH ROUTINE MCSRCH */
  /*     ---------------------------------------------------- */
 L165:
  nfev = 0;
  stp = one;
  if (iter == 1) {
    stp = stp1;
  }
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    /* L170: */
    w[i__] = g[i__];
  }
 L172:
  mcsrch_(n, &x[1], f, &g[1], &w[ispt + point * *n + 1], &stp, &ftol, xtol, 
	  &maxfev, &info, &nfev, &diag[1]);
  if (info == -1) {
    *iflag = 1;
    return 0;
  }
  if (info != 1) {
    goto L190;
  }
  nfun += nfev;

  /*     COMPUTE THE NEW STEP AND GRADIENT CHANGE */
  /*     ----------------------------------------- */

  npt = point * *n;
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    w[ispt + npt + i__] = stp * w[ispt + npt + i__];
    /* L175: */
    w[iypt + npt + i__] = g[i__] - w[i__];
  }
  ++point;
  if (point == *m) {
    point = 0;
  }

  /*     TERMINATION TEST */
  /*     ---------------- *

  gnorm = sqrt(ddot_(n, &g[1], &c__1, &g[1], &c__1));
  xnorm = sqrt(ddot_(n, &x[1], &c__1, &x[1], &c__1));
  xnorm = max(1.,xnorm);
  if (gnorm / xnorm <= *eps) {
    finish = TRUE_;
  }
  */

  /*    if (iprint[1] >= 0) {
	lb1_(&iprint[1], &iter, &nfun, &gnorm, n, m, &x[1], f, &g[1], &stp, &
	finish);
	}*/
  if (finish) {
    *iflag = 0;
    return 0;
  }
  goto L80;

  /*     ------------------------------------------------------------ */
  /*     END OF MAIN ITERATION LOOP. ERROR EXITS. */
  /*     ------------------------------------------------------------ */

 L190:
  *iflag = -1;
  return 0;
 L195:
  *iflag = -2;
  return 0;
 L196:
  *iflag = -3;

  return 0;
} /* lbfgs_ */

/*   ---------------------------------------------------------- */

/* Subroutine */ static int daxpy_(integer* n, doublereal* da, doublereal* dx, integer* incx, doublereal* dy, integer* incy)
/*      *n;
      *da, *dx;
      *incx;
      *dy;
      *incy;*/
{
  /* System generated locals */
  integer i__1;

  /* Local variables */
  static integer i__, m, ix, iy, mp1;


  /*     constant times a vector plus a vector. */
  /*     uses unrolled loops for increments equal to one. */
  /*     jack dongarra, linpack, 3/11/78. */


  /* Parameter adjustments */
  --dy;
  --dx;

  /* Function Body */
  if (*n <= 0) {
    return 0;
  }
  if (*da == 0.) {
    return 0;
  }
  if (*incx == 1 && *incy == 1) {
    goto L20;
  }

  /*        code for unequal increments or equal increments */
  /*          not equal to 1 */

  ix = 1;
  iy = 1;
  if (*incx < 0) {
    ix = (-(*n) + 1) * *incx + 1;
  }
  if (*incy < 0) {
    iy = (-(*n) + 1) * *incy + 1;
  }
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    dy[iy] += *da * dx[ix];
    ix += *incx;
    iy += *incy;
    /* L10: */
  }
  return 0;

  /*        code for both increments equal to 1 */


  /*        clean-up loop */

 L20:
  m = *n % 4;
  if (m == 0) {
    goto L40;
  }
  i__1 = m;
  for (i__ = 1; i__ <= i__1; ++i__) {
    dy[i__] += *da * dx[i__];
    /* L30: */
  }
  if (*n < 4) {
    return 0;
  }
 L40:
  mp1 = m + 1;
  i__1 = *n;
  for (i__ = mp1; i__ <= i__1; i__ += 4) {
    dy[i__] += *da * dx[i__];
    dy[i__ + 1] += *da * dx[i__ + 1];
    dy[i__ + 2] += *da * dx[i__ + 2];
    dy[i__ + 3] += *da * dx[i__ + 3];
    /* L50: */
  }
  return 0;
} /* daxpy_ */



/*   ---------------------------------------------------------- */

static doublereal ddot_(integer* n, doublereal* dx, integer* incx, doublereal* dy, integer* incy)
/*      *n;
      *dx;
      *incx;
      *dy;
      *incy;*/
{
  /* System generated locals */
  integer i__1;
  doublereal ret_val;

  /* Local variables */
  static integer i__, m;
  static doublereal dtemp;
  static integer ix, iy, mp1;


  /*     forms the dot product of two vectors. */
  /*     uses unrolled loops for increments equal to one. */
  /*     jack dongarra, linpack, 3/11/78. */


  /* Parameter adjustments */
  --dy;
  --dx;

  /* Function Body */
  ret_val = 0.;
  dtemp = 0.;
  if (*n <= 0) {
    return ret_val;
  }
  if (*incx == 1 && *incy == 1) {
    goto L20;
  }

  /*        code for unequal increments or equal increments */
  /*          not equal to 1 */

  ix = 1;
  iy = 1;
  if (*incx < 0) {
    ix = (-(*n) + 1) * *incx + 1;
  }
  if (*incy < 0) {
    iy = (-(*n) + 1) * *incy + 1;
  }
  i__1 = *n;
  for (i__ = 1; i__ <= i__1; ++i__) {
    dtemp += dx[ix] * dy[iy];
    ix += *incx;
    iy += *incy;
    /* L10: */
  }
  ret_val = dtemp;
  return ret_val;

  /*        code for both increments equal to 1 */


  /*        clean-up loop */

 L20:
  m = *n % 5;
  if (m == 0) {
    goto L40;
  }
  i__1 = m;
  for (i__ = 1; i__ <= i__1; ++i__) {
    dtemp += dx[i__] * dy[i__];
    /* L30: */
  }
  if (*n < 5) {
    goto L60;
  }
 L40:
  mp1 = m + 1;
  i__1 = *n;
  for (i__ = mp1; i__ <= i__1; i__ += 5) {
    dtemp = dtemp + dx[i__] * dy[i__] + dx[i__ + 1] * dy[i__ + 1] + dx[
								       i__ + 2] * dy[i__ + 2] + dx[i__ + 3] * dy[i__ + 3] + dx[i__ + 
															       4] * dy[i__ + 4];
    /* L50: */
  }
 L60:
  ret_val = dtemp;
  return ret_val;
} /* ddot_ */

/* Subroutine */ static int mcsrch_(integer* n, doublereal* x, doublereal* f, doublereal* g, doublereal* s, doublereal* stp, doublereal* ftol, doublereal* xtol, integer* maxfev, integer* info, integer* nfev, doublereal* wa)
/*      *n;
      *x, *f, *g, *s, *stp, *ftol, *xtol;
     integer *maxfev, *info, *nfev;
     doublereal *wa;*/
{
  /* Initialized data */

  static doublereal p5 = .5;
  static doublereal p66 = .66;
  static doublereal xtrapf = 4.;
  static doublereal zero = 0.;

  /* System generated locals */
  integer i__1;
  doublereal d__1;

  /* Local variables */
  static doublereal dgxm, dgym;
  static integer j, infoc;
  static doublereal finit, width, stmin, stmax;
  static logical stage1;
  static doublereal width1, ftest1, dg, fm, fx, fy;
  static logical brackt;
  static doublereal dginit, dgtest;
  static doublereal dgm, dgx, dgy, fxm, fym, stx, sty;

  /* Parameter adjustments */
  --wa;
  --s;
  --g;
  --x;

  /* Function Body */
  if (*info == -1) {
    goto L45;
  }
  infoc = 1;

  /*     CHECK THE INPUT PARAMETERS FOR ERRORS. */

  if (*n <= 0 || *stp <= zero || *ftol < zero || lb3_1.gtol < zero || *xtol 
      < zero || lb3_1.stpmin < zero || lb3_1.stpmax < lb3_1.stpmin || *
      maxfev <= 0) {
    return 0;
  }

  /*     COMPUTE THE INITIAL GRADIENT IN THE SEARCH DIRECTION */
  /*     AND CHECK THAT S IS A DESCENT DIRECTION. */

  dginit = zero;
  i__1 = *n;
  for (j = 1; j <= i__1; ++j) {
    dginit += g[j] * s[j];
    /* L10: */
  }
  if (dginit >= zero) {
    return 0;
  }

  /*     INITIALIZE LOCAL VARIABLES. */

  brackt = FALSE_;
  stage1 = TRUE_;
  *nfev = 0;
  finit = *f;
  dgtest = *ftol * dginit;
  width = lb3_1.stpmax - lb3_1.stpmin;
  width1 = width / p5;
  i__1 = *n;
  for (j = 1; j <= i__1; ++j) {
    wa[j] = x[j];
    /* L20: */
  }

  /*     THE VARIABLES STX, FX, DGX CONTAIN THE VALUES OF THE STEP, */
  /*     FUNCTION, AND DIRECTIONAL DERIVATIVE AT THE BEST STEP. */
  /*     THE VARIABLES STY, FY, DGY CONTAIN THE VALUE OF THE STEP, */
  /*     FUNCTION, AND DERIVATIVE AT THE OTHER ENDPOINT OF */
  /*     THE INTERVAL OF UNCERTAINTY. */
  /*     THE VARIABLES STP, F, DG CONTAIN THE VALUES OF THE STEP, */
  /*     FUNCTION, AND DERIVATIVE AT THE CURRENT STEP. */

  stx = zero;
  fx = finit;
  dgx = dginit;
  sty = zero;
  fy = finit;
  dgy = dginit;

  /*     START OF ITERATION. */

 L30:

  /*        SET THE MINIMUM AND MAXIMUM STEPS TO CORRESPOND */
  /*        TO THE PRESENT INTERVAL OF UNCERTAINTY. */

  if (brackt) {
    stmin = min(stx,sty);
    stmax = max(stx,sty);
  } else {
    stmin = stx;
    stmax = *stp + xtrapf * (*stp - stx);
  }

  /*        FORCE THE STEP TO BE WITHIN THE BOUNDS STPMAX AND STPMIN. */

  *stp = max(*stp,lb3_1.stpmin);
  *stp = min(*stp,lb3_1.stpmax);

  /*        IF AN UNUSUAL TERMINATION IS TO OCCUR THEN LET */
  /*        STP BE THE LOWEST POINT OBTAINED SO FAR. */

  if ((brackt && ((*stp <= stmin || *stp >= stmax) || *nfev >= *maxfev - 1 || 
		  infoc == 0)) || (brackt && (stmax - stmin <= *xtol * stmax))) {
    *stp = stx;
  }

  /*        EVALUATE THE FUNCTION AND GRADIENT AT STP */
  /*        AND COMPUTE THE DIRECTIONAL DERIVATIVE. */
  /*        We return to main program to obtain F and G. */

  i__1 = *n;
  for (j = 1; j <= i__1; ++j) {
    x[j] = wa[j] + *stp * s[j];
    /* L40: */
  }
  *info = -1;
  return 0;

 L45:
  *info = 0;
  ++(*nfev);
  dg = zero;
  i__1 = *n;
  for (j = 1; j <= i__1; ++j) {
    dg += g[j] * s[j];
    /* L50: */
  }
  ftest1 = finit + *stp * dgtest;

  /*        TEST FOR CONVERGENCE. */

  if (brackt && ((*stp <= stmin || *stp >= stmax) || infoc == 0)) {
    *info = 6;
  }
  if (*stp == lb3_1.stpmax && *f <= ftest1 && dg <= dgtest) {
    *info = 5;
  }
  if (*stp == lb3_1.stpmin && (*f > ftest1 || dg >= dgtest)) {
    *info = 4;
  }
  if (*nfev >= *maxfev) {
    *info = 3;
  }
  if (brackt && stmax - stmin <= *xtol * stmax) {
    *info = 2;
  }
  if (*f <= ftest1 && abs(dg) <= lb3_1.gtol * (-dginit)) {
    *info = 1;
  }

  /*        CHECK FOR TERMINATION. */

  if (*info != 0) {
    return 0;
  }

  /*        IN THE FIRST STAGE WE SEEK A STEP FOR WHICH THE MODIFIED */
  /*        FUNCTION HAS A NONPOSITIVE VALUE AND NONNEGATIVE DERIVATIVE. */

  if (stage1 && *f <= ftest1 && dg >= min(*ftol,lb3_1.gtol) * dginit) {
    stage1 = FALSE_;
  }

  /*        A MODIFIED FUNCTION IS USED TO PREDICT THE STEP ONLY IF */
  /*        WE HAVE NOT OBTAINED A STEP FOR WHICH THE MODIFIED */
  /*        FUNCTION HAS A NONPOSITIVE FUNCTION VALUE AND NONNEGATIVE */
  /*        DERIVATIVE, AND IF A LOWER FUNCTION VALUE HAS BEEN */
  /*        OBTAINED BUT THE DECREASE IS NOT SUFFICIENT. */

  if (stage1 && *f <= fx && *f > ftest1) {

    /*           DEFINE THE MODIFIED FUNCTION AND DERIVATIVE VALUES. */

    fm = *f - *stp * dgtest;
    fxm = fx - stx * dgtest;
    fym = fy - sty * dgtest;
    dgm = dg - dgtest;
    dgxm = dgx - dgtest;
    dgym = dgy - dgtest;

    /*           CALL CSTEP TO UPDATE THE INTERVAL OF UNCERTAINTY */
    /*           AND TO COMPUTE THE NEW STEP. */

    mcstep_(&stx, &fxm, &dgxm, &sty, &fym, &dgym, stp, &fm, &dgm, &brackt,
	    &stmin, &stmax, &infoc);

    /*           RESET THE FUNCTION AND GRADIENT VALUES FOR F. */

    fx = fxm + stx * dgtest;
    fy = fym + sty * dgtest;
    dgx = dgxm + dgtest;
    dgy = dgym + dgtest;
  } else {

    /*           CALL MCSTEP TO UPDATE THE INTERVAL OF UNCERTAINTY */
    /*           AND TO COMPUTE THE NEW STEP. */

    mcstep_(&stx, &fx, &dgx, &sty, &fy, &dgy, stp, f, &dg, &brackt, &
	    stmin, &stmax, &infoc);
  }

  /*        FORCE A SUFFICIENT DECREASE IN THE SIZE OF THE */
  /*        INTERVAL OF UNCERTAINTY. */

  if (brackt) {
    if ((d__1 = sty - stx, abs(d__1)) >= p66 * width1) {
      *stp = stx + p5 * (sty - stx);
    }
    width1 = width;
    width = (d__1 = sty - stx, abs(d__1));
  }

  /*        END OF ITERATION. */

  goto L30;

  /*     LAST LINE OF SUBROUTINE MCSRCH. */

} /* mcsrch_ */

/* Subroutine */ static int mcstep_(doublereal* stx, doublereal* fx, doublereal* dx, doublereal* sty, doublereal* fy, doublereal* dy, 
          doublereal* stp, doublereal* fp, doublereal* dp, logical* brackt, 
			     doublereal* stpmin, doublereal* stpmax, integer* info)
/*     doublereal *stx, *fx, *dx, *sty, *fy, *dy, *stp, *fp, *dp;
     logical *brackt;
     doublereal *stpmin, *stpmax;
     integer *info;*/
{
  /* System generated locals */
  doublereal d__1, d__2, d__3;

  /* Builtin functions */
//  double sqrt();

  /* Local variables */
  static doublereal sgnd, stpc, stpf, stpq, p, q, gamma, r__, s, theta;
  static logical bound;

  *info = 0;

  /*     CHECK THE INPUT PARAMETERS FOR ERRORS. */

  if (*brackt && ((*stp <= min(*stx,*sty) || *stp >= max(*stx,*sty)) || *dx *
		  (*stp - *stx) >= (float)0. || *stpmax < *stpmin)) {
    return 0;
  }

  /*     DETERMINE IF THE DERIVATIVES HAVE OPPOSITE SIGN. */

  sgnd = *dp * (*dx / abs(*dx));

  /*     FIRST CASE. A HIGHER FUNCTION VALUE. */
  /*     THE MINIMUM IS BRACKETED. IF THE CUBIC STEP IS CLOSER */
  /*     TO STX THAN THE QUADRATIC STEP, THE CUBIC STEP IS TAKEN, */
  /*     ELSE THE AVERAGE OF THE CUBIC AND QUADRATIC STEPS IS TAKEN. */

  if (*fp > *fx) {
    *info = 1;
    bound = TRUE_;
    theta = (*fx - *fp) * 3 / (*stp - *stx) + *dx + *dp;
    /* Computing MAX */
    d__1 = abs(theta), d__2 = abs(*dx), d__1 = max(d__1,d__2), d__2 = abs(
									  *dp);
    s = max(d__1,d__2);
    /* Computing 2nd power */
    d__1 = theta / s;
    gamma = s * sqrt(d__1 * d__1 - *dx / s * (*dp / s));
    if (*stp < *stx) {
      gamma = -gamma;
    }
    p = gamma - *dx + theta;
    q = gamma - *dx + gamma + *dp;
    r__ = p / q;
    stpc = *stx + r__ * (*stp - *stx);
    stpq = *stx + *dx / ((*fx - *fp) / (*stp - *stx) + *dx) / 2 * (*stp - 
								   *stx);
    if ((d__1 = stpc - *stx, abs(d__1)) < (d__2 = stpq - *stx, abs(d__2)))
      {
	stpf = stpc;
      } else {
	stpf = stpc + (stpq - stpc) / 2;
      }
    *brackt = TRUE_;

    /*     SECOND CASE. A LOWER FUNCTION VALUE AND DERIVATIVES OF */
    /*     OPPOSITE SIGN. THE MINIMUM IS BRACKETED. IF THE CUBIC */
    /*     STEP IS CLOSER TO STX THAN THE QUADRATIC (SECANT) STEP, */
    /*     THE CUBIC STEP IS TAKEN, ELSE THE QUADRATIC STEP IS TAKEN. */

  } else if (sgnd < (float)0.) {
    *info = 2;
    bound = FALSE_;
    theta = (*fx - *fp) * 3 / (*stp - *stx) + *dx + *dp;
    /* Computing MAX */
    d__1 = abs(theta), d__2 = abs(*dx), d__1 = max(d__1,d__2), d__2 = abs(
									  *dp);
    s = max(d__1,d__2);
    /* Computing 2nd power */
    d__1 = theta / s;
    gamma = s * sqrt(d__1 * d__1 - *dx / s * (*dp / s));
    if (*stp > *stx) {
      gamma = -gamma;
    }
    p = gamma - *dp + theta;
    q = gamma - *dp + gamma + *dx;
    r__ = p / q;
    stpc = *stp + r__ * (*stx - *stp);
    stpq = *stp + *dp / (*dp - *dx) * (*stx - *stp);
    if ((d__1 = stpc - *stp, abs(d__1)) > (d__2 = stpq - *stp, abs(d__2)))
      {
	stpf = stpc;
      } else {
	stpf = stpq;
      }
    *brackt = TRUE_;

    /*     THIRD CASE. A LOWER FUNCTION VALUE, DERIVATIVES OF THE */
    /*     SAME SIGN, AND THE MAGNITUDE OF THE DERIVATIVE DECREASES. */
    /*     THE CUBIC STEP IS ONLY USED IF THE CUBIC TENDS TO INFINITY */
    /*     IN THE DIRECTION OF THE STEP OR IF THE MINIMUM OF THE CUBIC */
    /*     IS BEYOND STP. OTHERWISE THE CUBIC STEP IS DEFINED TO BE */
    /*     EITHER STPMIN OR STPMAX. THE QUADRATIC (SECANT) STEP IS ALSO */
    /*     COMPUTED AND IF THE MINIMUM IS BRACKETED THEN THE THE STEP */
    /*     CLOSEST TO STX IS TAKEN, ELSE THE STEP FARTHEST AWAY IS TAKEN. */

  } else if (abs(*dp) < abs(*dx)) {
    *info = 3;
    bound = TRUE_;
    theta = (*fx - *fp) * 3 / (*stp - *stx) + *dx + *dp;
    /* Computing MAX */
    d__1 = abs(theta), d__2 = abs(*dx), d__1 = max(d__1,d__2), d__2 = abs(
									  *dp);
    s = max(d__1,d__2);

    /*        THE CASE GAMMA = 0 ONLY ARISES IF THE CUBIC DOES NOT TEND */
    /*        TO INFINITY IN THE DIRECTION OF THE STEP. */

    /* Computing MAX */
    /* Computing 2nd power */
    d__3 = theta / s;
    d__1 = 0., d__2 = d__3 * d__3 - *dx / s * (*dp / s);
    gamma = s * sqrt((max(d__1,d__2)));
    if (*stp > *stx) {
      gamma = -gamma;
    }
    p = gamma - *dp + theta;
    q = gamma + (*dx - *dp) + gamma;
    r__ = p / q;
    if (r__ < (float)0. && gamma != (float)0.) {
      stpc = *stp + r__ * (*stx - *stp);
    } else if (*stp > *stx) {
      stpc = *stpmax;
    } else {
      stpc = *stpmin;
    }
    stpq = *stp + *dp / (*dp - *dx) * (*stx - *stp);
    if (*brackt) {
      if ((d__1 = *stp - stpc, abs(d__1)) < (d__2 = *stp - stpq, abs(
								     d__2))) {
	stpf = stpc;
      } else {
	stpf = stpq;
      }
    } else {
      if ((d__1 = *stp - stpc, abs(d__1)) > (d__2 = *stp - stpq, abs(
								     d__2))) {
	stpf = stpc;
      } else {
	stpf = stpq;
      }
    }

    /*     FOURTH CASE. A LOWER FUNCTION VALUE, DERIVATIVES OF THE */
    /*     SAME SIGN, AND THE MAGNITUDE OF THE DERIVATIVE DOES */
    /*     NOT DECREASE. IF THE MINIMUM IS NOT BRACKETED, THE STEP */
    /*     IS EITHER STPMIN OR STPMAX, ELSE THE CUBIC STEP IS TAKEN. */

  } else {
    *info = 4;
    bound = FALSE_;
    if (*brackt) {
      theta = (*fp - *fy) * 3 / (*sty - *stp) + *dy + *dp;
      /* Computing MAX */
      d__1 = abs(theta), d__2 = abs(*dy), d__1 = max(d__1,d__2), d__2 = 
	abs(*dp);
      s = max(d__1,d__2);
      /* Computing 2nd power */
      d__1 = theta / s;
      gamma = s * sqrt(d__1 * d__1 - *dy / s * (*dp / s));
      if (*stp > *sty) {
	gamma = -gamma;
      }
      p = gamma - *dp + theta;
      q = gamma - *dp + gamma + *dy;
      r__ = p / q;
      stpc = *stp + r__ * (*sty - *stp);
      stpf = stpc;
    } else if (*stp > *stx) {
      stpf = *stpmax;
    } else {
      stpf = *stpmin;
    }
  }

  /*     UPDATE THE INTERVAL OF UNCERTAINTY. THIS UPDATE DOES NOT */
  /*     DEPEND ON THE NEW STEP OR THE CASE ANALYSIS ABOVE. */

  if (*fp > *fx) {
    *sty = *stp;
    *fy = *fp;
    *dy = *dp;
  } else {
    if (sgnd < (float)0.) {
      *sty = *stx;
      *fy = *fx;
      *dy = *dx;
    }
    *stx = *stp;
    *fx = *fp;
    *dx = *dp;
  }

  /*     COMPUTE THE NEW STEP AND SAFEGUARD IT. */

  stpf = min(*stpmax,stpf);
  stpf = max(*stpmin,stpf);
  *stp = stpf;
  if (*brackt && bound) {
    if (*sty > *stx) {
      /* Computing MIN */
      d__1 = *stx + (*sty - *stx) * (float).66;
      *stp = min(d__1,*stp);
    } else {
      /* Computing MAX */
      d__1 = *stx + (*sty - *stx) * (float).66;
      *stp = max(d__1,*stp);
    }
  }
  return 0;

  /*     LAST LINE OF SUBROUTINE MCSTEP. */

} /* mcstep_ */


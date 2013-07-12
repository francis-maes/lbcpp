/*-----------------------------------------.---------------------------------.
 | Filename: ParEGOOptimizer.cpp           | Wrapper for ParEGO               |
 | Author  : Denny Verbeeck                |                                  |
 | Started : 19/06/2013 13:41              |                                  |
 `-----------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#include "precompiled.h"
#include "ParEGOOptimizer.h"
#include <cstdlib>              /* for qsort() */

#define LARGE 2147000000
#define INFTY 1.0e35;
#define PI 3.141592653
#define RN rand()/(RAND_MAX+1.0)

// numerical integration defines
#define FUNC(x) ((*func)(x))
#define EPS 1.0e-6
#define JMAX 20

namespace lbcpp
{

void ParEGOOptimizer::cross(double *child, double *par1, double *par2)
{
  double xl, xu;
  double x1, x2;
  double alpha;
  double expp;
  double di=20;
  double beta;
  double betaq;
  double rnd;
  
  for(int d=0; d<dim; ++d)
  {
    /*Selected Two Parents*/ 
    
    xl = xmin[d];
    xu = xmax[d];
    
    /* Check whether variable is selected or not*/
    if(RN <= 0.5)
    {
      if(myabs(par1[d] - par2[d]) > 0.000001)
      {
        if(par2[d] > par1[d])
        {
          x2 = par2[d];
          x1 = par1[d];
        }
        else
        {
          x2 = par1[d];
          x1 = par2[d];
        }
        
        /*Find beta value*/
        if((x1 - xl) > (xu - x2))
          beta = 1 + (2*(xu - x2)/(x2 - x1));
        else
          beta = 1 + (2*(x1-xl)/(x2-x1));
        
        /*Find alpha*/
        expp = di + 1.0;
        beta = 1.0/beta;
        alpha = 2.0 - pow(beta,expp);
        if (alpha < 0.0) 
        {
          printf("ERRRROR %f %f %f\n",alpha,par1[d],par2[d]);
          exit(-1);
        }
        
        rnd = RN;
        if (rnd <= 1.0/alpha)
        {
          alpha = alpha*rnd;
          expp = 1.0/(di+1.0);
          betaq = pow(alpha,expp);
        }
        else
        {
          alpha = alpha*rnd;
          alpha = 1.0/(2.0-alpha);
          expp = 1.0/(di+1.0);
          if (alpha < 0.0) 
          {
            printf("ERRRORRR \n");
            exit(-1);
          }
          betaq = pow(alpha,expp);
        }
        
        /*Generating one child */
        child[d] = 0.5*((x1+x2) - betaq*(x2-x1));
      }
      else
      {
        betaq = 1.0;
        x1 = par1[d]; 
        x2 = par2[d];
        
        /*Generating one child*/
        child[d] = 0.5*((x1+x2) - betaq*(x2-x1)); 
      }
      
      if (child[d] < xl) child[d] = xl;
      if (child[d] > xu) child[d] = xu;
    }
  }
}


int ParEGOOptimizer::tourn_select(double *xsel, double **ax, double *ay, int iter, int t_size)
{
  int *idx;
  double besty=INFTY;
  int bestidx;
  cwr(&idx, t_size, iter);                    // fprintf(stderr,"cwr\n");
  for(int i=0;i<t_size;i++)
    if(ay[idx[i]]<besty)
    {
      besty=ay[idx[i]];
      bestidx=i;
    }
  
  for(int d=0;d<dim;d++)
    xsel[d] = ax[idx[bestidx]][d];
  return(idx[bestidx]);
}

void ParEGOOptimizer::mutate(double *x)
{
  // this mutation always mutates at least one gene (even if the m_rate is set to zero)
  // this is to avoid duplicating points which would cause the R matrix to be singular
  double m_rate=1.0/dim;
  double shift;
  bool mut[dim];
  int nmutations=0;
  for(int i=0;i<dim;i++)
  {
    mut[i]=false;
    if(RN<m_rate)
    {
      mut[i]=true;
      nmutations++;
    }
  }
  if(nmutations==0)
    mut[1+(int)(RN*dim)]=true;
  
  for(int d=0;d<dim;d++)
    if(mut[d])
    {
      shift = (0.0001+RN)*(xmax[d]-xmin[d])/100.0;
      if(RN<0.5)
        shift*=-1;
      
      x[d]+=shift;
      
      if(x[d]>xmax[d])
        x[d]=xmax[d]-myabs(shift);
      else if(x[d]<xmin[d])
        x[d]=xmin[d]+myabs(shift);
    }
}

double ParEGOOptimizer::Tcheby(double *wv, double *vec, double *ideal)
{
  // the augmented Tchebycheff measure with normalization
  int i;
  double sum;
  double diff;
  double d_max=-LARGE;
  double norm[MAX_K];
  double nideal[MAX_K];
  
  sum=0.0;
  
  for(i=0;i<nobjs;i++)
  {
    norm[i] = (vec[i]-absmin[i])/(absmax[i]-absmin[i]);
    nideal[i] = ideal[i]; 
    diff = wv[i]*(norm[i]-nideal[i]);
    sum += diff;
    if(diff>d_max)
      d_max = diff;
  }
  
  // fprintf(stdout, "d_max= %lf + 0.5 * sum= %lf\n", d_max, sum); 
  return(d_max + 0.05*sum);  
}

void ParEGOOptimizer::compute_n(int n, int *dig, int base)
{
  // given the number n, convert it to base base, and put the digits into dig
  // we already know the number n will be less than pow(base,dim) where dim is global 
  int d=dim-1;
  int div = (int)pow((double)base,(double)(dim-1));
  for(int i=0; i < dim; i++)
  {
    dig[i] = n/div;
    n-=dig[i]*div;
    d--;
    div = (int)pow(double(base),double(d));
  }
}

void ParEGOOptimizer::latin_hyp(double **ax, int iter)
{
  int v;
  
  double L[dim][iter];
  bool viable[dim][iter];
  
  for(int d=0; d<dim; d++)
    for(int i=0;i<iter;i++)
    {
      viable[d][i]=true;
      L[d][i] = xmin[d+1] + i*((xmax[d+1]-xmin[d+1])/double(iter));      
    }
  
  for(int i=0; i<iter; i++)
    for(int d = 0; d < dim; d++)
    {
      do
        v = int(RN*iter);
      while(!viable[d][v]);
      viable[d][v]=false;
      ax[i+1][d+1] = L[d][v]+RN*((xmax[d+1]-xmin[d+1])/double(iter));
    }
}

double ParEGOOptimizer::posdet(Matrix& R, int n)
{
  // *** NB: this function changes R !!! ***
  // computes the determinant of R. If it is non-positive, then it adjusts R and recomputes.
  // returns the final determinant and changes R.
  double detR = R.detSymm();
  double diag=1.01;
  while(detR<=0.0)
  {
    for(int i=0;i<n;i++)
      R(i,i)=diag;
    detR = R.detSymm();
    diag+=0.01;
  }
  return detR;
}

int ParEGOOptimizer::mypow(int x, int expon)
{
  return ((int)(pow((double)x, (double)expon)));
}

double ParEGOOptimizer::myabs(double v)
{
  if(v>=0.0)
    return v;
  else //if (v < 0.0)
    return -v;
}

double ParEGOOptimizer::myfit(ExecutionContext &context, double *x, double *ff)
{
  
  DenseDoubleVectorPtr xvector = new DenseDoubleVector(dim, 0.0);
  for (int i = 0; i < dim; ++i)
    xvector->setValue(i, x[i]);
  ff[0] = problem->evaluate(context, xvector)->toDouble();
  return Tcheby(gwv,&(ff[0]),gideal);
}


void ParEGOOptimizer::init_arrays(double ***ax, double **ay, int n, int dim)
{
  *ax = new double*[n];
  for(int i=0;i<n;i++)
    (*ax)[i]=new double[dim];
  (*ay)=new double[n];
}

void ParEGOOptimizer::get_params(double **param, double *theta, double *p, double *sigma, double *mu)
{
  // uses global dim value
  
  for (int i=0;i < dim; i++)
  {
    theta[i]=param[0][i];
    p[i]=param[0][i+dim];
  }
  // TODO: idx hieronder verminderen met 1???
  *sigma=param[0][2*dim];
  *mu=param[0][2*dim+1];
  
}

void ParEGOOptimizer::cwr(int **target, int k, int n)  // choose without replacement k items from n
{
  int i,j;
  int l_t; //(length of the list at time t)
  
  if(k>n)
  {
    fprintf(stdout,"trying to choose k items without replacement from n but k > n!!\n");
    exit(-1);
  }
  
  (*target) = new int[k];
  int *to;
  to = &((*target)[0]);
  
  int from[n];
  
  for(i=0;i<n;i++)
    from[i]=i;
  
  l_t = n;
  for(i=0;i<k;i++)
  {
    j=(int)(RN*l_t);
    to[i]=from[j];
    from[j]=from[l_t-1];
    l_t--;
  }
}



double ParEGOOptimizer::wrap_ei(double *x)
{
  for(int d=0;d<dim; d++)
    (*pax)[titer][d] = x[d];
  // fprintf(stdout,"%lf %lf\n", x[1], (*pax)[titer+1][1]);
  
  double fit;
  // predict the fitness
  fit=predict_y(*pax, *pInvR, *pgy, gmu, *gtheta, *gp, titer, dim);
  
  // fprintf(stdout,"predicted fitness in wrap_ei() = %lg\n", fit);
  
  
  // compute the error
  double ss;
  ss=s2(*pax, *gtheta, *gp, gsigma, dim, titer, *pInvR);
  // fprintf(stdout,"s^2 error in wrap_ei() = %lg\n", ss);
  //  fprintf(stderr,"%.9lf %.9lf ", x[1], ss);
  
  
  // compute the expected improvement	  
  double ei;
  ei=expected_improvement(fit, gymin, sqrt(ss));
  // fprintf(stdout,"-ei in wrap_ei() = %lg\n", -ei);
  
  
  for(int d=0; d<dim; ++d)
    if((x[d]>xmax[d])||(x[d]<xmin[d]))
      ei=-1000;
  
  // return the expected improvement
  return(-ei);
}

/*
 int main(int argc, char **argv) 
 {
 ParEGO U(1);
 
 long seed=47456536;
 if(argc>1)
 seed = atoi(argv[1]);
 srand(seed);
 
 sprintf(U.fitfunc, "f_vlmop2");
 if(argc>2)
 sprintf(U.fitfunc, argv[2]);
 
 U.init_ParEGO(); // starts up ParEGO and does the latin hypercube, outputting these to a file
 
 //  int i = 21;
 int i = U.iter;
 while ( i < U.MAX_ITERS )
 {
 U.iterate_ParEGO(); // takes n solution/point pairs as input and gives 1 new solution as output
 i++;
 }
 
 }*/

// returns the chosen sample
ObjectPtr ParEGOOptimizer::iterate_ParEGO(ExecutionContext& context, int iter)
{
  int stopcounter=0;
  
  double **param = new double*[pdim+2];
  for(int i=0;i<pdim+2;i++)
    param[i]=new double[pdim+1];
  
  double **bestparam;
  bestparam = new double*[pdim+2];
  for(int i=0;i<pdim+2;i++)
    bestparam[i]=new double[pdim+1];
  
  double *p_last;
  double *theta_last;
  p_last = new double[dim+1];
  theta_last = new double[dim+1];
  double sigma_last;
  double mu_last;
  
  int gapopsize=20;
  double **popx;
  popx = new double*[gapopsize];  // GA population for searching over modelled landscape
  for(int i=0;i<gapopsize;i++)
    popx[i]=new double[dim];
  
  double popy[gapopsize];
  
  double mutx[dim];
  double muty;
  
  static bool change=true;
  static int next=-1;
  static int add=1;
  
  do
  {
    if(iter%5==2)
      change = true;  // change the weight vector
    
    if(change)
    {
      if((next==9)||((next==0)&&(add==-1)))
        add*=-1;
      next+=add;
      for(int k=0;k<nobjs;k++)
        gwv[k]=normwv[next][k];	     
    }
    
    for(int i=0;i<iter;i++)
    {
      ay[i] = Tcheby(gwv,&(ff[i][0]),gideal); 
      if(ay[i]<ymin)
      {
        ymin=ay[i];
        best_ever=i;
      }
    }
    
    if(iter>dim+24)
    {
      titer =11*dim+24;
      
      int ind[iter];
      mysort(ind, ay, iter);
      
      for (int i=0; i<titer/2; i++)       
      {
        for(int d=0;d<dim;d++)
          tmpax[i][d] = ax[ind[i]][d];
        tmpay[i] = ay[ind[i]];                   // fprintf(stderr, "evaluate\n");
      }
      
      int *choose;
      cwr(&choose,iter-titer/2,iter-titer/2); // cwr(k,n) - choose without replacement k items from n
      
      for(int i=titer/2; i<titer; i++)
      {
        int j= ind[choose[i-titer/2]+titer/2];
        for(int d=0; d<dim; d++)
          tmpax[i][d] = ax[j][d];
        tmpay[i] = ay[j];
      }
      
      pax=&tmpax;
      pay=&tmpay;
      delete[] choose;
    }
    else // iter>11*dim+24
    {
      titer=iter;
      pax=&ax;
      pay=&ay;
    }
    
    ::Vector y(titer);
    build_y(*pay, titer,y);
    
    double best_lik=INFTY;  
    int besti;
    
    if(change)
    {
      for(int i=0;i<30;i++)
      {
        for(int d=0; d<dim; d++)
        {
          theta_last[d]=1+RN*2;
          p_last[d]=1.01+RN*0.98;
        }
        
        Matrix R(titer,titer); 
        build_R(*pax, theta_last, p_last, dim, titer, R);
        Matrix InvR = R.inverseSymmPositiveDefinite();
        
        mu_last=mu_hat(InvR, y, titer);
        sigma_last=sigma_squared_hat(InvR, y, mu_last, titer);
        
        for(int j=0; j<dim; ++j)
        {
          param[0][j]=theta_last[j];
          param[0][j+dim]=p_last[j];
        }
        param[0][2*dim]=sigma_last;
        param[0][2*dim+1]=mu_last;
        
        glik=likelihood(param[0]);	 
        
        if(glik<best_lik)
        {
          besti = i;
          best_lik=glik;
          for(int j=0;j<pdim;j++)
            bestparam[0][j]=param[0][j];
        }
        change=false;
      }
    }
    
    get_params(bestparam, theta_last, p_last, &sigma_last, &mu_last);
    
    /* Use the full R matrix */
    titer=iter;
    pax = &ax;
    Matrix Rpred(titer,titer);
    build_R(*pax, theta_last, p_last, dim, titer, Rpred);
    Matrix InvR = Rpred.inverseSymmPositiveDefinite();
    ::Vector fy(titer);
    build_y(ay, titer, fy);
    
    /* ***************************** */
    
    //   fprintf(stdout,"predicted R matrix built OK:\n");
    //   pr_sq_mat(Rpred,titer);
    
    
    double best_imp=INFTY;
    double best_x[dim+1];
    
    // set the global variables equal to the local ones
    gmu = mu_last;
    gsigma = sigma_last;
    gtheta = &theta_last;
    gp = &p_last;
    pgR=&Rpred;
    pInvR=&InvR;
    pgy=&fy;
    gymin = INFTY;
    
    for(int i=0;i<titer;i++)
      if((*pgy)(i)<gymin)
        gymin=(*pgy)(i);
    
    start = clock();
    
    /* BEGIN GA code */
    best_imp=INFTY;
    int parA;
    int parB;
    
    
    // initialize using the latin hypercube method
    latin_hyp(popx, gapopsize);
    for (int i=0; i<gapopsize; i++)       
      popy[i] = wrap_ei(popx[i]);
    
    // initialize with mutants of nearby good solutions
    int ind[iter];
    mysort(ind, ay, iter);
    
    for (int i=0; i<5; i++)       
    {
      parA=ind[i];
      for(int d=1;d<=dim;d++)
        popx[i][d] = ax[parA][d];
      mutate(popx[i]);                              // fprintf(stderr, "mutate\n");
      popy[i] = wrap_ei(popx[i]);                   // fprintf(stderr, "evaluate\n");
    }
    
    double p_cross=0.2;
    for (int i=0; i<10000; i++)
    {
      if(RN < p_cross)
      {
        parA=tourn_select(mutx, popx, popy, gapopsize, 2);
        do
          parB=tourn_select(mutx, popx, popy, gapopsize, 2);
        while(parB==parA);
        cross(mutx, popx[parA], popx[parB]);
      }
      else
        parA=tourn_select(mutx, popx, popy, gapopsize, 2);//  fprintf(stderr, "parent selected\n");
      
      mutate(mutx);                             // fprintf(stderr, "mutate\n");
      muty = wrap_ei(mutx);                      //  fprintf(stderr, "evaluate\n");
      if(muty<popy[parA])
      {
        for(int d=1;d<=dim;d++)
          popx[parA][d]=mutx[d];
        popy[parA]=muty;
      }
    }
    
    bool improved=false;
    for(int i=0; i<gapopsize; i++)
    {
      if(popy[i]<best_imp)
      {
        improved=true;
        best_imp=popy[i];
        for(int d=0; d<dim; d++)
          best_x[d]=popx[i][d];
      }	  
    }
    //    fprintf(stdout,"ei= %lf\n", best_imp);
    if(improved==false)
    {
      for(int d=0; d<dim; d++)
        best_x[d]=popx[0][d];
      mutate(best_x);
    }
    
    /* END GA code */
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    // timestr << iter << " " << cpu_time_used << endl;
    
    
    for(int d=0;d<dim;++d)
      ax[iter][d]=best_x[d];
    ay[iter]=myfit(context, ax[iter], ff[iter]);
    
    /*          
     fprintf(stdout, "%d ", iter+1+prior_it);
     for(int d=1;d <=dim; d++)
     fprintf(stdout, "%lg ", ax[iter+1][d]);
     fprintf(stdout,"decision\n");
     
     std::cout << iter+1 << " ";
     for(int i=1;i<=nobjs;i++)
     std::cout << ff[iter+1][i] <<" ";
     std::cout << "objective" << std::endl;
     */
    
    improvements[iter]=improvements[iter-1];
    if (ay[iter]>=ymin) 
    {
      // fprintf(stderr,"No actual improver found\n");
      stopcounter++;
    }
    else
    {
      improvements[iter]++;
      ymin = ay[iter];
      stopcounter=0;	
      best_ever=iter;
    }
    
    /*
     ~InvR;
     ~Rpred;
     ~fy;
     */
    iter++;
    
  } while(0);
  
  delete[] p_last;
  delete[] theta_last;
  
  for(int i=0;i<gapopsize;++i)
    delete[] popx[i];
  delete[] popx;
  
  for(int i=0;i<pdim+2;++i)
    delete[] param[i];
  delete[] param;
  
  for(int i=0;i<pdim+2;++i)
    delete[] bestparam[i];
  delete[] bestparam;
  
  return NULL;
}

// returns an OVectorPtr of DenseDoubleVectors with the initial sample
ObjectPtr ParEGOOptimizer::init_ParEGO(ExecutionContext& context)
{
  gz = new double[76];
  alph=1.0;
  minss = 1.0e-9;
  DEBUG=false;
  best_ever=1;
  niche=1.0;
  MAX_ITERS = 1000;
  
  improvements = new int[MAX_ITERS];
  
  init_gz();
  
  //  set_search();
  
  nobjs = problem->getNumObjectives();
  ScalarVectorDomainPtr domain = problem->getDomain().staticCast<ScalarVectorDomain>();
  dim = domain->getNumDimensions();
  xmin = new double[dim];
  xmax = new double[dim];
  
  absmax = new double[nobjs];
  absmin = new double[nobjs];
  gideal = new double[nobjs];
  gwv    = new double[nobjs];
  
  wv = new int*[40402];      // first dimension needs to be #weight vectors^nobjs
  dwv = new double*[40402];  // first dimension needs to be #weight vectors^nobjs
  for (int i=0; i < 40402; ++i)
  {
    wv[i] = new int[nobjs];
    dwv[i] = new double[nobjs];
  }
  normwv = new double*[MAX_ITERS]; // first dimension needs to be N+k-1 choose k-1 ($^{N+k-1}C_{k-1}$) , where N is a parameter and k=nobjs
  for (int i=0; i < MAX_ITERS; ++i)
    normwv[i] = new double[nobjs];
  
  for (int d=0;d<dim;++d)
  {
    xmin[d]=domain->getLowerLimit(d);
    xmax[d]=domain->getUpperLimit(d);
  }
  for (int obj=0; obj < nobjs; ++obj)
  {
    gideal[obj]=0.0;
    gwv[obj]=0.5;
    absmax[obj]=problem->getFitnessLimits()->getUpperLimit(obj);
    absmin[obj]=problem->getFitnessLimits()->getLowerLimit(obj);
  }
  pdim=dim*2+2;
  
  if(nobjs==2)
    N=10;  // gives 11 weight vectors  - the formula is: number of weight vectors = (N+k-1)!/N!(k-1)! where k is number of objectives
  else if(nobjs==3)
    N=4;  // gives 15 weight vectors
  else if(nobjs==4)
    N=3;   // gives 20 weight vectors
  
  snake_wv(N,nobjs);  // function to create evenly spaced normalized weight vectors
  
  init_arrays(&ax, &ay, MAX_ITERS, dim);
  init_arrays(&tmpax, &tmpay, MAX_ITERS, dim);
  
  ff = new double*[MAX_ITERS+2];
  for(int i=0;i<=MAX_ITERS+1;i++)
    ff[i]=new double[dim];
  
  int iter=10*dim+(dim-1);
  OVectorPtr result = new OVector(iter, NULL);

  for(int i=0; i<iter;++i)
    improvements[i]=0;
  
  ymin=INFTY;      
  latin_hyp(ax, iter);  // select the first solutions using the latin hypercube
  for (int i=0;i<iter;i++)
  {
    ay[i]=myfit(context, ax[i],ff[i]);
    DenseDoubleVectorPtr point = new DenseDoubleVector(dim, 0.0);
    for (int j=0; j<dim; ++j)
      point->setValue(j, ax[i][j]);
    result->setElement(i, point);
  }

  return result;
}




double ParEGOOptimizer::expected_improvement(double yhat, double ymin, double s)
{
  double E;
  double sdis;
  double sden;
  if(s<=0)
    return 0;
  if((ymin-yhat)/s < -7.0)
    sdis = 0.0;
  else if((ymin-yhat)/s > 7.0)
    sdis = 1.0;
  else 
    sdis = standard_distribution( (ymin-yhat)/s );
  
  sden = standard_density((ymin-yhat)/s);
  
  E = (ymin - yhat)*sdis + s*sden;
  return E;
}

double ParEGOOptimizer::standard_density(double z)
{
  double psi;
  
  psi = (1/sqrt(2*PI))*exp(-(z*z)/2.0);
  return (psi);
  
}

/*
 double standard_distribution(double z)
 {
 double phi;
 if(z>0.0)
 phi=0.5+qsimp(&standard_density, 0.0, z);
 else if(z<0.0)
 phi=0.5-qsimp(&standard_density, 0.0, -z);
 else
 phi=0.5;
 return(phi);
 }
 */

double ParEGOOptimizer::predict_y(double **ax, ::Matrix InvR, ::Vector y, double mu_hat, double *theta, double *p, int n, int dim)
{
  double y_hat;
  //  Matrix InvR = R.Inverse();
  ::Vector one(n);
  for(int i=0;i<n;++i)
    one(i)=1;
  
  ::Vector r(n);
  for(int i=0;i<n;++i)
    r[i] = correlation(ax[n],ax[i],theta,p,dim);
  
  y_hat = mu_hat + r * InvR * (y - mu_hat*one);
  
  return y_hat;
}

double ParEGOOptimizer::weighted_distance(double *xi, double *xj, double *theta, double *p, int dim)
{
  double sum=0.0;
  
  double nxi[dim];
  double nxj[dim];
  
  for(int h=0; h<dim; ++h)
  {
    nxi[h] = (xi[h]-xmin[h])/(xmax[h]-xmin[h]);
    nxj[h] = (xj[h]-xmin[h])/(xmax[h]-xmin[h]);
    
    sum += theta[h]*pow(myabs(nxi[h]-nxj[h]),p[h]);     
  }
  return sum;
}

double ParEGOOptimizer::correlation(double *xi, double *xj, double *theta, double *p, int dim)
{
  if(DEBUG)
    for(int d=0;d<dim;d++)
      fprintf(stdout, "CORRELATION: %lf %lf %lf %lf\n", xi[d],xj[d],theta[d],p[d]);
  return exp(-weighted_distance(xi,xj,theta,p,dim));
}

void ParEGOOptimizer::build_y(double *ay, int n, ::Vector y)
{
  for(int i=0;i<n;i++)
    y(i)=ay[i];
}

void ParEGOOptimizer::build_R(double **ax, double *theta, double *p, int dim, int n, Matrix R)
{
  // takes the array of x vectors, theta, and p, and returns the correlation matrix R.
  // TODO: can be optimized slightly since R is symmetric
  for(int i=0; i<n;i++)
    for(int j=0;j<n;j++)
      R[i][j]=correlation(ax[i],ax[j], theta, p, dim);
}

double ParEGOOptimizer::s2(double **ax, double *theta, double *p, double sigma, int dim, int n, Matrix InvR)
{
  double s2;
  //  Matrix InvR = R.Inverse();
  ::Vector one(n);
  for(int i=0;i<n;++i)
    one(i)=1;
  
  ::Vector r(n);
  for(int i=0;i<n;++i)
    r[i] = correlation(ax[n+1],ax[i],theta,p,dim);
  
  s2 = sigma * (1 - r*InvR*r + pow((1-one*InvR*r),2)/(one*InvR*one) );
  
  return s2;
}

double ParEGOOptimizer::sigma_squared_hat(Matrix InvR, ::Vector y, double mu_hat, int n)
{
  double numerator, denominator;
  
  ::Vector one(n);
  for (int i=0; i<n; i++)
    one(i)=1;
  
  ::Vector vmu=mu_hat * one;
  ::Vector diff = y - vmu;
  
  numerator = diff*InvR*diff;
  denominator = n;
  
  return numerator/denominator;
}

double ParEGOOptimizer::likelihood(double *param)
{
  // uses global variable storing the size of R: iter
  // uses global variable storing the dimension of the search space: dim
  // uses global ax and ay values
  
  double lik;
  
  // constraint handling
  double sum=0.0;
  for(int j=0;j<pdim;++j)
    if(param[j]<0.0)	
      sum+=param[j];
  
  if(sum<0)
    return(-sum);
  
  sum=0.0;
  bool outofrange=false;
  for(int j=0;j<dim;++j)
  {
    if(param[dim+j]>=2.0)
    {
      sum+=param[dim+j]-2.0;
      outofrange=true;
    }
    else if(param[dim+j]<1.0)
    {
      sum+=-(param[dim+j]-1.0);
      outofrange=true;
    }
  }
  if(outofrange)
    return(sum);
  
  double coefficient;
  double exponent;
  
  double mu=param[2*dim+1];
  double sigma=param[2*dim];
  
  Matrix R(titer,titer);
  build_R(*pax, &param[0], &param[dim], dim, titer, R);
  
  ::Vector y(titer);
  build_y(*pay, titer, y);
  
  double detR = posdet(R,titer);
  // fprintf(stdout,"R (after determinant = \n");
  //  pr_sq_mat(R, titer);
  
  //  fprintf(stdout,"determinant=%lg\n", detR);
  Matrix InvR = R.inverseSymmPositiveDefinite();
  // fprintf(stdout,"Inverse = \n");
  // pr_sq_mat(InvR, titer);
  
  ::Vector one(titer);
  for(int i=0; i<titer; i++)
    one(i)=1;
  
  ::Vector vmu=mu*one;
  ::Vector diff = y - vmu;
  
  // fprintf(stdout, "sigma= %lg  sqrt(detR)= %lg\n", sigma, sqrt(detR));
  coefficient = 1.0/(pow(2*PI,(double)titer/2.0)*pow(sigma,(double)titer/2.0)*sqrt(detR));
  // fprintf(stdout,"coefficient = %lg", coefficient);
  exponent = (diff*InvR*diff)/(2*sigma);
  
  //TODO: WTF is going on here???
  //  lik = coefficient*exp(-exponent);
  lik = coefficient*exp(-(double)titer/2.0);
  
  return(-lik);  
}


double ParEGOOptimizer::mu_hat(Matrix InvR, ::Vector y, int n)
{
  double numerator, denominator;
  ::Vector one(n);
  for(int i=0; i<n; i++)
    one(i)=1;
  
  numerator = one*InvR*y;
  denominator = one*InvR*one;
  
  return(numerator/denominator);
}


void ParEGOOptimizer::pr_sq_mat(Matrix m, int dim)
{
  for (int i=0; i<dim; ++i) 
  {
    for (int j=0; j<dim; ++j) 
      std::cout << m(i,j) << " ";
    std::cout << std::endl;
  }
}

void ParEGOOptimizer::pr_vec(::Vector v, int dim)
{
  for (int i=0; i<dim; ++i)
    std::cout << v(i) << " ";
  std::cout << std::endl;
}

double ParEGOOptimizer::standard_distribution(double z)
{
  double zv;
  int idx;
  if(z<0.0)
  {
    z *= -1;
    if(z>=7.5)
      zv = gz[75];
    else
    {
      idx = (int)(z*10);
      zv = gz[idx]+((10*z)-idx)*(gz[idx+1]-gz[idx]);
    }
    zv = 1-zv;
  }
  else if(z==0.0)
    zv = 0.5;
  else
  {
    if(z>=7.5)
      zv = gz[75];
    else
    {
      idx = (int)(z*10);
      zv = gz[idx]+((10*z)-idx)*(gz[idx+1]-gz[idx]);
    }   
  }
  return(zv);
}

void ParEGOOptimizer::init_gz()
{
  gz[0]=0.50000000000000;
  gz[1]=0.53982783727702;
  gz[2]=0.57925970943909;
  gz[3]=0.61791142218894;
  gz[4]=0.65542174161031;
  gz[5]=0.69146246127400;
  gz[6]=0.72574688224993;
  gz[7]=0.75803634777718;
  gz[8]=0.78814460141985;
  gz[9]=0.81593987468377;
  gz[10]=0.84134474629455;
  gz[11]=0.86433393905361;
  gz[12]=0.88493032977829;
  gz[13]=0.90319951541439;
  gz[14]=0.91924334076623;
  gz[15]=0.93319279873114;
  gz[16]=0.94520070830044;
  gz[17]=0.95543453724146;
  gz[18]=0.96406968088707;
  gz[19]=0.97128344018400;
  gz[20]=0.97724986805182;
  gz[21]=0.98213557943718;
  gz[22]=0.98609655248650;
  gz[23]=0.98927588997832;
  gz[24]=0.99180246407540;
  gz[25]=0.99379033467422;
  gz[26]=0.99533881197628;
  gz[27]=0.99653302619696;
  gz[28]=0.99744486966957;
  gz[29]=0.99813418669962;
  gz[30]=0.99865010196838;
  gz[31]=0.99903239678678;
  gz[32]=0.99931286206208;
  gz[33]=0.99951657585762;
  gz[34]=0.99966307073432;
  gz[35]=0.99976737092097;
  gz[36]=0.99984089140984;
  gz[37]=0.99989220026652;
  gz[38]=0.99992765195608;
  gz[39]=0.99995190365598;
  gz[40]=0.99996832875817;
  gz[41]=0.99997934249309;
  gz[42]=0.99998665425098;
  gz[43]=0.99999146009453;
  gz[44]=0.99999458745609;
  gz[45]=0.99999660232688;
  gz[46]=0.99999788754530;
  gz[47]=0.99999869919255;
  gz[48]=0.99999920667185;
  gz[49]=0.99999952081672;
  gz[50]=0.99999971332813;
  gz[51]=0.99999983016330;
  gz[52]=0.99999990035088;
  gz[53]=0.99999994209631;
  gz[54]=0.99999996667842;
  gz[55]=0.99999998100990;
  gz[56]=0.99999998928215;
  gz[57]=0.99999999400951;
  gz[58]=0.99999999668420;
  gz[59]=0.99999999818247;
  gz[60]=0.99999999901340;
  gz[61]=0.99999999946965;
  gz[62]=0.99999999971768;
  gz[63]=0.99999999985118;
  gz[64]=0.99999999992231;
  gz[65]=0.99999999995984;
  gz[66]=0.99999999997944;
  gz[67]=0.99999999998958;
  gz[68]=0.99999999999477;
  gz[69]=0.99999999999740;
  gz[70]=0.99999999999872;
  gz[71]=0.99999999999938;
  gz[72]=0.99999999999970;
  gz[73]=0.99999999999986;
  gz[74]=0.99999999999993;
  gz[75]=0.99999999999997;
}

void ParEGOOptimizer::snake_wv(int s, int k)
{
  // This funtion uses the method of generating reflected k-ary Gray codes
  // in order to generate every normalized weight vector of k dimensions and 
  // s divisions, so that they `snake' in the space, i.e., each wv is near the
  // previous one. This is useful for MOO search using weight vectors.
  
  int i,j;
  int n;
  int m;
  int d=k-1;
  int sum;
  int count=0;
  
  m=s;
  
  
  for(i=0;i<s;i++)
    wv[i][d]=i;
  
  n=s;
  
  while(m<mypow(s,k))
  {
    reverse(k,m,s);
    d--;
    m=s*m;
    for(i=0;i<m;i++)
      wv[i][d]=i/(m/s);
  }
  
  for(i=0;i<mypow(s,k);i++)
    for(j=0;j<k;j++)
      dwv[i][j]=(double)wv[i][j]/(s-1.0);
  
  count=0;
  for(i=0;i<mypow(s,k);i++)
  {
    sum=0;
    for(j=0;j<k;j++)
      sum+=wv[i][j];
    if(sum==s-1)
    {
      for(j=0;j<k;j++)
        normwv[count][j]=dwv[i][j];
      count++;
    }
  }
  nwvs=count;
}

void ParEGOOptimizer::reverse(int k, int n, int s)
{
  int h,i,j;
  for(i=0;i<n;i++)
    for(h=0;h<s-1;h++)
      for(j=0;j<k;j++)
      {
        if(h%2==0)
          wv[h*n+n+i][j]=wv[n-1-i][j];
        else
          wv[h*n+n+i][j]=wv[i][j];
      }
}


void ParEGOOptimizer::mysort(int *idx, double *val, int num)
{    
  SOR ss[num];
  
  for(int i=0;i<num;++i)
  {
    ss[i].y = val[i];
    ss[i].indx = i;
  }
  
  qsort(&(ss[0]),num,sizeof(SOR), ParEGOOptimizer::pcomp);
  
  for(int i=0;i<num;i++)
    idx[i]=ss[i].indx;
}

int ParEGOOptimizer::pcomp(const void *i, const void *j)
{
  double diff;
  diff = ((SOR *)i)->y - ((SOR *)j)->y;
  if (diff <0)
    return -1;
  else if (diff > 0)
    return 1;
  else
    return 0;
}

void ParEGOOptimizer::cleanup()
{
  delete[] improvements;
  delete[] absmax;
  delete[] absmin;
  delete[] gideal;
  delete[] gwv;
  
  for (int i=0; i<40402; ++i)
  {
    delete[] wv[i];
    delete[] dwv[i];
  }
  delete[] wv;
  delete[] dwv;
  
  for (int i=0; i<300; ++i)
    delete[] normwv[i];
  delete[] normwv;
  
  delete[] gz;
}
  
  
}; /* namespace lbcpp */

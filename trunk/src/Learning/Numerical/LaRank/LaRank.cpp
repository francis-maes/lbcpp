// -*- C++ -*-
// Main functions of the LaRank algorithm for solving Multiclass SVM
// Copyright (C) 2008- Antoine Bordes

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA

#include "precompiled.h"

#include <iostream>
#include <vector>
#include <algorithm>

#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cfloat>
#include <cassert>

#include "kcache.h"
#include "vectors.h"
#include "LaRank.h"

/*
** OUTPUT: one per class of the raining set, keep tracks of support vectors and their beta coefficients
*/
class LaRankOutput
{
public:
  LaRankOutput () {}
   ~LaRankOutput () {}

  // Initializing an output class (basically creating a kernel cache for it)
  void initialize (larank_kernel_t kfunc, long cache, void* closure)
  {
    kernel = larank_kcache_create (kfunc, closure);
    larank_kcache_set_maximum_size (kernel, cache * 1024 * 1024);
    l = 0;
  }

  // Destroying an output class (basically destroying the kernel cache)
  void destroy ()
  {
    larank_kcache_destroy (kernel);
    beta.resize (0);
    g.resize (0);
  }
  // Writing the output in a file
  void save_output (std::ostream & ostr, int ythis) const
  {
    int *r2i = larank_kcache_r2i (kernel, l);
    LaFVector rtoi;
    for (int r = 0; r < l; r++)
        rtoi.set (r, r2i[r]);

      ostr << ythis << " " << beta;
      ostr << l << " " << rtoi;
  }

  // Organizing the cache of an output 
  void load_output (LaFVector nbeta, int nl, SVector & rtoi)
  {
    beta = nbeta;
    l = nl;
    for (int r = 0; r < l; r++)
      larank_kcache_swap_ri (kernel, r, (int) rtoi.get (r));
  }

  // !Important! Computing the score of a given input vector for the actual output
  double computeScore (int x_id)
  {
    if (l == 0)
      return 0;
    else
      {
	float *row = larank_kcache_query_row (kernel, x_id, l);
	return dot (beta, row, l);
      }
  }

  // !Important! Computing the gradient of a given input vector for the actual output           
  double computeGradient (int xi_id, int yi, int ythis)
  {
    return (yi == ythis ? 1 : 0) - computeScore (xi_id);
  }

  // Updating the solution in the actual output
  void update (int x_id, double lambda, double gp)
  {
    int *r2i = larank_kcache_r2i (kernel, l);
    int xr = l + 1;
    for (int r = 0; r < l; r++)
      if (r2i[r] == x_id)
	{
	  xr = r;
	  break;
	}

    // updates the cache order and the beta coefficient
    if (xr < l)
      {
	double oldb = beta.get (xr);
	beta.set (xr, oldb + lambda);
      }
    else
      {
	g.set (l, gp);
	beta.set (l, lambda);
	larank_kcache_swap_ri (kernel, l, x_id);
	l++;
      }

    // update stored gradients
    float *row = larank_kcache_query_row (kernel, x_id, l);
    for (int r = 0; r < l; r++)
      {
	double oldg = g.get (r);
	g.set (r, oldg - lambda * row[r]);
      }
  }

  // Linking the cahe of this output to the cache of an other "buddy" output
  // so that if a requested value is not found in this cache, you can ask your buddy if it has it.                              
  void set_kernel_buddy (larank_kcache_t * bud)
  {
    larank_kcache_set_buddy (bud, kernel);
  }

  // Removing useless support vectors (for which beta=0)                
  int cleanup ()
  {
    int count = 0;
    std::vector < int >idx;
    for (int x = 0; x < l; x++)
      {
	if ((beta.get (x) < FLT_EPSILON) && (beta.get (x) > -FLT_EPSILON))
	  {
	    idx.push_back (x);
	    count++;
	  }
      }
    int new_l = l - count;
    for (int xx = 0; xx < count; xx++)
      {
	int i = idx[xx] - xx;
	for (int r = i; r < (l - 1); r++)
	  {
	    larank_kcache_swap_rr (kernel, r, r + 1);
	    beta.set (r, beta.get (r + 1));
	    g.set (r, g.get (r + 1));
	  }
      }
    l = new_l;
    beta.resize (l);
    g.resize (l);
    return count;
  }

  // --- Below are information or "get" functions --- //

  //                            
  larank_kcache_t *getKernel ()
  {
    return kernel;
  }

  //
  double getW2 ()
  {
    double sum = 0;
    int *r2i = larank_kcache_r2i (kernel, l + 1);
    for (int r = 0; r < l; r++)
      {
	float *row_r = larank_kcache_query_row (kernel, r2i[r], l);
	sum += beta.get (r) * dot (beta, row_r, l);
      }
    return sum;
  }

  //
  double getKii (int x_id)
  {
    return larank_kcache_query (kernel, x_id, x_id);
  }

  //
  double getBeta (int x_id)
  {
    int *r2i = larank_kcache_r2i (kernel, l);
    int xr = -1;
    for (int r = 0; r < l; r++)
      if (r2i[r] == x_id)
	{
	  xr = r;
	  break;
	}
    return (xr < 0 ? 0 : beta.get (xr));
  }

  //
  double getGradient (int x_id)
  {
    int *r2i = larank_kcache_r2i (kernel, l);
    int xr = -1;
    for (int r = 0; r < l; r++)
      if (r2i[r] == x_id)
	{
	  xr = r;
	  break;
	}
    return (xr < 0 ? 0 : g.get (xr));
  }

  //
  bool isSupportVector (int x_id) const
  {
    int *r2i = larank_kcache_r2i (kernel, l);
    int xr = -1;
    for (int r = 0; r < l; r++)
      if (r2i[r] == x_id)
	{
	  xr = r;
	  break;
	}
    return (xr >= 0);
  }

  //
  int getSV (LaFVector & sv) const
  {
    sv.resize (l);
    int *r2i = larank_kcache_r2i (kernel, l);
    for (int r = 0; r < l; r++)
        sv.set (r, r2i[r]);
      return l;
  }

private:
  // the solution of LaRank relative to the actual class is stored in this parameters

  LaFVector beta;		// Beta coefficiens
  LaFVector g;			// Strored gradient derivatives
  larank_kcache_t *kernel;	// Cache for kernel values
  int l;			// Number of support vectors 
};

/*
** LARANK: here is the big stuff
*/
class LaRank:public Machine
{
public:
  LaRank (void* closure): nb_seen_examples (0), nb_removed (0),
    n_pro (0), n_rep (0), n_opt (0),
    w_pro (1), w_rep (1), w_opt (1), y0 (0), dual (0), closure(closure) {}

   ~LaRank () {}

  // LEARNING FUNCTION: add new patterns and run optimization steps selected with adaptative schedule
  virtual int add (int x_id, int yi)
  {
    ++nb_seen_examples;
    // create a new output object if this one has never been seen before 
    if (!getOutput (yi))
      {
	outputs.insert (std::make_pair (yi, LaRankOutput ()));
	LaRankOutput *cur = getOutput (yi);
	cur->initialize (kfunc, cache, closure);
	if (outputs.size () == 1)
	  y0 = outputs.begin ()->first;
	// link the cache of this new output to a buddy 
	if (outputs.size () > 1)
	  {
	    LaRankOutput *out0 = getOutput (y0);
	    cur->set_kernel_buddy (out0->getKernel ());
	  }
      }

    LaRankPattern tpattern (x_id, yi);
    LaRankPattern & pattern = (patterns.isPattern (x_id)) ? patterns.getPattern (x_id) : tpattern;

    // ProcessNew with the "fresh" pattern
    double time1 = getTime ();
    process_return_t pro_ret = process (pattern, processNew);
    double dual_increase = pro_ret.dual_increase;
    double duration = (getTime () - time1);
    double coeff = dual_increase / (0.00001 + duration);
    dual += dual_increase;
    n_pro++;
    w_pro = 0.05 * coeff + (1 - 0.05) * w_pro;

    // ProcessOld & Optimize until ready for a new processnew
    // (Adaptative schedule here)
    for (;;)
      {
	double w_sum = w_pro + w_rep + w_opt;
	double prop_min = w_sum / 20;
	if (w_pro < prop_min)
	  w_pro = prop_min;
	if (w_rep < prop_min)
	  w_rep = prop_min;
	if (w_opt < prop_min)
	  w_opt = prop_min;
	w_sum = w_pro + w_rep + w_opt;
	double r = rand () / (double) RAND_MAX * w_sum;
	if (r <= w_pro)
	  {
	    break;
	  }
	else if ((r > w_pro) && (r <= w_pro + w_rep))	// ProcessOld here
	  {
	    double time1 = getTime ();
	    double dual_increase = reprocess ();
	    double duration = (getTime () - time1);
	    double coeff = dual_increase / (0.00001 + duration);
	    dual += dual_increase;
	    n_rep++;
	    w_rep = 0.05 * coeff + (1 - 0.05) * w_rep;
	  }
	else			// Optimize here 
	  {
	    double time1 = getTime ();
	    double dual_increase = optimize ();
	    double duration = (getTime () - time1);
	    double coeff = dual_increase / (0.00001 + duration);
	    dual += dual_increase;
	    n_opt++;
	    w_opt = 0.05 * coeff + (1 - 0.05) * w_opt;
	  }
      }
    if (nb_seen_examples % 100 == 0)	// Cleanup useless Support Vectors/Patterns sometimes
      nb_removed += cleanup ();
    return pro_ret.ypred;
  }

  // PREDICTION FUNCTION: main function in la_rank_classify
  virtual int predict (int x_id)
  {
    int res = -1;
    double score_max = -DBL_MAX;
    for (outputhash_t::iterator it = outputs.begin (); it != outputs.end ();++it)
      {
	double score = it->second.computeScore (x_id);
	if (score > score_max)
	  {
	    score_max = score;
	    res = it->first;
	  }
      }
    return res;
  }

  virtual void destroy ()
  {
    for (outputhash_t::iterator it = outputs.begin (); it != outputs.end ();++it)
      it->second.destroy ();
  }


  // Used for saving a model file
  virtual void save_outputs (std::ostream & ostr)
  {
    for (outputhash_t::const_iterator it = outputs.begin (); it != outputs.end (); ++it)
      it->second.save_output (ostr, it->first);
  }

  // Used for loading a model file
  virtual void add_outputs (Exampler model)
  {
    for (int out = 0; out < model.nb_ex; out++)
      {
	int label = model.data[out].cls;
	LaFVector betas = LaFVector (model.data[out].inpt);
	++out;
	int l = model.data[out].cls;
	SVector r2i = model.data[out].inpt;

	outputs.insert (std::make_pair (label, LaRankOutput ()));
	LaRankOutput *cur = getOutput (label);
	cur->initialize (kfunc, cache, closure);
	cur->load_output (betas, l, r2i);
      }
  }

  // Compute Duality gap (costly but used in stopping criteria in batch mode)                     
  virtual double computeGap ()
  {
    double sum_sl = 0;
    double sum_bi = 0;
    for (unsigned i = 0; i < patterns.maxcount (); ++i)
      {
	const LaRankPattern & p = patterns[i];
	if (!p.exists ())
	  continue;
	LaRankOutput *out = getOutput (p.y);
	if (!out)
	  continue;
	sum_bi += out->getBeta (p.x_id);
	double gi = out->computeGradient (p.x_id, p.y, p.y);
	double gmin = DBL_MAX;
	for (outputhash_t::iterator it = outputs.begin (); it != outputs.end (); ++it)
	  {
	    if (it->first != p.y && it->second.isSupportVector (p.x_id))
	      {
		double g =
		  it->second.computeGradient (p.x_id, p.y, it->first);
		if (g < gmin)
		  gmin = g;
	      }
	  }
	sum_sl += jmax (0, gi - gmin);
      }
    return jmax (0, computeW2 () + C * sum_sl - sum_bi);
  }

  // Display stuffs along learning
  virtual void printStuff (double initime, bool print_dual)
  {
    std::cout << "Current duration (CPUs): " << getTime () - initime << std::endl;
    if (print_dual)
      std::cout << "Dual: " << dual << std::endl;
    std::cout << "Number of Support Patterns: " << patterns.size () << " / " << nb_seen_examples << " (removed:" << nb_removed <<")" << std::endl;
    std::cout << "Number of Support Vectors: " << getNSV () << " (~ " << getNSV () / (double) patterns.size () << " SV/Pattern)" << std::endl;
    double w_sum = w_pro + w_rep + w_opt;
    std::cout << "ProcessNew:" << n_pro << " (" << w_pro / w_sum << ") ProcessOld:" 
	      << n_rep << " (" << w_rep / w_sum << ") Optimize:" << n_opt << " (" << w_opt / w_sum << ")" << std::endl;
    std::cout << "----" << std::endl;

  }


  // Nuber of classes so far
  virtual unsigned getNumOutputs () const
  {
    return outputs.size ();
  }

  // Number of Support Vectors
  int getNSV ()
  {
    int res = 0;
    for (outputhash_t::const_iterator it = outputs.begin (); it != outputs.end (); ++it)
      {
	LaFVector sv;
	it->second.getSV (sv);
	res += sv.size ();
      }
    return res;
  }

  // Norm of the parameters vector
  double computeW2 ()
  {
    double res = 0;
    for (unsigned i = 0; i < patterns.maxcount (); ++i)
      {
	const LaRankPattern & p = patterns[i];
	if (!p.exists ())
	  continue;
	for (outputhash_t::iterator it = outputs.begin (); it != outputs.end (); ++it)
	  if (it->second.getBeta (p.x_id))
	    res += it->second.getBeta (p.x_id) * it->second.computeScore (p.x_id);
      }
    return res;
  }

  // Compute Dual objective value
  double getDual ()
  {
    double res = 0;
    for (unsigned i = 0; i < patterns.maxcount (); ++i)
      {
	const LaRankPattern & p = patterns[i];
	if (!p.exists ())
	  continue;
	LaRankOutput *out = getOutput (p.y);
	if (!out)
	  continue;
	res += out->getBeta (p.x_id);
      }
    return res - computeW2 () / 2;
  }


private:
  /*
   ** MAIN DARK OPTIMIZATION PROCESSES
   */

  // Hash Table used to store the different outputs
  typedef std_hash_map < int, LaRankOutput > outputhash_t;	// class index -> LaRankOutput
  outputhash_t outputs;
  LaRankOutput *getOutput (int index)
  {
    outputhash_t::iterator it = outputs.find (index);
    return it == outputs.end ()? NULL : &it->second;
  }

  // 
  LaRankPatterns patterns;

  // Parameters
  int nb_seen_examples;
  int nb_removed;

  // Numbers of each operation performed so far
  int n_pro;
  int n_rep;
  int n_opt;

  // Running estimates for each operations 
  double w_pro;
  double w_rep;
  double w_opt;

  int y0;
  double dual;
  
  void* closure;

  struct outputgradient_t
  {
    outputgradient_t (int output, double gradient)
      : output (output), gradient (gradient) {}
    outputgradient_t ()
      : output (0), gradient (0) {}

    int output;
    double gradient;

    bool operator < (const outputgradient_t & og) const
    {
      return gradient > og.gradient;
    }
  };

  //3 types of operations in LaRank               
  enum process_type
  {
    processNew,
    processOld,
    processOptimize
  };

  struct process_return_t
  {
    process_return_t (double dual, int ypred) 
      : dual_increase (dual), ypred (ypred) {}
    process_return_t () {}
    double dual_increase;
    int ypred;
  };

  // IMPORTANT Main SMO optimization step
  process_return_t process (const LaRankPattern & pattern, process_type ptype)
  {
    process_return_t pro_ret = process_return_t (0, 0);

    /*
     ** compute gradient and sort   
     */
    std::vector < outputgradient_t > outputgradients;
    outputgradients.reserve (getNumOutputs ());

    std::vector < outputgradient_t > outputscores;
    outputscores.reserve (getNumOutputs ());

    for (outputhash_t::iterator it = outputs.begin (); it != outputs.end (); ++it)
      if (ptype != processOptimize
	  || it->second.isSupportVector (pattern.x_id))
	{
	  double g =
	    it->second.computeGradient (pattern.x_id, pattern.y, it->first);
	  outputgradients.push_back (outputgradient_t (it->first, g));
	  if (it->first == pattern.y)
	    outputscores.push_back (outputgradient_t (it->first, (1 - g)));
	  else
	    outputscores.push_back (outputgradient_t (it->first, -g));
	}
    std::sort (outputgradients.begin (), outputgradients.end ());

    /*
     ** determine the prediction
     */
    std::sort (outputscores.begin (), outputscores.end ());
    pro_ret.ypred = outputscores[0].output;

    /*
     ** Find yp (1st part of the pair)
     */
    outputgradient_t ygp;
    LaRankOutput *outp = NULL;
    unsigned p;
    for (p = 0; p < outputgradients.size (); ++p)
      {
	outputgradient_t & current = outputgradients[p];
	LaRankOutput *output = getOutput (current.output);
	bool support = ptype == processOptimize || output->isSupportVector (pattern.x_id);
	bool goodclass = current.output == pattern.y;
	if ((!support && goodclass) || (support && output->getBeta (pattern.x_id) < (goodclass ? C : 0)))
	  {
	    ygp = current;
	    outp = output;
	    break;
	  }
      }
    if (p == outputgradients.size ())
      return pro_ret;
    
    /*
    ** Find ym (2nd part of the pair)
    */
    outputgradient_t ygm;
    LaRankOutput *outm = NULL;
    int m;
    for (m = outputgradients.size () - 1; m >= 0; --m)
      {
	outputgradient_t & current = outputgradients[m];
	LaRankOutput *output = getOutput (current.output);
	bool support = ptype == processOptimize || output->isSupportVector (pattern.x_id);
	bool goodclass = current.output == pattern.y;
	if (!goodclass || (support && output->getBeta (pattern.x_id) > 0))
	  {
	    ygm = current;
	    outm = output;
	    break;
	  }
      }
    if (m < 0)
      return pro_ret;

    /*
     ** Throw or Insert pattern
     */
    if ((ygp.gradient - ygm.gradient) < tau)
      return pro_ret;
    if (ptype == processNew)
      patterns.insert (pattern);

    /*
     ** compute lambda and clip it
     */
    double kii = outp->getKii (pattern.x_id);
    double lambda = (ygp.gradient - ygm.gradient) / (2 * kii);
    if (ptype == processOptimize || outp->isSupportVector (pattern.x_id))
      {
	double beta = outp->getBeta (pattern.x_id);
	if (ygp.output == pattern.y)
	  lambda = jmin (lambda, C - beta);
	else
	  lambda = jmin (lambda, fabs (beta));
      }
    else
      lambda = jmin (lambda, C);

    /*
     ** update the solution
     */
    outp->update (pattern.x_id, lambda, ygp.gradient);
    outm->update (pattern.x_id, -lambda, ygm.gradient);

    pro_ret.dual_increase = lambda * ((ygp.gradient - ygm.gradient) - lambda * kii);
    return pro_ret;
  }

  // ProcessOld
  double reprocess ()
  {
    if (patterns.size ())
      for (int n = 0; n < 10; ++n)
	{
	  process_return_t pro_ret = process (patterns.sample (), processOld);
	  if (pro_ret.dual_increase)
	    return pro_ret.dual_increase;
	}
    return 0;
  }
  
  // Optimize
  double optimize ()
  {
    double dual_increase = 0;
    if (patterns.size ())
      for (int n = 0; n < 10; ++n)
	{
	  process_return_t pro_ret =
	    process (patterns.sample (), processOptimize);
	  dual_increase += pro_ret.dual_increase;
	}
    return dual_increase;
  }
  
  // remove patterns and return the number of patterns that were removed
  unsigned cleanup ()
  {
    for (outputhash_t::iterator it = outputs.begin (); it != outputs.end (); ++it)
      it->second.cleanup ();
    unsigned res = 0;
    for (unsigned i = 0; i < patterns.size (); ++i)
      {
	LaRankPattern & p = patterns[i];
	if (p.exists () && !outputs[p.y].isSupportVector (p.x_id))
	  {
	    patterns.remove (i);
	    ++res;
	  }
      }
    return res;
  }
};

Machine *
create_larank (void* closure)
{
  return new LaRank (closure);
}

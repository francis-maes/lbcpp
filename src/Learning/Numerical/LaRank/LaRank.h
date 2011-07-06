// -*- C++ -*-
// Main functions of the LaRank algorithm for soving Multiclass SVM
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

#ifndef LARANK_H
#define LARANK_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
# include <ctime>
# include <sys/time.h>
# include <ext/hash_map>
# include <ext/hash_set>
# include <cmath>
# define STDEXT_NAMESPACE __gnu_cxx
# define std_hash_map STDEXT_NAMESPACE::hash_map
# define std_hash_set STDEXT_NAMESPACE::hash_set

#include "kcache.h"
#include "vectors.h"

#define jmin(a,b) (((a)<(b))?(a):(b))
#define jmax(a,b) (((a)>(b))?(a):(b))


/*
**	EXAMPLER: to read and store data and model files.	
*/
class Exampler
{
public:

  struct example_t
  {
    example_t (SVector x, int y, double n)
      :inpt (x), cls (y), norm (n) {}
    example_t () {}

    SVector inpt;
    int cls;
    double norm;
  };

  typedef std::vector < example_t > examples_t;

  int libsvm_load_data (char *filename, bool model)
  {
    int index;
    double value;
    int elements, i;
    FILE *fp = fopen (filename, "r");

    if (fp == NULL)
      {
	fprintf (stderr, "Can't open input file \"%s\"\n", filename);
	exit (1);
      }
    else
      printf ("loading \"%s\"..  \n", filename);
    int msz = 0;
    elements = 0;
    while (1)
      {
	int c = fgetc (fp);
	switch (c)
	  {
	  case '\n':
	    ++msz;
	    elements = 0;
	    break;
	  case ':':
	    ++elements;
	    break;
	  case EOF:
	    goto out;
	  default:
	    ;
	  }
      }
  out:
    rewind (fp);
    max_index = 0;
    nb_labels = 0;
    for (i = 0; i < msz; i++)
      {
	int label;
	SVector v;
	fscanf (fp, "%d", &label);
	if ((int) label >= nb_labels)
	  nb_labels = label;
	while (1)
	  {
	    int c;
	    do
	      {
		c = getc (fp);
		if (c == '\n')
		  goto out2;
	      }
	    while (isspace (c));
	    ungetc (c, fp);
	    fscanf (fp, "%d:%lf", &index, &value);
	    v.set (index, value);
	    if (index > max_index)
	      max_index = index;
	  }

      out2:
	data.push_back (example_t (v, label, dot (v, v)));
      }
    fclose (fp);
    if (!model)
      printf ("examples: %d features: %d labels: %d\n", msz, max_index,nb_labels);
    else
      printf ("classes: %d\n", msz / 2);
    nb_ex = msz;
    return msz;
  }

  examples_t data;
  int nb_ex;
  int max_index;
  int nb_labels;
};


/*
**	LARANKPATTERN: to keep track of the support patterns
*/
class LaRankPattern
{
public:
  LaRankPattern (int x_id, int y) 
    : x_id (x_id), y (y) {}
  LaRankPattern () 
    : x_id (0) {}

  bool exists () const
  {
    return x_id >= 0;
  }

  void clear ()
  {
    x_id = -1;
  }

  int x_id;
  int y;
};


/*
**  LARANKPATTERNS: the collection of support patterns
*/
class LaRankPatterns
{
public:
  LaRankPatterns () {}
   ~LaRankPatterns () {}

  void insert (const LaRankPattern & pattern)
  {
    if (!isPattern (pattern.x_id))
      {
	if (freeidx.size ())
	  {
	    std_hash_set < unsigned >::iterator it = freeidx.begin ();
	    patterns[*it] = pattern;
	    x_id2rank[pattern.x_id] = *it;
	    freeidx.erase (it);
	  }
	else
	  {
	    patterns.push_back (pattern);
	    x_id2rank[pattern.x_id] = patterns.size () - 1;
	  }
      }
    else
      {
	int rank = getPatternRank (pattern.x_id);
	patterns[rank] = pattern;
      }
  }

  void remove (unsigned i)
  {
    x_id2rank[patterns[i].x_id] = 0;
    patterns[i].clear ();
    freeidx.insert (i);
  }

  bool empty () const
  {
    return patterns.size () == freeidx.size ();
  }

  unsigned size () const
  {
    return patterns.size () - freeidx.size ();
  }

  LaRankPattern & sample ()
  {
    assert (!empty ());
    while (true)
      {
	unsigned r = rand () % patterns.size ();
	if (patterns[r].exists ())
	  return patterns[r];
      }
    return patterns[0];
  }

  unsigned getPatternRank (int x_id)
  {
    return x_id2rank[x_id];
  }

  bool isPattern (int x_id)
  {
    return x_id2rank[x_id] != 0;
  }

  LaRankPattern & getPattern (int x_id)
  {
    unsigned rank = x_id2rank[x_id];
    return patterns[rank];
  }

  unsigned maxcount () const
  {
    return patterns.size ();
  }

  LaRankPattern & operator [] (unsigned i)
  {
    return patterns[i];
  }

  const LaRankPattern & operator [] (unsigned i) const
  {
    return patterns[i];
  }

private:
  std_hash_set < unsigned >freeidx;
  std::vector < LaRankPattern > patterns;
  std_hash_map < int, unsigned >x_id2rank;
};


/*
** MACHINE: the main thing, which is trained.
*/
class Machine
{
public:
  virtual ~ Machine () {};
  virtual void destroy () = 0;

  // MAIN functions for straining and testing      
  virtual int add (int x_id, int classnumber) = 0;
  virtual int predict (int x_id) = 0;

  // Functions for saving and loading model
  virtual void save_outputs (std::ostream & ostr) = 0;
  virtual void add_outputs (Exampler model) = 0;

  // Information functions
  virtual void printStuff (double initime, bool print_dual) = 0;
  virtual double computeGap () = 0;

  std_hash_set < int >classes;

  unsigned class_count () const
  {
    return classes.size ();
  }

  double C;
  double tau;
  int nb_train;
  long cache;
  larank_kernel_t kfunc;

};

extern Machine *create_larank (void* closure);

inline double
getTime ()
{
  struct timeval tv;
  struct timezone tz;
  long int sec;
  long int usec;
  double mytime;
  gettimeofday (&tv, &tz);
  sec = (long int) tv.tv_sec;
  usec = (long int) tv.tv_usec;
  mytime = (double) sec + usec * 0.000001;
  return mytime;
}

#endif

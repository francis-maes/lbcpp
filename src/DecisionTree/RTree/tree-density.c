
/*
 * Author: Pierre Geurts (p.geurts@ulg.ac.be)
 *
 * (Please do not distribute without permission of the author)
 *
 */

/*
 * Various functions related to unsupervised trees
 */

/* compute_node_subset_current_ensemble */

/* we add information about the subset associated to each node
 * To not add computational requirement to the original algorithm, 
 * we will compute this information afterwards for the current 
 * ensemble of trees
 */

int node_subset_desc_size=0;
typedef int nsd_type[2];

nsd_type *node_subset_desc=NULL; /* [0] is the number of cases in that node
				  * [1] is the index in the node_object_list
				  */

void free_node_susbet_desc() {
  if (node_subset_desc_size>0)
    MyFree((nsd_type *)node_subset_desc);
}

void allocate_node_subset_desc(int size) {
  if (node_subset_desc_size<size) {
    free_node_susbet_desc();
    node_subset_desc=(nsd_type *)MyMalloc((size_t)size*2*sizeof(int));
    if (!node_subset_desc) {
      printf("impossible d'allouer de la memoire dans allocate_node_subset_desc\n");
      exit(0);
    }
    node_subset_desc_size=size;
  }
}

int *node_object_list;
int node_object_list_size=0;

/* pour renvoyer les resultats au lisp */
float *object_weight2;

DllExport void compute_node_subset_current_ensemble(float *ow) {
  int t,i,j,s;
  int pos_nol; /* position in node_object_list */

  object_weight2=ow;

  s=current_nb_of_ensemble_terms*global_learning_set_size;
  if (node_object_list_size<s) {
    printf("coucou2");fflush(stdout);
    if (node_object_list_size>0) {
      MyFree((int *)node_object_list_size);
    }
    node_object_list=(int *)MyMalloc((size_t)s*sizeof(int));
    node_object_list_size=s;
  }

  /* alloue la mémoire pour la table node_subset_desc */
  allocate_node_subset_desc(index_nodes+1);

  /* we fill node_object_list with all objects from the ls (index to object_mapping) */
  pos_nol=0;
  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    
    /* printf("."); fflush(stdout); */
    
    index_stack_open_nodes=-1;
    index_stack_open_nodes++;
    stack_open_nodes[index_stack_open_nodes][0]=ltrees[t];
    stack_open_nodes[index_stack_open_nodes][1]=pos_nol;
    stack_open_nodes[index_stack_open_nodes][2]=pos_nol+global_learning_set_size-1;

    /* on remplit le vector */
    node_subset_desc[ltrees[t]][0]=global_learning_set_size;
    node_subset_desc[ltrees[t]][1]=pos_nol;
    for (i=0; i<global_learning_set_size; i++)
      node_object_list[pos_nol++]=i;

    while(index_stack_open_nodes>=0) {
      int node=stack_open_nodes[index_stack_open_nodes][0];
      int start=stack_open_nodes[index_stack_open_nodes][1];
      int end=stack_open_nodes[index_stack_open_nodes][2];
      int node_size=end-start+1;
      
      index_stack_open_nodes--;

      if (left_successor[node]!=-1) { /* not a leaf */
	int borne=separate_ls_vector(tested_attribute[node], threshold[node], node_object_list, start, end);
	int left=node+left_successor[node], right=node+right_successor[node];

	node_subset_desc[left][0]=borne-start;
	node_subset_desc[left][1]=start;
	node_subset_desc[right][0]=end-borne+1;
	node_subset_desc[right][1]=borne;

	index_stack_open_nodes++;
	stack_open_nodes[index_stack_open_nodes][0]=node+left_successor[node];
	stack_open_nodes[index_stack_open_nodes][1]=start;
	stack_open_nodes[index_stack_open_nodes][2]=borne-1;
	index_stack_open_nodes++;
	stack_open_nodes[index_stack_open_nodes][0]=node+right_successor[node];
	stack_open_nodes[index_stack_open_nodes][1]=borne;
	stack_open_nodes[index_stack_open_nodes][2]=end;
      } 
    }
  }
  /* printf("\n");fflush(stdout); */
}

/* recupere la taille du noeud dans node_subset_desc */

DllExport int get_node_size(int node) {
  return node_subset_desc[node][0];
}

/* describe the unknown attribute for the test */

int *unknown_attribute_flag;

DllExport void set_unknown_attribute_flag(int *vector) {
  unknown_attribute_flag=vector;
}

/* renvoie dans object_weights le poids des objets au point considere
    en tenant compte des missing_attribute_description puis ensuite 
   calcule les valeurs des attributes de sortie correspondant à ces poids
   pour les attributs numeriques: moyenne ponderee
   pour les attributs symboliques: valeur la plus probable
*/

/* get the object weight from one tree (assume that 
 * compute_node_subset_current_ensemble has been run and that object_weight
 * has been initialized)
 */

int weight_prop=1;

DllExport void set_weight_prop(int i) {
  weight_prop=i;
  printf("weight_prop=%d\n",weight_prop);
  fflush(stdout);
}

DllExport void get_ls_object_weight_from_one_tree(int tree, int obj) {
  int i, start,end;
  int current_node=tree;
  SCORE_TYPE sum_weight=0.0;

  node_weight[tree]=1.0;
  index_stack_open_nodes=-1;
  index_stack_open_nodes++;
  stack_open_nodes[index_stack_open_nodes][0]=tree;

  for (i=0; i<global_learning_set_size; i++) {
    object_weight[i]=0.0;
  }
  
  while(index_stack_open_nodes>=0) {
    int current_node, left, right;

    current_node=stack_open_nodes[index_stack_open_nodes][0];
    index_stack_open_nodes--;

    if (left_successor[current_node]==-1) { /* a leaf */
      /* we add the weight to each object from this node */
      float weight;
      if (weight_prop)
	weight=node_weight[current_node]/node_subset_desc[current_node][0];
      else
	weight=node_weight[current_node];
	
      start=node_subset_desc[current_node][1];
      end=start+node_subset_desc[current_node][0];
      for (i=start; i<end; i++) {
	object_weight[node_object_list[i]]+=weight;
	sum_weight+=weight;
      }
    } else { /* a test node */
      if (unknown_attribute_flag[tested_attribute[current_node]]==0) {
	/* attribute known */
	if (check_test(tested_attribute[current_node],
		       getattval(obj,tested_attribute[current_node]), 
		       threshold[current_node])) {
	  left=current_node+left_successor[current_node];
	  index_stack_open_nodes++;
	  stack_open_nodes[index_stack_open_nodes][0]=left;
	  node_weight[left]=node_weight[current_node];
	} else {
	  right=current_node+right_successor[current_node];
	  index_stack_open_nodes++;
	  stack_open_nodes[index_stack_open_nodes][0]=right;
	  node_weight[right]=node_weight[current_node];
	}
      } else {
	/* attribute unknown */

	left=current_node+left_successor[current_node];
	right=current_node+right_successor[current_node];

	index_stack_open_nodes++;
	stack_open_nodes[index_stack_open_nodes][0]=left;
	if (weight_prop) {
	  node_weight[left]=node_weight[current_node]*node_subset_desc[left][0]/node_subset_desc[current_node][0];
	} else {
	  node_weight[left]=1.0;
	}

	index_stack_open_nodes++;
	stack_open_nodes[index_stack_open_nodes][0]=right;
	if (weight_prop) {
	  node_weight[right]=node_weight[current_node]*node_subset_desc[right][0]/node_subset_desc[current_node][0];
	} else {
	  node_weight[right]=1.0;
	}
      }
    }
  }
  
  /* normalization */
  if (!weight_prop) {
    for (i=0; i<global_learning_set_size; i++) {
      /*
	if (object_weight[i]!=0) {
	printf("%d=%f ", i, object_weight[i]/sum_weight);
	}
      */
      object_weight2[i]+=(object_weight[i]/sum_weight);
    }
    /*
      printf("\n");
      fflush(stdout);
    */
  } else {
    for (i=0; i<global_learning_set_size; i++) {
      /*
      if (object_weight[i]!=0) {
	printf("%d=%f ", i, object_weight[i]);
      }
      */
      object_weight2[i]+=object_weight[i];
    }
    /*
      printf("\n");
      fflush(stdout);
    */
  }
}

/* at the moment, does not take into account the weight of the trees in the ensemble */
     
void get_ls_object_weight_from_ensemble(int obj) {
  int i,t;
  /* init object_weight */
  for (i=0; i<global_learning_set_size; i++) {
    object_weight2[i]=0.0;
  }
  
  /* examine all trees */
  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    get_ls_object_weight_from_one_tree(ltrees[t], obj);
  }
}

/* Applications */

/* Supervised learning with missing attributes */
/* we suppose that unknown_attribute_flag has been defined (from lisp ??) */

DllExport float apply_current_ltrees_obj_regr_unknown(int obj) {
  int i;
  float sum=0.0;
  /* compute the weight */
  get_ls_object_weight_from_ensemble(obj);
  /* do the average */
  for (i=0; i<global_learning_set_size; i++) {
    sum+=object_weight2[i]*getobjy(object_mapping[i]);
  }

  /* normalize */
  return (sum/current_nb_of_ensemble_terms);
}

DllExport int apply_current_ltrees_obj_clas_unknown(int obj, float *vector) {
  int i,t,best_class;
  float sum=0.0, max_proba=-1.0;
  
  for (i=0; i<nb_classes; i++) {
    vector[i]=0.0;
  }
  /*
  printf("obj=%d, vrai classe=%d, nb_class=%d, goal_class=%d, nb_obj_ct=%d\n", obj, getobjclass(obj), nb_classes, goal, nb_obj_in_core_table);
  printf("coretable[122]=%f,=%f\n", core_table[122], core_table[goal*nb_obj_in_core_table+obj]);
  */

  /* compute the weight */
  get_ls_object_weight_from_ensemble(obj);
  /* do the average */
  for (i=0; i<global_learning_set_size; i++) {
    int c=getobjclass(object_mapping[i]);
    vector[c]+=object_weight2[i];
  }

  /* normalize */  
  for (i=0; i<nb_classes; i++) {
    sum+=vector[i];
    if (vector[i]>max_proba) {
      max_proba=vector[i];
      best_class=i;
    }
  }

  for (i=0; i<nb_classes; i++) {
    vector[i]/=sum;
  }
  /*
  printf("pred=%d\n", best_class);
  fflush(stdout);
  */
  return best_class;
}


/*
 * Author: Pierre Geurts (p.geurts@ulg.ac.be)
 *
 * (Please do not distribute without permission of the author)
 *
 */

/* Differentes mesures de similarite avec des ensemble d'arbres */

/* PREMIERE MESURE (Breiman, random forests) */
/* ------------------------------------------*/

/* devrait etre reimplemente. Ne fonctionne pas si les arbres sont non supervise car dans ce
 * cas, pred1 et pred2 ont toujours egaux a NULL
 */

DllExport int ensemble_similarity(int o1, int o2) {
  int t;
  int c=0;
  
  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    float *pred1=get_tree_prediction_vector(ltrees[t], o1);
    float *pred2=get_tree_prediction_vector(ltrees[t], o2);
    if (pred1==pred2)
      c++;
  }
  return c;
}

/* on suppose que l'ensemble des objets se trouve dans */

int last_1nn_ltrees_sim=0;

int get_nn_ltrees(int o) {
  int t,i;
  int current_max_sim=0;
  int current_nn=-1;

  for (i=0; i<global_learning_set_size; i++) {
    int current_sum=0;
/*     t=0; */
/*     do { */
/*       float *pred1=get_tree_prediction_vector(ltrees[t], o); */
/*       float *pred2=get_tree_prediction_vector(ltrees[t], object_mapping[i]); */
/*       if (pred1==pred2) */
/* 	current_sum++; */
/*       t++; */
/*     } while ((t<current_nb_of_ensemble_terms) && (current_sum+current_nb_of_ensemble_terms-t-1>current_max_sim)); */

    for (t=0; t<current_nb_of_ensemble_terms; t++) {
      float *pred1=get_tree_prediction_vector(ltrees[t], o);
      float *pred2=get_tree_prediction_vector(ltrees[t], object_mapping[i]);
      if (pred1==pred2) 
 	current_sum++; 
    }

    if (current_sum>current_max_sim) {
      current_max_sim=current_sum;
      current_nn=object_mapping[i];
    }
  }
  last_1nn_ltrees_sim=current_max_sim;

  printf("current_nn=%d, current_max_sim=%d\n", current_nn, current_max_sim);
  fflush(stdout);

  return current_nn;
}

DllExport int get_1nn_ltrees_class_pred(int o) {
  return (getobjclass(get_nn_ltrees(o)));
}

DllExport float get_1nn_ltrees_regr_pred(int o) {
  return (getobjy(get_nn_ltrees(o)));
}

/* more efficient version of the test (but requires more memory) */

float **matrix_pred;
int matrix_pred_size=0;

DllExport void compute_matrix_pred() {
  int i,j,t;
  int size=current_nb_of_ensemble_terms*(global_learning_set_size+1);

  if (matrix_pred_size<size) {
    if (matrix_pred_size!=0) {
      MyFree((float **)matrix_pred);
      matrix_pred_size=0;
    }
    matrix_pred=(float **)MyMalloc((size_t)size*sizeof(float *));
    if (!matrix_pred) {
      printf("Impossible d'allouer de la memoire dans compute_matrix_pred\n");
      exit(0);
    }
    matrix_pred_size=size;
  }
  
  /* We fill the matrix */
  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    matrix_pred[t]=NULL;
  }
  j=current_nb_of_ensemble_terms;
  for (i=0; i<global_learning_set_size; i++) {
    int o=object_mapping[i];
    for (t=0; t<current_nb_of_ensemble_terms; t++) {
      matrix_pred[j]=get_tree_prediction_vector(ltrees[t], o);
      j++;
    }
  }

  if (print_result) {
    printf("matrix_size=%d\n", matrix_pred_size);
    fflush(stdout);
  }
}

DllExport int get_nn_ltrees_fast(int o) {
  int t,i;
  int current_max_sim=0;
  int current_nn=-1;

  /* we fill the matrix */
  
  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    matrix_pred[t]=get_tree_prediction_vector(ltrees[t], o);
  } 
  
  
  for (i=0; i<global_learning_set_size; i++) {
    int current_sum=0;

    for (t=0; t<current_nb_of_ensemble_terms; t++) {
      if (matrix_pred[t]==matrix_pred[(i+1)*current_nb_of_ensemble_terms+t])
	current_sum++;
    } 
    
    if (current_sum>current_max_sim) {
      current_max_sim=current_sum;
      current_nn=object_mapping[i];
    }
  }
  last_1nn_ltrees_sim=current_max_sim;

  return current_nn;
}

DllExport int get_1nn_ltrees_class_pred_fast(int o) {
  return (getobjclass(get_nn_ltrees_fast(o)));
}

DllExport float get_1nn_ltrees_regr_pred_fast(int o) {
  return (getobjy(get_nn_ltrees_fast(o)));
}

/* fonctions de test */

DllExport float test_1nn_ltrees_clas(int *ts_vector, int length_ts_vector) {
  int i, nb_error=0;

  /* calcul de l'erreur */
  for (i=0; i<length_ts_vector; i++) {
    int class_1nn=get_1nn_ltrees_class_pred_fast(ts_vector[i]);
    if (class_1nn!=getobjclass(ts_vector[i])) {
      nb_error++;
    }
  }
  return (nb_error*100.0/length_ts_vector);
}

DllExport float test_1nn_ltrees_regr(int *ts_vector, int length_ts_vector) {
  int i;
  ERROR_TYPE sum_error=0.0;

  /* calcul de l'erreur */
  for (i=0; i<length_ts_vector; i++) {
    ERROR_TYPE err=getobjy(ts_vector[i])-(get_1nn_ltrees_regr_pred_fast(ts_vector[i]));
    sum_error+=(err*err);
  }
  sum_error=sum_error/length_ts_vector;
  return (float)sum_error;
}

/* autre variante qui fait voter tous les objets */

DllExport int apply_allnn_ltrees_class_pred_fast(int o, float *vect) {
  int t, i, best_clas;
  float tot_vot=0.0;
  float max_prob=-1.0;

  for (i=0; i<nb_classes; i++)
    vect[i]=0.0;

  /* we fill the matrix */
  
  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    matrix_pred[t]=get_tree_prediction_vector(ltrees[t], o);
  }

  for (i=0; i<global_learning_set_size; i++) {
    int current_sum=0;
    
    for (t=0; t<current_nb_of_ensemble_terms; t++) {
      if (matrix_pred[t]==matrix_pred[(i+1)*current_nb_of_ensemble_terms+t])
	current_sum++;
    }
  
    if (current_sum!=0) {
      int c=getobjclass(object_mapping[i]);
      float v=(current_sum*1.0/current_nb_of_ensemble_terms);
      vect[c]+=v;
      tot_vot+=v;
    }
  }

  for (i=0; i<nb_classes; i++) {
    if (vect[i]>max_prob) {
      max_prob=vect[i];
      best_clas=i;
    }
    vect[i]/=tot_vot;
  }

  return best_clas;
}

DllExport float apply_allnn_ltrees_regr_pred_fast(int o) {
  int t, i;
  double tot_vot=0.0;
  double sum=0.0;

  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    //float *pred=get_tree_prediction_vector(ltrees[t], o);
    matrix_pred[t]=get_tree_prediction_vector(ltrees[t], o);
  }

  for (i=0; i<global_learning_set_size; i++) {
    int current_sum=0;
    
    for (t=0; t<current_nb_of_ensemble_terms; t++) {
      if (matrix_pred[t]==matrix_pred[(i+1)*current_nb_of_ensemble_terms+t])
	current_sum++;
    }
    
    if (current_sum!=0) {
      float y=getobjy(object_mapping[i]);
      float v=(current_sum*1.0/current_nb_of_ensemble_terms);
      sum+=y*v;
      tot_vot+=v;
    }
  }
  
  return (sum/tot_vot);
}

/* DEUXIEME MESURE (Eugene Tuv, Pierre Geurts) */
/*---------------------------------------------*/

/* deuxieme variante plus subtile (modif de la presentation de Tuv a nips 2003) */
/* on calcule la profondeur maximale atteinte par les deux objets simulatenement
 * et on divise par le max des profondeurs finales atteintes par les deux objets
 * revient a la premiere si ens_sim_d est tres tres grand
 */

int ens_sim_d=1;

float tree_similarity_2(int tree, int o1, int o2) {
  int current_node=tree;
  int current_node_1, current_node_2;
  int check_1, check_2;
  int p=0;
  int p1, p2;

  while (left_successor[current_node]!=-1 &&
	 ((check_1=check_test(tested_attribute[current_node],
				 getattval(o1,tested_attribute[current_node]), 
				 threshold[current_node]))
	  ==(check_2=check_test(tested_attribute[current_node],
				    getattval(o2,tested_attribute[current_node]), 
				threshold[current_node])))) {
    p++;
    if (check_1)
      current_node+=left_successor[current_node];
    else
      current_node+=right_successor[current_node];
  }
  
  /* Est-on dans une feuille ? */
  if (left_successor[current_node]==-1) {
    /* on s'arrete, les deux objets tombent dans la meme feuille */
    return 1.0;
  }
  
  /* sinon, on propage les deux objets */
  if (check_1)
    current_node_1=current_node+left_successor[current_node];
  else
    current_node_1=current_node+right_successor[current_node];
  p1=p+1;
  while(left_successor[current_node_1]!=-1) {
    if (check_test(tested_attribute[current_node_1],
		   getattval(o1,tested_attribute[current_node_1]), 
		   threshold[current_node_1]))
      current_node_1+=left_successor[current_node_1];
    else
      current_node_1+=right_successor[current_node_1];
    p1++;
  }
  
  if (check_2)
    current_node_2=current_node+left_successor[current_node];
  else
    current_node_2=current_node+right_successor[current_node];
  p2=p+1;
  while(left_successor[current_node_2]!=-1) {
    if (check_test(tested_attribute[current_node_2],
		   getattval(o2,tested_attribute[current_node_2]), 
		   threshold[current_node_2]))
      current_node_2+=left_successor[current_node_2];
    else
      current_node_2+=right_successor[current_node_2];
    p2++;
  }
  
  return pow((p*1.0/(p1<p2? p2: p1)),ens_sim_d);
}


DllExport float ensemble_similarity_2(int o1, int o2) {
  int t;
  float d=0;

  for (t=0; t< current_nb_of_ensemble_terms; t++) {
    d+=tree_similarity_2(ltrees[t],o1,o2);
  }

  return (d/current_nb_of_ensemble_terms);
}

/* pour fixer la valeur de l'exposant */
DllExport void set_ens_sim_d(int d) {
  ens_sim_d=d;
}

/* fonction d'application des modeles */

/* version 1nn */

DllExport int get_nn_ens_sim_2(int o) {
  int i;
  float current_max_sim=0;
  int current_nn=-1;

  for (i=0; i<global_learning_set_size; i++) {
    float sim=ensemble_similarity_2(o, object_mapping[i]);
    if (sim>current_max_sim) {
      current_max_sim=sim;
      current_nn=i;
    }
  }
  return object_mapping[current_nn];
}

DllExport int apply_1nn_ens_sim_2_clas(int o) {
  return (getobjclass(get_nn_ens_sim_2(o)));
}

DllExport float apply_1nn_ens_sim_2_regr(int o) {
  return (getobjy(get_nn_ens_sim_2(o)));
}

/* version moyenne sur tous les objets */

DllExport int apply_allnn_ens_sim2_clas(int o, float *vect) {
  int i, best_clas;
  float tot_vot=0.0;
  float max_prob=-1.0;

  for (i=0; i<nb_classes; i++)
    vect[i]=0.0;

  for (i=0; i<global_learning_set_size; i++) {
    float current_sim=ensemble_similarity_2(o, object_mapping[i]);
    if (current_sim!=0) {
      int c=getobjclass(object_mapping[i]);
      vect[c]+=current_sim;
      tot_vot+=current_sim;
    }
  }

  for (i=0; i<nb_classes; i++) {
    if (vect[i]>max_prob) {
      max_prob=vect[i];
      best_clas=i;
    }
    vect[i]/=tot_vot;
  }

  return best_clas;
}

DllExport float apply_allnn_ens_sim2_regr(int o) {
  int i;
  double tot_vot=0.0;
  double sum=0.0;

  for (i=0; i<global_learning_set_size; i++) {
    float current_sim=ensemble_similarity_2(o, object_mapping[i]);
    if (current_sim!=0) {
      float y=getobjy(object_mapping[i]);
      sum+=(current_sim*y);
      tot_vot+=current_sim;
    }
  }

  return (sum/tot_vot);
}


/* version plus rapide utilisant les node_subset_desc suppose calcule */

/* calcule la profondeur atteinte dans chaque arbre par les objets
 * du LS
 */
int *matrix_depth;
int matrix_depth_size=0;
int object_weight2_size=0;

int get_depth_obj_in_tree(int tree, int o) {
  int current_node=tree;
  int p=0;

  while (left_successor[current_node]!=-1) {
    if (check_test(tested_attribute[current_node],
		   getattval(o,tested_attribute[current_node]), 
		   threshold[current_node]))
      current_node+=left_successor[current_node];
    else
      current_node+=right_successor[current_node];
    p++;
  }
  return p;
}

DllExport void prepare_ens_sim2_fast() {
  int i,j,t;
  int size=current_nb_of_ensemble_terms*(global_learning_set_size);

  /* construction de la matrice matrix_depth */

  if (matrix_depth_size<size) {
    if (matrix_depth_size!=0) {
      MyFree((int *)matrix_depth);
      matrix_depth_size=0;
    }
    matrix_depth=(int *)MyMalloc((size_t)size*sizeof(int));
    if (!matrix_depth) {
      printf("Impossible d'allouer de la memoire dans compute_matrix_depth\n");
      exit(0);
    }
    matrix_depth_size=size;
  }
  
  /* We fill the matrix */
  j=0;
  for (i=0; i<global_learning_set_size; i++) {
    int o=object_mapping[i];
    for (t=0; t<current_nb_of_ensemble_terms; t++) {
      matrix_depth[j]=get_depth_obj_in_tree(ltrees[t], o);
      j++;
    }
  }
  
  if (print_result) {
    printf("matrix_depth_size=%d\n", matrix_depth_size);
    fflush(stdout);
  }

  /* allocation du vecteur object_weight2 */
  if (object_weight2_size<global_learning_set_size) {
    if (object_weight2_size>0)
      MyFree((float *)object_weight2);
    object_weight2=(float *)MyMalloc((size_t)global_learning_set_size*sizeof(float));
    object_weight2_size=global_learning_set_size;
  }

  /* calcule des descripteurs de noeuds */
  compute_node_subset_current_ensemble(object_weight2);

}

/* calcul des poids des différents objets pour un objet donne */
/* resultat dans object_weight */

void get_ens_sim2_weight_from_tree(int t, int o) {
  int i,start,end;
  int current_node=ltrees[t];
  int p=0;

  while(left_successor[current_node]!=-1) {
    if (check_test(tested_attribute[current_node],
		   getattval(o,tested_attribute[current_node]), 
		   threshold[current_node])) {
      /* tous les objets à droite sont à une distance p de o */
      start=node_subset_desc[current_node+right_successor[current_node]][1];
      end=start+node_subset_desc[current_node+right_successor[current_node]][0];
      for (i=start; i<end; i++) {
	object_weight[node_object_list[i]]=p;
      }
      current_node+=left_successor[current_node];
    } else {
      /* tous les objets à gauche sont à une distance p de o */
      start=node_subset_desc[current_node+left_successor[current_node]][1];
      end=start+node_subset_desc[current_node+left_successor[current_node]][0];
      for (i=start; i<end; i++) {
	object_weight[node_object_list[i]]=p;
      }
      current_node+=right_successor[current_node];
    }
    p++;
  }

  /* on termine */
  start=node_subset_desc[current_node][1];
  end=start+node_subset_desc[current_node][0];
  for (i=start; i<end; i++) {
    object_weight[node_object_list[i]]=p;
  }

  /* on normalise */
  for (i=0; i<global_learning_set_size; i++) {
    if (object_weight[i]!=0.0)
      object_weight[i]=pow((object_weight[i]/MAX(p,matrix_depth[i*current_nb_of_ensemble_terms+t])),ens_sim_d);
  }
}

void get_ens_sim2_weight(int o) {
  int i,t;

  /* init */
  for (i=0; i<global_learning_set_size; i++) {
    object_weight2[i]=0.0;
  }

  /* loop on trees */
  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    get_ens_sim2_weight_from_tree(t,o);
    for (i=0; i<global_learning_set_size; i++) {
      object_weight2[i]+=object_weight[i];
    }
  }

  /* normalize */
  for (i=0; i<global_learning_set_size; i++) {
    object_weight2[i]/=current_nb_of_ensemble_terms;
  }
}

/* ne marche que pour le ls */

DllExport float ensemble_similarity_2_fast_ls(int o1, int o2) {
  int i;
  get_ens_sim2_weight(o1);

  for (i=0; i<global_learning_set_size; i++) {
    if (object_mapping[i]==o2)
      break;
  }
  if (i<global_learning_set_size) {
    return object_weight2[i];
  } else
    return -1.0;
}

/* fonction de similarité entre deux objets */

int get_nn_ens_sim_2_fast(int o) {
  int i;
  float max_sim=-1.0;
  int max_pos;
  
  /* calcul object_weight */
  get_ens_sim2_weight(o);
  
  /* recherche le plus proche voisin */
  for (i=0; i<global_learning_set_size; i++) {
    if (object_weight2[i]>max_sim) {
      max_sim=object_weight2[i];
      max_pos=i;
    }
  }
  return object_mapping[max_pos];
}

DllExport int apply_1nn_ens_sim_2_clas_fast(int o) {
  return (getobjclass(get_nn_ens_sim_2_fast(o)));
}

DllExport float apply_1nn_ens_sim_2_regr_fast(int o) {
  return (getobjy(get_nn_ens_sim_2_fast(o)));
}

/* version moyenne sur tous les objets */

DllExport int apply_allnn_ens_sim2_clas_fast(int o, float *vect) {
  int i, best_clas;
  float tot_vot=0.0;
  float max_prob=-1.0;

  for (i=0; i<nb_classes; i++)
    vect[i]=0.0;

  get_ens_sim2_weight(o);

  for (i=0; i<global_learning_set_size; i++) {
    float current_sim=object_weight2[i];
    if (current_sim!=0) {
      int c=getobjclass(object_mapping[i]);
      vect[c]+=current_sim;
      tot_vot+=current_sim;
    }
  }

  for (i=0; i<nb_classes; i++) {
    if (vect[i]>max_prob) {
      max_prob=vect[i];
      best_clas=i;
    }
    vect[i]/=tot_vot;
  }

  return best_clas;
}

DllExport float apply_allnn_ens_sim2_regr_fast(int o) {
  int i;
  double tot_vot=0.0;
  double sum=0.0;

  get_ens_sim2_weight(o);

  for (i=0; i<global_learning_set_size; i++) {
    float current_sim=object_weight2[i];
    if (current_sim!=0) {
      float y=getobjy(object_mapping[i]);
      sum+=(current_sim*y);
      tot_vot+=current_sim;
    }
  }

  return (sum/tot_vot);
}


/* TROISIEME MESURE
 * ----------------
 */
/* vrai kernel qui donne exactement le même prédiction que l'ensemble d'arbre */
/* suppose qu'on a utilisé compute_node_subset_current_ensemble */

DllExport float ensemble_true_similarity(int obj1, int obj2) {
  int t;
  float sim=0.0;

  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    int l1=get_tree_leaf(ltrees[t], obj1);
    int l2=get_tree_leaf(ltrees[t], obj2);
    if (l1==l2) {
      sim+=(1.0/node_subset_desc[l1][0]);
    }
  }
  return (sim/t);
}

/* version avec attributs pour article et présentation */

DllExport float ensemble_true_similarity_att(float x1, float x2, float y1, float y2) {
  core_table[0]=x1;
  core_table[1]=y1;
  core_table[nb_obj_in_core_table]=x2;
  core_table[nb_obj_in_core_table+1]=y2;

  return ensemble_true_similarity(0,1);
}

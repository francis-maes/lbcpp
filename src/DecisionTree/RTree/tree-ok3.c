
/*
 * Author: Pierre Geurts (p.geurts@ulg.ac.be)
 *
 * (Please do not distribute without permission of the author)
 *
 */

/* Output kernel trees implementation */

#define GETDIST_POINTER 1

#ifdef GETDIST_POINTER
float (*getdist_learn)(int index_i, int index_j);
float (*getkernel_learn)(int index_i, int index_j);
#endif

/******************************/
/* ALGORITHME DE CONSTRUCTION */
/******************************/

/* particularise à ce problème certaines fonctions de tree-model.c */

/* paramètre qui contraint le nombre minimale d'objets à gauche et à droite d'un split */
/* uniquement pour la recherche du meilleur seuil */

int min_node_size_split=1;

/* La matrice étant de diagonale nulle et symétrique, on suppose que
 * la matrice ne comprend que la partie triangulaire de la matrice. Ca
 * permet de réduire considérablement la mémoire nécessaire (un
 * facteur 2).
 */

float *distance_matrix;
float *kernel_matrix;

#ifdef GETDIST_POINTER
float getdist_normal(int index_i, int index_j) {
  if (index_i==index_j)
    return 0.0;
  else if (index_j>index_i)
    return distance_matrix[index_i*global_learning_set_size+index_j-(index_i+1)*(index_i+2)/2];
  else
    return distance_matrix[index_j*global_learning_set_size+index_i-(index_j+1)*(index_j+2)/2];
}
#else
float getdist_learn(int index_i, int index_j) {
  if (index_i==index_j)
    return 0.0;
  else if (index_j>index_i)
    return distance_matrix[index_i*global_learning_set_size+index_j-(index_i+1)*(index_i+2)/2];
  else    
    return distance_matrix[index_j*global_learning_set_size+index_i-(index_j+1)*(index_j+2)/2];
}
#endif

void setdist_learn(int index_i, int index_j, float val) {
  if (index_i==index_j)
    return;
  else if (index_j>index_i)
    distance_matrix[index_i*global_learning_set_size+index_j-(index_i+1)*(index_i+2)/2]=val;
  else
    distance_matrix[index_j*global_learning_set_size+index_i-(index_j+1)*(index_j+2)/2]=val;
}

#ifdef GETDIST_POINTER
float getkernel_normal(int index_i, int index_j) {
  if (index_j>=index_i)
    return kernel_matrix[index_i*global_learning_set_size+index_j-index_i*(index_i+1)/2];
  else
    return kernel_matrix[index_j*global_learning_set_size+index_i-index_j*(index_j+1)/2];
}
#else
float getkernel_learn(int index_i, int index_j) {
  if (index_j>=index_i)
    return kernel_matrix[index_i*global_learning_set_size+index_j-index_i*(index_i+1)/2];
  else
    return kernel_matrix[index_j*global_learning_set_size+index_i-index_j*(index_j+1)/2];
}
#endif

void inckernel_learn(int index_i, int index_j, float val) {
  if (index_j>=index_i)
    kernel_matrix[index_i*global_learning_set_size+index_j-index_i*(index_i+1)/2]+=val;
  else
    kernel_matrix[index_j*global_learning_set_size+index_i-index_j*(index_j+1)/2]+=val;
}


/* calcule la distance moyenne entre chaque paire d'objets du LS
   courant */
/* dans le cas de l'apprentissage sur données complexes, cela correspond à la variance sur
   le vecteur de sortie dans le feature space */
/* attention le calcul est quadratique */

void summarize_vector_dist(int *vector, int start, int end) {
  int i,j;
  SCORE_TYPE wi,wj;
  
  table_score[0][0]=0.0;
  table_score[0][1]=0.0;

  for (i=start; i<=end; i++) {
    wi=object_weight[vector[i]];
    table_score[0][0]+=wi;
    for (j=i+1; j<=end; j++) {
      wj=object_weight[vector[j]];
      table_score[0][1]+=wi*wj*getdist_learn(vector[i],vector[j]);
    }
  }

  /*
  printf("summary: start=%d, end=%d, table_score[0][1]=%e, d_tot=%e\n", start, end, table_score[0][1], table_score[0][1]/table_score[0][0], table_score[0][1]/(table_score[0][0]*table_score[0][0]));
  fflush(stdout);
  */

  /* table_score[0][0] contient la somme des poids */
  /* table_score[0][1] contient la somme des distances (ponderees) entre toutes
     les paires d'objets du LS divisée par 2. Cela equivaut exactement à la
     variance dans le feature space correspondant à la distance */
}

SCORE_TYPE d_tot;

SCORE_TYPE compute_distance_score_from_table() {

  /* info contient N*var-N_l*var_l-N_r*var_r (la réduction de variance TOTALE)
   *      avec N*var=N*(1/N^2)*somme des distances/2=1/N*somme des distances/2=tables_score[x][1]/table_score[x][0]
   */

  info=d_tot-table_score[1][1]/table_score[1][0]-table_score[2][1]/table_score[2][0];

  return (info/d_tot); /* le score est compris entre 0 et 1. La
			  normalisation n'est pas nécessaire */
}

/* stop splitting criterio pour la distance */

SCORE_TYPE d_min=0.0;

int stop_splitting_criterio_distance() {
  d_tot=table_score[0][1]/table_score[0][0];

  return ((d_tot/table_score[0][0])<=d_min);
}


int not_significant_test_distance() {
  PRECISION f_stat;

  /* sans doute tout à fait incorrect d'un point de vue statistique */
  if (best_threshold_score>0.0) {
    f_stat=(1.0/(1.0-best_threshold_score))*(table_score[0][0]-2)/(table_score[0][0]-1);

    /* 
       printf("F-stat=%e, F-thres=%e\n", f_stat, f_test_threshold(1,table_score[0][0]-2, alfa_rt));
       fflush(stdout);
    */

    if (print_detailled_result) {
      printf("N=%e, Total variance=%e, Variance reduction=%e, score=%e\n",table_score[0][0], d_tot, best_threshold_score*d_tot, best_threshold_score);
      printf("F-stat=%e, F-thres=%e\n", f_stat, f_test_threshold(1,table_score[0][0]-2, alfa_rt));
      fflush(stdout);
    }
    return (f_stat<f_test_threshold(1,table_score[0][0]-2, alfa_rt));
  } else {
    return 1;
  }
}

SCORE_TYPE best_first_node_priority_function_dist() {
  return (table_score[0][1]/table_score[0][0]); /* Nx"la distance moyenne entre les paires d'objets" */
}

int make_leaf_prediction_distance() { /* on enregistre la distance moyenne mais on ne l'utilisera certainement pas */

  //int i;
  index_prediction_values++;
  
  prediction_values[index_prediction_values][0]=(float)(table_score[0][1]/(table_score[0][0]*table_score[0][0]));
  /* prediction_values[index_prediction_values][1]=(float)(table_score[0][2]); */ /* valeur min */
  /* prediction_values[index_prediction_values][2]=(float)(table_score[0][3]); */ /* valeur max */

  return index_prediction_values;
}

/* recherche le meilleur seuil pour la distance */

/* attribut numérique */

void find_the_best_threshold_dist(int att, int *ls_vector, int start, int end) {
  float old_val, new_val;
  SCORE_TYPE best_score=-1.0, best_info, current_score, w;
  float best_threshold;
  int st=start,i,nextst;

  /* initialisation de la table */
  table_score[1][0]=0.0; /* somme des poids des objets à gauche */
  table_score[1][1]=0.0; /* somme des distances à gauche /2 */

  table_score[2][0]=table_score[0][0]; /* somme des poids à droite */
  table_score[2][1]=table_score[0][1]; /* somme des distances à droite /2 */

  /* on trie l'ensemble selon l'attribut */
  sort_ls_vector(ls_vector, start, end, att);
  /* printlsvectoratt(ls_vector,start,end,att); */

  /* on parcourt toutes les valeurs de seuils possibles */
  /* on recherche le premier objets de poids non nul à partir de start */
  st=start;
  while (object_weight[ls_vector[st]]==0.0)
    st++;
  old_val=getattval(object_mapping[ls_vector[st]],att);
  while (st<end) {
    w=object_weight[ls_vector[st]];
    /* on met à jour la table */
    /* objet st ajouté à gauche */

    table_score[1][0]+=w;
    for (i=start; i<st; i++) {
      table_score[1][1]+=w*object_weight[ls_vector[i]]*getdist_learn(ls_vector[st], ls_vector[i]);
    }

    /* au cas où la distance d'un objet à lui-même ne serait pas zero */
    /* table_score[1][1]+=w*w*getdist_learn(ls_vector[st],ls_vector[st]); */

    /* objet st enlevé à droite */
    table_score[2][0]-=w;
    for (i=st+1; i<=end; i++) {
      table_score[2][1]-=w*object_weight[ls_vector[i]]*getdist_learn(ls_vector[st], ls_vector[i]);
    }

    /* au cas où la distance d'un objet à lui-même ne serait pas zero */
    /* table_score[2][1]-=w*w*getdist_learn(ls_vector[st],ls_vector[st]); */
    
    /* on cherche la valeur de seuil suivante en skippant les objets de poids nuls */
    nextst=st+1;
    while ((nextst<=end) && (object_weight[ls_vector[nextst]]==0.0))
      nextst++;

    if ((object_weight[ls_vector[nextst]]>0.0) && ((new_val=getattval(object_mapping[ls_vector[nextst]],att))!=old_val)) { /* un nouveau seuil a considerer */
      /* printf("old_val=%f, new_val=%f\n",old_val,new_val);fflush(stdout); */
      if (((st-start+1)>=min_node_size_split) && ((end-st)>=min_node_size_split)) {
	current_score=compute_score_from_table();
	if (current_score>best_score) {
	  best_score=current_score;
	  best_info=info;
	  best_threshold=(old_val+new_val)/2.0;
	  if (old_val>=best_threshold) /* problem d'arrondi */
	    best_threshold=new_val;
	}
	old_val=new_val;
      }
    }
    st=nextst;
  }
  if (best_score>=0.0) {
    /* printf("att %d, thres %f, score %f\n", att, best_threshold, best_score); */
    current_threshold.f=best_threshold;
    current_threshold_score=best_score;
    current_threshold_info=best_info;
  } else {
    current_threshold_score=-1.0;
  }
}

/* attribut symbolique */

void summarize_symb_att_dist(int att, int *vector, int start, int end) {
  int i;

  /* set to zero */
  for (i=0; i<NB_VAL(att); i++) {
    table_score_symb[i][0]=0.0;
    table_score_symb[i][1]=0.0;
    table_score_symb[i][2]=0.0;
  }

  /* fill the table with frequency */
  for (i=start; i<=end; i++) {
    int v=(int)getattval(object_mapping[vector[i]],att);
    table_score_symb[v][0]+=object_weight[vector[i]];
  }
}

/* ATTENTION version super inefficace et naive */

void find_the_best_threshold_symb_dist(int att, int *ls_vector, int start, int end) {
  int i, v, eff_v;
  int nb_val=NB_VAL(att);
  int nb_val_ls=0;
  int borne, st;
  union threshold_type current_subset, current_subset_threshold;
  SCORE_TYPE best_score=-1.0, best_info, current_score, w;

  /* on precalcule la table avec tous les frequences pour toutes les valeurs */
  summarize_symb_att_dist(att, ls_vector, start, end);

  /* Check that all elements do not belong to the same class */
  for (i=0; i<nb_val; i++) {
    if (table_score_symb[i][0]!=0) 
      nb_val_ls++;
  }
  if (nb_val_ls==1) { /* all objects have the same value of this attribute */
    current_threshold_score=-1.0;
    return;
  } 

  init_threshold_type(&current_subset);
  add1_threshold_type(&current_subset);

  do {
    /* calcule le vrai subset associé à current_subset (rempli les trous) */
    init_threshold_type(&current_subset_threshold);
    eff_v=0;
    for (v=0; v<nb_val; v++) {
      if (table_score_symb[v][0]!=0.0) {
	if (BITN(current_subset, eff_v)) {
	  SET_BITN(current_subset_threshold,v);
	}
	eff_v++;
      }
    }

    /* separe le LS selon ce subset */
    borne=separate_ls_vector(att, current_subset_threshold, ls_vector, start, end);

    if (((borne-start)>=min_node_size_split) && ((end-borne+1)>=min_node_size_split)) {
    /* fill the table score according to the current subset */
      table_score[1][0]=0.0;
      table_score[1][1]=0.0;

      table_score[2][0]=table_score[0][0];
      table_score[2][1]=table_score[0][1];
      
      st=start;
      do {
	w=object_weight[ls_vector[st]];
	
	/* on met à jour la table */
	/* objet st ajouté à gauche */
	table_score[1][0]+=w;
	for (i=start; i<st; i++) {
	  table_score[1][1]+=w*object_weight[ls_vector[i]]*getdist_learn(ls_vector[st], ls_vector[i]);
	}
	/* objet st enlevé à droite */
	table_score[2][0]-=w;
	for (i=st+1; i<=end; i++) {
	  table_score[2][1]-=w*object_weight[ls_vector[i]]*getdist_learn(ls_vector[st], ls_vector[i]);
	}
	/* on passe au suivant */
	st++;
      } while(st<borne); /* tant que le test est vrai -> tant que st est < borne */
      
      /* compute the score */
      current_score=compute_score_from_table();
      
      if (current_score>best_score) {
	best_score=current_score;
	best_info=info;
	current_threshold=current_subset_threshold;
      }
    }
    add1_threshold_type(&current_subset);
  } while (!(BITN(current_subset,(nb_val_ls-1))));
	 
  if (best_score>=0.0) {
    current_threshold_score=best_score;    
    current_threshold_info=best_info;
  } else {
    current_threshold_score=-1.0;
  }
}

/* version extra-trees pour les distances */

/* attribut numérique */

void find_a_threshold_at_random_dist(int att, int *ls_vector, int start, int end) {
  int i; int nb_obj=end-start+1; int st;
  float min=getattval(object_mapping[ls_vector[start]],att);
  float max=min;
  SCORE_TYPE w;
  
  current_threshold_score=-1.0;

  /* calcule les stats sur l'attribut */
  for (i=start+1; i<=end; i++) {
    float val=getattval(object_mapping[ls_vector[i]],att);
    if (val<min)
      min=val;
    else if (val>max)
      max=val;
  }
  
  if (min==max) { /* toutes les valeurs sont egales */
    return;
  }
  
  /* tirage du seuil (uniformément entre min et max) */
  current_threshold.f=max-(max-min)*get_random_float();
  
  /* calcul du score */
  table_score[1][0]=0.0;
  table_score[1][1]=0.0;

  table_score[2][0]=table_score[0][0];
  table_score[2][1]=table_score[0][1];

  /* ATTENTION Pas très efficace, on trie et on fait comme precedemment. Ca pourrait etre accelerer */
  
  /* un autre moyen d'accelerer est de regarder si borne est plus proche de start ou de end
   * et d'inverser la boucle en fonction de cela (on itererait moins sur st)
   */

  /* on trie l'ensemble selon l'attribut */
  sort_ls_vector(ls_vector, start, end, att);
  
  st=start;
  do {
    w=object_weight[ls_vector[st]];
    
    /* on met à jour la table */
    /* objet st ajouté à gauche */
    table_score[1][0]+=w;
    for (i=start; i<st; i++) {
      table_score[1][1]+=w*object_weight[ls_vector[i]]*getdist_learn(ls_vector[st], ls_vector[i]);
    }
    /* table_score[1][1]+=w*w*getdist_learn(ls_vector[st], ls_vector[st]); */
    /* objet st enlevé à droite */
    table_score[2][0]-=w;
    for (i=st+1; i<=end; i++) {
      table_score[2][1]-=w*object_weight[ls_vector[i]]*getdist_learn(ls_vector[st], ls_vector[i]);
    }
    /* table_score[2][1]-=w*w*getdist_learn(ls_vector[st], ls_vector[st]); */
    /* on passe au suivant */
    st++;
  } while((getattval(object_mapping[ls_vector[st]],att))<current_threshold.f);
  
  current_threshold_score=compute_score_from_table();

}

void find_a_threshold_symb_at_random_dist(int att, int *ls_vector, int start, int end) {
  int i, v, eff_v;
  int nb_val=NB_VAL(att);
  int nb_val_ls=0;
  int borne, st;
  union threshold_type random_subset;
  SCORE_TYPE w;

  /* on precalcule la table avec les frequences pour toutes les valeurs */
  summarize_symb_att_dist(att, ls_vector, start, end);
  
  /* Check that all elements do not belong to the same class */
  for (i=0; i<nb_val; i++) {
    if (table_score_symb[i][0]!=0)
      nb_val_ls++;
  }
  if (nb_val_ls==1) { /* all objects have the same value of this attribute */
    current_threshold_score=-1.0;
    return;
  }

  /* choisit un subset aleatoirement (entre 1 et 2**nb_val_ls-2 inclus) */
  get_random_subset_non_empty_full(&random_subset, nb_val_ls);

  /* on l'insere dans un subset aleatoire sur toutes les valeurs */
  init_threshold_type(&current_threshold);
  eff_v=0;
  for (v=0; v<nb_val; v++) {
    if (table_score_symb[v][0]!=0.0) {
      if (BITN(random_subset, eff_v)) {
	SET_BITN(current_threshold,v);
      }
      eff_v++;
    } else {
      if (get_random_float()<0.5) { /* on flippe aleatoirement le bit */
	SET_BITN(current_threshold,v);
      }
    }
  }

  /* compute the score */

  /* Sort the table according to the test */
  borne=separate_ls_vector(att, current_threshold, ls_vector, start, end);
  /* entre start et borne-1 le test est vrai et entre borne et end, le test est faux */

  /* fill the table score according to the current subset */
  table_score[1][0]=0.0;
  table_score[1][1]=0.0;

  table_score[2][0]=table_score[0][0];
  table_score[2][1]=table_score[0][1];
    
  st=start;
  do {
    w=object_weight[ls_vector[st]];
    
    /* on met à jour la table */
    /* objet st ajouté à gauche */
    table_score[1][0]+=w;
    for (i=start; i<st; i++) {
      table_score[1][1]+=w*object_weight[ls_vector[i]]*getdist_learn(ls_vector[st], ls_vector[i]);
    }
    /* objet st enlevé à droite */
    table_score[2][0]-=w;
    for (i=st+1; i<=end; i++) {
      table_score[2][1]-=w*object_weight[ls_vector[i]]*getdist_learn(ls_vector[st], ls_vector[i]);
    }
    /* on passe au suivant */
    st++;
  } while(st<borne); /* tant que le test est vrai -> tant que st est < borne */

  /* compute the score */
  current_threshold_score=compute_score_from_table();
}

/* COMPUTE_NODE_SUBSET_CURRENT_ENSEMBLE_WEIGHTED */

/* utilise le LS sauvegardé lors de la construction et prend en compte les poids */

/* separe le LS et le object_weight */

int separate_ls_and_weight_vector_local(int best_attribute, union threshold_type best_threshold, int *ls_vector, float *weight_vector, int start, int end) {
  
  while (start!=end) {
    
    while (start!=end && (check_test(best_attribute,
				     getattval(object_mapping[ls_vector[start]],best_attribute), 
				     best_threshold))) {
      start++;
    }
    while (start!=end && !(check_test(best_attribute,
				      getattval(object_mapping[ls_vector[end]],best_attribute), 
				      best_threshold))) {
      end--;
    }
    if (start!=end) { /* on inverse les deux */
      int temp; float temp_f;

      temp=ls_vector[start];
      ls_vector[start]=ls_vector[end];
      ls_vector[end]=temp;

      temp_f=weight_vector[start];
      weight_vector[start]=weight_vector[end];
      weight_vector[end]=temp_f;
      
      start++;
    }
  }
  /* ici, on a start=end, on renvoie la borne */

  if (check_test(best_attribute,getattval(object_mapping[ls_vector[start]],best_attribute), best_threshold))
    return (start+1);
  else
    return start;
}

/* int *node_object_list; defini dans tree-density.c */
float *node_object_weight;
int node_object_allocatedp=0;

/* float *object_weight2; defini dans tree-density.c, permet de renvoyer un vecteur au lisp */
/* (ne sert à rien dans la fonction compute_node_subset_current_ensemble_weighted) */

void compute_node_subset_current_ensemble_weighted(float *ow) {
  int t,i,s,size_ls;
  int pos_nol; /* position in node_object_list */

  object_weight2=ow;

  if (save_ensemble_ls && (save_ensemble_ls_nb_ls>0)) { /* si on a sauvegarde le LS, on le recupere */
                                  /* Notons que ça peut être mieux de reprendre le LS 
				   *  complet à chaque fois (p.e. pour le bagging)
				   */
    if (node_object_allocatedp && (node_object_weight!=NULL))
      free((float *)node_object_weight);
    if (node_object_allocatedp && (node_object_list!=NULL))
      free((int *)node_object_list);
    node_object_allocatedp=0;

    node_object_list=save_ensemble_ls_vector;
    node_object_weight=save_ensemble_ls_weight;

    /* affiche les LS */
    
    /*
    pos_nol=0;
    printf("save_ensemble_ls_pos=%d\n",save_ensemble_ls_pos);
    for (i=0;i<current_nb_of_ensemble_terms; i++) {
      for (j=0; j<save_ensemble_ls_size[i]; j++) {
	printf("%d,o=%d,w=%f ", pos_nol, node_object_list[pos_nol], save_ensemble_ls_weight[pos_nol]);
	pos_nol++;
      }
      printf("\n");
    }
    fflush(stdout);
    */
    
  } else { /* sinon, on cree tout */
    if (node_object_allocatedp && (node_object_weight!=NULL))
      free((float *)node_object_weight);
    if (node_object_allocatedp && (node_object_list!=NULL))
      free((int *)node_object_list);
    s=current_nb_of_ensemble_terms*global_learning_set_size;
    node_object_list=(int *)malloc((size_t)s*sizeof(int));
    node_object_weight=(float *)malloc((size_t)s*sizeof(float));
    node_object_allocatedp=1;

    pos_nol=0;
    for (t=0; t<current_nb_of_ensemble_terms; t++) {
      for (i=0; i<global_learning_set_size; i++) {
	node_object_list[pos_nol]=i;
	node_object_weight[pos_nol]=1.0;
	pos_nol++;
      }
    }
  }

  /*
  printf("pos_nol=%d, global_learning_set_size=%d, save_ensemble_ls=%d\n",pos_nol,global_learning_set_size,save_ensemble_ls);
  fflush(stdout);
  */

  /* alloue la mémoire pour la table node_subset_desc */
  allocate_node_subset_desc(index_nodes+1);

  /* we fill node_object_list with all objects from the ls (index to object_mapping) */
  pos_nol=0;

  for (t=0; t<current_nb_of_ensemble_terms; t++) {
        
    if (save_ensemble_ls && (save_ensemble_ls_nb_ls>0))
      size_ls=save_ensemble_ls_size[t];
    else
      size_ls=global_learning_set_size;

    index_stack_open_nodes=-1;
    index_stack_open_nodes++;
    stack_open_nodes[index_stack_open_nodes][0]=ltrees[t];
    stack_open_nodes[index_stack_open_nodes][1]=pos_nol;
    stack_open_nodes[index_stack_open_nodes][2]=pos_nol+size_ls-1;

    /* on remplit le vector */
    node_subset_desc[ltrees[t]][0]=size_ls;
    node_subset_desc[ltrees[t]][1]=pos_nol;
    
    /* 
    for (i=0; i<global_learning_set_size; i++)
      node_object_list[pos_nol++]=i;
    */
    
    pos_nol+=size_ls;
    
    while(index_stack_open_nodes>=0) {
      int node=stack_open_nodes[index_stack_open_nodes][0];
      int start=stack_open_nodes[index_stack_open_nodes][1];
      int end=stack_open_nodes[index_stack_open_nodes][2];
      int node_size=end-start+1;

      /*
	printf("node=%d,start=%d,end=%d\n",node,start,end);fflush(stdout);
	printlsvector(node_object_list, start, end);
      */
      
      index_stack_open_nodes--;

      if (left_successor[node]!=-1) { /* not a leaf */
	int borne=separate_ls_and_weight_vector_local(tested_attribute[node], threshold[node], node_object_list, node_object_weight, start, end);
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
      } /* else {
	   ici, on pourrait normaliser les poids entre start et end pour qu'il somme à 1
	   cela eviterait de devoir normaliser avant chaque prédiction
	   } */
    }
    /* printf("."); fflush(stdout); */
  }
}

/**************************/
/* PREDICTION DE DISTANCE */
/**************************/

/* fonction pour faire une prediction de distance a partir d'une arbre */
/* on prend en compte les poids calculés par compute_node_subset_current_ensemble_weighted */

float get_tree_distance_prediction(int tree, int obj1, int obj2) {
  int l1=get_tree_leaf(tree, obj1);
  int l2=get_tree_leaf(tree, obj2);
  /* s'ils tombent dans la meme feuille, la distance n'est pas necessairement nulle, on prend
   * aussi la distance moyenne entre les objets. Dans le cas ou il n'y a qu'un objet, la distance
   * est nulle. On pourrait accelerer en maintenant un vecteur avec la prediction pour les feuilles */
  /* if (l1==l2)
    return 0.0;
    else { */
    int i,j;
    int st1=node_subset_desc[l1][1]; int end1=st1+node_subset_desc[l1][0];
    int st2=node_subset_desc[l2][1]; int end2=st2+node_subset_desc[l2][0];
    double sum_dist=0.0, sum_weight=0.0;
    
    for (i=st1; i<end1; i++) {
      int o1=node_object_list[i];
      float w1=node_object_weight[i];
      for (j=st2; j<end2; j++) {
	int o2=node_object_list[j];
	float w2=node_object_weight[j];
	sum_dist+=w1*w2*getdist_learn(o1,o2);
	sum_weight+=w1*w2;
      }
    }
    
    return (float)(sum_dist/sum_weight);
 /*}*/
}

float get_ensemble_distance_prediction(int obj1, int obj2) {
  int t;
  double sum_dist=0.0;

  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    double res=get_tree_distance_prediction(ltrees[t], obj1, obj2);
    sum_dist+=res;
  }

  return (float)(sum_dist/current_nb_of_ensemble_terms);
}

/**************************/
/* INTERFACE AVEC LE LISP */
/**************************/

/* definit un probleme de distance learning */

void init_distance_problem(int *att_vect, int nb_att, float *dm, float *km) {
  goal_type=DISTANCE;
  attribute_vector=att_vect;
  nb_attributes=nb_att;
  distance_matrix=dm;
  kernel_matrix=km;

#ifdef LOAD_DB_HANDLER
  if (core_table_type==SPARSE_CORE_TABLE) {    
    getattval=getattval_sparse;
  } else if (core_table_type==TWO_DIM_CORE_TABLE) {
    getattval=getattval_2dim;
  } else if (core_table_type==NORMAL_CORE_TABLE) {
    getattval=getattval_normal;
  } else {
    printf("wrong type of core_table\n");
    fflush(stdout);
  }
#else 
#ifdef GETATTVAL_POINTER
  getattval=getattval_normal;
#endif
#endif
#ifdef GETDIST_POINTER
  getdist_learn=getdist_normal;
  getkernel_learn=getkernel_normal;
#endif
}

void init_distance_trees(int n_min, int n_min_split, float dmin, float ar) {
  min_node_size=n_min;
  min_node_size_split=n_min_split;
  alfa_rt=ar;
  d_min=dmin;
  summarize_vector=summarize_vector_dist;
  make_leaf_prediction=make_leaf_prediction_distance;
  stop_splitting_criterio=stop_splitting_criterio_distance;
  best_first_node_priority_function=best_first_node_priority_function_dist;
  not_significant_test=not_significant_test_distance;
  compute_score_from_table=compute_distance_score_from_table;
}

void set_find_a_threshold_num_function_dist(int i, int nb_rt) {
  
  nb_of_random_thresholds=nb_rt;
  
  /* pour le moment, un seul choix */
  if (i==0) {
    /* version arbres classiques */
    find_a_threshold_num=find_the_best_threshold_dist;
  } else if (i==3) {
    /* version extra-trees uniform */
    find_a_threshold_num=find_a_threshold_at_random_dist;
  }
}

void set_find_a_threshold_symb_function_dist(int i) {
  if (i==1) {
    find_a_threshold_symb=find_a_threshold_symb_at_random_dist;
  } else {
    find_a_threshold_symb=find_the_best_threshold_symb_dist;
  }
}

/****************************/
/* IMPORTANCE DES VARIABLES */
/****************************/

/* code separé par rapport aux arbres classiques mais les deux pourraient être combinés sans problème */
/* pour le moment, ts_vector contient des index dans object_mapping (de 0 à N) et les distances 
 * sont obtenues de distance_matrix 
 */

SCORE_TYPE get_vi_dist(int *ts_vector, int start, int end, int borne) {
  int i,j, st;
  SCORE_TYPE current_score;

  /* summarize_vector */
  for (i=0; i<2; i++) {
    table_score[0][i]=0.0;
    table_score[1][i]=0.0;
  }

  for (i=start; i<=end; i++) {
    table_score[0][0]++;
    for (j=i+1; j<=end; j++) {
      table_score[0][1]+=getdist_learn(ts_vector[i],ts_vector[j]);
    }
  }
  
  /* calcule d_tot en fait */
  stop_splitting_criterio_distance();
  
  if ((start>=borne)||(borne>end)) {
    return 0.0;
  }

  /* fill the table_score, il y a sûrement une manière plus efficace que de le faire
   * objet par objet */
  table_score[2][0]=table_score[0][0];
  table_score[2][1]=table_score[0][1];

  for (st=start; st<borne; st++) {

    table_score[1][0]++;
    for (i=start; i<st; i++) {
      table_score[1][1]+=getdist_learn(ts_vector[st], ts_vector[i]);
    }
    /* table_score[1][1]+=getdist_learn(ts_vector[st], ts_vector[st]); */
    
    table_score[2][0]--;
    for (i=st+1; i<=end; i++) {
      table_score[2][1]-=getdist_learn(ts_vector[st], ts_vector[i]);
    }
    /* table_score[2][1]-=getdist_learn(ts_vector[st], ts_vector[st]); */
  }

  /* compute the score */
  current_score=compute_distance_score_from_table();
  return (info);
}

SCORE_TYPE compute_one_tree_variable_importance_dist(int tree, int *ts_vector, int length_ts_vector, float weight,
					  SCORE_TYPE *attribute_importance, int obj) {
  SCORE_TYPE total_var=-1.0;
  
  index_stack_open_nodes++;
  stack_open_nodes[index_stack_open_nodes][0]=tree;
  stack_open_nodes[index_stack_open_nodes][1]=0;
  stack_open_nodes[index_stack_open_nodes][2]=length_ts_vector-1;
  
  while (index_stack_open_nodes>=0) {
    int node=stack_open_nodes[index_stack_open_nodes][0];
    int start=stack_open_nodes[index_stack_open_nodes][1];
    int end=stack_open_nodes[index_stack_open_nodes][2];
    int node_size=end-start+1;
    float vi;

    if ((left_successor[node]==-1)||(node_size==1)) {
      index_stack_open_nodes--;
      /*
	printf("term start=%d, end=%d, tested attribute=%d isop=%d\n", start, end, tested_attribute[node], index_stack_open_nodes);
	fflush(stdout);
      */
    } else {
      /* separation */
      int borne=separate_ls_vector_local(tested_attribute[node], threshold[node], ts_vector, start, end);

      /*
	printf("current_node=%d, left=%d, right=%d\n",node, left_successor[node], right_successor[node]);
	printf("start=%d, borne=%d, end=%d, tested attribute=%d isop=%d\n", start, borne, end, tested_attribute[node], index_stack_open_nodes);
	fflush(stdout);
      */

      /* calcul de l'importance (seulement si borne OK */
      vi=get_vi_dist(ts_vector, start, end, borne);
      
      /*
	printf("start=%d borne=%d end=%d vi=%f\n",start,borne,end, vi);
	fflush(stdout);
      */
      
      if (node==tree) {
	  total_var=d_tot;
      }
	/* mis a jour du vecteur */
      attribute_importance[attribute_position[tested_attribute[node]]]+=(weight*vi);

      /* left and right successors are put on the stack */
      index_stack_open_nodes--;
      if (obj<0) {
	if (start<borne) {
	  index_stack_open_nodes++;
	  stack_open_nodes[index_stack_open_nodes][0]=node+left_successor[node];
	  stack_open_nodes[index_stack_open_nodes][1]=start;
	  stack_open_nodes[index_stack_open_nodes][2]=borne-1;
	}
	if (borne<=end) {
	  index_stack_open_nodes++;
	  stack_open_nodes[index_stack_open_nodes][0]=node+right_successor[node];
	  stack_open_nodes[index_stack_open_nodes][1]=borne;
	  stack_open_nodes[index_stack_open_nodes][2]=end;
	}
      } else {
	if (check_test(tested_attribute[node],getattval(obj,tested_attribute[node]),threshold[node])) {
	  /* on met le gauche */
	  index_stack_open_nodes++;
	  stack_open_nodes[index_stack_open_nodes][0]=node+left_successor[node];
	  stack_open_nodes[index_stack_open_nodes][1]=start;
	  stack_open_nodes[index_stack_open_nodes][2]=borne-1;
	} else {
	  /* on met le droit */
	  index_stack_open_nodes++;
	  stack_open_nodes[index_stack_open_nodes][0]=node+right_successor[node];
	  stack_open_nodes[index_stack_open_nodes][1]=borne;
	  stack_open_nodes[index_stack_open_nodes][2]=end;
	}
      }
    }
  }
  return total_var;
}

void compute_ltrees_variable_importance_dist(SCORE_TYPE *attribute_importance, int obj) {
  /* si obj est à -1 -> calcul classique
   * sinon, on ne parcourt que la branche de l'arbre par laquelle l'objet passe 
   * utile surtout pour un ensemble d'arbre. Permet de retrouver une sorte de branche
   * de tests (floues ou prototype) correspondant à cet objet.
   */

  int i,t;
  SCORE_TYPE sum_val=0.0;
  SCORE_TYPE sum_val2=0.0;
  SCORE_TYPE total_var_one_tree;
  int *ts_vector=current_learning_set;
  int length_ts_vector=current_learning_set_size;
  
  /* remplit le vecteur */
  for (i=0; i<length_ts_vector; i++)  
    ts_vector[i]=i;

  for (i=0; i<nb_attributes; i++) {
    attribute_importance[i]=0.0;
    attribute_position[attribute_vector[i]]=i;
  }

  /* boucle sur les arbres */
  if ((current_nb_of_ensemble_terms==0) && (index_nodes>=0)) {
    /* il y a un arbre mais pas d'ensemble. On calcule l'importance de cet arbre uniquement */
    if (missing_values) {
      replace_missing_values();
    }
    total_var_one_tree=
      compute_one_tree_variable_importance_dist(0, ts_vector, length_ts_vector, 1.0, attribute_importance,obj);
    sum_val2+=total_var_one_tree;
  
  } else {
    for (t=0; t<current_nb_of_ensemble_terms; t++) {
      if (missing_values) {
	replace_missing_values();      
      }

      total_var_one_tree=
	compute_one_tree_variable_importance_dist(ltrees[t], ts_vector, length_ts_vector, 
						  ltrees_weight[t], attribute_importance,obj);
      if (total_var_one_tree!=-1) {
	/* toutes les valeurs sont les memes */
	sum_val2=total_var_one_tree;
      }
      /* 
	 printf("%f\n", total_var_one_tree);
	 fflush(stdout);
      */
    }
  }

  /* normalizing the values */
  sum_val=0.0;
  for (i=0; i<nb_attributes; i++) {
    sum_val+=attribute_importance[i];
  }
  
  if (sum_val!=0.0) { /* arbre ne se reduit pas a la racine */
    for (i=0; i<nb_attributes; i++) {
      attribute_importance[i]/=sum_val;
    }
  }
  if (print_result) {
    printf("Total impurity(N.I(S))=%d*%f=%f\nAverage Impurity explained by trees=%f\n", 
	   length_ts_vector, 
	   sum_val2/length_ts_vector, sum_val2, 
	   sum_val);
    fflush(stdout);
  }
}

/******************/
/* MODE SUPERVISE */
/******************/

/* CALCUL DE DISTANCE A PARTIR D'UN (ENSEMBLE D') ARBRE CONSTRUIT EN SUPERVISE */

/* fonction pour faire une prediction de distance a partir d'une arbre de décision */

/* classification: on se sert du vecteur de prediction */

float get_clas_tree_distance_prediction(int tree, int obj1, int obj2) {
  int l1=get_tree_leaf(tree, obj1);
  int l2=get_tree_leaf(tree, obj2);
  
  if (l1==l2) {
    return 0.0;
  } else {
    double sum_dist=0.0;
    float *pred1=prediction_values[prediction[l1]];
    float *pred2=prediction_values[prediction[l2]];
    int i;

    for (i=0; i<nb_classes; i++) {
    /* printf("%f %f\n",pred1[i],pred2[i]); 
       fflush(stdout);
     */
      sum_dist+=(pred1[i]*(1.0-pred2[i]));
    }

  /* return (float)(sum_dist/(node_subset_desc[l1][0]*node_subset_desc[l2][0]));*/
    return (float) sum_dist;
  }
}

float get_clas_ensemble_distance_prediction(int obj1, int obj2) {
  int t;
  double sum_dist=0.0;

  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    double res=get_clas_tree_distance_prediction(ltrees[t], obj1, obj2);
    sum_dist+=res;
  }

  return (float)(sum_dist/current_nb_of_ensemble_terms);
}

/* regression: */

float get_regr_tree_distance_prediction(int tree, int obj1, int obj2) {
  int l1=get_tree_leaf(tree, obj1);
  int l2=get_tree_leaf(tree, obj2);
  int i,j;

  /*
  printf("obj1=%d, l1=%d, obj2=%d, l2=%d\n",obj1, l1, obj2, l2);
  fflush(stdout);
  */

  if (l1==l2) {
    /* la distance moyenne dans un ensemble vaut 2 fois la variance */
    /* return (2.0*prediction_values[prediction[l2]][1]); */
    return 0.0;
  } else {
    int st1=node_subset_desc[l1][1]; int end1=st1+node_subset_desc[l1][0];
    int st2=node_subset_desc[l2][1]; int end2=st2+node_subset_desc[l2][0];

    double sum_dist=0.0;
    double sum_weight=0.0;
    
    for (i=st1; i<end1; i++) {
      int o1=object_mapping[node_object_list[i]];
      float w1=node_object_weight[i];
      for (j=st2; j<end2; j++) {
	int o2=object_mapping[node_object_list[j]];
	float w2=node_object_weight[j];
	float diff=(getobjy(o1)-getobjy(o2));
	/*
	printf("o1=%d, o2=%d, diff=%f\n", o1, o2, diff);
	fflush(stdout);
	*/
	sum_dist+=w1*w2*(diff*diff);
	sum_weight+=w1*w2;
      }
    }
    
    return (float)(sum_dist/sum_weight);
  }
}

float get_regr_ensemble_distance_prediction(int obj1, int obj2) {
  int t;
  double sum_dist=0.0;

  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    double res=get_regr_tree_distance_prediction(ltrees[t], obj1, obj2);
    sum_dist+=res;
  }

  return (float)(sum_dist/current_nb_of_ensemble_terms);
}

/* Distance simplifiée proposée par Breiman (le nombre de fois que les deux objets tombent dans la même feuille 
 * (valide en classification et en regression)
 */

float get_ensemble_simple_distance_prediction(int obj1, int obj2) {
  int t;
  int nb_diff=0;
  
  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    int l1=get_tree_leaf(ltrees[t], obj1);
    int l2=get_tree_leaf(ltrees[t], obj2);
    if (l1!=l2)
      nb_diff++;
  }

  return (float)(nb_diff*1.0/current_nb_of_ensemble_terms);  
}

/* recherche les k NN a partir d'un ensemble d'arbres */

float (*get_distance_function)(int o1, int o2)=get_clas_ensemble_distance_prediction;

void get_k_nn_dist(int o, int k, int *lobj, float *ldist, int *ref_set, int ref_set_size) {
  int i,j=0, pos;
  float current_max_distance=PLUS_INFINI;

  for (i=0; i<k; i++) {
    ldist[i]=PLUS_INFINI;
  }

  for (i=0; i<ref_set_size; i++) {
    int o2=ref_set[i];
    /* on exclut l'objet lui-meme (pour faire du loo) */
    if (o2!=o) {
      float current_dist=get_distance_function(o,o2);
      if (current_dist<current_max_distance) {
	/* insert l'objet dans la liste */
	pos=k-1;
	while ((pos-1>=0) && (ldist[pos-1]>current_dist)) {
	  ldist[pos]=ldist[pos-1];
	  lobj[pos]=lobj[pos-1];
	  pos--;
	}
	ldist[pos]=current_dist;
	lobj[pos]=o2;
	current_max_distance=ldist[k-1];
      }
    }
  }
}

/* effectue un test systématique avec le knn et la mesure apprise */

float test_knn_dist_clas(int *ts_vector, int length_ts_vector, int max_k, double *lerrors, int *ref_set, int ref_set_size) {
  int i, j, nb_error=0;
  int *lobj;
  float *ldist;
  
  lobj=(int *)malloc((size_t)max_k*sizeof(int));
  ldist=(float *)malloc((size_t)max_k*sizeof(float));

  for (i=0; i<max_k; i++) {
    lerrors[i]=0.0;
  }

  get_distance_function=get_clas_ensemble_distance_prediction;

  for (i=0; i<length_ts_vector; i++) {
    int obj_class=getobjclass(ts_vector[i]);
    int pred_class;

    for (j=0;j<nb_classes; j++) {
      table_score[0][j]=0.0;
    }

    get_k_nn_dist(ts_vector[i], max_k, lobj, ldist, ref_set, ref_set_size);

    if (print_result) {
      printf(".");
      fflush(stdout);
    }
    for (j=0; j<max_k; j++) {
      table_score[0][getobjclass(lobj[j])]++;
      pred_class=my_best_class_double(table_score[0], nb_classes);
      if (pred_class!=obj_class) {
	lerrors[j]++;
      } 
    }
  }
  
  if (print_result) {
    printf("\n");
    fflush(stdout);
  }

  for (i=0; i<max_k; i++) {
    lerrors[i]=lerrors[i]/length_ts_vector;
  }

  free((int *)lobj);
  free((float *)ldist);
  return 0.f;
}

float test_knn_dist_regr(int *ts_vector, int length_ts_vector, int max_k, double *lerrors, int *ref_set, int ref_set_size) {
  int i, j, nb_error=0;
  int *lobj;
  float *ldist;
  
  lobj=(int *)malloc((size_t)max_k*sizeof(int));
  ldist=(float *)malloc((size_t)max_k*sizeof(float));

  for (i=0; i<max_k; i++) {
    lerrors[i]=0.0;
  }

  get_distance_function=get_regr_ensemble_distance_prediction;

  for (i=0; i<length_ts_vector; i++) {
    float y=getobjy(ts_vector[i]);
    ERROR_TYPE pred_regr=0.0;
    ERROR_TYPE err;
    
    get_k_nn_dist(ts_vector[i], max_k, lobj, ldist, ref_set, ref_set_size);

    /*
    printf("\nO=%d -K-NN\n",ts_vector[i]);
    for (j=0; j<max_k; j++) {
      printf("O2=%d, d=%f\n",lobj[j],ldist[j]);
    }
    fflush(stdout);
    */

    if (print_result) {
      printf(".");
      fflush(stdout);
    }

    for (j=0; j<max_k; j++) {
      
      pred_regr+=getobjy(lobj[j]);
      err=y-(pred_regr/(j+1));
      lerrors[j]+=(err*err);
    }
  }
  
  if (print_result) {
    printf("\n");
    fflush(stdout);
  }

  for (i=0; i<max_k; i++) {
    lerrors[i]=lerrors[i]/length_ts_vector;
  }

  free((int *)lobj);
  free((float *)ldist);
  return 0.f;
}

/* idem avec la distance simple proposée par breiman (nb de vote commun) */

float test_knn_simple_dist_clas(int *ts_vector, int length_ts_vector, int max_k, double *lerrors, int *ref_set, int ref_set_size) {
  int i, j, nb_error=0;
  int *lobj;
  float *ldist;
  
  lobj=(int *)malloc((size_t)max_k*sizeof(int));
  ldist=(float *)malloc((size_t)max_k*sizeof(float));

  for (i=0; i<max_k; i++) {
    lerrors[i]=0.0;
  }

  get_distance_function=get_ensemble_simple_distance_prediction;

  for (i=0; i<length_ts_vector; i++) {
    int obj_class=getobjclass(ts_vector[i]);
    int pred_class;

    for (j=0;j<nb_classes; j++) {
      table_score[0][j]=0.0;
    }

    get_k_nn_dist(ts_vector[i], max_k, lobj, ldist, ref_set, ref_set_size);

    if (print_result) {
      printf(".");
      fflush(stdout);
    }
    for (j=0; j<max_k; j++) {
      table_score[0][getobjclass(lobj[j])]++;
      pred_class=my_best_class_double(table_score[0], nb_classes);
      if (pred_class!=obj_class) {
	lerrors[j]++;
      } 
    }
  }
  
  if (print_result) {
    printf("\n");
    fflush(stdout);
  }

  for (i=0; i<max_k; i++) {
    lerrors[i]=lerrors[i]/length_ts_vector;
  }

  free((int *)lobj);
  free((float *)ldist);
  return 0.f;
}

float test_knn_simple_dist_regr(int *ts_vector, int length_ts_vector, int max_k, double *lerrors, int *ref_set, int ref_set_size) {
  int i, j, nb_error=0;
  int *lobj;
  float *ldist;
  
  lobj=(int *)malloc((size_t)max_k*sizeof(int));
  ldist=(float *)malloc((size_t)max_k*sizeof(float));

  for (i=0; i<max_k; i++) {
    lerrors[i]=0.0;
  }

  get_distance_function=get_ensemble_simple_distance_prediction;

  for (i=0; i<length_ts_vector; i++) {
    float y=getobjy(ts_vector[i]);
    ERROR_TYPE pred_regr=0.0;
    ERROR_TYPE err;
    
    get_k_nn_dist(ts_vector[i], max_k, lobj, ldist, ref_set, ref_set_size);

    if (print_result) {
      printf(".");
      fflush(stdout);
    }

    for (j=0; j<max_k; j++) {
      
      pred_regr+=getobjy(lobj[j]);
      err=y-(pred_regr/(j+1));
      lerrors[j]+=(err*err);
    }
  }
  
  if (print_result) {
    printf("\n");
    fflush(stdout);
  }

  for (i=0; i<max_k; i++) {
    lerrors[i]=lerrors[i]/length_ts_vector;
  }

  free((int *)lobj);
  free((float *)ldist);
  return 0.f;
}

/************************************/
/* APPRENTISSAGE DE SORTIE COMPLEXE */
/************************************/

/* définit des fonctions particulières pour l'apprentissage sur des données complexes */

/* calcul de l'erreur quadratique sur un ensemble de test */

/* retrouve l'ensemble de voisins sur un ensemble d'arbres */
/* permet de calculer du côté lisp, l'erreur quadratique sur la prédiction du point,
   la prédiction la plus proche du centre de masse, médiane, etc. */

void get_tree_object_weight(int tree, int obj) {
  int l=get_tree_leaf(tree, obj);
  int i;
  float sum_weight=0.0;
  
  int st=node_subset_desc[l][1]; int end=st+node_subset_desc[l][0];

  for (i=st; i<end; i++) {
    float w=node_object_weight[i];
    object_weight[node_object_list[i]]+=w;
    sum_weight+=w;
  }
  /* on normalise */
  for (i=st; i<end; i++) {
    object_weight[node_object_list[i]]/=sum_weight;
  }
}

void get_ens_object_weight(int obj) {
  int i, t;
  float sum_tree_weight=0.0;
  /* init */
  for (i=0; i<global_learning_set_size; i++) {
    object_weight2[i]=0.0;
    object_weight[i]=0.0;
  }

  /* loop on trees */
  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    float w=ltrees_weight[t];
    sum_tree_weight+=w;
    get_tree_object_weight(ltrees[t],obj);
    for (i=0; i<global_learning_set_size; i++) {
      object_weight2[i]+=w*object_weight[i];
      object_weight[i]=0.0;
    }
  }

  /* normalize si nécessaire */
  if (average_predictions_ltrees) {
    for (i=0; i<global_learning_set_size; i++) {
      object_weight2[i]/=sum_tree_weight;
    }
  }
}

void get_tree_node_object_weight(int node) {
  int i;
  float sum_weight=0.0;
  
  int st=node_subset_desc[node][1]; int end=st+node_subset_desc[node][0];
  
  for (i=0; i<global_learning_set_size; i++) {
    object_weight2[i]=0.0;
  }
  
  for (i=st; i<end; i++) {
    float w=node_object_weight[i];
    object_weight2[node_object_list[i]]+=w;
    sum_weight+=w;
  }
  /* on normalise */
  for (i=st; i<end; i++) {
    object_weight2[node_object_list[i]]/=sum_weight;
  }
}

/* fait une prédiction du kernel entre deux objets */

int *o1_obj;
float *o1_w;
int *o2_obj;
float *o2_w;

double get_ensemble_kernel_prediction(int o1, int o2) {
  int i,j,n1,n2;
  double sum;
  
  /*  alloue la mémoire */
  o1_obj=(int *)malloc((size_t)global_learning_set_size*sizeof(int));
  o2_obj=(int *)malloc((size_t)global_learning_set_size*sizeof(int));
  o1_w=(float *)malloc((size_t)global_learning_set_size*sizeof(float));
  o2_w=(float *)malloc((size_t)global_learning_set_size*sizeof(float));

  if ((o1_obj==NULL) || (o2_obj==NULL) || (o1_w==NULL) || (o2_w==NULL)) {
    printf("Allocation failed in get_ensemble_kernel_prediction_norm\n");
    fflush(stdout);
    exit(0);
  }
  
  /* on retrouve les poids */
  get_ens_object_weight(o1);
  n1=0;
  for (i=0;i<global_learning_set_size; i++) {
    if (object_weight2[i]!=0.0) {
      o1_w[n1]=object_weight2[i];
      o1_obj[n1]=i;
      n1++;
    }
  }
  get_ens_object_weight(o2);
  n2=0;
  for (i=0;i<global_learning_set_size; i++) {
    if (object_weight2[i]!=0.0) {
      o2_w[n2]=object_weight2[i];
      o2_obj[n2]=i;
      n2++;
    }
  }

  /* on calcule le kernel sur base des objets */
  sum=0.0;
  for (i=0;i<n1;i++) {
    for (j=0;j<n2;j++) {
      sum+=o1_w[i]*o2_w[j]*getkernel_learn(o1_obj[i],o2_obj[j]);
    }
  }

  free((int *)o1_obj);
  free((float *)o1_w);
  free((int *)o2_obj);
  free((float *)o2_w);
  
  return sum;
}

double get_ensemble_kernel_prediction_norm(int o1, int o2) {
  int i,j,n1,n2;
  double sum, norm1, norm2;
  
  /*  alloue la mémoire */
  o1_obj=(int *)malloc((size_t)global_learning_set_size*sizeof(int));
  o2_obj=(int *)malloc((size_t)global_learning_set_size*sizeof(int));
  o1_w=(float *)malloc((size_t)global_learning_set_size*sizeof(float));
  o2_w=(float *)malloc((size_t)global_learning_set_size*sizeof(float));
  
  if ((o1_obj==NULL) || (o2_obj==NULL) || (o1_w==NULL) || (o2_w==NULL)) {
    printf("Allocation failed in get_ensemble_kernel_prediction_norm\n");
    fflush(stdout);
    exit(0);
  }

  /* on retrouve les poids */
  get_ens_object_weight(o1);
  n1=0;
  for (i=0;i<global_learning_set_size; i++) {
    if (object_weight2[i]!=0.0) {
      o1_w[n1]=object_weight2[i];
      o1_obj[n1]=i;
      n1++;
    }
  }
  get_ens_object_weight(o2);
  n2=0;
  for (i=0;i<global_learning_set_size; i++) {
    if (object_weight2[i]!=0.0) {
      o2_w[n2]=object_weight2[i];
      o2_obj[n2]=i;
      n2++;
    }
  }

  /* on calcule le kernel sur base des objets */
  sum=0.0;
  for (i=0;i<n1;i++) {
    for (j=0;j<n2;j++) {
      sum+=o1_w[i]*o2_w[j]*getkernel_learn(o1_obj[i],o2_obj[j]);
    }
  }
  
  /* on calcule les normes */
  norm1=0.0;
  for (i=0;i<n1;i++) {
    norm1+=o1_w[i]*o1_w[i]*getkernel_learn(o1_obj[i],o1_obj[i]);
    for (j=i+1;j<n1;j++) {
      norm1+=2.0*o1_w[i]*o1_w[j]*getkernel_learn(o1_obj[i],o1_obj[j]);
    }
  }
  norm2=0.0;
  for (i=0;i<n2;i++) {
    norm2+=o2_w[i]*o2_w[i]*getkernel_learn(o2_obj[i],o2_obj[i]);
    for (j=i+1;j<n2;j++) {
      norm2+=2.0*o2_w[i]*o2_w[j]*getkernel_learn(o2_obj[i],o2_obj[j]);
    }
  }
    
  free((int *)o1_obj);
  free((float *)o1_w);
  free((int *)o2_obj);
  free((float *)o2_w);
  
  return (sum/sqrt(norm1*norm2));
}

/******************
 * ELAGAGE PAR CV *
 ******************/

/* #include "tree-ok3-pruning.c" */

/****************/
/* OK3 BOOSTING */
/****************/

/* version kernelisée de l'algorithme MART de Friedman */

/* Mai 2006 */
/* squared error (residual fitting) */
/* (version kernelisee) */
/* programmation pas très optimisée */

/* Note pour l'implémentation:
 * - on doit a tout prix: conserver la matrice kernel originale
 *   pour eviter d'avoir à la recalculer lors du test
 * - on suppose également que le matrice kernel est pré-calculée et passée au lisp
 * - pour le calcul, on met la matrice kernel dans la matrice de distance
 *   qu'on utilise comme matrice temporaire. A la fin, on en déduit la nouvelle matrice
 *   de distance (mais alors on perd la matrice kernel courante -> je pense qu'il faut 
 *   trois matrices de noyau
 * - On a besoin d'une matrice temporaire (complète) pour les calculs
 *   intermédiare
 * - Il faut egalement calculer les sous-ensembles correspondants à chaque feuille
 * - l'idéal serait d'avoir un vecteur [0 0 0 1 1 1 2 2 2 3 3 3 4 4 4]
 *  avec pour chaque objet le numéro de sa feuille (dans l'ordre des feuilles)
 *  (on parcourt ensuite ce vecteur en mettant à jour la matrice K^m K^{t_m } lignes par lignes)
 * - Pour calculer ce vecteur, on peut soit le recalculer à la fin de la construction, soit, le calculer
 *   dynamiquement pendant la construction (en définissant une nouvelle fonction 
 */

/* différentes matrices à initialiser lors du test */

float *mart_temp_matrix;
int size_mart_temp_matrix;
float *mart_temp_matrix2;
int size_mart_temp_matrix2;

int *ok3mart_index_leaf;
int ok3mart_index_leaf_size=0;
int ok3mart_index_leaf_current_shift=0;

/* decide si on supprime les objets les plus mal prédits */
int outliers_trimming=0;
float outliers_trimming_perc=0.1;

void set_mart_outliers_trimming_parameters(int b, float p) {
  outliers_trimming=b;
  outliers_trimming_perc=p;
}

/* On redefinit la fonction pour qu'elle enregistre pour chaque objet le numéro de la feuille qui lui correspond
 * On devrait certainement conserver cette information plus globalement pour le test futur de l'arbre. Ok, faisons le
 * directement.
 */

int make_leaf_prediction_distance_mart() {
  
  int i,node,start,end;
  index_prediction_values++;
  
  prediction_values[index_prediction_values][0]=(float)(table_score[0][1]/(table_score[0][0]*table_score[0][0]));
  prediction_values[index_prediction_values][1]=(float)(table_score[0][0]); /* taille de la feuille */
  /* prediction_values[index_prediction_values][2]=(float)(table_score[0][3]); */ /* valeur max */
  
  node=stack_open_nodes[index_stack_open_nodes][0];
  start=stack_open_nodes[index_stack_open_nodes][1];
  end=stack_open_nodes[index_stack_open_nodes][2];
  
  /* printf("%d %d %d",node,start,end);fflush(stdout); */
  
  /* on enregistre l'index de la feuille correspondante */
  for (i=start; i<=end; i++) {
    ok3mart_index_leaf[ok3mart_index_leaf_current_shift+i]=node;
  }
  
  return index_prediction_values;
}

float make_ls_vector_mart_distance(int tree) {
  int i,j, size_km;

  if (tree<0) { /* INITIALISATION (M=0) */
    for (i=0; i<global_learning_set_size; i++)
      /* on initialise le poids des objets */
      if (weight_attributep) {
	object_weight[i]=getattval(object_mapping[i],weight_attribute);
      } else
	object_weight[i]=1.0;

    /* on initialise les deux matrices temporaires */

    /* on alloue le vecteur ok3mart_index_leaf (qui servira également pour le test) */
    size_km=number_of_ensemble_terms*global_learning_set_size;
    if (ok3mart_index_leaf_size<size_km) {
      if (ok3mart_index_leaf_size!=0)
	free((int *)ok3mart_index_leaf);
      ok3mart_index_leaf=(int *)malloc((size_t)size_km*sizeof(int));
      if (!ok3mart_index_leaf) {
	printf("impossible d'allouer de la memoire dans make_ls_vector_mart_distance\n");
	fflush(stdout);
	exit(0);
      }
      ok3mart_index_leaf_size=size_km;
    }
    ok3mart_index_leaf_current_shift=0;

    /* une matrice NxN pour les calculs intermédiares */
    size_km=global_learning_set_size*global_learning_set_size;
    if (size_mart_temp_matrix<size_km) {
      if (size_mart_temp_matrix!=0)
	free((float *)mart_temp_matrix);
      mart_temp_matrix=(float *)malloc((size_t)size_km*sizeof(float));
      if (!mart_temp_matrix) {
	printf("impossible d'allouer de la memoire dans make_ls_vector_mart_distance\n");
	fflush(stdout);
	exit(0);
      }
      size_mart_temp_matrix=size_km;
    }

    /* une matrice NxN symetrique pour le kernel courant */
    size_km=global_learning_set_size*(global_learning_set_size+1)/2;
    if (size_mart_temp_matrix2<size_km) {
      if (size_mart_temp_matrix2!=0)
	free((float *)mart_temp_matrix2);
      mart_temp_matrix2=(float *)malloc((size_t)size_km*sizeof(float));
      if (!mart_temp_matrix2) {
	printf("impossible d'allouer de la memoire dans make_ls_vector_mart_distance\n");
	fflush(stdout);
	exit(0);
      }
      size_mart_temp_matrix2=size_km;
    }
    /* on la remplit à partir du kernel initial */
    for (i=0; i<size_km; i++)
      mart_temp_matrix2[i]=kernel_matrix[i];
    /* on l'associe à la matrice noyau pour la construction des arbres */
    /* (la matrice restera accessible du lisp mais plus du c) */
    kernel_matrix=mart_temp_matrix2;

    /* pour que le premier modele ait une complexite de 1 (pas de test) */
    temp_best_first_max_nb_tests=best_first_max_nb_tests;
    best_first_max_nb_tests=0;
    
    if (print_result) {
      printf("fin premiere mise a jour (size ok3=%d,%d)\n",ok3mart_index_leaf_size, ok3mart_index_leaf_current_shift);
      fflush(stdout);
    }

    if (print_detailled_result) {
      printf("t=0\n");
      for (i=0; i<global_learning_set_size; i++) {
	printf("%d",i);
	for (j=0; j<global_learning_set_size; j++) {
	  printf(" %f",getkernel_learn(i,j));
	}
	printf("\n");
      }
      fflush(stdout);
    }

  } else if (current_nb_of_ensemble_terms<number_of_ensemble_terms-1) {  /* RESIDUAL COMPUTATION (M>0) */
    /* on pourrait peut-être se passer de la matrice temporaire */
    int c_leaf, c_pos, p_pos, posfl, c_leaf2, c_pos2, p_pos2; float new_mart_mu=mart_mu;
    float ns, kval;
    
    if (print_result) {
      printf("MART output update...");
      fflush(stdout);
    }

    if (current_nb_of_ensemble_terms==0)
      new_mart_mu=1.0; /* à la première itération */

    if (print_detailled_result) {
      printf("\nt=%d\n",current_nb_of_ensemble_terms+1);

      printf("Vecteur d'index de feuilles:\n\n");
      for (i=0; i<global_learning_set_size; i++) {
	j=0;
	while(current_learning_set[j]!=i)
	  j++;
	printf(" %d", ok3mart_index_leaf[ok3mart_index_leaf_current_shift+j]);
      }
    }

    /* met à jour la matrice de kernel */

    /* on calcule dans mart_temp_matrix le produit K^{m-1}.W^{m-1}  */
    c_pos=0; /* on parcourt les objets dans l'ordre des feuilles de l'arbre courant */
    while (c_pos<global_learning_set_size) {
      c_leaf=ok3mart_index_leaf[ok3mart_index_leaf_current_shift+c_pos];
      p_pos=c_pos;
      posfl=current_learning_set[p_pos];
      /* on met la colonne à 0 */
      for (i=0; i<global_learning_set_size; i++) {
	mart_temp_matrix[i*global_learning_set_size+posfl]=0.0;
      }
      /* parcourt la feuille en sommant les colonnes de la matrice de gram */
      while ((c_pos<global_learning_set_size) && (ok3mart_index_leaf[ok3mart_index_leaf_current_shift+c_pos]==c_leaf)) {
	float w=object_weight[current_learning_set[c_pos]];
	for (i=0; i<global_learning_set_size; i++) {
	  mart_temp_matrix[i*global_learning_set_size+posfl]+=
	    (w*getkernel_learn(i,current_learning_set[c_pos]));
	}
	c_pos++;
      }

      /* printf("1 leaf %d -> %d\n",p_pos,c_pos);fflush(stdout); */

      /* on divise par la taille de la feuille */
      ns=prediction_values[prediction[c_leaf]][1];
      for (i=0; i<global_learning_set_size; i++) {
	mart_temp_matrix[i*global_learning_set_size+posfl]/=ns;
      }

      /* on propage cette colonne à toutes les colonnes correspondant à la feuille */
      for (j=p_pos+1; j<c_pos; j++) {	
	for (i=0; i<global_learning_set_size; i++) {
	  int dec=i*global_learning_set_size;
	  mart_temp_matrix[dec+current_learning_set[j]]=mart_temp_matrix[dec+posfl];
	}	
      }
    }

    /* on affiche la matrice temporaire */
    if (print_detailled_result) {
      printf("\n\nK^{m-1}.W^{m-1}\n\n");
      for (i=0; i<global_learning_set_size; i++) {
	for (j=0; j<global_learning_set_size; j++) {
	  printf("%f ",mart_temp_matrix[i*global_learning_set_size+j]);
	}
	printf("\n");
      }
      fflush(stdout);
    }


    /* on met à jour kernel_matrix en soustrayant les elements i,j et j,i de mart_temp_matrix */
    for (i=0; i<global_learning_set_size; i++) {
      for (j=i;j<global_learning_set_size; j++) {
	inckernel_learn(i,j,-new_mart_mu*
			(mart_temp_matrix[i*global_learning_set_size+j]
			 +mart_temp_matrix[j*global_learning_set_size+i]));
      }
    }

    /* affichage de la matrice K^{m-1}-K^{m-1}*W^{m-1}-W^{m-1}'*K^{m-1} */
    if (print_detailled_result) {
      printf("\n\nK^m-K^m*K^{t_m}-K^{t_m}*K^m\n\n");
      for (i=0; i<global_learning_set_size; i++) {
	printf("%d",i);
	for (j=0; j<global_learning_set_size; j++) {
	  printf(" %f",getkernel_learn(i,j));
	}
	printf("\n");
      }
     
      printf("\n\nW^{m-1}*K^{m-1}*W^{m-1}\n\n");
      
      fflush(stdout);
      
    }

    /* on calcule la moitié de W^{m-1}*K^{m-1}*W^{m-1} et on */
    /* l'ajoute directement à la matrice de kernel */
    c_pos=0;
    while (c_pos<global_learning_set_size) {
      float w;
      c_leaf=ok3mart_index_leaf[ok3mart_index_leaf_current_shift+c_pos];
      p_pos=c_pos;
      posfl=current_learning_set[p_pos];

      /* on recherche les bornes de la feuille en calculant la valeur de kii (somme des elements diagonaux) */
      kval=0.0;
      w=object_weight[current_learning_set[c_pos]];
      while ((c_pos<global_learning_set_size) && (ok3mart_index_leaf[ok3mart_index_leaf_current_shift+c_pos]==c_leaf)) {
	kval+=w*mart_temp_matrix[current_learning_set[c_pos]*(global_learning_set_size+1)];
	c_pos++;
      }

      kval/=prediction_values[prediction[c_leaf]][1];
      /* on soustrait kii de la matrice de gram */
      for (j=p_pos; j<c_pos; j++) {
	int p1=current_learning_set[j];
	for (i=j; i<c_pos; i++) {
	  int p2=current_learning_set[i];
	  inckernel_learn(p1,p2,new_mart_mu*new_mart_mu*kval);
	}
      }

      if (print_detailled_result) 
	printf("K(%d,%d)=%f\n",c_leaf,c_leaf,kval);

      /* on calcule les elements kij */
      c_pos2=c_pos;
      while (c_pos2<global_learning_set_size) {
	c_leaf2=ok3mart_index_leaf[ok3mart_index_leaf_current_shift+c_pos2];
	p_pos2=c_pos2;
	/* on calcule la moyenne kij */
	kval=0.0;	
	while ((c_pos2<global_learning_set_size) && 
	       (ok3mart_index_leaf[ok3mart_index_leaf_current_shift+c_pos2]==c_leaf2)) {
	  kval+=object_weight[current_learning_set[c_pos2]]*mart_temp_matrix[current_learning_set[c_pos2]*global_learning_set_size+posfl];
	  c_pos2++;
	}
	
	/* printf("3 leaf %d -> %d\n",p_pos2,c_pos2);fflush(stdout); */
	
	kval/=prediction_values[prediction[c_leaf2]][1];
	/* on la soustrait de la matrice de gram */
	for (j=p_pos; j<c_pos; j++) {
	  int p1=current_learning_set[j];
	  for (i=p_pos2; i<c_pos2; i++) {
	    int p2=current_learning_set[i];
	    inckernel_learn(p1,p2,new_mart_mu*new_mart_mu*kval);
	  }
	}
	if (print_detailled_result) 
	  printf("K(%d,%d)=%f\n",c_leaf,c_leaf2,kval);

      }
    }
    
    /* on calcule la nouvelle matrice de distance */
    for (i=0; i<global_learning_set_size; i++) {
      float kii=getkernel_learn(i,i);
      for (j=i+1; j<global_learning_set_size; j++) {
	float kjj=getkernel_learn(j,j);
	setdist_learn(i,j,kii+kjj-2*getkernel_learn(i,j));
      }
    }

    /* on remet best_first si on est à la première itération */
    if (current_nb_of_ensemble_terms==0) {
      best_first_max_nb_tests=temp_best_first_max_nb_tests;
    } else if (outliers_trimming) { /* sinon on envisage de supprimer les objets les plus mal prédits */
      int nb_outliers=(int)(outliers_trimming_perc*global_learning_set_size);
      float a; int o,po;

      if (print_detailled_result) {
	printf("NB outliers=%d\n",nb_outliers);
	fflush(stdout);
      }

      /* on trie les objets de current_learning_set selon la diagonale de la matrice noyau */
      for (j=1; j<nb_outliers; j++) { /* d'abord les nb_outliers premiers */
	o=current_learning_set[j];
	a=getkernel_learn(o,o);
	for (i=j-1; i>=0; i--) {
	  if (getkernel_learn(current_learning_set[i],current_learning_set[i])>=a)
	    break;
	  current_learning_set[i+1]=current_learning_set[i];
	}
	current_learning_set[i+1]=o;
      }
      /* ensuite les autres par insertion */
      for (j=nb_outliers; j<global_learning_set_size; j++) {
	po=j;
	o=current_learning_set[j];
	a=getkernel_learn(o,o);
	/* on l'insere */
	for (i=nb_outliers-1; i>=0; i--) {
	  if (getkernel_learn(current_learning_set[i],current_learning_set[i])>=a)
	    break;
	  current_learning_set[po]=current_learning_set[i];
	  po=i;
	}
	current_learning_set[po]=o;
      } /* ici, on a current_learning_set trié par ordre décroissant de poids sur les nb_outliers premiers */
      /* verification */
      if (print_detailled_result) {
	for (i=0; i<global_learning_set_size; i++) {
	  printf("%d -> %d %f\n",i,current_learning_set[i],getkernel_learn(current_learning_set[i],current_learning_set[i]));
	  fflush(stdout);
	}
      }
      
      /* on vire les nb_outliers premiers */
      for (i=0; i<nb_outliers; i++) {
	object_weight[current_learning_set[i]]=0.0;
      }
      /* on garde les autres */
      for (i=nb_outliers;i<global_learning_set_size; i++) {
	object_weight[current_learning_set[i]]=1.0;
      }
    }
    
    if (print_result) {
      printf("T");fflush(stdout);
    }

    if (print_detailled_result) {
      
      for (i=0; i<global_learning_set_size; i++) {
	printf("%d",i);
	for (j=0; j<global_learning_set_size; j++) {
	  printf(" %f",getkernel_learn(i,j));
	}
	printf("\n");
      }
      fflush(stdout);
    }
    
    ok3mart_index_leaf_current_shift+=global_learning_set_size;

    return new_mart_mu;
  } else
    return mart_mu;

  return 0.f;
}

/* calcul des prédictions pour un ensemble de points de test */

/* on calcule en fait l'influence des différents points du ls
 * sur ceux de tests. Le résultat est placé dans le vecteur W:
 * chaque ligne correspond à un objet de test, chaque colonne à un objet d'apprentissage
 * w_ij représente l'influence de l'objet d'apprentissage j sur sur l'objet de test i
 *
 * On suppose qu'on a fait compute_node_subset_current_ensemble
 */

void get_ens_objects_weight_mart(int *ts_vector, int ts_vector_size, float *W) {
  int i,j,t, shift_ls;
  float val=1.0/global_learning_set_size;
  int *ts_vector_leaf_index;

  ts_vector_leaf_index=(int *)malloc((size_t)ts_vector_size*sizeof(int));
  if (!ts_vector_leaf_index) {
    printf("impossible d'allouer de la memoire dans get_ens_objects_weight_mart\n");
    exit(0);
  }

  /* initialisation de la matrice mart_temp_matrix qui va contenir les coefficients
   * de combinaison pour chaque objet d'apprentissage */
  for (i=0; i<(global_learning_set_size*global_learning_set_size); i++)
    mart_temp_matrix[i]=0.0;
  for (i=0; i<global_learning_set_size; i++)
    mart_temp_matrix[i*(global_learning_set_size+1)]=1.0;
  /* initialisation de la matrice de poids */
  val=1.0/global_learning_set_size;
  for (i=0; i<(global_learning_set_size*ts_vector_size); i++)
    W[i]=0.0;

  /* boucle sur les arbres */
  shift_ls=0;
  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    int c_leaf, c_pos, p_pos;

    if (print_result) {
      printf("t=%d (tree weight=%f)\n",t,ltrees_weight[t]);fflush(stdout);
    }

    if (print_detailled_result) {

      printf("\nVecteur d'index de feuilles:\n\n");
      for (i=0; i<global_learning_set_size; i++) {
	j=0;
	while(node_object_list[shift_ls+j]!=i)
	  j++;
	printf(" %d", ok3mart_index_leaf[shift_ls+j]);
      }
      printf("\nMatrice O^m\n");
      
      for (i=0; i<global_learning_set_size; i++) {
	printf("%d->",i);
	for (j=0; j<global_learning_set_size; j++) {
	  printf(" %f",mart_temp_matrix[i*global_learning_set_size+j]);
	}
	printf("\n");
      }
      fflush(stdout);
    }

    if (print_result) {
      printf("test...",t);fflush(stdout);
    }

    /* on teste les objets de test et on met la feuille atteinte dans ts_vector_leaf_index */
    for (i=0; i<ts_vector_size; i++) {
      ts_vector_leaf_index[i]=get_tree_leaf(ltrees[t],ts_vector[i]);
    }
    
    if (print_detailled_result) {
      printf("\nVecteur d'index de feuilles pour le test:\n\n");
      for (i=0; i<ts_vector_size; i++) {
	printf("%d ",ts_vector_leaf_index[i]);;
      }
      printf("\n"); fflush(stdout);
    }

    if (print_result) {
      printf("\nmise a jour de W...");fflush(stdout);
    }
    
    /* mise à jour simultanément de la matrice de poids des sorties et de la matrice W */
    c_pos=0;
    while (c_pos<global_learning_set_size) {
      float ns;
      c_leaf=ok3mart_index_leaf[shift_ls+c_pos];
      p_pos=c_pos;

      /* on initialise object_weight */
      for (i=0; i<global_learning_set_size; i++)
	object_weight[i]=0.0;
      
      /* parcourt la feuille en sommant les lignes de la matrice O^m */
      while ((c_pos<global_learning_set_size) && (ok3mart_index_leaf[shift_ls+c_pos]==c_leaf)) {
	float w=node_object_weight[shift_ls+c_pos];
	int nb_lines_c=node_object_list[shift_ls+c_pos]*global_learning_set_size;
	for (i=0; i<global_learning_set_size; i++) {
	  object_weight[i]+=w*mart_temp_matrix[nb_lines_c+i];
	}
	c_pos++;
      }

      /* on divise par la taille de la feuille */
      ns=prediction_values[prediction[c_leaf]][1];
      for (i=0; i<global_learning_set_size; i++)
	object_weight[i]*=(ltrees_weight[t]/ns);

      if (print_detailled_result) {
	printf("feuille %d:",c_leaf);
	for (i=0; i<global_learning_set_size; i++) {
	  printf("%f ",object_weight[i]);;
	}
	printf("\n"); fflush(stdout);
      }

      /* on met a jour la matrice W pour les objets de test qui arrivent dans cette feuille */
      for (j=0; j<ts_vector_size; j++) {
	if (ts_vector_leaf_index[j]==c_leaf) {
	  for (i=0; i<global_learning_set_size; i++)
	    W[i*ts_vector_size+j]+=object_weight[i];
	}
      }
      
      /* on met à jour les lignes de la matrice O (sauf au dernier arbre) */
      if (t<(current_nb_of_ensemble_terms-1)) {
	for (j=p_pos; j<c_pos; j++) {
	  int nb_lines_c=node_object_list[shift_ls+j]*global_learning_set_size;
	  for (i=0; i<global_learning_set_size; i++)
	    mart_temp_matrix[nb_lines_c+i]-=object_weight[i];
	}
      }
    }
    
    if (print_detailled_result) {
      printf("Matrice W:");
      for (j=0; j<ts_vector_size; j++) {
	for (i=0; i<global_learning_set_size; i++) {
	  printf("%f ",W[i*ts_vector_size+j]);;
	}
	printf("\n"); fflush(stdout);
      }
    }

    if (print_result) {
      printf("\n");fflush(stdout);
    }

    shift_ls+=global_learning_set_size;
  }

  free((int *)ts_vector_leaf_index);

}

/* fait des prédictions de kernel à partir de la matrice de poids */

double get_ensemble_kernel_prediction_mart(int i1, int i2, float *W) {
  int i,j,n1,n2,p;
  double sum;
  
  /*  alloue la mémoire */
  o1_obj=(int *)malloc((size_t)global_learning_set_size*sizeof(int));
  o2_obj=(int *)malloc((size_t)global_learning_set_size*sizeof(int));
  o1_w=(float *)malloc((size_t)global_learning_set_size*sizeof(float));
  o2_w=(float *)malloc((size_t)global_learning_set_size*sizeof(float));

  if ((o1_obj==NULL) || (o2_obj==NULL) || (o1_w==NULL) || (o2_w==NULL)) {
    printf("Allocation failed in get_ensemble_kernel_prediction_norm\n");
    fflush(stdout);
    exit(0);
  }
  
  /* on retrouve les poids */
  n1=0; p=i1*global_learning_set_size;
  for (i=0;i<global_learning_set_size; i++) {
    if (W[p]!=0.0) {
      o1_w[n1]=W[p];
      o1_obj[n1]=i;
      n1++;
    }
    p++;
  }

  n2=0; p=i2*global_learning_set_size;
  for (i=0;i<global_learning_set_size; i++) {
    if (W[p]!=0.0) {
      o2_w[n2]=W[p];
      o2_obj[n2]=i;
      n2++;
    }
    p++;
  }

  /* on calcule le kernel sur base des objets */
  sum=0.0;
  for (i=0;i<n1;i++) {
    for (j=0;j<n2;j++) {
      sum+=o1_w[i]*o2_w[j]*getkernel_learn(o1_obj[i],o2_obj[j]);
    }
  }

  free((int *)o1_obj);
  free((float *)o1_w);
  free((int *)o2_obj);
  free((float *)o2_w);
  
  return sum;
}

double get_ensemble_kernel_prediction_norm_mart(int i1, int i2, float *W) {
  int i,j,n1,n2,p;
  double sum, norm1, norm2;
  
  /*  alloue la mémoire */
  o1_obj=(int *)malloc((size_t)global_learning_set_size*sizeof(int));
  o2_obj=(int *)malloc((size_t)global_learning_set_size*sizeof(int));
  o1_w=(float *)malloc((size_t)global_learning_set_size*sizeof(float));
  o2_w=(float *)malloc((size_t)global_learning_set_size*sizeof(float));
  
  if ((o1_obj==NULL) || (o2_obj==NULL) || (o1_w==NULL) || (o2_w==NULL)) {
    printf("Allocation failed in get_ensemble_kernel_prediction_norm\n");
    fflush(stdout);
    exit(0);
  }

  /* on retrouve les poids */
  n1=0; p=i1*global_learning_set_size;
  for (i=0;i<global_learning_set_size; i++) {
    if (W[p]!=0.0) {
      o1_w[n1]=W[p];
      o1_obj[n1]=i;
      n1++;
    }
    p++;
  }

  n2=0; p=i2*global_learning_set_size;
  for (i=0;i<global_learning_set_size; i++) {
    if (W[p]!=0.0) {
      o2_w[n2]=W[p];
      o2_obj[n2]=i;
      n2++;
    }
    p++;
  }

  /* on calcule le kernel sur base des objets */
  sum=0.0;
  for (i=0;i<n1;i++) {
    for (j=0;j<n2;j++) {
      sum+=o1_w[i]*o2_w[j]*getkernel_learn(o1_obj[i],o2_obj[j]);
    }
  }
  
  /* on calcule les normes */
  norm1=0.0;
  for (i=0;i<n1;i++) {
    norm1+=o1_w[i]*o1_w[i]*getkernel_learn(o1_obj[i],o1_obj[i]);
    for (j=i+1;j<n1;j++) {
      norm1+=2.0*o1_w[i]*o1_w[j]*getkernel_learn(o1_obj[i],o1_obj[j]);
    }
  }
  norm2=0.0;
  for (i=0;i<n2;i++) {
    norm2+=o2_w[i]*o2_w[i]*getkernel_learn(o2_obj[i],o2_obj[i]);
    for (j=i+1;j<n2;j++) {
      norm2+=2.0*o2_w[i]*o2_w[j]*getkernel_learn(o2_obj[i],o2_obj[j]);
    }
  }
    
  free((int *)o1_obj);
  free((float *)o1_w);
  free((int *)o2_obj);
  free((float *)o2_w);
  
  return (sum/sqrt(norm1*norm2));
}

/***********************
 * OK BOOST UNIREGR *
 ***********************/

/* boosting dans un espace kernelise avec un regresseur de base à sortie unidimensionnelle */

/* on suppose à ce stade qu'on a initialisé les matrices noyaux et distance et qu'on
 * a également initialisé la régression comme pour le boosting classique
 */

/* vecteur qui va servir à stoquer les directions v de projection */

float *okmartuniregr_v;
int okmartuniregr_v_size=0;
int okmartuniregr_v_current_shift=0;

float *mart_temp_matrix3;

int (*compute_mart_ok_uniregr_direction)(int tree, float *v, ERROR_TYPE *output, int ls_size);

/* calcule une direction pour le boosting et les sorties correspondantes */

/* cas de base, on prend le centre de masse */

int compute_mart_ok_uniregr_direction_masscenter(int tree, float *v, ERROR_TYPE *output, int ls_size) {
  int i,j;
  double norm=0.0;

  if (print_result) {
    printf("Computation of the center of mass\n");
    fflush(stdout);
  }
  
  for (i=0; i<global_learning_set_size; i++) output[i]=0.0;
    
  for (i=0; i<global_learning_set_size; i++) {
    output[i]+=getkernel_learn(i,i);
    for (j=i+1; j<global_learning_set_size; j++) {
      float val=getkernel_learn(i,j);
      output[i]+=val;
      output[j]+=val;
    }
  }   
  
  norm=0.0;
  for (i=0; i<global_learning_set_size; i++)
    norm+=output[i];
  norm=sqrt(norm);
  
  for (i=0; i<global_learning_set_size; i++) {
    v[i]=1/norm;
    output[i]/=norm;
  }
  return 0;
  
}

/* première version: on choisit la direction de plus grande variance */

int compute_mart_ok_uniregr_direction_maxvar(int tree, float *v, ERROR_TYPE *output, int ls_size) {
  int i,j;
  double norm=0.0;

  if (tree<0) { /* cas de base, on choisit la direction correspondant au centre de masse */

    return compute_mart_ok_uniregr_direction_masscenter(tree,v,output,ls_size);

  } else {
    float max_var=0.0, current_var, val;
    double sum, sum_sqr;
    int index_max_var=-1;

    if (print_result) {
      printf("Computation of a new direction\n");
    }

    for (i=0; i<global_learning_set_size; i++) {
      if (getkernel_learn(i,i)>0.0) {
	sum=0.0; sum_sqr=0.0;
	for (j=0; j<global_learning_set_size; j++) {
	  val=getkernel_learn(i,j);
	  sum+=val;
	  sum_sqr+=val*val;
	}
	val=getkernel_learn(i,i);
	sum/=(global_learning_set_size*sqrt(val));
	sum_sqr/=(global_learning_set_size*val);
	current_var=sum_sqr-sum*sum;
	if (print_detailled_result) {
	  printf("Direction %d -> var %f\n", i, current_var);
	  fflush(stdout);
	}
	if (current_var>max_var) {
	  max_var=current_var;
	  index_max_var=i;
	}
      }
    }

    if (print_detailled_result) {
      printf("Best direction %d (var=%f)\n", index_max_var, max_var);
      fflush(stdout);
    }
    
    if (max_var<=0.0) {
      if (print_result) {
	printf("The best direction has zero variance -> we stop here\n");
	fflush(stdout);
      }
      return -1;
    }
    
    val=sqrt(getkernel_learn(index_max_var,index_max_var));
    for (i=0; i<global_learning_set_size; i++)
      v[i]=0.0;
    v[index_max_var]=1/val;

    for (i=0; i<global_learning_set_size; i++)
      output[i]=getkernel_learn(i,index_max_var)/val;

    if (print_detailled_result) {
      printf("\n-> New outputs:\n");
      for (i=0; i<global_learning_set_size; i++)
	printf("%f ", output[i]);
      printf("\n\n");
    }

    return 0;
  }
}

/* deuxième version, on choisit une direction aléatoire (parmi les directions du LS) */

int compute_mart_ok_uniregr_direction_random(int tree, float *v, ERROR_TYPE *output, int ls_size) {
  int i,j;
  double norm=0.0;

  if (tree<0) { /* cas de base, on choisit la direction correspondant au centre de masse */

    return compute_mart_ok_uniregr_direction_masscenter(tree,v,output,ls_size);

  } else {
    float val;
    int index_max_var=-1, non_null_dir=0, index_random;

    if (print_result) {
      printf("Computation of a random direction\n");
    }

    /* on vérifie qu'il reste des directions de variance non nulle */
    for (i=0; i<global_learning_set_size; i++) {
      if (getkernel_learn(i,i)>0.0)
	non_null_dir++;
    }
    
    if (non_null_dir==0) {
      if (print_result) {
	printf("All directions have zero variance -> we stop here\n");
	fflush(stdout);
      }
      return -1;
    }

    index_random=get_random_integer(non_null_dir);
    
    i=-1;j=-1;
    do {
      i++;
      if (getkernel_learn(i,i)>0.0)
	j++;
    } while(j!=index_random);
    
    index_random=i;
    
    if (print_detailled_result) {
      printf("Random direction (out of %d): %d\n",non_null_dir,index_random);
      fflush(stdout);
    }
    
    val=sqrt(getkernel_learn(index_random,index_random));
    for (i=0; i<global_learning_set_size; i++)
      v[i]=0.0;
    v[index_random]=1/val;

    for (i=0; i<global_learning_set_size; i++)
      output[i]=getkernel_learn(i,index_random)/val;

    if (print_detailled_result) {
      printf("\n-> New outputs:\n");
      for (i=0; i<global_learning_set_size; i++)
	printf("%f ", output[i]);
      printf("\n\n");
    }

    return 0;
 
  }
}

/* troisième version: on choisit k direction au hasard et on prend celle de variance max */

int compute_mart_ok_uniregr_direction_randommaxvar(int tree, float *v, ERROR_TYPE *output, int ls_size) {
  int i,j;
  double norm=0.0;

  if (tree<0) { /* cas de base, on choisit la direction correspondant au centre de masse */

    return compute_mart_ok_uniregr_direction_masscenter(tree,v,output,ls_size);

  } else {
    float val;
    int index_max_var=-1, non_null_dir=0, index_random;

    if (print_result) {
      printf("Computation of a random direction\n");
    }

    /* on vérifie qu'il reste des directions de variance non nulle */
    for (i=0; i<global_learning_set_size; i++) {
      if (getkernel_learn(i,i)>0.0)
	non_null_dir++;
    }
    
    if (non_null_dir==0) {
      if (print_result) {
	printf("All directions have zero variance -> we stop here\n");
	fflush(stdout);
      }
      return -1;
    }

    index_random=get_random_integer(non_null_dir);
    
    i=-1;j=-1;
    do {
      i++;
      if (getkernel_learn(i,i)>0.0)
	j++;
    } while(j!=index_random);
    
    index_random=i;
    
    if (print_detailled_result) {
      printf("Random direction (out of %d): %d\n",non_null_dir,index_random);
      fflush(stdout);
    }
    
    val=sqrt(getkernel_learn(index_random,index_random));
    for (i=0; i<global_learning_set_size; i++)
      v[i]=0.0;
    v[index_random]=1/val;

    for (i=0; i<global_learning_set_size; i++)
      output[i]=getkernel_learn(i,index_random)/val;

    if (print_detailled_result) {
      printf("\n-> New outputs:\n");
      for (i=0; i<global_learning_set_size; i++)
	printf("%f ", output[i]);
      printf("\n\n");
    }

    return 0;
 
  }
}


/* troisième version, on choisit une direction complètement aléatoire */

/* nombre d'essais pour trouver une direction de variance non nulle */

int nb_try_dir=10;

int compute_mart_ok_uniregr_direction_fully_random(int tree, float *v, ERROR_TYPE *output, int ls_size) {
  int i,j;
  double norm=0.0;

  if (tree<0) { /* cas de base, on choisit la direction correspondant au centre de masse */

    return compute_mart_ok_uniregr_direction_masscenter(tree,v,output,ls_size);

  } else {
    float var,norm;
    int k;

    if (print_result) {
      printf("Computation of a fully random direction\n"); 
    }

    /* on vérifie qu'il reste des directions de variance non nulle */
    k=0; var=0.0;

    while (var==0.0 && k<nb_try_dir) {
      double sum=0.0,sum_sqr=0.0;
      /* on choisit une direction aléatoire */
      for (i=0; i<global_learning_set_size; i++) 
	v[i]=get_random_float();
      /* on normalise et on projete en même temps */
      norm=0.0;
      for (i=0; i<global_learning_set_size; i++) {
	output[i]=0.0;
	for (j=0; j<global_learning_set_size; j++) {
	  output[i]+=getkernel_learn(i,j)*v[j];
	}
	norm+=(output[i]*v[i]);
	sum+=output[i];
	sum_sqr+=(output[i]*output[i]);
      }
      sum/=(global_learning_set_size*sqrt(norm));
      sum_sqr/=(norm*global_learning_set_size);
      var=sum_sqr-(sum*sum);
      k++;
    }

    if (var==0.0) {
      if (print_result) {
	printf("No variance found in %d trials -> we stop here\n",nb_try_dir);
	fflush(stdout);
      }
      return -1;
    }

    if (print_detailled_result) {
      printf("Var=%f",var);
      fflush(stdout);
    }
    
    /* on normalise le vecteur et les projections */
    norm=sqrt(norm);
    for (i=0; i<global_learning_set_size; i++) {
      output[i]/=norm;
      v[i]/=norm;
    }

    if (print_detailled_result) {
      printf("\n-> New outputs:\n");
      for (i=0; i<global_learning_set_size; i++)
	printf("%f ", output[i]);
      printf("\n\n");
    }

    return 0;
 
  }
}

void set_okboost_uniregr_direction_function(int m) {
  if (m==0)
    compute_mart_ok_uniregr_direction=compute_mart_ok_uniregr_direction_maxvar;
  else if (m==1)
    compute_mart_ok_uniregr_direction=compute_mart_ok_uniregr_direction_random;
  else if (m==2)
    compute_mart_ok_uniregr_direction=compute_mart_ok_uniregr_direction_fully_random;
}

/* fonction de mise a jour des matrices */

float make_ls_vector_mart_ok_uniregr(int tree) {
  int i,j, size_km, pos;

  if (tree<0) { /* INITIALISATION (M=0) */
    for (i=0; i<global_learning_set_size; i++)
      /* on initialise le poids des objets (pas vraiment pris en compte ici) */
      if (weight_attributep) {
	object_weight[i]=getattval(object_mapping[i],weight_attribute);
      } else
	object_weight[i]=1.0;

    /* on alloue le vecteur de sortie temporaire */
    if (length_temp_output<global_learning_set_size) {
      if (length_temp_output!=0)
	free((ERROR_TYPE *)temp_output);
      temp_output=(ERROR_TYPE *)malloc((size_t)global_learning_set_size*sizeof(ERROR_TYPE));
      if (!temp_output) {
	printf("impossible d'allouer de la memoire dans make_ls_vector_mart_ok_uniregr\n");
	exit(0);
      }
      length_temp_output=global_learning_set_size;
    }
    
    /* on alloue le vecteur okmartuniregr_v (qui va stocker les directions) */
    size_km=number_of_ensemble_terms*global_learning_set_size;
    if (okmartuniregr_v_size<size_km) {
      if (okmartuniregr_v_size!=0)
	free((float *)okmartuniregr_v);
      okmartuniregr_v=(float *)malloc((size_t)size_km*sizeof(float));
      if (!okmartuniregr_v) {
	printf("impossible d'allouer de la memoire dans make_ls_vector_mart_ok_uniregr\n");
	fflush(stdout);
	exit(0);
      }
      okmartuniregr_v_size=size_km;
    }
    okmartuniregr_v_current_shift=0;

    /* on alloue un vecteur temporaire de la taille du LS pour les calculs intermédiares */
    size_km=global_learning_set_size;
    if (size_mart_temp_matrix<size_km) {
      if (size_mart_temp_matrix!=0)
	free((float *)mart_temp_matrix);
      mart_temp_matrix=(float *)malloc((size_t)size_km*sizeof(float));
      if (!mart_temp_matrix) {
	printf("impossible d'allouer de la memoire dans make_ls_vector_mart_ok_uniregr\n");
	fflush(stdout);
	exit(0);
      }
      size_mart_temp_matrix=size_km;
    }

    /* matrice NxN non symmetrique pour le poids des objets */
    /* permet de precalculer les vecteurs v' pour le test */
    size_km=global_learning_set_size*global_learning_set_size;
    mart_temp_matrix3=(float *)malloc((size_t)size_km*sizeof(float));
    if (!mart_temp_matrix3) {
      printf("impossible d'allouer de la memoire dans make_ls_vector_mart_ok_uniregr\n");
      fflush(stdout);
      exit(0);
    }
    /* initialisation a la matrice identite */
    pos=0;
    for (i=0; i<global_learning_set_size; i++) {
      for (j=0; j<global_learning_set_size; j++) {
	mart_temp_matrix3[pos]=0.0;
	pos++;
      }
      mart_temp_matrix3[i*(global_learning_set_size+1)]=1.0;
    }

    /* une matrice NxN symetrique pour le kernel courant */
    size_km=global_learning_set_size*(global_learning_set_size+1)/2;
    if (size_mart_temp_matrix2<size_km) {
      if (size_mart_temp_matrix2!=0)
	free((float *)mart_temp_matrix2);
      mart_temp_matrix2=(float *)malloc((size_t)size_km*sizeof(float));
      if (!mart_temp_matrix2) {
	printf("impossible d'allouer de la memoire dans make_ls_vector_mart_ok_uniregr\n");
	fflush(stdout);
	exit(0);
      }
      size_mart_temp_matrix2=size_km;
    }

    /* on la remplit à partir du kernel initial */
    for (i=0; i<size_km; i++)
      mart_temp_matrix2[i]=kernel_matrix[i];
    /* on l'associe à la matrice noyau pour la construction des arbres */
    /* (la matrice restera accessible du lisp mais plus du c) */
    kernel_matrix=mart_temp_matrix2;

    /* on choisit la première direction et on calcule les sorties */
    /* v_i=1/Z avec Z la norme de la somme de phi(y_i) (=la racine de la somme des elements de 
     * la matrice noyau) */
    /* (on passe par une fonction, ça permet de paramétrer cette fonction pour tester différentes
       variantes) */
    compute_mart_ok_uniregr_direction(tree, okmartuniregr_v, temp_output, global_learning_set_size);

    /* on fixe la nouvelle fonction d'accès aux sorties */
    getobjy_learn=getobjy_learn_temp;

    /* pour que le premier modele ait une complexite de 1 (pas de test) */
    temp_best_first_max_nb_tests=best_first_max_nb_tests;
    best_first_max_nb_tests=0;

    if (print_detailled_result) {
      printf("t=0\nKernel:\n");
      for (i=0; i<global_learning_set_size; i++) {
	printf("%d",i);
	for (j=0; j<global_learning_set_size; j++) {
	  printf(" %f",getkernel_learn(i,j));
	}
	printf("\n");
      }
      fflush(stdout);
    }

  } else { /* RESIDUAL COMPUTATION (M>0) */
    /* on évite la dernière mise à jour inutile */
    float *current_v;
    float new_mart_mu=mart_mu;

    if (print_result) {
      printf("MART output update...");     
    }
    
    current_v=okmartuniregr_v+okmartuniregr_v_current_shift;

    if (current_nb_of_ensemble_terms<number_of_ensemble_terms-1) { /* pas la derniere fois */
      if (print_result) {
	printf("K...");
	fflush(stdout);
      }

      if (current_nb_of_ensemble_terms==0)
	new_mart_mu=1.0; /* à la première itération, on utilise mu=1 pour centrer la matrice */

      /* On met à jour la matrice de kernel */
    
      /* On calcule toutes les prédictions sur le LS -> mart_temp_matrix */
      /* (vecteur f^{m-1}(x_i)) */
      for (i=0; i<global_learning_set_size; i++) {
	mart_temp_matrix[i]=(get_tree_prediction_vector(tree, object_mapping[i]))[0];
      }
    
      /* On ne doit pas calculer le produit K^{m-1}.v^{m-1}, ça correspond exactement à temp_output */

      /* on affiche la matrice temporaire et des sorties */
      if (print_detailled_result) {
	printf("\n\n(K^{m-1}.v^{m-1})^T\n\n");
	for (i=0; i<global_learning_set_size; i++) {
	  printf("%f ",temp_output[i]);
	}
	printf("\n");
	printf("\n\n(f^{m-1})^T\n\n");
	for (i=0; i<global_learning_set_size; i++) {
	  printf("%f ",mart_temp_matrix[i]);
	}
	fflush(stdout);
      }
      
      /* on met à jour la kernel_matrix courante sur base de temp_output et mart_temp_matrix (cf le papier pour les détails) */
      for (i=0; i<global_learning_set_size; i++) {
	for (j=i;j<global_learning_set_size; j++) {
	  inckernel_learn(i,j,
			  -new_mart_mu*(temp_output[i]*mart_temp_matrix[j]+temp_output[j]*mart_temp_matrix[i])
			  +new_mart_mu*new_mart_mu*mart_temp_matrix[i]*mart_temp_matrix[j]);
	}
      }

      /* affichage du nouveau noyau */
      if (print_detailled_result) {
	printf("\n\nK^{m}\n\n");
	for (i=0; i<global_learning_set_size; i++) {
	  printf("%d",i);
	  for (j=0; j<global_learning_set_size; j++) {
	    printf(" %f",getkernel_learn(i,j));
	  }
	  printf("\n");
	}
	fflush(stdout);	
      }
      
      /* on calcule la nouvelle matrice de distance */
      for (i=0; i<global_learning_set_size; i++) {
	float kii=getkernel_learn(i,i);
	for (j=i+1; j<global_learning_set_size; j++) {
	  float kjj=getkernel_learn(j,j);
	  setdist_learn(i,j,kii+kjj-2*getkernel_learn(i,j));
	}
      }
    }
    
    /* correction de la direction pour accelerer le test */
    if (print_result) {
      printf("v...");
      fflush(stdout);
    }
    
    if (print_detailled_result) {
      printf("\n\nOld v^m:\n");
      for (i=0; i<global_learning_set_size; i++) {
	printf("%f ", current_v[i]);
      }
      printf("\n");
      fflush(stdout);
    }
    /* on calcule v^m^T O^{m} dans temp_output */
    for (i=0; i<global_learning_set_size; i++) {
      temp_output[i]=0;
      for (j=0; j<global_learning_set_size; j++) {
	/* ATTENTION ATTENTION INVERSION DE I ET J */
	temp_output[i]+=current_v[j]*mart_temp_matrix3[j*global_learning_set_size+i];
      }
    }
    /* on corrige le v */
    for (i=0; i<global_learning_set_size; i++) {
      current_v[i]=temp_output[i];
    }
    if (print_detailled_result) {
      printf("New v^m (v^m'):\n");
      for (i=0; i<global_learning_set_size; i++) {
	printf("%f ", current_v[i]);
      }
      printf("\n\n");
      fflush(stdout);
    }

    if (current_nb_of_ensemble_terms<number_of_ensemble_terms-1) { /* pas la derniere fois */
      /* on calcule O^{m+1}=O^m-mu*f^m v^m^T O^m=O^m-f^m v^m' */

      if (print_result) {
	printf("O...");
	fflush(stdout);
      }

      if (print_detailled_result) {
	printf("\n\nO^{m+1}=O^m-f^m v^m^T O^m=O^m-f^m v^m'\n\n");
	fflush(stdout);
      }
      /* current_v contient déjà v^m^T O^m = v^m' et mart_temp_matrix contient f^m */
      pos=0;
      for (i=0; i<global_learning_set_size; i++) {
	for (j=0; j<global_learning_set_size; j++) {
	  mart_temp_matrix3[pos]-=new_mart_mu*mart_temp_matrix[i]*current_v[j];
	  pos++;
	}
      }
      
      if (print_detailled_result) {
	for (i=0; i<global_learning_set_size; i++) {
	  printf("%d ",i);
	  for (j=0; j<global_learning_set_size; j++)
	    printf(" %f",mart_temp_matrix3[i*global_learning_set_size+j]);
	  printf("\n");
	}
	printf("\n");
	fflush(stdout);
      }

      /* on calcule les nouvelles direction et sortie à partir de la matrice de noyau courante */
      okmartuniregr_v_current_shift+=global_learning_set_size;
      if (compute_mart_ok_uniregr_direction(tree, okmartuniregr_v+okmartuniregr_v_current_shift, temp_output, global_learning_set_size)<0) {
	return 0.0;
      }
    }

    /* on remet best_first si on est à la première itération */
    if (current_nb_of_ensemble_terms==0) {
      best_first_max_nb_tests=temp_best_first_max_nb_tests;
    }
    /* on libere la memoire a la derniere iteration */
    if (current_nb_of_ensemble_terms==number_of_ensemble_terms-1) {
      free((float *)mart_temp_matrix3);
    }

    if (print_result) {
      printf("T");fflush(stdout);
    }

    return new_mart_mu;
  }

  return 0.f;
}

/* calcul des prédictions (même schéma que pour Boosting-OK3) */

/* on calcule en fait l'influence des différents points du ls
 * sur ceux de tests. Le résultat est placé dans le vecteur W:
 * chaque ligne correspond à un objet de test, chaque colonne à un objet d'apprentissage
 * w_ij représente l'influence de l'objet d'apprentissage j sur sur l'objet de test i
 *
 * Pour le ok_uniregr, on pourrait faire ce calcul objet par objet (pas de calcul
 * commun à tous les objets)
 */

void get_ens_objects_weight_mart_ok_uniregr(int *ts_vector, int ts_vector_size, float *W) {
  int i,j,t, pos;
  float *current_v,current_mu;
  float *ts_predictions;

  ts_predictions=(float *)malloc((size_t)ts_vector_size*sizeof(float));
  if (!ts_predictions) {
    printf("impossible d'allouer de la memoire dans get_ens_objects_weight_mart_ok_uniregr\n");
    exit(0);
  }

  /* initialisation de la matrice de poids */
  for (i=0; i<(global_learning_set_size*ts_vector_size); i++)
    W[i]=0.0;

  /* boucle sur les arbres */
  current_v=okmartuniregr_v;

  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    
    if (print_result) {
      printf("t=%d (tree weight=%f)\n",t,ltrees_weight[t]);fflush(stdout);
      printf("test...",t);fflush(stdout);
    }
    /* on teste les objets de test et on met la prediction dans ts_predictions */
    for (i=0; i<ts_vector_size; i++) {
      ts_predictions[i]=(get_tree_prediction_vector(ltrees[t], ts_vector[i]))[0];
    }
    
    if (print_detailled_result) {
      printf("\nVecteur de predictions sur l'ensemble de test:\n\n");
      for (i=0; i<ts_vector_size; i++) {
	printf("%f ",ts_predictions[i]);;
      }
      printf("\n"); fflush(stdout);
    }
    
    if (print_result) {
      printf("\nmise a jour de W...");fflush(stdout); 
    }

    /* mise à jour de la matrice W */
    pos=0;
    current_mu=ltrees_weight[t];
    for (i=0; i<ts_vector_size; i++) {
      for (j=0; j<global_learning_set_size; j++) {
	W[pos]+=(current_mu*ts_predictions[i]*current_v[j]);
	pos++;
      }
    }
    
    if (print_detailled_result) {
      printf("\nMatrice W:");
      for (j=0; j<ts_vector_size; j++) {
	for (i=0; i<global_learning_set_size; i++) {
	  printf("%f ",W[j*global_learning_set_size+i]);;
	}
	printf("\n"); fflush(stdout);
      }
    }
    
    if (print_result) {
      printf("\n");fflush(stdout);    
    }
    current_v+=global_learning_set_size;
  }

  free((int *)ts_predictions);
}

/******************
 * TREE-BASED PCA *
 ******************/

/* Calcule une prédiction à chaque feuille qui respecte
 * les centres de masse. Permet de visualiser les données
 * dans une espace de petite dimension (sorte de PCA)
 *
 * La fonction suppose que la fonction compute_node_subset_current_ensemble_weighted
 * a déjà été appelée
 */

void compute_leaf_unidim_prediction() {
  int t;
  /* vecteur temporaire contenant les predictions */
  float *temp_y_vect=(float *)malloc((size_t)MAX_OPEN_NODES*sizeof(float));
  if (!temp_y_vect) {
    printf("impossible d'allouer de la memoire dans compute_leaf_unidim_prediction\n");
    exit(0);
  }

  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    
    index_stack_open_nodes=-1;

    index_stack_open_nodes++;
    stack_open_nodes[index_stack_open_nodes][0]=ltrees[t];
    temp_y_vect[index_stack_open_nodes]=0.0; /* on suppose que la sortie est centrée en 0 */
    
    while(index_stack_open_nodes>=0) {
      int node=stack_open_nodes[index_stack_open_nodes][0];
      float current_y=temp_y_vect[index_stack_open_nodes];
      
      index_stack_open_nodes--;

      if (left_successor[node]==-1) {
	/* si c'est une feuille, on cree une prediction (on efface le contenu de
	 * la table prediction_values, qui normalement n'est pas utilisé) */
	prediction_values[prediction[node]][0]=current_y;
      } else {
	/* sinon, on calcule l'info sur les successeurs gauche et droit */
	/* calcul de d */
	int i,j;
	int left=node+left_successor[node], right=node+right_successor[node];
	int size_left=node_subset_desc[left][0], size_right=node_subset_desc[right][0];
	SCORE_TYPE weight_left=0.0, weight_right=0.0;
	int start_left=node_subset_desc[left][1], start_right=node_subset_desc[right][1];
	SCORE_TYPE sum_left=0.0,sum_right=0.0,sum_left_right=0.0, d;

	for (i=start_left; i<start_left+size_left;i++) {
	  weight_left+=node_object_weight[i];
	  sum_left+=(node_object_weight[i]*node_object_weight[i]*getkernel_learn(node_object_list[i],node_object_list[i]));
	  for (j=i+1; j<start_left+size_left; j++) {
	    sum_left+=(2.0*node_object_weight[i]*node_object_weight[j]*getkernel_learn(node_object_list[i],node_object_list[j]));
	  }
	}
	sum_left=sum_left/(weight_left*weight_left);

	for (i=start_right; i<start_right+size_right;i++) {
	  weight_right+=node_object_weight[i];
	  sum_right+=(node_object_weight[i]*node_object_weight[i]*getkernel_learn(node_object_list[i],node_object_list[i]));
	  for (j=i+1; j<start_right+size_right; j++) {
	    sum_right+=(2*node_object_weight[i]*node_object_weight[j]*getkernel_learn(node_object_list[i],node_object_list[j]));
	  }
	}
	sum_right=sum_right/(weight_right*weight_right);

	for (i=start_left; i<start_left+size_left;i++) {
	  for (j=start_right; j<start_right+size_right; j++) {
	    sum_left_right+=node_object_weight[i]*node_object_weight[j]*getkernel_learn(node_object_list[i],node_object_list[j]);
	  }
	}
	sum_left_right=sum_left_right/(weight_left*weight_right);

	d=sqrt(fabs(sum_right+sum_left-2.0*sum_left_right));
	/* printf("current_y=%f, d=%f(%f), left=%f, right=%f\n", current_y, d, sum_right+sum_left-2.0*sum_left_right, current_y-d*weight_right/(weight_left+weight_right), current_y+d*weight_left/(weight_left+weight_right)); */
	/* fflush(stdout); */

	/* place les successeurs sur la pile */
	index_stack_open_nodes++;
	stack_open_nodes[index_stack_open_nodes][0]=left;
	temp_y_vect[index_stack_open_nodes]=current_y-d*weight_right/(weight_left+weight_right);
	index_stack_open_nodes++;
	stack_open_nodes[index_stack_open_nodes][0]=right;
	temp_y_vect[index_stack_open_nodes]=current_y+d*weight_left/(weight_left+weight_right);	
      }
    }
  }

  free((float *)temp_y_vect);
}

/*********************
 * various functions *
 *********************/

/* gaussian kernel on different attributes */

float compute_gaussian_kernel(int o1, int o2, int *att_vect, int nb_att, float sigma) {
  int i;
  double sum=0.0;

  for (i=0; i<nb_att; i++) {
    float diff=getattval_normal(o1,att_vect[i])-getattval_normal(o2,att_vect[i]);
    sum+=(diff*diff);
  }
  return exp(-sum/(2*nb_att*sigma*sigma));
}


#ifndef FILE_TREE_MODEL_C
# define FILE_TREE_MODEL_C

#ifdef WIN32
# pragma warning(disable:4244) // warning C4244: '=' : conversion from 'double' to 'float', possible loss of data
# pragma warning(disable:4305) // warning C4305: 'initializing' : truncation from 'double' to 'float'
# pragma warning(disable:4996) // warning C4996: '...': This function or variable may be unsafe. Consider using ..._s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
# pragma warning(disable:4129) // warning C4129: '%' : unrecognized character escape sequence
#endif

/*
 * Author: Pierre Geurts (p.geurts@ulg.ac.be)
 *
 * (Please do not distribute without permission of the author)
 *
 */

/* main functions to build single trees and ensembles */

void init_threshold_type(union threshold_type *u) {
  int i;
  for (i=0;i<MAX_NUMBER_OF_SYMBOLIC_VALUES_DIV_32; i++)
    (*u).i[i]=0;
}

void add1_threshold_type(union threshold_type *u) {
  int i=0;
  do {
    (*u).i[i]++;
  } while (((*u).i[i++]==0)&&(i<=MAX_NUMBER_OF_SYMBOLIC_VALUES_DIV_32));
}

/**********************/
/* VARIABLES GLOBALES */
/**********************/

/* DEFINITION DE LA "TABLE" POUR STOCKER LES ARBRES */

int size_current_tree_table=0;
int size_current_tree_table_pred=0;

union threshold_type *threshold=NULL; /* le seuil associe a un noeud (sur 64 BITS pour représenter plus de valeurs
				                      * symboliques */
int *left_successor=NULL; /* la position RELATIVE du successeur par rapport */
int *right_successor=NULL; /* a ce noeud */
int *tested_attribute=NULL; /* l'attribut teste en ce noeud */
int *prediction=NULL; /* l'index de la prediction associe au noeud */
float *node_weight=NULL; /* un poids pour le test en dual perturb and combine */
/* Pour missing values: la taille de la feuille en nb d'elements */
float *node_size=NULL;

float **prediction_values=NULL;
int index_prediction_values=-1;

int index_nodes=-1;

int stack_open_nodes[MAX_OPEN_NODES][3];

/* pour le best first */
SCORE_TYPE priority_open_nodes[MAX_OPEN_NODES];
/* pour le true best first */
int *tbf_best_attribute=NULL;
int *tbf_stop_splitting=NULL;
int *tbf_not_significant=NULL;
SCORE_TYPE *tbf_best_threshold_score=NULL;
SCORE_TYPE *tbf_best_threshold_info=NULL;
union threshold_type *tbf_best_threshold=NULL;
int size_tbf=0;

int index_stack_open_nodes=-1;

/* table des valeurs */

CORETABLE_TYPE *core_table=NULL;

int nb_obj_in_core_table;

/* vector with attribute descriptor (0 if numerical, the number of values
 * if symbolic)
 */
int *attribute_descriptors=NULL;
int length_attribute_descriptors;

/* parametres generaux pour la construction */

int goal; /* goal_classification ou goal_regression */

int goal_type; /* CLASSIFICATION, REGRESSION ou DISTANCE */

int *attribute_vector=NULL; /* indice des attributs */
int nb_attributes;

int *current_learning_set=NULL; /* l'ensemble d'apprentissage */
int current_learning_set_size;
int *object_mapping=NULL;
SCORE_TYPE *object_weight=NULL;
int global_learning_set_size=0; /* taille des table object_mapping et object_weight: peut être plus grand
				   que current_learning_set_size si certains objets sont de poids nuls*/

int min_node_size; /* nombre min d'objets dans un noeud */

int best_first=0; /* si 1 arbre construit en best_first (sur base de la variance dans le noeud) */
int true_best_first=0; /* si arbre construit en vrai best first (sur base du score du meilleur test) */
                       /* on suppose que true_best_first a priorité sur best_first si les deux sont vrais */

int best_first_max_nb_tests=1; /* nb maximum de tests si en (true) best first */

int missing_values=0; /* valeurs manquantes (1) ou pas (0)*/

/* parametres du tri: *
 * par defaut, on trie chaque fois localement */
void (*sort_ls_vector)(int *ls_vector, int start, int end, int att)=quicksort_ls_vector;
int (*separate_ls_vector)(int best_attribute, union threshold_type best_threshold, int *ls_vector, int start, int end)=separate_ls_vector_local; 

int sorting_variant=LOCAL_SORT;

/* methodes d'ensemble */

float (*make_ls_vector)(int);
int number_of_ensemble_terms=50;
int test_ensemble_while_growing=1;
int save_ensemble_while_growing=0;
int store_ensemble=1;

/* Niveau d'affichage de messages a l'ecran (0 -> rien, 1 -> tout) */
int print_result=1;

/* parametres generaux pour la classification */

int nb_classes; /* nombre de classes pour la classification */

/* parametres generaux pour la regression */ 

/* ... */

/* pour le calcul du score */

SCORE_TYPE **table_score=NULL; /* [3][MAX_NUMBER_OF_PREDICTION_VALUES+1]; */
SCORE_TYPE **table_score_symb=NULL; /* [MAX_NUMBER_OF_SYMBOLIC_VALUES][MAX_NUMBER_OF_PREDICTION_VALUES+1]; */
int nb_of_predictions;

#ifdef LOAD_MULTIREGR
/* on pourrait utiliser table_score et table_score_symb mais je préfère utiliser
 * des tables dédiées pour que MAX_GOAL_MULTIREGR soit indépendant de MAX_NUMBER_OF_NODES
 */
SCORE_TYPE **table_score_multiregr=NULL; /* [3][2*MAX_GOAL_MULTIREGR+1]; */
SCORE_TYPE **table_score_symb_multiregr=NULL; /* [MAX_NUMBER_OF_SYMBOLIC_VALUES][2*MAX_GOAL_MULTIREGR+1]; */
#endif

/* pour tenir compte d'un poids différent par classe 
 * (on en tient compte uniquement pour la definition des
 * object_weight) 
 */
int class_weightp;
float *class_weight_vector=NULL;

/* pour permettre à l'utilisateur de fournir son attribut de poids */
int weight_attributep;
int weight_attribute;

/* parametre de stop splitting pour les arbres */

float h_min;

/* variables globales pour le calcul des meilleures score */

int best_attribute;
union threshold_type best_threshold;
SCORE_TYPE best_threshold_score;

union threshold_type current_threshold;
SCORE_TYPE current_threshold_score;

/* variables globales pour le stop splitting criterio a posteriori (not-significant) */
SCORE_TYPE current_threshold_info;
SCORE_TYPE best_threshold_info;

/***************/
/* CODE ARBRES */
/***************/

/* BUILD_ONE_TREE */
/*----------------*/
/* fonction generale de construction d'un arbre */

/* print_stack */
void print_stack(int size) {
  int i;
  printf("Begin stack:\n");
  for (i=0; i<=size; i++) {
    printf("Pos %d - node=%d (%d-%d) - score=%e - att=%d\n",i,stack_open_nodes[i][0], stack_open_nodes[i][1], stack_open_nodes[i][2], tbf_best_threshold_info[i], tbf_best_attribute[i]);
  }
  printf("End stack\n");
  fflush(stdout);
}

DllExport int build_one_tree() {
  int tree;
  int nb_tests=0;

  /* on construit un noeud et on le place sur le stack */
  index_nodes++;
  prediction[index_nodes]=-1; /* par defaut, c'est une feuille */
  tested_attribute[index_nodes]=-1; /* pas de test */
  left_successor[index_nodes]=-1;
  right_successor[index_nodes]=-1;
  tree=index_nodes;
  
  index_stack_open_nodes++;
  stack_open_nodes[index_stack_open_nodes][0]=tree;
  stack_open_nodes[index_stack_open_nodes][1]=0;
  stack_open_nodes[index_stack_open_nodes][2]=current_learning_set_size-1;
  
  if (true_best_first) {
    /* on recherche le meilleur split et on stocke l'info */
    summarize_vector(current_learning_set, 0, current_learning_set_size-1);

    if (goal_type==MULTIREGR)
      node_size[tree]=table_score[0][0]; /* valeurs manquantes ? */
    else
      node_size[tree]=table_score_multiregr[0][0];

    tbf_stop_splitting[index_stack_open_nodes]=((current_learning_set_size==1) || (current_learning_set_size<min_node_size) || stop_splitting_criterio());
    if (!tbf_stop_splitting[index_stack_open_nodes]) {
      find_a_split(current_learning_set, 0, current_learning_set_size-1);
      tbf_best_attribute[index_stack_open_nodes]=best_attribute;
      /* tbf_best_threshold_score[index_stack_open_nodes]=best_threshold_score; */
      tbf_best_threshold_info[index_stack_open_nodes]=best_threshold_info;
      tbf_best_threshold[index_stack_open_nodes]=best_threshold;
      tbf_not_significant[index_stack_open_nodes]=not_significant_test();
    } else {
      tbf_best_threshold_info[index_stack_open_nodes]=-1.0;
    }
  } else if (best_first) {
    /* on calcule la variance totale */
    summarize_vector(current_learning_set, 0, current_learning_set_size-1);
    priority_open_nodes[index_stack_open_nodes]=best_first_node_priority_function();
  }

  /*
    printlsvectoratt(current_learning_set, 0, current_learning_set_size, goal);
    printlsvector(object_mapping, 0, current_learning_set_size);
    printf("Goal class=%d, nb_att=%d ls_size=%d\n", goal, nb_attributes, current_learning_set_size);
    for (i=0; i<nb_attributes; i++) {
    printf("%d ", attribute_vector[i]);
    }
    printf("\n");
    fflush(stdout);
  */

  /* on lance la boucle de developpement, tant qu'il y a des noeuds ouverts */
  while (index_stack_open_nodes>=0) {
    int node=stack_open_nodes[index_stack_open_nodes][0];
    int start=stack_open_nodes[index_stack_open_nodes][1];
    int end=stack_open_nodes[index_stack_open_nodes][2];
    int nodesize=end-start+1;

    /* 
    if (true_best_first)
      print_stack(index_stack_open_nodes);
    */
    /*
      printf("start:%d end:%d index_stack:%d\n", start, end, index_stack_open_nodes);
      fflush(stdout);
    */
    
    if (!true_best_first) {
      /* resume du vecteur */
      summarize_vector(current_learning_set, start, end);
      /* pour missing_values */
      if (goal_type!=MULTIREGR)
	node_size[node]=table_score[0][0];
      else
	node_size[node]=table_score_multiregr[0][0];
    }

    /* condition d'arret: */
    if ((true_best_first && (tbf_stop_splitting[index_stack_open_nodes] || (nb_tests>=best_first_max_nb_tests)))||
	(!true_best_first && ((best_first && (nb_tests>=best_first_max_nb_tests)) ||
			      (nodesize==1) ||
			      (nodesize<min_node_size) || 
			      stop_splitting_criterio()))) {
      /* c'est une feuille */
      if (true_best_first && (goal_type==CLASSIFICATION || goal_type==REGRESSION || goal_type==MULTIREGR)) {
	summarize_vector(current_learning_set, start, end);
      }
      prediction[node]=make_leaf_prediction();
      index_stack_open_nodes--;

    } else {

      if (!true_best_first) {
	/* on recherche le meilleur split */
	find_a_split(current_learning_set, start, end);
      } else {
	/* on reprend l'info qu'on avait stockée */
	best_attribute=tbf_best_attribute[index_stack_open_nodes];
	best_threshold=tbf_best_threshold[index_stack_open_nodes];
	best_threshold_score=tbf_best_threshold_score[index_stack_open_nodes];
	best_threshold_info=tbf_best_threshold_info[index_stack_open_nodes];
      }
      
      /*
	printf("best_attribute=%d, best_threshold=%f, best_threshold_score=%e best_info=%e\n",best_attribute, best_threshold.f,
	       best_threshold_score, best_threshold_info);
	fflush(stdout);      
      */
	
      /* le resultat est dans best_threshold_score), best_attribute */
      if ((true_best_first && tbf_not_significant[index_stack_open_nodes])
	  ||(!true_best_first && not_significant_test())) {
	/* c'est une feuille */
	/* printf("feuille");fflush(stdout); */
	prediction[node]=make_leaf_prediction();
	index_stack_open_nodes--;
      } else {
	/* on separe les objets sur base */
	int left,right,borne;

	borne=separate_ls_vector(best_attribute, best_threshold, current_learning_set,start,end);

	/* /\* if ((borne<=start) || (borne>end)) { *\/ */
 	/*
	printf("start=%d, borne=%d, end=%d\n", start, borne, end);fflush(stdout);
	printf("best_attribute=%d, best_threshold=%f, best_threshold_score=%e best_info=%e\n",best_attribute, best_threshold.f,
	       best_threshold_score, best_threshold_info);
	fflush(stdout);	
	*/

	nb_tests++;

	/* on cree deux nouveaux noeuds */
	index_nodes++; left=index_nodes;
	index_nodes++; right=index_nodes;
	prediction[left]=-1; prediction[right]=-1;
	tested_attribute[left]=-1; tested_attribute[right]=-1;
	left_successor[left]=-1; left_successor[right]=-1;
	right_successor[left]=-1; right_successor[right]=-1;

	/* on met a jour le noeud courant */
	threshold[node]=best_threshold;
	tested_attribute[node]=best_attribute;
	left_successor[node]=left-node;
	right_successor[node]=right-node;
	
	/* on place les nouveaux noeuds sur la pile */
	/* La manière de les placer depend de true_best_first et best_first */

	if (true_best_first) { /* on trie les noeuds par ordre décroissant du réduction de variance (ou info) */
	  int i; int ns; int stsp;
	  float pr_val;

	  /* left successor */
	  ns=borne-start;
	  summarize_vector(current_learning_set, start, borne-1);

	  if (goal_type!=MULTIREGR)
	    node_size[node]=table_score[0][0]; /* valeurs manquantes ? */
	  else
	    node_size[node]=table_score_multiregr[0][0];

	  stsp=((ns==1) || (ns<min_node_size) || stop_splitting_criterio());
	  if (!stsp) {
	    find_a_split(current_learning_set, start, borne-1);	  
	    pr_val=best_threshold_info;
	  } else {
	    pr_val=-1.0;
	  }
	  
	  i=index_stack_open_nodes-1;
	  while ((i>=0) && (tbf_best_threshold_info[i]>pr_val)) {
	      /* on pousse le noeud jusqu'à sa place */
	      stack_open_nodes[i+1][0]=stack_open_nodes[i][0];
	      stack_open_nodes[i+1][1]=stack_open_nodes[i][1];
	      stack_open_nodes[i+1][2]=stack_open_nodes[i][2];

	      tbf_best_attribute[i+1]=tbf_best_attribute[i];
	      tbf_best_threshold_score[i+1]=tbf_best_threshold_score[i];
	      tbf_best_threshold_info[i+1]=tbf_best_threshold_info[i];
	      tbf_best_threshold[i+1]=tbf_best_threshold[i];
	      tbf_not_significant[i+1]=tbf_not_significant[i];
	      tbf_stop_splitting[i+1]=tbf_stop_splitting[i];

	      i--;
	  } 
	  stack_open_nodes[i+1][0]=left;
	  stack_open_nodes[i+1][1]=start;
	  stack_open_nodes[i+1][2]=borne-1;

	  tbf_stop_splitting[i+1]=stsp;
	  tbf_best_threshold_info[i+1]=pr_val;
	  if (!stsp) {
	    tbf_best_attribute[i+1]=best_attribute;
	    tbf_best_threshold_score[i+1]=best_threshold_score;
	    tbf_best_threshold[i+1]=best_threshold;
	    tbf_not_significant[i+1]=not_significant_test();
	  }

	  /* right successor */
	  index_stack_open_nodes++;
	  ns=end-borne+1;
	  summarize_vector(current_learning_set, borne, end);

	  if (goal_type!=MULTIREGR)
	    node_size[node]=table_score[0][0]; /* valeurs manquantes ? */
	  else
	    node_size[node]=table_score_multiregr[0][0];

	  stsp=((ns==1) || (ns<min_node_size) || stop_splitting_criterio());
	  if (!stsp) {
	    find_a_split(current_learning_set, borne, end);	  
	    pr_val=best_threshold_info;
	  } else {
	    pr_val=-1.0;
	  }
	  
	  i=index_stack_open_nodes-1;
	  while ((i>=0) && (tbf_best_threshold_info[i]>pr_val)) {
	      /* on pousse le noeud jusqu'à sa place */
	      stack_open_nodes[i+1][0]=stack_open_nodes[i][0];
	      stack_open_nodes[i+1][1]=stack_open_nodes[i][1];
	      stack_open_nodes[i+1][2]=stack_open_nodes[i][2];

	      tbf_best_attribute[i+1]=tbf_best_attribute[i];
	      tbf_best_threshold_score[i+1]=tbf_best_threshold_score[i];
	      tbf_best_threshold_info[i+1]=tbf_best_threshold_info[i];
	      tbf_best_threshold[i+1]=tbf_best_threshold[i];
	      tbf_not_significant[i+1]=tbf_not_significant[i];
	      tbf_stop_splitting[i+1]=tbf_stop_splitting[i];
	      
	      i--;
	  } 
	  stack_open_nodes[i+1][0]=right;
	  stack_open_nodes[i+1][1]=borne;
	  stack_open_nodes[i+1][2]=end;
	  
	  tbf_stop_splitting[i+1]=stsp;
	  tbf_best_threshold_info[i+1]=pr_val;
	  if (!stsp) {
	    tbf_best_attribute[i+1]=best_attribute;
	    tbf_best_threshold_score[i+1]=best_threshold_score;
	    tbf_best_threshold[i+1]=best_threshold;
	    tbf_not_significant[i+1]=not_significant_test();
	  }

	} else if (best_first) { /* on trie les noeuds par ordre de priorite. */
	  int i;
	  float pr_val;

	  /* left successor */
	  summarize_vector(current_learning_set, start, borne-1);
	  pr_val=best_first_node_priority_function();
	  i=index_stack_open_nodes-1;
	  while ((i>=0) && (priority_open_nodes[i]>pr_val)) {
	      /* on pousse le noeud jusqu'à sa place */
	      stack_open_nodes[i+1][0]=stack_open_nodes[i][0];
	      stack_open_nodes[i+1][1]=stack_open_nodes[i][1];
	      stack_open_nodes[i+1][2]=stack_open_nodes[i][2];
	      priority_open_nodes[i+1]=priority_open_nodes[i];
	      i--;
	  } 
	  stack_open_nodes[i+1][0]=left;
	  stack_open_nodes[i+1][1]=start;
	  stack_open_nodes[i+1][2]=borne-1;
	  priority_open_nodes[i+1]=pr_val;

	  /* right successor */
	  index_stack_open_nodes++;
	  summarize_vector(current_learning_set, borne, end);
	  pr_val=best_first_node_priority_function();
	  i=index_stack_open_nodes-1;
	  while ((i>=0) && (priority_open_nodes[i]>pr_val)) {
	      /* on pousse le noeud jusqu'à sa place */
	      stack_open_nodes[i+1][0]=stack_open_nodes[i][0];
	      stack_open_nodes[i+1][1]=stack_open_nodes[i][1];
	      stack_open_nodes[i+1][2]=stack_open_nodes[i][2];
	      priority_open_nodes[i+1]=priority_open_nodes[i];
	      i--;
	  } 
	  stack_open_nodes[i+1][0]=right;
	  stack_open_nodes[i+1][1]=borne;
	  stack_open_nodes[i+1][2]=end;
	  priority_open_nodes[i+1]=pr_val;


	} else { /* pas de best_first, on les met betement sur la pile */

	  stack_open_nodes[index_stack_open_nodes][0]=left;
	  stack_open_nodes[index_stack_open_nodes][1]=start;
	  stack_open_nodes[index_stack_open_nodes][2]=borne-1;
	  index_stack_open_nodes++;
	  stack_open_nodes[index_stack_open_nodes][0]=right;
	  stack_open_nodes[index_stack_open_nodes][1]=borne;
	  stack_open_nodes[index_stack_open_nodes][2]=end;

	}	  
      }     
    }
  }
  return tree;
}

/* SORT AND PRE-SORT OF SAMPLES */
/* ---------------------------- */

/* algorithme de tri et de rearrangement du vecteur */

/* 1) tout est fait localement */

/* separe le vecteur en fonction du test (version normale sans pretri) */

int separate_ls_vector_local(int best_attribute, union threshold_type best_threshold, int *ls_vector, int start, int end) {
  
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
      int temp;
      temp=ls_vector[start];
      ls_vector[start]=ls_vector[end];
      ls_vector[end]=temp;
      start++;
    }
  }
  /* ici, on a start=end, on renvoie la borne */

  if (check_test(best_attribute,getattval(object_mapping[ls_vector[start]],best_attribute), best_threshold))
    return (start+1);
  else
    return start;
}

/* fonction de tri (from the numerical recipes in C) */

#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp
#define M_QS 7
#define QUICK_SORT_STACK_SIZE 50
#define VAL(a) getattval(object_mapping[a],att)

/* Here M is the size of subarrays sorted by straight insertion and NSTACK is the required auxiliary storage. */

unsigned long *istack[QUICK_SORT_STACK_SIZE];

void quicksort_ls_vector(int *ls_vector, int start, int end, int att) {

  /* Sorts an array arr[1..n] into ascending numerical order using the Quicksort algorithm. n is input; 
     arr is replaced on output by its sorted rearrangement. */

  int i,ir=end,j,k,l=start;
  int istack[QUICK_SORT_STACK_SIZE];
  int jstack=-1,o;
  int temp;
  float a;
  
  for (;;) {
    /* Insertion sort when subarray small enough.*/ 
    if (ir-l < M_QS) {
      for (j=l+1;j<=ir;j++) {
	o=ls_vector[j];
	a=VAL(o);
	for (i=j-1;i>=l;i--) {
	  if (VAL(ls_vector[i]) <= a)
	    break;
	  ls_vector[i+1]=ls_vector[i];
	}
	ls_vector[i+1]=o; 
      } 
      if (jstack == -1) 
	break; 
      ir=istack[jstack--];  /* Pop stack and begin a new round of partitioning. */ 
      l=istack[jstack--]; 
    } else { 
      k=(l+ir) >> 1; /* Choose median of left, center, and right elements as partitioning element a. */
                     /* Also rearrange so that a[l]<=a[l+1]<=a[ir]. */
      if (VAL(ls_vector[l]) > VAL(ls_vector[ir])) {
	SWAP(ls_vector[l],ls_vector[ir]);
      }
      if (VAL(ls_vector[l+1]) > VAL(ls_vector[ir])) {
	SWAP(ls_vector[l+1],ls_vector[ir]);
      }
      if (VAL(ls_vector[l]) > VAL(ls_vector[l+1])) {
	SWAP(ls_vector[l],ls_vector[l+1]);
      }
      i=l+1; /* Initialize pointers for partitioning. */
      j=ir;
      o=ls_vector[l+1];
      a=VAL(o); /* Partitioning element. */ 
      for (;;) { /* Beginning of innermost loop.*/ 
	do i++; while (VAL(ls_vector[i]) < a); /* Scan up to  nd element > a. */ 
	do j--; while (VAL(ls_vector[j]) > a); /* Scan down to  nd element < a. */ 
	if (j < i) 
	  break; /* Pointers crossed. Partitioning complete. */
	SWAP(ls_vector[i],ls_vector[j]); /* Exchange elements. */ 
      } /* End of innermost loop. */
      ls_vector[l+1]=ls_vector[j]; /* Insert partitioning element. */
      ls_vector[j]=o;
      jstack += 2; /* Push pointers to larger subarray on stack, process smaller subarray immediately. */
      if (jstack > QUICK_SORT_STACK_SIZE) {
	printf("Stack too small in quicksort.\n");
	return;
      }
      if (ir-i+1 >= j-l) { 
	istack[jstack]=ir; istack[jstack-1]=i; ir=j-1; 
      } else { 
	istack[jstack]=j-1; istack[jstack-1]=l; l=i;
      }
    }
  }
}

/* 2) tri par projection: pretrie l'echantillon et ensuite trie les sous-ensembles
 *    en les projetant sur l'echantillon complet */

int *sorted_matrix; /* version fixe */
int sorted_matrix_size=0;
int sorted_matrix_nb_obj=0;
int sorted_matrix_nb_att=0;

int attribute_position[MAX_NUMBER_OF_ATTRIBUTES]; /* position des attributs dans la table sorted_matrix */

int *object_mark_cells;
int mark_cells_number=0;

void prepare_sorted_matrix(int *ls_vector, int length_ls_vector) {
  int size,i,j,pos, pos_att, nb_num_att=0;

  /* count the number of numerical attributes */
  for (i=0; i<nb_attributes; i++) {
    if (NUMERICALp(attribute_vector[i]))
      nb_num_att++;
  }
  if (nb_num_att==0) 
    return;

  /* creation de la matrice si necessaire */
  size=length_ls_vector*nb_num_att;
  if (sorted_matrix_size<size) {
    if (sorted_matrix_size!=0) 
      MyFree((int *)sorted_matrix);
    sorted_matrix=(int *)MyMalloc((size_t)size*sizeof(int));
    if (!sorted_matrix) {
      printf("impossible d'allouer de la memoire dans prepare_sorted_matrix\n");
      exit(0);
    }
    sorted_matrix_size=size;
  }
  sorted_matrix_nb_obj=length_ls_vector;
  sorted_matrix_nb_att=nb_num_att;

  /* on recopie l'ensemble d'apprentissage a toutes les lignes de la matrice et on le trie */
  if (print_result) {
    printf("Building the global matrix for sorting subsets (%d num.att. among %d)...\n", nb_num_att, nb_attributes);
  }

  pos=0; pos_att=0;
  for (i=0; i<nb_attributes; i++) {
    if (NUMERICALp(attribute_vector[i])) {
  
      if (print_result) {
	printf("%d...", attribute_vector[i]);
	fflush(stdout);
      }
      
      for (j=0; j<length_ls_vector; j++) {
	sorted_matrix[pos]=ls_vector[j];
	pos++;
      }    
      /* on trie */
      quicksort_ls_vector(sorted_matrix, pos_att*length_ls_vector, pos-1, attribute_vector[i]);
      /* on fixe la position */
      attribute_position[attribute_vector[i]]=pos_att;
      
      pos_att++;
    }
  }
  if (print_result) printf("\n");

  /* on cree le vecteur de marqueur (de la taille de la BD)... */
  if (mark_cells_number<length_ls_vector) {
    if (mark_cells_number!=0)
      MyFree((int *)object_mark_cells);
    object_mark_cells=(int *)MyMalloc((size_t)length_ls_vector*sizeof(int));
    if (!object_mark_cells) {
      printf("impossible d'allouer de la memoire dans prepare_sorted_matrix\n");
      exit(0);
    }
    mark_cells_number=length_ls_vector;
  }
  
  /* ...qu'on initialise */
  for (i=0; i<length_ls_vector; i++)
    object_mark_cells[i]=0;
  printf("ok done\n");
  fflush(stdout);
}

/* premiere version qui utilise la matrice sorted_matrix directement */
/* ne permet pas de gagner grand-chose */

void sort_ls_vector_by_projection(int *ls_vector, int start, int end, int att) {
  int i, j, pos, s, mc;
  int size_v=(end-start+1);

  /* si la taille du vecteur est trop petite, on utilise quicksort */
  if ((size_v*size_v)<sorted_matrix_nb_obj) {
    quicksort_ls_vector(ls_vector, start, end, att);
    return;
  }

  /* on marque les objets du ls */
  for (i=start; i<=end; i++) {
    object_mark_cells[ls_vector[i]]++;
  }
  
  /* on les recherche dans tous les objets du tableau trie */
  pos=attribute_position[att]*sorted_matrix_nb_obj;
  s=start;
  for (i=0; i<sorted_matrix_nb_obj; i++) {
    if ((mc=object_mark_cells[sorted_matrix[pos]])!=0) {
      for (j=0; j<mc; j++) { /* logiquement, cette boucle n'est plus nécessaire avec les poids */
	ls_vector[s]=sorted_matrix[pos];
	s++;
      }
      /* on demarque l'objet */
      object_mark_cells[sorted_matrix[pos]]=0;
    }
    pos++;
  }  
}

/* 3) tri implicite par rearrangement de la matrice sorted_matrix:
 *    on ne trie pas vraiment, on rearrange la matrice en fonction des tests successifs.
 *    Pour le bagging, necessite deux versions de la sorted_matrix.
 */

int *temp_sorted_matrix; /* version triee temporaire pour la construction */
int temp_sorted_matrix_nb_obj=0;
int temp_sorted_matrix_size=0;

void make_sorted_matrix_copy(int *ls_vector, int length_ls_vector) {
  int i,j,k,pos,mc,s;
  int size=length_ls_vector*nb_attributes;
  
  /* construit la matrice si necessaire */
  if (temp_sorted_matrix_size<size) {
    if (temp_sorted_matrix_size!=0)
      MyFree((int *)temp_sorted_matrix);
    temp_sorted_matrix=(int *)MyMalloc((size_t)size*sizeof(int));
    if (!temp_sorted_matrix) {
      printf("impossible d'allouer de la memoire dans make_sorted_matrix_copy\n");
      exit(0);
    }
    temp_sorted_matrix_size=size;
  }
  temp_sorted_matrix_nb_obj=length_ls_vector;

  /* marque les objets du LS */
  for (i=0; i<length_ls_vector; i++) {
    object_mark_cells[ls_vector[i]]++;
  }
  
  /* remplit la matrice a partir de sorted_matrix */
  pos=0; s=0;
  for (j=0; j<sorted_matrix_nb_att; j++) {
    for (i=0; i<global_learning_set_size; i++) {
      if ((mc=object_mark_cells[sorted_matrix[pos]])!=0) {
	for (k=0; k<mc; k++) { /* cette boucle n'est pas nécessaire avec les poids */
	  temp_sorted_matrix[s]=sorted_matrix[pos];
	  s++;
	}
      }
      pos++;
    }    
  }
  
  /* demarque les objets */
  for (i=0; i<length_ls_vector; i++)
    object_mark_cells[ls_vector[i]]=0;
}

void sort_ls_vector_by_copy(int *ls_vector, int start, int end, int att) {
  int i,pos;
  pos=attribute_position[att]*temp_sorted_matrix_nb_obj+start;
  
  /*  printlsvector(ls_vector, start, end); */

  for (i=start; i<=end; i++) {
    ls_vector[i]=temp_sorted_matrix[pos];
    pos++;
  }
    
}

/* rearrange la matrice temporaire en fonction du test */

int rearrange_ls_vector(int best_attribute, union threshold_type best_threshold, int *ls_vector, int start, int end) {
  int i, j, middle_pos=start, pos_matrix, pos;
  
  /* on marque les objets qui vont a droite du seuil */
  for (i=start; i<=end; i++) {
    if (check_test(best_attribute,
		   getattval(object_mapping[ls_vector[i]],best_attribute), 
		   best_threshold)) {
      object_mark_cells[ls_vector[i]]=1;
      middle_pos++;
    }
  }
  
  /* on rearrange la table pour tous les attributs */
  pos_matrix=0;
  for (i=0; i<sorted_matrix_nb_att; i++) {
    int pos_left=start, pos_right=middle_pos, pos;
    
    /* on recopie dans ls_vector les objets de temp_sorted_matrix en
     * les repartissant correctement. Ensuite, on copie ls_vector
     * dans temp_sorted_matrix (ls_vector est utilise comme vecteur temporaire).
     */
    pos=pos_matrix+start;
    for (j=start; j<=end; j++) {
      int o=temp_sorted_matrix[pos];
      if (object_mark_cells[o]==1) {
	ls_vector[pos_left]=o;
	pos_left++;
      } else {
	ls_vector[pos_right]=o;
	pos_right++;
      }
      pos++;
    }
    pos=pos_matrix+start;
    for (j=start; j<=end; j++) {
      temp_sorted_matrix[pos]=ls_vector[j];
      pos++;
    }
    
    pos_matrix+=temp_sorted_matrix_nb_obj;
  }

  /* on remet a zero les object_mark_cells et on separe les elements du LS sur base
   * de la matrice temp_sorted_matrix */
  pos=attribute_position[best_attribute]*temp_sorted_matrix_nb_obj+start;
  for (i=start; i<=end; i++) {
    ls_vector[i]=temp_sorted_matrix[pos];
    object_mark_cells[ls_vector[i]]=0;
    pos++;
  }
  /*
  printlsvector(ls_vector, start, end);
  for (i=start; i<=end; i++) {
    printf("%d ", object_mark_cells[ls_vector[i]]);
  }
  printf("\n"); fflush(stdout);
  */
  /*
      pos=attribute_position[best_attribute]*temp_sorted_matrix_nb_obj+start;
  for (i=start; i<=end; i++) {
    printf("O%d A%f ", temp_sorted_matrix[pos], getattval(temp_sorted_matrix[pos],best_attribute));
    pos++;
  }
  printf("\n");
  fflush(stdout);
  printlsvector(ls_vector, start, end);
  printlsvectoratt(ls_vector, start, end, best_attribute);
  */
  return middle_pos;
}

/* DISPATCH ON ATTRIBUTE TYPE */
/*----------------------------*/

void find_a_threshold(int att, int *ls_vector, int start, int end) {
  /* simple dispatch sur le type de l'attribut */
  if (NUMERICALp(att)) {
    find_a_threshold_num(att, ls_vector, start, end);
  } else if (SYMBOLICp(att)) {
    find_a_threshold_symb(att, ls_vector, start, end);
  }
}

/* CHECK A TEST */
/* ------------ */

int check_test(int att, float val, union threshold_type threshold) {
  if (NUMERICALp(att)) {
    return (val<threshold.f);
  } else {
    return (BITN(threshold,(int)val));
  }
}

/* ARBRES CLASSIQUES */
/* ----------------- */

/* GENERAL */

/* find_the_best_split */
/* recherche le meilleur split sur tous les attributs */

int print_detailled_result=0;

/* il faudrait peut-être randomizer le choix de l'attribut lorsque les scores
 * sont identiques. Sinon, il y a un biais pour le premier attribut
 * Peut-être simplement randomiser l'ordre des attributs d'une itération à l'autre.
 * Au mieux au sein de chaque noeud.
 */

void find_the_best_split(int *ls_vector, int start, int end) {
  int i;

  best_attribute=-1;
  best_threshold_score=-1.0;
  best_threshold_info=-1.0;

  if (print_detailled_result) {
    printf("Node %d (s=%d -> e=%d)\n", stack_open_nodes[index_stack_open_nodes][0], start, end);
    fflush(stdout);
  }

  for(i=0; i<nb_attributes; i++) { /* on considere tous les attributs */

    if (print_detailled_result) {
      if ((i%100)==0) {
	printf("%d...",i);
	fflush(stdout);
      }
    }
    find_a_threshold(attribute_vector[i], ls_vector, start, end);

    if (print_detailled_result) {
      printf("att %d, threshold %f, score %e\n", attribute_vector[i], current_threshold.f, current_threshold_score);
      fflush(stdout);
    }
    
    if ((current_threshold_score>=0.0) && (current_threshold_score>best_threshold_score)) {
      best_threshold_score=current_threshold_score;
      best_threshold_info=current_threshold_info; /* utile seulement pour la classification avec score log */
      best_threshold=current_threshold;
      best_attribute=attribute_vector[i];
    }
  }
  
  if (print_detailled_result) {
    printf("\n");
    printf("best split: Att %d - Score %e \n", best_attribute, best_threshold_score); 
    fflush(stdout);
    fflush(stdout);
  }

}

/* CLASSIFICATION */

/* calcul de score a partir de la table */

SCORE_TYPE  hs; /* variable partagee par compute_classification_score et
		 * stop_splitting
		 */
SCORE_TYPE info;

/* METHODE ULG (LWH) */

/* score base sur l'entropie de shannon */
SCORE_TYPE compute_classification_score_from_table_log() {
  int i,j;
  SCORE_TYPE tmp, h, ht;

  /* complete la table */
  for(i=0; i<=nb_classes; i++) {
    table_score[2][i]=table_score[0][i]-table_score[1][i];
  }

  /* calcule l'entropie du test */
  ht=0.0e0;
  
  for (i=1; i<=2; i++) {
    tmp=table_score[i][0];
    if (tmp>0)
      ht+=(tmp*log(tmp));
  }
  ht=log(table_score[0][0])-ht/table_score[0][0];

  /* calcule l'entropie conjointe */
  h=table_score[0][0]*log(table_score[0][0]);
  for (i=1; i<=nb_classes; i++) {
    for (j=1; j<=2; j++) {
      tmp=table_score[j][i];
      if (tmp>0)
	h-=(tmp*log(tmp));
    }
  }
  h/=table_score[0][0];
  info=hs+ht-h;

  /*
    printf("hs=%e, ht=%e, h=%e\n",hs,ht,h);
    printtablescore();
  */

  return (2.0*(1.0-(h/(hs+ht))));
}

/* stop splitting criterio pour la classification */

int stop_splitting_criterio_classification_log() {
  int i;
  hs=0.0e0;

  for(i=1; i<=nb_classes; i++) {
    if (table_score[0][i]>0.0)
      hs+=(table_score[0][i]*log(table_score[0][i]));
  }
  hs=log(table_score[0][0])-hs/table_score[0][0];

  return (hs<=h_min);
}

/* best first priority value */
/* compute the total shannon entropy in the current learning subsample */

SCORE_TYPE best_first_node_priority_function_class_log() {
  int i;
  float h=0.0;
  
  for(i=1; i<=nb_classes; i++) {
    if (table_score[0][i]>0.0)
      h+=(table_score[0][i]*log(table_score[0][i]));
  }
  h=log(table_score[0][0])-h/table_score[0][0];

  return (table_score[0][0]*h);
}

/* test de significativite pris de Wehenkel, automatic learning for power systems */

float alfa_dt=1.0;

int not_significant_test_classification_log() {
  int i, nb_c=0, ddl;

  for (i=1; i<=nb_classes; i++) {
    if (table_score[0][i]>0.0)
      nb_c++;
  }
  ddl=nb_c-1; /* toujours 2 issues aux tests */

  return (2*table_score[0][0]*best_threshold_info <= chi_square(ddl, 1.0-alfa_dt));
}

/* METHODE CART */

/* score base sur l'entropie gini (normalise) */

SCORE_TYPE hc;

SCORE_TYPE compute_classification_score_from_table_gini() {
  int i,j;
  SCORE_TYPE ht, hct, htc;
  
  /* complete la table */
  for(i=0; i<=nb_classes; i++) {
    table_score[2][i]=table_score[0][i]-table_score[1][i];
  }

  /* hc a deja ete calcule */
  /* ... */

  /* calcule l'entropie du test */
  ht=0.0;
  for (i=1; i<=2; i++) {
    ht+=(table_score[i][0]*table_score[i][0]);
  }
  ht=2.0*(1.0-ht/(table_score[0][0]*table_score[0][0]));

  /* calcule des entropies conditionnelles */
  /* hct */
  hct=0.0;
  for (j=1; j<=2; j++) {
    SCORE_TYPE sum=0.0;
    for (i=1; i<=nb_classes; i++) {
      sum+=(table_score[j][i]*table_score[j][i]);
    }
    hct+=(sum/table_score[j][0]);
  }
  hct=2.0*(1.0-hct/table_score[0][0]);
  
  /* htc */
  htc=0.0;
  for (i=1; i<=nb_classes; i++) {
    SCORE_TYPE sum=0.0;
    if (table_score[0][i]!=0.0) {
      for (j=1; j<=2; j++) {
	sum+=(table_score[j][i]*table_score[j][i]);
      }
      htc+=(sum/table_score[0][i]);
    }
  }
  htc=2.0*(1.0-htc/table_score[0][0]);

  /* printf("hc=%f, ht=%f, hct=%f, htc=%f\n",hc,ht,hct,htc); */
  return ((ht-htc+hc-hct)/(ht+hc));

}

SCORE_TYPE compute_classification_score_from_table_gini_unnorm() {
  int i,j;
  SCORE_TYPE hct;
  
  /* complete la table */
  for(i=0; i<=nb_classes; i++) {
    table_score[2][i]=table_score[0][i]-table_score[1][i];
  }

  /* hc a deja ete calcule */
  /* ... */

  /* calcule l'entropie du test */
  /*
  ht=0.0;
  for (i=1; i<=2; i++) {
    ht+=(table_score[i][0]*table_score[i][0]);
  }
  ht=2.0*(1.0-ht/(table_score[0][0]*table_score[0][0]));
  */
  /* calcule des entropies conditionnelles */
  /* hct */
  hct=0.0;
  for (j=1; j<=2; j++) {
    SCORE_TYPE sum=0.0;
    for (i=1; i<=nb_classes; i++) {
      sum+=(table_score[j][i]*table_score[j][i]);
    }
    hct+=(sum/table_score[j][0]);
  }
  hct=2.0*(1.0-hct/table_score[0][0]);
  
  /* htc */

  /*
  htc=0.0;
  for (i=1; i<=nb_classes; i++) {
    SCORE_TYPE sum=0.0;
    if (table_score[0][i]!=0.0) {
      for (j=1; j<=2; j++) {
	sum+=(table_score[j][i]*table_score[j][i]);
      }
      htc+=(sum/table_score[0][i]);
    }
  }
  htc=2.0*(1.0-htc/table_score[0][0]);
  */

  /* printf("hc=%f, ht=%f, hct=%f, htc=%f\n",hc,ht,hct,htc); */
  /* return (ht-htc+hc-hct); */
  return ((hc-hct)/hc);
}

int stop_splitting_criterio_classification_gini() {
  int i;

  hc=0.0;
  for(i=1; i<=nb_classes; i++) {
    hc+=(table_score[0][i]*table_score[0][i]);
  }
  hc=2.0*(1.0-hc/(table_score[0][0]*table_score[0][0]));
  
  return (hc<h_min);
}

/* best first priority value */
/* compute the total gini entropy in the current learning subsample */

SCORE_TYPE best_first_node_priority_function_class_gini() {
  int i;
  float h=0.0;
  
  for(i=1; i<=nb_classes; i++) {
    h+=(table_score[0][i]*table_score[0][i]);
  }
  h=2.0*(1.0-h/(table_score[0][0]*table_score[0][0]));
  
  return (table_score[0][0]*h);
}


int not_significant_test_classification_gini() {
  return (best_threshold_score<0.0);
}

/* METHODE C4.5 */

/* information gain de Quinlan */

SCORE_TYPE compute_classification_score_from_table_C45() {
  int i,j;
  SCORE_TYPE tmp, h, ht;

  /* complete la table */
  for(i=0; i<=nb_classes; i++) {
    table_score[2][i]=table_score[0][i]-table_score[1][i];
  }

  /* calcule l'entropie du test */
  ht=0.0e0;
  
  for (i=1; i<=2; i++) {
    tmp=table_score[i][0];
    if (tmp>0)
      ht+=(tmp*log(tmp));
  }
  ht=log(table_score[0][0])-ht/table_score[0][0];

  /* calcule l'entropie conjointe */
  h=table_score[0][0]*log(table_score[0][0]);
  for (i=1; i<=nb_classes; i++) {
    for (j=1; j<=2; j++) {
      tmp=table_score[j][i];
      if (tmp>0)
	h-=(tmp*log(tmp));
    }
  }
  h/=table_score[0][0];
  info=hs+ht-h;

  return (info/ht);
}

/* en fait rien, il n'y a pas de stop-splitting dans C45 mais il faut bien calculer hs */

int stop_splitting_criterio_classification_C45() {
  int i;
  hs=0.0;

  for(i=1; i<=nb_classes; i++) {
    if (table_score[0][i]>0.0)
      hs+=(table_score[0][i]*log(table_score[0][i]));
  }
  hs=log(table_score[0][0])-hs/table_score[0][0];

  return (hs<h_min);
}

/* fait un resume de la repartition de classes */

void summarize_vector_clas(int *vector, int start, int end) {
  int i;
  SCORE_TYPE w;
  
  for (i=0; i<=nb_classes; i++) {
    table_score[0][i]=0.0;
  }
  for (i=start; i<= end; i++) {
    w=object_weight[vector[i]];
    table_score[0][0]+=w;
    table_score[0][getobjclass_learn(vector[i])+1]+=w;
  }  
}

void summarize_symb_att_clas(int att, int *vector, int start, int end) {
  int i,j;

  /* set to zero */
  for (i=0; i<NB_VAL(att); i++) {
    for (j=0; j<=nb_classes; j++) {
      table_score_symb[i][j]=0.0;
    }
  }

  /* fill the table with frequency */
  for (i=start; i<=end; i++) {
    SCORE_TYPE w=object_weight[vector[i]];
    int v=(int)getattval(object_mapping[vector[i]],att);
    int c=getobjclass_learn(vector[i]);
    table_score_symb[v][0]+=w;
    table_score_symb[v][c+1]+=w;
  }
}

/* construit un vecteur de proba a partir de la table de score */

int make_leaf_prediction_classification() {

  int i;
  index_prediction_values++;

  for (i=0; i<nb_classes; i++) {
    prediction_values[index_prediction_values][i] =(float)(table_score[0][i+1]/table_score[0][0]);
  }
  return index_prediction_values;
}

/* find_the_best_threshold_clas */
/* recherche le seuil qui maximise le score, version classique des arbres */

void find_the_best_threshold_clas(int att, int *ls_vector, int start, int end) {
  float old_val, new_val;
  SCORE_TYPE best_score=-1.0, best_info, current_score, w;
  float best_threshold;
  int st=start, i;

  /* on remet a zero la table */
  for (i=0; i<=nb_classes; i++) {
    table_score[1][i]=0.0;
  }

  /* on trie l'ensemble selon l'attribut */
  sort_ls_vector(ls_vector, start, end, att);

  /* on parcourt toutes les valeurs de seuils possibles */
  old_val=getattval(object_mapping[ls_vector[start]],att);
  for(st=start; st<end; st++) {
    w=object_weight[ls_vector[st]];
    table_score[1][getobjclass_learn(ls_vector[st])+1]+=w;
    table_score[1][0]+=w;
    if ((new_val=getattval(object_mapping[ls_vector[st+1]],att))!=old_val) { /* un nouveau seuil a considerer */
      current_score=compute_score_from_table();

      /*
	printf("att %d seuil entre %e et %e -> score %e\n", att, old_val, new_val, current_score);
	printtablescore();
	fflush(stdout);
      */

      if (current_score>best_score) {
	best_score=current_score;
	best_info=info;
	best_threshold=(double)(old_val+new_val)/(double)2.0;
	if (old_val>=best_threshold) /* probeme d'arrondi */
	  best_threshold=new_val;
      }
      old_val=new_val;
    }
  }
  if (best_score>=0.0) {
    /*    printf("att %d, thres %f, score %f\n", att, best_threshold, best_score); */
    current_threshold.f=best_threshold;
    current_threshold_score=best_score;
    current_threshold_info=best_info;
  } else {
    current_threshold_score=-1.0;
  }
}

/* idem mais avec randomization du seuil plutôt que de choisir la moyenne */

void find_the_best_threshold_clas_randomized(int att, int *ls_vector, int start, int end) {
  float old_val, new_val;
  SCORE_TYPE best_score=-1.0, best_info, current_score, w;
  float best_threshold1, best_threshold2;
  int st=start, i;

  /* on remet a zero la table */
  for (i=0; i<=nb_classes; i++) {
    table_score[1][i]=0.0;
  }

  /* on trie l'ensemble selon l'attribut */
  sort_ls_vector(ls_vector, start, end, att);

  /* on parcourt toutes les valeurs de seuils possibles */
  old_val=getattval(object_mapping[ls_vector[start]],att);
  for(st=start; st<end; st++) {
    w=object_weight[ls_vector[st]];
    table_score[1][getobjclass_learn(ls_vector[st])+1]+=w;
    table_score[1][0]+=w;
    if ((new_val=getattval(object_mapping[ls_vector[st+1]],att))!=old_val) { /* un nouveau seuil a considerer */
      current_score=compute_score_from_table();

      /*
	printf("att %d seuil entre %e et %e -> score %e\n", att, old_val, new_val, current_score);
	printtablescore();
	fflush(stdout);
      */

      if (current_score>best_score) {
	best_score=current_score;
	best_info=info;
	best_threshold1=old_val;
	best_threshold2=new_val;	  
	/* best_threshold=(double)(old_val+new_val)/(double)2.0; */
	/* if (old_val>=best_threshold) */ /* probeme d'arrondi */
	/*      best_threshold=new_val;
	  */
      }
      old_val=new_val;
    }
  }
  if (best_score>=0.0) {
    /* printf("att %d, thres %f, score %f\n", att, best_threshold, best_score); */
    /* on tire un seuil au hasard */
    current_threshold.f=best_threshold2-(best_threshold2-best_threshold1)*get_random_float();
    std::cout << "1390: " << "bt1: " << best_threshold1 << "bt2: " << best_threshold2 << "current: " << current_threshold.f << std::endl;
    if (current_threshold.f<=best_threshold1) /* probleme d'arrondi */
      current_threshold.f=best_threshold2;
    current_threshold_score=best_score;
    current_threshold_info=best_info;
  } else {
    current_threshold_score=-1.0;
  }
}

/* recherche la meilleure partition pour un attribut symbolique */
/* recherche exhaustive parmi toutes les partitions possibles (des */
/* valeurs presentes dans l'échantillon local) */

void find_the_best_threshold_symb_clas(int att, int *ls_vector, int start, int end) {
  int i, v, eff_v;
  int nb_val=NB_VAL(att);
  int nb_val_ls=0;
  union threshold_type current_subset, best_subset;
  SCORE_TYPE best_score=-1.0, best_info, current_score;

  /* on precalcule la table avec tous les frequences pour toutes les classes */
  summarize_symb_att_clas(att, ls_vector, start, end);

  /*
    printf("attribute=%d, nb_val=%d\n\n", att, nb_val);
    printtablescoresymb(nb_val, nb_classes+1);
  */

  /* Check that all elements do not belong to the same class */
  for (i=0; i<nb_val; i++) {
    if (table_score_symb[i][0]!=0) 
      nb_val_ls++;
  }
  if (nb_val_ls==1) { /* all objects have the same value of this attribute */
    current_threshold_score=-1.0;
    return;
  } 
  /*
    printf("nb_val_ls=%d\n", nb_val_ls);
    fflush(stdout);
  */

  /*
    printf("nb_of_subsets=%d\n", nb_of_subsets);
    fflush(stdout);
  */

  init_threshold_type(&current_subset);
  add1_threshold_type(&current_subset);
  
  do {
    /* fill the table score according to the current subset */
    for (i=0; i<=nb_classes; i++) {
      table_score[1][i]=0.0;
    }
    
    eff_v=0;
    for (v=0; v<nb_val; v++) {
      if (table_score_symb[v][0]!=0.0) {
	/* check bit eff_v in current_subset */
	if (BITN(current_subset, eff_v)) {
	  for (i=0; i<=nb_classes; i++) {
	    table_score[1][i]+=table_score_symb[v][i];
	  }
	}
	eff_v++;
      }
    }

    /* compute the score */
    current_score=compute_score_from_table();

    if (current_score>best_score) {
      best_score=current_score;
      best_info=info;
      best_subset=current_subset;
    }
    add1_threshold_type(&current_subset);
  } while (!(BITN(current_subset,(nb_val_ls-1))));
  
  if (best_score>=0.0) {
    current_threshold_score=best_score;
    current_threshold_info=best_info;
    /* translate current_subset into a proper subset */
    init_threshold_type(&current_threshold);
    eff_v=0;
    for (v=0; v<nb_val; v++) {
      if (table_score_symb[v][0]!=0.0) {
	if (BITN(best_subset, eff_v)) {
	  SET_BITN(current_threshold,v);
	}
	eff_v++;
      }
    }
  } else {
    current_threshold_score=-1.0;
  }
}

/* REGRESSION */

/* calcul de score a partir de la table */

SCORE_TYPE v_tot;

SCORE_TYPE compute_regression_score_from_table() {
  SCORE_TYPE y_tot_var, n_tot_var;

  /* complete la table */
  table_score[2][0]=table_score[0][0]-table_score[1][0];
  table_score[2][1]=table_score[0][1]-table_score[1][1];
  table_score[2][2]=table_score[0][2]-table_score[1][2];
  
  y_tot_var=fabs(table_score[1][2]-(table_score[1][1]*table_score[1][1])/table_score[1][0]);
  n_tot_var=fabs(table_score[2][2]-(table_score[2][1]*table_score[2][1])/table_score[2][0]);
  
  info=v_tot-(y_tot_var+n_tot_var);

  return (info/v_tot);

}

/* stop splitting criterio pour la regression */

SCORE_TYPE v_min=0.0;

int stop_splitting_criterio_regression() {
  v_tot=table_score[0][2]-(table_score[0][1]*table_score[0][1])/table_score[0][0];
  return ((v_tot/table_score[0][0])<=v_min);
}

SCORE_TYPE best_first_node_priority_function_regr() {
  return (table_score[0][2]-(table_score[0][1]*table_score[0][1])/table_score[0][0]);
}

float alfa_rt=1.0;

int not_significant_test_regression() {
  return (best_threshold_score<0.0);
}

/* fait un resume de la repartition de classes */

void summarize_vector_regr(int *vector, int start, int end) {
  int i;
  SCORE_TYPE w;
  
  table_score[0][0]=0.0;
  table_score[0][1]=0.0;
  table_score[0][2]=0.0;

  for (i=start; i<= end; i++) {
    float y=getobjy_learn(vector[i]);
    w=object_weight[vector[i]];
    /*
      printf("-%d %d %f", vector[i], object_mapping[vector[i]], w);
      fflush(stdout);
    */
    table_score[0][0]+=w; /* on n'utilise pas les poids ici */
    table_score[0][1]+=w*y;
    table_score[0][2]+=w*(y*y);
  }
}

/* fait un résumé de la répartition de classes d'un attribut symbolique */

void summarize_symb_att_regr(int att, int *vector, int start, int end) {
  int i;

  /* set to zero */
  for (i=0; i<NB_VAL(att); i++) {
    table_score_symb[i][0]=0.0;
    table_score_symb[i][1]=0.0;
    table_score_symb[i][2]=0.0;
  }

  /* fill the table with frequency */
  for (i=start; i<=end; i++) {
    float y=getobjy_learn(vector[i]);
    SCORE_TYPE w=object_weight[vector[i]];
    int v=(int)getattval(object_mapping[vector[i]],att);

    table_score_symb[v][0]+=w;
    table_score_symb[v][1]+=w*y;
    table_score_symb[v][2]+=w*(y*y);
  }
}

/* construit un vecteur de proba a partir de la table de score */

int make_leaf_prediction_regression() {

  index_prediction_values++;
  
  prediction_values[index_prediction_values][0]=(float)(table_score[0][1]/table_score[0][0]); /* valeur moyenne */
  prediction_values[index_prediction_values][1]=(float)((table_score[0][2]-(table_score[0][1]*table_score[0][1])/table_score[0][0])/table_score[0][0]); /* variance */
  
  return index_prediction_values;
}

/* find_the_best_threshold_regr */
/* recherche le seuil qui maximise le score, version classique des arbres */

void find_the_best_threshold_regr(int att, int *ls_vector, int start, int end) {
  float old_val, new_val;
  SCORE_TYPE best_score=-1.0, best_info, current_score, w;
  float best_threshold; 
  int st=start;

  /* initialisation de la table */
  table_score[1][0]=0.0;
  table_score[1][1]=0.0;
  table_score[1][2]=0.0;
  /* on trie l'ensemble selon l'attribut */
  sort_ls_vector(ls_vector, start, end, att);

  /* on parcourt toutes les valeurs de seuils possibles */
  old_val=getattval(object_mapping[ls_vector[start]],att);
  for(st=start; st<end; st++) {
    float y=getobjy_learn(ls_vector[st]);
    w=object_weight[ls_vector[st]];
    
    table_score[1][0]+=w;
    table_score[1][1]+=w*y;
    table_score[1][2]+=w*(y*y);

    if ((new_val=getattval(object_mapping[ls_vector[st+1]],att))!=old_val) { /* un nouveau seuil a considerer */
      
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
  if (best_score>=0.0) {
    current_threshold.f=best_threshold;
    current_threshold_score=best_score;
    current_threshold_info=best_info;
  } else {
    current_threshold_score=-1.0;
  }
}

void find_the_best_threshold_regr_randomized(int att, int *ls_vector, int start, int end) {
  float old_val, new_val;
  SCORE_TYPE best_score=-1.0, best_info, current_score, w;
  float best_threshold1, best_threshold2;
  int st=start;

  /* initialisation de la table */
  table_score[1][0]=0.0;
  table_score[1][1]=0.0;
  table_score[1][2]=0.0;
  /* on trie l'ensemble selon l'attribut */
  sort_ls_vector(ls_vector, start, end, att);

  /* on parcourt toutes les valeurs de seuils possibles */
  old_val=getattval(object_mapping[ls_vector[start]],att);
  for(st=start; st<end; st++) {
    float y=getobjy_learn(ls_vector[st]);
    w=object_weight[ls_vector[st]];
    
    table_score[1][0]+=w;
    table_score[1][1]+=w*y;
    table_score[1][2]+=w*(y*y);

    if ((new_val=getattval(object_mapping[ls_vector[st+1]],att))!=old_val) { /* un nouveau seuil a considerer */
      
      current_score=compute_score_from_table();
      if (current_score>best_score) {
	best_score=current_score;
	best_info=info;
	best_threshold1=old_val;
	best_threshold2=new_val;
	/* best_threshold=(old_val+new_val)/2.0; */
      }
      old_val=new_val;
    }
  }
  if (best_score>=0.0) {
    current_threshold.f=best_threshold2-(best_threshold2-best_threshold1)*get_random_float();
    std::cout << "1677: " << "bt1: " << best_threshold1 << "bt2: " << best_threshold2 << "current: " << current_threshold.f << std::endl;
    if (current_threshold.f<=best_threshold1) /* probleme d'arrondi */
      current_threshold.f=best_threshold2;
    current_threshold_score=best_score;
    current_threshold_info=best_info;
  } else {
    current_threshold_score=-1.0;
  }
}

/* recherche le meilleur subset sur un attribut symbolique */
/* recherche exhaustive (ok s'il y a très peu de valeurs) */

void find_the_best_threshold_symb_regr(int att, int *ls_vector, int start, int end) {
  int i, v, eff_v;
  int nb_val=NB_VAL(att);
  int nb_val_ls=0;
  union threshold_type current_subset, best_subset;
  SCORE_TYPE best_score=-1.0, best_info, current_score;

  /* on precalcule la table avec tous les frequences pour toutes les classes */
  summarize_symb_att_regr(att, ls_vector, start, end);

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
    /* fill the table score according to the current subset */
    table_score[1][0]=0.0;
    table_score[1][1]=0.0;
    table_score[1][2]=0.0;
      
    eff_v=0;
    for (v=0; v<nb_val; v++) {
      if (table_score_symb[v][0]!=0.0) {
	/* check bit eff_v in current_subset */
	if (BITN(current_subset, eff_v)) {
	  table_score[1][0]+=table_score_symb[v][0];
	  table_score[1][1]+=table_score_symb[v][1];
	  table_score[1][2]+=table_score_symb[v][2];
	}
	eff_v++;
      }
    }
 
    /* compute the score */
    current_score=compute_score_from_table();

    if (current_score>best_score) {
      best_score=current_score;
      best_info=info;
      best_subset=current_subset;
    }
    add1_threshold_type(&current_subset);
  } while (!(BITN(current_subset,(nb_val_ls-1))));
	 
  if (best_score>=0.0) {
    current_threshold_score=best_score;
    current_threshold_info=best_info;
    /* translate current_subset into a proper subset */
    init_threshold_type(&current_threshold);
    eff_v=0;
    for (v=0; v<nb_val; v++) {
      if (table_score_symb[v][0]!=0.0) {
	if (BITN(best_subset, eff_v)) {
	  SET_BITN(current_threshold,v);
	}
	eff_v++;
      }
    }    
  } else {
    current_threshold_score=-1.0;
  }
}

/******************************
 * EXTRA-TREES (clas et regr) *
 ******************************/

/* FIND_A_SPLIT_AT_RANDOM():
 * recherche un split aleatoire
 * Param:
 *  random_split_score_threshold: un seuil sur le score
 *  find_a_threshold: la fonction qui permet de trouver le seuil
 */

/* original algorithm (in my thesis) */

float random_split_score_threshold=0.1;

void find_a_split_at_random(int *ls_vector, int start, int end) {
  int remaining_att=nb_attributes;
  int random_att_pos, temp;

  best_attribute=-1;
  best_threshold_score=-1.0;
  best_threshold_info=-1.0;

  do {
    random_att_pos=get_random_integer(remaining_att);
    if (random_att_pos>=remaining_att) {
      printf("erreur dans le generateur de nombres aleatoires\n");
    }

    find_a_threshold(attribute_vector[random_att_pos], ls_vector, start, end);

    if ((current_threshold_score>=0.0) && (current_threshold_score<random_split_score_threshold)) {
      if (current_threshold_score>best_threshold_score) {
	best_threshold_score=current_threshold_score;
	best_threshold_info=current_threshold_info;
	best_threshold=current_threshold;
	best_attribute=attribute_vector[random_att_pos];
      }
    } else if (current_threshold_score>=0.0) {
      best_threshold_score=current_threshold_score;
      best_threshold_info=current_threshold_info;
      best_threshold=current_threshold;
      best_attribute=attribute_vector[random_att_pos];
    }
    /* swap les attributs dans la liste */
    remaining_att--;
    if (remaining_att!=0) {
      temp=attribute_vector[random_att_pos];
      attribute_vector[random_att_pos]=attribute_vector[remaining_att];
      attribute_vector[remaining_att]=temp;
    }
  } while ((current_threshold_score<random_split_score_threshold) && (remaining_att!=0));
}

/* deuxieme version: on prend le meilleur test parmi N tests aléatoires */
/* Probleme si l'attribut est constant dans l'echantillon, on ne va pas plus loin ici*/
/* alors que la methode precedente, elle, tire un autre attribut */
/* il faudrait tester si l'attribut est constant (current_threshold_score=-1.0) et */
/* dans ce cas, mettre l'attribut de cote et ne considerer que les autres attributs */

int nb_of_random_tests=1;

void find_a_split_at_random_2(int *ls_vector, int start, int end) {
  int random_att_pos, temp, remaining_att=nb_attributes;
  int nb_try=0;
 
  best_attribute=-1;
  best_threshold_score=-1.0;
  best_threshold_info=-1.0;
  nb_try=0;
 
  do {
    nb_try++;
    random_att_pos=get_random_integer(remaining_att);
    if (random_att_pos>=remaining_att) {
      printf("erreur dans le generateur de nombres aleatoires\n");
    }

    find_a_threshold(attribute_vector[random_att_pos], ls_vector, start, end);
    
    if (current_threshold_score<0.0) {
      /* l'attribut est constant dans l'échantillon */
      /* on le vire de la liste */
      remaining_att--;
      if (remaining_att!=0) {
	temp=attribute_vector[random_att_pos];
	attribute_vector[random_att_pos]=attribute_vector[remaining_att];
	attribute_vector[remaining_att]=temp;
      }
      nb_try--; /* ce test ne compte pas */
    } else if (current_threshold_score>best_threshold_score) {
      best_threshold_score=current_threshold_score;
      best_threshold_info=current_threshold_info;
      best_threshold=current_threshold;
      best_attribute=attribute_vector[random_att_pos];
    }

    /* swap les attributs dans la liste */
  } while((remaining_att!=0) && (nb_try<nb_of_random_tests));
}

/* troisième version: mixe les deux on cherche le meilleur test parmi N tests 
 * aléatoires et on s'arrête lorsqu'on trouve un test dont le score est supérieure
 * à un seuil donné
 */

long mean_nb_random_splits=0;
long nb_call_to_find_a_split=0;

void find_a_split_at_random_3(int *ls_vector, int start, int end) {
  int random_att_pos, temp, nb_try=0;
  int remaining_att=nb_attributes;

  best_attribute=-1;
  best_threshold_score=-1.0;
  best_threshold_info=-1.0;

  nb_try=0;
  do {
    nb_try++;
    random_att_pos=get_random_integer(remaining_att);
    if (random_att_pos>=remaining_att) {
      printf("erreur dans le generateur de nombres aleatoires\n");
    }

    find_a_threshold(attribute_vector[random_att_pos], ls_vector, start, end);

    if (current_threshold_score<0.0) {
      remaining_att--;
      if (remaining_att!=0) {
	temp=attribute_vector[random_att_pos];
	attribute_vector[random_att_pos]=attribute_vector[remaining_att];
	attribute_vector[remaining_att]=temp;
      }
      nb_try--; /* ce test ne compte pas */
    } else if (current_threshold_score>best_threshold_score) {
      best_threshold_score=current_threshold_score;
      best_threshold_info=current_threshold_info;
      best_threshold=current_threshold;
      best_attribute=attribute_vector[random_att_pos];
    }
  } while ((remaining_att!=0) && (best_threshold_score<random_split_score_threshold) && (nb_try<nb_of_random_tests));
  /* mean_nb_random_splits+=nb_try; */
  /* nb_call_to_find_a_split++; */
}

/* 4ieme version: Comme la precedente mais on ne cherche pas deux fois
 * sur le meme attribut
 */

void find_a_split_at_random_4(int *ls_vector, int start, int end) {
  int random_att_pos, temp, nb_try=0;
  int remaining_att=nb_attributes;

  best_attribute=-1;
  best_threshold_score=-1.0;
  best_threshold_info=-1.0;

  if (print_detailled_result) {
    printf("Node %d (s=%d -> e=%d)\n", stack_open_nodes[index_stack_open_nodes][0], start, end);
    fflush(stdout);
  }
  
  nb_try=0;
  do {
    nb_try++;
    random_att_pos=get_random_integer(remaining_att);
    if (random_att_pos>=remaining_att) {
      printf("erreur dans le generateur de nombres aleatoires\n");
    }

    find_a_threshold(attribute_vector[random_att_pos], ls_vector, start, end);

    if (print_detailled_result) {
      printf("att %d, threshold %f, score %e\n", attribute_vector[random_att_pos], current_threshold.f, current_threshold_score);
      fflush(stdout);
    }

    if (current_threshold_score>best_threshold_score) {
      best_threshold_score=current_threshold_score;
      best_threshold_info=current_threshold_info;
      best_threshold=current_threshold;
      best_attribute=attribute_vector[random_att_pos];
    }

    remaining_att--;
    if (remaining_att!=0) {
      temp=attribute_vector[random_att_pos];
      attribute_vector[random_att_pos]=attribute_vector[remaining_att];
      attribute_vector[remaining_att]=temp;
    }
    
    if (current_threshold_score<0.0)
      /* l'attribut etait constant, ca ne compte pas */
      nb_try--; 
    
  } while ((remaining_att!=0) && (best_threshold_score<random_split_score_threshold) && (nb_try<nb_of_random_tests));

  if (print_detailled_result) {
    printf("\n");
    printf("best split: Att %d - Score %e \n", best_attribute, best_threshold_score); 
    fflush(stdout);
    fflush(stdout);
  }
  /* mean_nb_random_splits+=nb_try; */
  /* nb_call_to_find_a_split++; */
}


/* CLASSIFICATION */

/* find_a_threshold_at_random */

/* recherche un seuil aleatoire sur un attribut dans l'ensemble donne
 * met le resultat dans current_threshold_score current_threshold_score
 */

void find_a_threshold_at_random_1_clas(int att, int *ls_vector, int start, int end) {
  int i; int nb_obj=end-start+1;
  float sum=getattval(object_mapping[ls_vector[start]],att);
  float sumsqr=sum*sum;
  float min=sum;
  float max=sum;
  SCORE_TYPE w;

  current_threshold_score=-1.0;

  /* calcule les stats sur l'attribut */
  for (i=start+1; i<=end; i++) {
    float val=getattval(object_mapping[ls_vector[i]],att);
    if (val<min)
      min=val;
    else if (val>max)
      max=val;
    sum+=val;
    sumsqr+=(val*val);
  }
  
  if (min==max) { /* toutes les valeurs sont egales */
    return;
  }
  
  /* tirage du seuil */
  sum/=nb_obj;
  sumsqr=sumsqr/nb_obj-sum*sum;
  if (sumsqr<0)
    sumsqr=0.0;
  else
    sumsqr=sqrt(sumsqr);

  current_threshold.f=sum+sumsqr*get_random_normal();

  if (current_threshold.f<=min)
    current_threshold.f=min+(max-min)/nb_obj;
  else if (current_threshold.f>max)
    current_threshold.f=max;

  /* calcul du score */
  for (i=0; i<=nb_classes; i++) {
    table_score[1][i]=0.0;
  }
  
  for (i=start; i<=end; i++) {
    if (getattval(object_mapping[ls_vector[i]],att)<current_threshold.f) {
      w=object_weight[ls_vector[i]];
      table_score[1][getobjclass_learn(ls_vector[i])+1]+=w;
      table_score[1][0]+=w;
    }
  }
  current_threshold_score=compute_score_from_table();
  current_threshold_info=info;
}

/* version encore plus rapide, en selectionnant une valeur au hasard
 *
 */

void find_a_threshold_at_random_2_clas(int att, int *ls_vector, int start, int end) {
  int i; int nb_obj=end-start+1;
  SCORE_TYPE w;
  
  current_threshold.f=getattval(object_mapping[ls_vector[start+get_random_integer(nb_obj)]],att);

  /* calcul du score */
  for (i=0; i<=nb_classes; i++) {
    table_score[1][i]=0.0;
  }
  
  for (i=start; i<=end; i++) {
    if (getattval(object_mapping[ls_vector[i]],att)<current_threshold.f) {
      w=object_weight[ls_vector[i]];
      table_score[1][getobjclass_learn(ls_vector[i])+1]+=w;
      table_score[1][0]+=w;
    }
  }

  if (table_score[1][0]!=0.0 && table_score[0][0]!=table_score[1][0]) {
    current_threshold_score=compute_score_from_table();
    current_threshold_info=info;
  } else {
    current_threshold_score=-1.0;
    current_threshold_info=-1.0;
  }
}

/* on prend un seuil au hasard entre min et max */

void find_a_threshold_at_random_3_clas(int att, int *ls_vector, int start, int end) {
  int i;
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
  
  /* tirage du seuil */
  current_threshold.f=max-(max-min)*get_random_float();

  /* calcul du score */
  for (i=0; i<=nb_classes; i++) {
    table_score[1][i]=0.0;
  }
  
  for (i=start; i<=end; i++) {
    if (getattval(object_mapping[ls_vector[i]],att)<current_threshold.f) {
      w=object_weight[ls_vector[i]];
      table_score[1][getobjclass_learn(ls_vector[i])+1]+=w;
      table_score[1][0]+=w;
    }
  }
  current_threshold_score=compute_score_from_table();
  current_threshold_info=info;
}

/* idem mais on se sert du tri pour trouver le min et le max */

void find_a_threshold_at_random_3_clas_sort(int att, int *ls_vector, int start, int end) {
  int i;
  float min,max;
  SCORE_TYPE w;

  current_threshold_score=-1.0;

  sort_ls_vector(ls_vector, start, end, att);

  min=getattval(object_mapping[ls_vector[start]],att);
  max=getattval(object_mapping[ls_vector[end]],att);
  
  if (min==max) { /* toutes les valeurs sont egales */
    return;
  }
  
  /* tirage du seuil */
  current_threshold.f=max-(max-min)*get_random_float();

  /* calcul du score */
  for (i=0; i<=nb_classes; i++) {
    table_score[1][i]=0.0;
  }
  
  i=start;
  do {
    w=object_weight[ls_vector[i]];
    table_score[1][getobjclass_learn(ls_vector[i])+1]+=w;
    table_score[1][0]+=w;
    i++;
  } while (getattval(object_mapping[ls_vector[i]],att)<current_threshold.f);

  current_threshold_score=compute_score_from_table();
  current_threshold_info=info;
}

/* version gaussienne avec retirage si on sort de l'intervalle */

int max_nb_try=1000;

void find_a_threshold_at_random_4_clas(int att, int *ls_vector, int start, int end) {
  int i; int nb_obj=end-start+1; int n;
  float sum=getattval(object_mapping[ls_vector[start]],att);
  float sumsqr=sum*sum;
  float min=sum;
  float max=sum;
  SCORE_TYPE w;

  current_threshold_score=-1.0;

  /* calcule les stats sur l'attribut */
  for (i=start+1; i<=end; i++) {
    float val=getattval(object_mapping[ls_vector[i]],att);
    if (val<min)
      min=val;
    else if (val>max)
      max=val;
    sum+=val;
    sumsqr+=(val*val);
  }
  
  if (min==max) { /* toutes les valeurs sont egales */
    return;
  }
  
  /* tirage du seuil */
  sum/=nb_obj;
  sumsqr=sumsqr/nb_obj-sum*sum;
  if (sumsqr<0)
    sumsqr=0.0;
  else
    sumsqr=sqrt(sumsqr);

  n=0;
  do {
    n++;
    current_threshold.f=sum+sumsqr*get_random_normal();
  } while (((current_threshold.f<=min) || (current_threshold.f>max)) && (n<=max_nb_try));

  if (n>max_nb_try) {
    printf("Trop de tir a cote, on le prend de maniere uniforme\n");
    fflush(stdout);
    current_threshold.f=max-(max-min)*get_random_float();
  }
  
  /* calcul du score */
  for (i=0; i<=nb_classes; i++) {
    table_score[1][i]=0.0;
  }
  
  for (i=start; i<=end; i++) {
    if (getattval(object_mapping[ls_vector[i]],att)<current_threshold.f) {
      w=object_weight[ls_vector[i]];
      table_score[1][getobjclass_learn(ls_vector[i])+1]+=w;
      table_score[1][0]+=w;
    }
  }
  current_threshold_score=compute_score_from_table();
  current_threshold_info=info;
}

/* version avec contrôle de la randomization du seuil */
/* on tire nb_of_random_thresholds au hasard et on prend le meilleur */

/* on suppose que nb_of_random_thresholds reste plus petit que 100 */
int nb_of_random_thresholds=1;
float random_thresholds[200];

void find_a_threshold_at_random_5_clas(int att, int *ls_vector, int start, int end) {
  float old_val, new_val, min, max;
  SCORE_TYPE best_score=-1.0, best_info, current_score, w;
  float best_threshold=-2.0, rt;
  int st=start, i, j, current_rt;

  /* on trie l'ensemble selon l'attribut */
  sort_ls_vector(ls_vector, start, end, att);

  min=getattval(object_mapping[ls_vector[start]],att);
  max=getattval(object_mapping[ls_vector[end]],att);
  
  if (min==max) { /* toutes les valeurs sont egales */
    current_threshold_score=-1.0;
    return;
  }
  
  /* on tire le nombre de seuils et on les place dans la liste (qu'on maintient triee) */
  for (j=0; j<nb_of_random_thresholds; j++) {
    rt=max-(max-min)*get_random_float();
    i=j-1;
    while ((i>=0) && (random_thresholds[i]>rt)) {
      random_thresholds[i+1]=random_thresholds[i];
      i--;
    }
    random_thresholds[i+1]=rt;
  }

  /* on parcourt toutes les valeurs de seuils possibles mais on n'evalue que
   * les seuils tires aleatoirement
   */

  /* on remet a zero la table */
  for (i=0; i<=nb_classes; i++) {
    table_score[1][i]=0.0;
  }

  current_rt=0;
  old_val=getattval(object_mapping[ls_vector[start]],att);

  for(st=start; ((st<end) && (current_rt<nb_of_random_thresholds)); st++) {
    w=object_weight[ls_vector[st]];
    table_score[1][getobjclass_learn(ls_vector[st])+1]+=w;
    table_score[1][0]+=w;

    if ((new_val=getattval(object_mapping[ls_vector[st+1]],att))!=old_val) { /* un nouveau seuil a considerer ?*/
      if (new_val>=random_thresholds[current_rt]) { /* on a un seuil aleatoire dans l'intervalle */

	current_score=compute_score_from_table();
	j=current_rt;
	while ((j+1<nb_of_random_thresholds) && (new_val>=random_thresholds[j+1]))
	  j++;
	if (current_score>best_score) {
	  best_score=current_score;
	  best_info=info;
	  best_threshold=random_thresholds[current_rt+get_random_integer(j-current_rt+1)];
	}
	current_rt=j+1;
      }
      old_val=new_val;
    }
  }

  if (best_score>=0.0) {
    current_threshold.f=best_threshold;
    current_threshold_score=best_score;
    current_threshold_info=best_info;
  } else {
    current_threshold_score=-1.0;
  }
}


/* attribute symbolique: subset aleatoire */

void find_a_threshold_symb_at_random_1_clas(int att, int *ls_vector, int start, int end) {
  int i, v, eff_v;
  int nb_val=NB_VAL(att);
  int nb_val_ls=0;
  union threshold_type random_subset;

  /* on precalcule la table avec tous les frequences pour toutes les classes */
  summarize_symb_att_clas(att, ls_vector, start, end);

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
  /* fill the table score according to the current subset */
  for (i=0; i<=nb_classes; i++) {
    table_score[1][i]=0.0;
  }
    
  eff_v=0;
  for (v=0; v<nb_val; v++) {
    if (table_score_symb[v][0]!=0.0) {
      /* check bit eff_v in current_subset */
      if (BITN(random_subset, eff_v)) {
	for (i=0; i<=nb_classes; i++) {
	  table_score[1][i]+=table_score_symb[v][i];
	}
      }
      eff_v++;
    }
  }

  /* compute the score */
  current_threshold_score=compute_score_from_table();
  current_threshold_info=info;
}


/* REGRESSION */

/* find_a_threshold_at_random for regression */

/* recherche un seuil aleatoire sur un attribut dans l'ensemble donne
 * met le resultat dans current_threshold_score current_threshold_score
 */

float sigma_factor=1.0;

DllExport void set_sigma_factor(float val) {
  sigma_factor=val;
}

void find_a_threshold_at_random_1_regr(int att, int *ls_vector, int start, int end) {
  int i; int nb_obj=end-start+1;
  float sum=getattval(object_mapping[ls_vector[start]],att);
  float sumsqr=sum*sum;
  float min=sum;
  float max=sum;
  SCORE_TYPE w;
  
  current_threshold_score=-1.0;

  /* calcule les stats sur l'attribut */
  for (i=start+1; i<=end; i++) {
    float val=getattval(object_mapping[ls_vector[i]],att);
    if (val<min)
      min=val;
    else if (val>max)
      max=val;
    sum+=val;
    sumsqr+=(val*val);
  }
  
  if (min==max) { /* toutes les valeurs sont egales */
    return;
  }
  
  /* tirage du seuil */
  sum/=nb_obj;
  sumsqr=sumsqr/nb_obj-sum*sum;
  if (sumsqr<0)
    sumsqr=0.0;
  else
    sumsqr=sqrt(sumsqr);
  
  current_threshold.f=sum+sumsqr*sigma_factor*get_random_normal();
  
  if (current_threshold.f<=min)
    current_threshold.f=min+(max-min)/nb_obj;
  else if (current_threshold.f>max)
    current_threshold.f=max-(max-min)/nb_obj;

  /* calcul du score */
  table_score[1][0]=0.0;
  table_score[1][1]=0.0;
  table_score[1][2]=0.0;

  for (i=start; i<=end; i++) {
    if (getattval(object_mapping[ls_vector[i]],att)<current_threshold.f) {
      float y=getobjy_learn(ls_vector[i]);
      w=object_weight[ls_vector[i]];
      
      table_score[1][0]+=w;
      table_score[1][1]+=w*y;
      table_score[1][2]+=w*y*y;
    }
  }
  current_threshold_score=compute_score_from_table();

}

/* version encore plus rapide, en selectionnant une valeur au hasard
 *
 */

void find_a_threshold_at_random_2_regr(int att, int *ls_vector, int start, int end) {
  int i; int nb_obj=end-start+1;
  SCORE_TYPE w;
  
  current_threshold.f=getattval(object_mapping[ls_vector[start+get_random_integer(nb_obj)]],att);

  /* calcul du score */
  table_score[1][0]=0.0;
  table_score[1][1]=0.0;
  table_score[1][2]=0.0;

  for (i=start; i<=end; i++) {
    if (getattval(object_mapping[ls_vector[i]],att)<current_threshold.f) {
      float y=getobjy_learn(ls_vector[i]);
      w=object_weight[ls_vector[i]];

      table_score[1][0]+=w;
      table_score[1][1]+=w*y;
      table_score[1][2]+=w*y*y;
    }
  }

  if (table_score[1][0]!=0.0 && table_score[0][0]!=table_score[1][0])
    current_threshold_score=compute_score_from_table();
  else
    current_threshold_score=-1.0;
}

/* distribution uniforme du seuil */

void find_a_threshold_at_random_3_regr(int att, int *ls_vector, int start, int end) {
  int i;
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
  
  /*
  printf("min=%f max=%f thres=%f\n", min, max, current_threshold);
  fflush(stdout);
  */

  /* calcul du score */
  table_score[1][0]=0.0;
  table_score[1][1]=0.0;
  table_score[1][2]=0.0;

  for (i=start; i<=end; i++) {
    if (getattval(object_mapping[ls_vector[i]],att)<current_threshold.f) {
      float y=getobjy_learn(ls_vector[i]);
      w=object_weight[ls_vector[i]];
      
      table_score[1][0]+=w;
      table_score[1][1]+=w*y;
      table_score[1][2]+=w*y*y;
    }
  }
  current_threshold_score=compute_score_from_table();
}

/* idem avec le tri */

void find_a_threshold_at_random_3_regr_sort(int att, int *ls_vector, int start, int end) {
  int i;
  float min,max;
  SCORE_TYPE w; float y;
  
  current_threshold_score=-1.0;

  sort_ls_vector(ls_vector, start, end, att);

  min=getattval(object_mapping[ls_vector[start]],att);
  max=getattval(object_mapping[ls_vector[end]],att);
  
  if (min==max) { /* toutes les valeurs sont egales */
    return;
  }
  
  /* tirage du seuil */
  current_threshold.f=max-(max-min)*get_random_float();

  /* calcul du score */
  table_score[1][0]=0.0;
  table_score[1][1]=0.0;
  table_score[1][2]=0.0;

  i=start;
  do {
    y=getobjy_learn(ls_vector[i]);
    w=object_weight[ls_vector[i]];
    
    table_score[1][0]+=w;
    table_score[1][1]+=w*y;
    table_score[1][2]+=w*y*y;
    i++;
  } while (getattval(object_mapping[ls_vector[i]],att)<current_threshold.f);

  current_threshold_score=compute_score_from_table();
}

/* version gaussienne avec retirage si on sort de l'intervalle */

void find_a_threshold_at_random_4_regr(int att, int *ls_vector, int start, int end) {
  int i; int nb_obj=end-start+1; int n;
  float sum=getattval(object_mapping[ls_vector[start]],att);
  float sumsqr=sum*sum;
  float min=sum;
  float max=sum;
  SCORE_TYPE w;
  
  current_threshold_score=-1.0;

  /* calcule les stats sur l'attribut */
  for (i=start+1; i<=end; i++) {
    float val=getattval(object_mapping[ls_vector[i]],att);
    if (val<min)
      min=val;
    else if (val>max)
      max=val;
    sum+=val;
    sumsqr+=(val*val);
  }
  
  if (min==max) { /* toutes les valeurs sont egales */
    return;
  }
  
  /* tirage du seuil */
  sum/=nb_obj;
  sumsqr=sumsqr/nb_obj-sum*sum;
  if (sumsqr<0)
    sumsqr=0.0;
  else
    sumsqr=sqrt(sumsqr);
  
  n=0;
  do {
    current_threshold.f=sum+sumsqr*sigma_factor*get_random_normal();
    n++;
  } while (((current_threshold.f<=min) || (current_threshold.f>max)) && (n<=max_nb_try));
  
  if (n>max_nb_try) {
    printf("Trop de tir à cote, on le prend de manière uniforme\n");
    fflush(stdout);
    current_threshold.f=max-(max-min)*get_random_float();
  }

  /* calcul du score */
  table_score[1][0]=0.0;
  table_score[1][1]=0.0;
  table_score[1][2]=0.0;

  for (i=start; i<=end; i++) {
    if (getattval(object_mapping[ls_vector[i]],att)<current_threshold.f) {
      float y=getobjy_learn(ls_vector[i]);
      w=object_weight[ls_vector[i]];
      
      table_score[1][0]+=w;
      table_score[1][1]+=w*y;
      table_score[1][2]+=w*y*y;
    }
  }
  current_threshold_score=compute_score_from_table();

}

/* version avec contrôle de la randomization du seuil */
/* on tire nb_of_random_thresholds au hasard et on prend le meilleur */

/* on suppose que nb_of_random_thresholds reste plus petit que 100 */
/* int nb_of_random_thresholds=1; */
/* float random_thresholds[100]; */

void find_a_threshold_at_random_5_regr(int att, int *ls_vector, int start, int end) {
  float old_val, new_val, min, max;
  SCORE_TYPE best_score=-1.0, best_info, current_score, w;
  float best_threshold, rt;
  int st=start, i, j, current_rt;

  /* on trie l'ensemble selon l'attribut */
  sort_ls_vector(ls_vector, start, end, att);

  min=getattval(object_mapping[ls_vector[start]],att);
  max=getattval(object_mapping[ls_vector[end]],att);

  if (min==max) { /* toutes les valeurs sont egales */
    current_threshold_score=-1.0;
    return;
  }
  
  /* on tire le nombre de seuils et on les place dans la liste (qu'on maintient triee) */
  for (j=0; j<nb_of_random_thresholds; j++) {
    rt=max-(max-min)*get_random_float();
    if (rt==min) {
      printf("kwakkwakkwak\n");
      fflush(stdout);
      j--;
      break;
    }
    i=j-1;
    while ((i>=0) && (random_thresholds[i]>rt)) {
      random_thresholds[i+1]=random_thresholds[i];
      i--;
    }
    random_thresholds[i+1]=rt;
  }

  /* on parcourt toutes les valeurs de seuils possibles mais on n'evalue que
   * les seuils tires aleatoirement
   */

  /* on remet a zero la table */
  table_score[1][0]=0.0;
  table_score[1][1]=0.0;
  table_score[1][2]=0.0;

  current_rt=0;
  old_val=getattval(object_mapping[ls_vector[start]],att);

  for(st=start; ((st<end) && (current_rt<nb_of_random_thresholds)) ; st++) {
    float y=getobjy_learn(ls_vector[st]);
    w=object_weight[ls_vector[st]];

    table_score[1][0]+=w;
    table_score[1][1]+=w*y;
    table_score[1][2]+=w*(y*y);

    if ((new_val=getattval(object_mapping[ls_vector[st+1]],att))!=old_val) { /* un nouveau seuil a considerer ?*/
      if (new_val>=random_thresholds[current_rt]) { /* on a un seuil aleatoire dans l'intervalle */
	current_score=compute_score_from_table();
	j=current_rt;
	while ((j+1<nb_of_random_thresholds) && (new_val>=random_thresholds[j+1]))
	  j++;
	if (current_score>best_score) {
	  best_score=current_score;
	  best_info=info;
	  best_threshold=random_thresholds[current_rt+get_random_integer(j-current_rt+1)];
	}
	current_rt=j+1;
      }
      old_val=new_val;
    }
  }

  if (best_score>=0.0) {
    current_threshold.f=best_threshold;
    current_threshold_score=best_score;
    current_threshold_info=best_info;
  } else {
    current_threshold_score=-1.0;
  }
}


/* attribute symbolique: subset aleatoire */

void find_a_threshold_symb_at_random_1_regr(int att, int *ls_vector, int start, int end) {
  int i, v, eff_v;
  int nb_val=NB_VAL(att);
  int nb_val_ls=0;
  union threshold_type random_subset;

  /* on precalcule la table avec tous les frequences pour toutes les classes */
  summarize_symb_att_regr(att, ls_vector, start, end);
  
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
  /* fill the table score according to the current subset */
  table_score[1][0]=0.0;
  table_score[1][1]=0.0;
  table_score[1][2]=0.0;
    
  eff_v=0;
  for (v=0; v<nb_val; v++) {
    if (table_score_symb[v][0]!=0.0) {
      /* check bit eff_v in current_subset */
      if (BITN(random_subset, eff_v)) {
	table_score[1][0]+=table_score_symb[v][0];
	table_score[1][1]+=table_score_symb[v][1];
	table_score[1][2]+=table_score_symb[v][2];
      }
      eff_v++;
    }
  }

  /* compute the score */
  current_threshold_score=compute_score_from_table();
}

/********************
 * RANDOM FORESTS   *
 ********************/

/* implementation de la variante de Breiman pour les arbres aleatoires
 * on selectionne un certain nombre d'attributs aleatoirement dans l'ensemble
 * des attributs candidats pour lesquels on recherche les meilleurs splits
 */

int rf_k=1;

void find_the_best_split_among_k(int *ls_vector, int start, int end) {
  int i;
  int remaining_att=nb_attributes;
  int random_att_pos, temp;

  best_attribute=-1;
  best_threshold_score=-1.0;
  best_threshold_info=-1.0;

  for (i=0; (i<rf_k)&&(remaining_att!=0) ; i++) {
    random_att_pos=get_random_integer(remaining_att);
    if (random_att_pos>=remaining_att) {
      printf("erreur dans le generateur de nombres aleatoires\n");
    }

    find_a_threshold(attribute_vector[random_att_pos], ls_vector, start, end);

    if (current_threshold_score>best_threshold_score) {
      best_threshold_score=current_threshold_score;
      best_threshold_info=current_threshold_info;
      best_threshold=current_threshold;
      best_attribute=attribute_vector[random_att_pos];
    }
    
    /* swap les attributs dans la liste */
    remaining_att--;
    if (remaining_att!=0) {
      temp=attribute_vector[random_att_pos];
      attribute_vector[random_att_pos]=attribute_vector[remaining_att];
      attribute_vector[remaining_att]=temp;
    }
  }
}

/**********************
 * UNSUPERVISED TREES *
 **********************/

/* version totalement aleatoire (on prend le premier test choisi
 * parmi les attributs non constants)
 */

/* fonctions generales */

/* on initialise juste le table_score[0][0] avec le nombre d'objets */

void summarize_vector_unsupervised(int *vector, int start, int end) {
  int i;

  table_score[0][0]=0.0;

  for (i=start; i<=end; i++) {
    table_score[0][0]+=object_weight[vector[i]];
  }
}

int no_stop_splitting_criterio() {
  return 0;
}

int make_leaf_prediction_unsupervised() {
  return -1;
}

/* not significant if attributes are constant */

int not_significant_test_if_constant_atts() {
  return (best_threshold_score<0.0);
}

/* we first split the largest node */

SCORE_TYPE best_first_node_priority_function_unsupervised() {
  return (table_score[0][0]);
}

void find_a_split_totally_at_random(int *ls_vector, int start, int end) {
  int random_att_pos, temp;
  int remaining_att=nb_attributes;

  best_attribute=-1;
  best_threshold_score=-1.0;
  best_threshold_info=-1.0;

  do {
    random_att_pos=get_random_integer(remaining_att);
    if (random_att_pos>=remaining_att) {
      printf("erreur dans le generateur de nombres aleatoires\n");
    }

    find_a_threshold(attribute_vector[random_att_pos], ls_vector, start, end);

    if (current_threshold_score<0.0) { /* l'attribut est constant */
      remaining_att--;
      if (remaining_att!=0) {
	temp=attribute_vector[random_att_pos];
	attribute_vector[random_att_pos]=attribute_vector[remaining_att];
	attribute_vector[remaining_att]=temp;
      }
    } else {
      best_threshold_score=current_threshold_score;
      best_threshold_info=current_threshold_info;
      best_threshold=current_threshold;
      best_attribute=attribute_vector[random_att_pos];
    }
  } while ((remaining_att!=0) && (best_threshold_score<0.0));
}

/* on prend un seuil au hasard entre min et max et on ne regarde pas
 * du tout le score
 */

void find_a_threshold_totally_at_random(int att, int *ls_vector, int start, int end) {
  int i; int nb_try=0;
  float min=getattval(object_mapping[ls_vector[start]],att);
  float max=min;

  current_threshold_score=-1.0;
  current_threshold_info=-1.0;

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
  
  /* tirage du seuil (on evite le cas ou current_threshold=min) */
  do {
    current_threshold.f=max-(max-min)*get_random_float();
  } while ((min>=current_threshold.f) && (nb_try++<10));

  if (current_threshold.f==min) {
    /* a mettre en commentaire. On laisse pour le moment */
    printf("current_threshold=%f, min=%f, max=%f, nb_try=%d\n", current_threshold.f, min, max, nb_try);
    fflush(stdout);
    current_threshold_score=-1;
    current_threshold_info=1.0;
  } else {
   /* on ne renvoye que 1.0 pour dire qu'on a trouve quelque-chose */
    current_threshold_score=1.0;
    current_threshold_info=1.0;
  }
}

/* void find_a_threshold_totally_at_random_2(int att, int *ls_vector, int start, int end) { */
/*   int i; int nb_obj=end-start+1; int nb_try=0; */
/*   float t1,t2; */
/*   SCORE_TYPE w; */

/*   current_threshold_score=-1.0; */
/*   current_threshold_info=-1.0; */

/*   /\* calcule les stats sur l'attribut *\/ */
/*   if (nb_obj==2) { */
/*     t1=getattval(object_mapping[ls_vector[start+0]],att); */
/*     t2=getattval(object_mapping[ls_vector[start+1]],att); */
/*   } else { */
/*     t1=getattval(object_mapping[ls_vector[start+get_random_integer(nb_obj)]],att); */
/*     t2=getattval(object_mapping[ls_vector[start+get_random_integer(nb_obj)]],att); */
/*   } */

/*   if (t1==t2) { */
    
/*   } */

/*   if (min==max) { /\* toutes les valeurs sont egales *\/ */
/*     return; */
/*   } */
  
/*   /\* tirage du seuil (on evite le cas ou current_threshold=min) *\/ */
/*   do { */
/*     current_threshold.f=max-(max-min)*get_random_float(); */
/*   } while ((min>=current_threshold.f) && (nb_try++<10)); */

/*   if (current_threshold.f==min) { */
/*     /\* a mettre en commentaire. On laisse pour le moment *\/ */
/*     printf("current_threshold=%f, min=%f, max=%f, rn=%f, nb_try=%d\n", current_threshold.f, min, max, rn, nb_try); */
/*     fflush(stdout); */
/*     current_threshold_score=-1; */
/*     current_threshold_info=1.0; */
/*   } else { */
/*    /\* on ne renvoye que 1.0 pour dire qu'on a trouve quelque-chose *\/ */
/*     current_threshold_score=1.0; */
/*     current_threshold_info=1.0; */
/*   } */
/* } */


/* attribute symbolique totalement aleatoire */

void find_a_threshold_symb_totally_at_random(int att, int *ls_vector, int start, int end) {
  int i, v, eff_v;
  int nb_val=NB_VAL(att);
  int nb_val_ls=0;
  union threshold_type random_subset;

  /* on precalcule la table avec tous les frequences pour toutes les classes */
  summarize_symb_att_clas(att, ls_vector, start, end);
  
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
  get_random_subset_non_empty_full(&random_subset,nb_val_ls); 

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
  current_threshold_score=1.0;
  current_threshold_info=1.0;
}

/******************************************************************************/
/* FONCTIONS UTILITAIRES DIVERSES */

/* valable uniquement si max_val est faible par rapport a rand_max */
/* sinon, il vaut mieux utiliser get_random_long_long */

DllExport int get_random_integer(int max_val) {
  return (int)floor((double)rand()*max_val*1.0/(RAND_MAX+1.0));
}

DllExport float get_random_float() {
  return (float)((double)rand()*1.0/(RAND_MAX+1.0));
}

void get_random_subset_non_empty_full(union threshold_type *rs, int nb_val) {
  int nb_try=0,i,  nb_1;

  /* printf("%Lu\n",subsetmax); */

  if (nb_val<5) {
    init_threshold_type(rs);
    (*rs).i[0]=1+get_random_integer((1<<nb_val)-2);
  } else {
    do {
      init_threshold_type(rs);
      nb_1=0;
      for (i=0;i<nb_val; i++) {
	if (get_random_float()<0.5) {
	  SET_BITN((*rs),i);
	  nb_1++;
	}
      }
      nb_try++;
    } while (((nb_1==0)||(nb_1==nb_val))&&(nb_try<10));
  }
}


float get_random_normal() {
  float x1=get_random_float();
  float x2=get_random_float();
  return (sqrt(-2.0*log(x1))*cos(2*VALUE_PI*x2));
}

int my_best_class(float *proba_vector, int nb_classes) {
  int i;
  float max=proba_vector[0];
  int index_max=0;
  
  for (i=0; i<nb_classes; i++) {
    if (proba_vector[i]>max) {
      max=proba_vector[i];
      index_max=i;
    }
  }
  return index_max;
}

int my_best_class_double(double *proba_vector, int nb_classes) {
  int i;
  double max=proba_vector[0];
  int index_max=0;

  for (i=0; i<nb_classes; i++) {
    if (proba_vector[i]>max) {
      max=proba_vector[i];
      index_max=i;
    }
  }
  return index_max;
}

const float chi_table[12][6]={{0.99995, 16.5, 20.0, 22.7, 25.0, 27.5},
			   {0.9999, 15.2, 18.5, 21.2, 23.6, 25.8},
			   {0.99975, 13.4, 16.6, 19.2, 21.6, 23.8},
			   {0.9995, 12.1, 15.2, 17.7, 20.0, 22.1},
			   {0.999, 10.8, 13.8, 16.3, 18.5, 20.5}, 
			   {0.995, 7.88, 10.6, 12.8, 14.9, 16.7},
			   {0.99, 6.63, 9.21, 11.3, 13.3, 15.1},
			   {0.975, 5.02, 7.38, 9.35, 11.1, 12.8},
			   {0.95, 3.84, 5.99, 7.81, 9.49, 11.1},
			   {0.9, 2.71, 4.61, 6.25, 7.78, 9.24},
			   {0.75, 1.32, 2.77, 4.11, 5.39, 6.63},
			   {0.00, .000000001, .000000001, .000000001, .000000001, .000000001}};

float chi_square(int ddl, float conf) {
  int i=0;
  /* recherche de la ligne */
  while ((i<12) && (fabs(chi_table[i][0]-conf)>0.00000000001))
    i++;
  if (i==12)/* si la valeur n'est pas bonne, tout est significatif */
    i=11;
  if (ddl<=5)
    return chi_table[i][ddl];
  else
    return chi_table[i][5];
}

/* accede a la table de valeurs */

#ifdef GETATTVAL_POINTER
float getattval_normal(int obj, int att) {
  return (float)core_table[att*nb_obj_in_core_table+obj];
}
#else
float getattval(int obj, int att) {
  return (float)core_table[att*nb_obj_in_core_table+obj];
}
#endif

DllExport float getattval_lsp(int obj, int att) {
  return core_table[att*nb_obj_in_core_table+obj];
}

/* pour debugger */
DllExport void print_obj_att(int obj) {
  int i;
  printf("o%d", obj);
  for (i=0; i<nb_attributes; i++) {
    printf(" %f", getattval(obj,attribute_vector[i]));
  }
  printf(" %f", getobjy(obj));
  fflush(stdout);
}

int getobjclass(int obj) {
#ifdef LOAD_DB_HANDLER
  if (core_table_type==NORMAL_CORE_TABLE) {
    return (int)core_table[goal*nb_obj_in_core_table+obj];
  } else if (core_table_type==TWO_DIM_CORE_TABLE) {
    return (int)core_table_2dim[obj][goal];
  } else if (core_table_type==SPARSE_CORE_TABLE) {
    return getobjclass_sparse(obj);
  } else {
    printf("wrong type of core_table\n");
    return -1;
  }
#else
  return (int)core_table[goal*nb_obj_in_core_table+obj];
#endif
    
}

float getobjy(int obj) {
#ifdef LOAD_DB_HANDLER
  if (core_table_type==NORMAL_CORE_TABLE) {
    return core_table[goal*nb_obj_in_core_table+obj];
  } else if (core_table_type==TWO_DIM_CORE_TABLE) {
    return (float)core_table_2dim[obj][goal];
  } else if (core_table_type==SPARSE_CORE_TABLE) {
    return getobjy_sparse(obj);
  } else {
    printf("wrong type of core_table\n");
    return -1.0;
  }
#else
    return core_table[goal*nb_obj_in_core_table+obj];
#endif
}

int getobjclass_learn_true(int index) {
  return getobjclass(object_mapping[index]);
}

float getobjy_learn_true(int index) {
  return getobjy(object_mapping[index]);
}

/* impression de tableau pour le debuggage */

void printtablescore() {
  int i,j;
  for (j=0; j<3; j++) {
    for(i=0; i<=nb_classes; i++) {
      printf("%e ", table_score[j][i]);
    }
    printf("\n");
  }
  fflush(stdout);
}

void printtablescoresymb(int nb_v, int nb_c) {
  int i,j;
  for (j=0; j<nb_v; j++) {
    for(i=0; i<nb_c; i++) {
      printf("%e ", table_score_symb[j][i]);
    }
    printf("\n");
  }
  printf("\n");
  fflush(stdout);
}

void printlsvector(int *ls_vector, int start, int end) {
  int i;
  for (i=start; i<=end; i++) {
    printf("%d ", ls_vector[i]);
  }
  printf("\n");
  fflush(stdout);
}

DllExport int test_array(int *vect, int size) {
  int i;
  for (i=0; i<size; i++) 
    printf("%d ", vect[i]);
  fflush(stdout);
  return vect[0];
}

void printlsvectoratt(int *ls_vector, int start, int end, int att) {
  int i;
  for (i=start; i<=end; i++) {
    printf("%f ",getattval(object_mapping[ls_vector[i]],att));
  }
  printf("\n");
  fflush(stdout);
}

void printmatrix(int *matrix, int nb_row, int nb_col) {
  int i,j;
  
  for (i=0; i<nb_row; i++) {
    for (j=0; j<nb_col; j++) {
      printf("%d ", matrix[i*nb_col+j]);
    }
    printf("\n");
  }
  printf("\n");
  fflush(stdout);
}

void copy_matrix_a_to_b(int *a, int *b, int size) {
  int i;
  for (i=0; i<size; i++)
    b[i]=a[i];
}

void print_bit_repr(int a) {
  int i;
  int mask=1;
  for (i=0; i<32; i++) {
    printf("%d",a&mask? 1 : 0);
    fflush(stdout);
    mask=mask<<1;
  }
}

void print_n_spaces(int n) {
  int i;
  for (i=0; i<n; i++) {
    printf(" ");
  }
  fflush(stdout);
}

/* fonction de randomization d'un vecteur */
/* melange les elements d'un vecteur (pour la validation croisee) */

void randomize_vector(int *vector, int length_vector) {
  int i,temp;
  for (i=0; i<length_vector; i++) {
    int rn=get_random_integer(length_vector-i);
    temp=vector[i];
    vector[i]=vector[i+rn];
    vector[i+rn]=temp;
  }
}

/* UTILISATION DES ARBRES */

/*********************
 * PRETTY-PRINT TREE *
 *********************/

DllExport void prettyprinttree(int tree, int max_depth) {
  int current_depth, current_node, i;
  
  index_stack_open_nodes=-1;
  index_stack_open_nodes++;
  stack_open_nodes[index_stack_open_nodes][0]=tree;
  stack_open_nodes[index_stack_open_nodes][1]=0;
  
  while(index_stack_open_nodes>=0) {
    current_node=stack_open_nodes[index_stack_open_nodes][0];
    current_depth=stack_open_nodes[index_stack_open_nodes][1];
    index_stack_open_nodes--;
    if (current_node==-1) { /* else */
      print_n_spaces(current_depth);
      printf("} else {\n");
      fflush(stdout);
    } else if (current_node==-2) { /* close */
      print_n_spaces(current_depth);
      printf("}\n");
      fflush(stdout);
    } else { /* noeud normal */
      
      if (left_successor[current_node]==-1) { /* noeud terminal */
	print_n_spaces(current_depth);
	printf("return (");
	for (i=0; i<nb_classes; i++) {
	  printf("%f", prediction_values[prediction[current_node]][i]);
	  if (i==nb_classes-1)
	    printf(");\n");
	  else
	    printf(",");
	}
	fflush(stdout);
      } else if (current_depth==max_depth) { /* trop bas */
	print_n_spaces(current_depth);
	printf("...\n");
	fflush(stdout);
      } else { /* noeud test */
	print_n_spaces(current_depth);
	if (NUMERICALp(tested_attribute[current_node])) {
	  printf("if (ATT%d < %f) {\n", tested_attribute[current_node], threshold[current_node].f);
	} else {
	  printf("if (ATT%d in (", tested_attribute[current_node]);
	  for (i=0; i<attribute_descriptors[tested_attribute[current_node]]; i++) {
	    if (BITN(threshold[current_node],i)) {
	      printf("%d,",i);
	    }
	  }
	  printf(")) {\n");
	  fflush(stdout);
	}
	index_stack_open_nodes++;
	stack_open_nodes[index_stack_open_nodes][0]=-2;
	stack_open_nodes[index_stack_open_nodes][1]=current_depth;
	index_stack_open_nodes++;
	stack_open_nodes[index_stack_open_nodes][0]=current_node+right_successor[current_node];
	stack_open_nodes[index_stack_open_nodes][1]=current_depth+1;
	index_stack_open_nodes++;
	stack_open_nodes[index_stack_open_nodes][0]=-1;
	stack_open_nodes[index_stack_open_nodes][1]=current_depth;
	index_stack_open_nodes++;
	stack_open_nodes[index_stack_open_nodes][0]=current_node+left_successor[current_node];	
	stack_open_nodes[index_stack_open_nodes][1]=current_depth+1;
      }
    }
  }
}

/****************************************
 * propagation d'un objet dans un arbre *
 ****************************************/

/* application d'un arbre a un objet */

float *get_tree_prediction_vector_classical(int tree, int obj) {
  int current_node=tree;
  while (left_successor[current_node]!=-1) {
    if (check_test(tested_attribute[current_node],
		   getattval(obj,tested_attribute[current_node]), 
		   threshold[current_node]))
      current_node+=left_successor[current_node];
    else
      current_node+=right_successor[current_node];
  }
  return prediction_values[prediction[current_node]];
}

/* idem mais renvoit la feuille plutot que la prediction */

int get_tree_leaf(int tree, int obj) {
  int current_node=tree;
  while (left_successor[current_node]!=-1) {
    if (check_test(tested_attribute[current_node],
		   getattval(obj,tested_attribute[current_node]), 
		   threshold[current_node]))
      current_node+=left_successor[current_node];
    else
      current_node+=right_successor[current_node];
  }
  return current_node;
}


/***********
 * dual PC *
 ***********/

/* implementation du dual PC, recherche le vecteur de prediction moyen associe a
 * un arbre
 */

/* ici, il faudrait gere le vecteur de maniere dynamique, limite a 50 classes */
#define MAX_DUALPC_PREDICTION_VALUES 50

float test_prediction_vector[MAX_DUALPC_PREDICTION_VALUES];
float lambda_noise=0.0;

/* table avec les valeurs de proba */
const float z_table[35][10]={{0.5, 0.504, 0.508, 0.512, 0.516, 0.5199, 0.5239, 0.5279, 0.5319, 0.5359}, 
			     {0.5398, 0.5438, 0.5478, 0.5517, 0.5557, 0.5596, 0.5636, 0.5675, 0.5714, 0.5753},
			     {0.5793, 0.5832, 0.5871, 0.591, 0.5948, 0.5987, 0.6026, 0.6064, 0.6103, 0.6141},
			     {0.6179, 0.6217, 0.6255, 0.6293, 0.6331, 0.6368, 0.6406, 0.6443, 0.648, 0.6517},
			     {0.6554, 0.6591, 0.6628, 0.6664, 0.67, 0.6736, 0.6772, 0.6808, 0.6844, 0.6879},
			     {0.6915, 0.695, 0.6985, 0.7019, 0.7054, 0.7088, 0.7123, 0.7157, 0.719, 0.7224},
			     {0.7257, 0.7291, 0.7324, 0.7357, 0.7389, 0.7422, 0.7454, 0.7486, 0.7517,
			      0.7549},
			     {0.758, 0.7611, 0.7642, 0.7673, 0.7704, 0.7734, 0.7764, 0.7794, 0.7823, 0.7852},
			     {0.7881, 0.791, 0.7939, 0.7967, 0.7995, 0.8023, 0.8051, 0.8078, 0.8106, 0.8133},
			     {0.8159, 0.8186, 0.8212, 0.8238, 0.8264, 0.8289, 0.8315, 0.834, 0.8365, 0.8389},
			     {0.8413, 0.8438, 0.8461, 0.8485, 0.8508, 0.8531, 0.8554, 0.8577, 0.8599,
			      0.8621},
			     {0.8643, 0.8665, 0.8686, 0.8708, 0.8729, 0.8749, 0.877, 0.879, 0.881, 0.883},
			     {0.8849, 0.8869, 0.8888, 0.8907, 0.8925, 0.8944, 0.8962, 0.898, 0.8997, 0.9015},
			     {0.9032, 0.9049, 0.9066, 0.9082, 0.9099, 0.9115, 0.9131, 0.9147, 0.9162,
			      0.9177},
			     {0.9192, 0.9207, 0.9222, 0.9236, 0.9251, 0.9265, 0.9279, 0.9292, 0.9306,
			      0.9319},
			     {0.9332, 0.9345, 0.9357, 0.937, 0.9382, 0.9394, 0.9406, 0.9418, 0.9429, 0.9441},
			     {0.9452, 0.9463, 0.9474, 0.9484, 0.9495, 0.9505, 0.9515, 0.9525, 0.9535,
			      0.9545},
			     {0.9554, 0.9564, 0.9573, 0.9582, 0.9591, 0.9599, 0.9608, 0.9616, 0.9625,
			      0.9633},
			     {0.9641, 0.9649, 0.9656, 0.9664, 0.9671, 0.9678, 0.9686, 0.9693, 0.9699,
			      0.9706},
			     {0.9713, 0.9719, 0.9726, 0.9732, 0.9738, 0.9744, 0.975, 0.9756, 0.9761, 0.9767},
			     {0.9772, 0.9778, 0.9783, 0.9788, 0.9793, 0.9798, 0.9803, 0.9808, 0.9812
			      , 0.9817},
			     {0.9821, 0.9826, 0.983, 0.9834, 0.9838, 0.9842, 0.9846, 0.985, 0.9854, 0.9857},
			     {0.9861, 0.9864, 0.9868, 0.9871, 0.9875, 0.9878, 0.9881, 0.9884, 0.9887, 0.989},
			     {0.9893, 0.9896, 0.9898, 0.9901, 0.9904, 0.9906, 0.9909, 0.9911, 0.9913,
			      0.9916},
			     {0.9918, 0.992, 0.9922, 0.9925, 0.9927, 0.9929, 0.9931, 0.9932, 0.9934, 0.9936},
			     {0.9938, 0.994, 0.9941, 0.9943, 0.9945, 0.9946, 0.9948, 0.9949, 0.9951, 0.9952},
			     {0.9953, 0.9955, 0.9956, 0.9957, 0.9959, 0.996, 0.9961, 0.9962, 0.9963, 0.9964},
			     {0.9965, 0.9966, 0.9967, 0.9968, 0.9969, 0.997, 0.9971, 0.9972, 0.9973, 0.9974},
			     {0.9974, 0.9975, 0.9976, 0.9977, 0.9977, 0.9978, 0.9979, 0.9979, 0.998, 0.9981},
			     {0.9981, 0.9982, 0.9982, 0.9983, 0.9984, 0.9984, 0.9985, 0.9985, 0.9986,
			      0.9986},
			     {0.9987, 0.9987, 0.9987, 0.9988, 0.9988, 0.9989, 0.9989, 0.9989, 0.999, 0.999},
			     {0.999, 0.9991, 0.9991, 0.9991, 0.9992, 0.9992, 0.9992, 0.9992, 0.9993, 0.9993},
			     {0.9993, 0.9993, 0.9994, 0.9994, 0.9994, 0.9994, 0.9994, 0.9995, 0.9995,
			      0.9995},
			     {0.9995, 0.9995, 0.9995, 0.9996, 0.9996, 0.9996, 0.9996, 0.9996, 0.9996,
			      0.9997},
			     {0.9997, 0.9997, 0.9997, 0.9997, 0.9997, 0.9997, 0.9997, 0.9997, 0.9997,
			      0.9998}};

/* pile pour retenir ou on est deja passe */
int index_stack_nodes=-1;
int stack_nodes[MAX_OPEN_NODES];

float *get_tree_prediction_vector_dualpc(int tree, int obj) {
  int i, index_row, index_column;
  float denom;

  node_weight[tree]=1.0;
  index_stack_nodes++;
  stack_nodes[index_stack_nodes]=tree;

  /* initialisation du vecteur */
  for (i=0; i<nb_classes; i++) {
    test_prediction_vector[i]=0.0;
  }
  
  /* propagation dans l'arbre */
  while (index_stack_nodes>=0) {
    int current_node, left, right;
    float a, newa, proba;

    /* on retire le noeud au sommet de la pile */
    current_node=stack_nodes[index_stack_nodes];
    index_stack_nodes--;

    if (left_successor[current_node]==-1) {
      int index_proba=prediction[current_node];
      if ((proba=node_weight[current_node])!=0.0) {
	
	for (i=0; i<nb_classes; i++) {
	  test_prediction_vector[i]+=(proba*prediction_values[index_proba][i]);
	}
      }
    } else {
      /* on calcule les probas d'aller a droite et de gauche */
      denom=(lambda_noise*get_att_stddev(tested_attribute[current_node]));
      if (denom==0.0)
	denom=0.00001;
      a=(getattval(obj, tested_attribute[current_node])-threshold[current_node].f)/
	(lambda_noise*get_att_stddev(tested_attribute[current_node]));      
      newa=fabs(a);
      
      if (newa>3.49)
	proba=1.0;
      else {
	index_row=(int)floor(10.0*newa);
	index_column=(int)(100.0*(newa-(index_row/10.0)));
	if (index_column==10) {
	  index_column=0;
	  index_row++;
	}
	proba=z_table[index_row][index_column];
      }
      if (a>=0.0)
	proba=1.0-proba;
      
      if (proba==0.5 && get_att_stddev(tested_attribute[current_node])==0.0) {
	if (getattval(obj,tested_attribute[current_node])<threshold[current_node].f)
	  proba=1.0;
	else
	  proba=0.0;
      }

      if (proba!=0.0) {
	left=current_node+left_successor[current_node];
	index_stack_nodes++;
	stack_nodes[index_stack_nodes]=left;
	node_weight[left]=node_weight[current_node]*proba;
      } 
      if (proba!=1.0) {
	right=current_node+right_successor[current_node];
	index_stack_nodes++;
	stack_nodes[index_stack_nodes]=right;
	node_weight[right]=node_weight[current_node]*(1.0-proba);
      }
    } 
  }
  
  return test_prediction_vector;
}

/* calcul des deviations standards des attributs */

float *att_stddev_vector;
int max_att_index=0;

float get_att_stddev(int att) {
  return att_stddev_vector[att];
}

/* suppose que le LS est defini */

DllExport void compute_att_stddev_from_ls() {
  int max_int=0,i,j;
  if (att_stddev_vector) {
    MyFree((float *)att_stddev_vector);
  }
  
  for (i=0; i<nb_attributes; i++) {
    if (attribute_vector[i]>max_int)
      max_int=attribute_vector[i];
  }
  max_att_index=max_int;

  att_stddev_vector=(float *)MyMalloc((size_t)(max_int+1)*sizeof(float));
  if (!att_stddev_vector) {
    printf("impossible d'allouer de la memoire dans compute_att_stddev_from_ls\n");
    exit(0);
  }
  
  for (i=0; i<nb_attributes; i++) {
    float sum=0.0,sumsqr=0.0;
    int att=attribute_vector[i];
    for (j=0; j<global_learning_set_size; j++) {
      float val=getattval(object_mapping[j],att);
      sum+=val;
      sumsqr+=(val*val);
    }
    sum/=global_learning_set_size;
    sumsqr/=global_learning_set_size;
    sumsqr-=(sum*sum);
    if (sumsqr<=0.0)
      att_stddev_vector[att]=0.0;
    else
      att_stddev_vector[att]=sqrt(sumsqr);
    if (print_result) {
      printf("att%d (/%d)-> %f\n", att, nb_attributes,get_att_stddev(att));
      fflush(stdout);
    }
  }
}

/* version de test speciale pour les extra-trees */

/* application d'un arbre a un objet */

float *get_tree_prediction_vector_without_obj(int tree, int obj) {
  int current_node=tree, next_node;

  while (left_successor[current_node]==-1) {
    if (check_test(tested_attribute[current_node],
		   getattval(obj,tested_attribute[current_node]), 
		   threshold[current_node])) {
      next_node=current_node+left_successor[current_node];
      if (prediction[next_node]!=-1) { /* c'est un noeud terminal */
	return get_tree_prediction_vector_classical(current_node+right_successor[current_node], obj);
      } else
	current_node=next_node;
    } else {
      next_node=current_node+right_successor[current_node];
      if (prediction[next_node]!=-1) { /* c'est un noeud terminal */
	return get_tree_prediction_vector_classical(current_node+left_successor[current_node], obj);
      } else
	current_node=next_node;
    }
  }
  /* ne devrait pas etre utilise */
  return prediction_values[prediction[current_node]];
}

/************************************************************************
 * INTERFACE AVEC LE LISP *
 **************************/

/* initialise la BD */

DllExport void init_db1(CORETABLE_TYPE *core_t, int nb_obj, int *att_desc) {
  /* initialise la table des attributs */
  core_table=core_t;
  nb_obj_in_core_table=nb_obj;
  attribute_descriptors=att_desc;
#ifdef LOAD_DB_HANDLER
  core_table_type=NORMAL_CORE_TABLE;
#endif
}

DllExport void init_db2(CORETABLE_TYPE *core_t, int nb_obj, int *att_desc) {
  /* initialise la table des attributs */
  core_table=core_t;
  nb_obj_in_core_table=nb_obj;
  attribute_descriptors=att_desc;
#ifdef LOAD_DB_HANDLER
  core_table_type=NORMAL_CORE_TABLE;
#endif
}

/* efface tous les arbres */

DllExport void clean_all_trees() {
  /* efface toutes les tables */
  index_nodes=-1;
  index_prediction_values=-1;
  index_stack_open_nodes=-1;
  clean_all_ensemble();
}

/* alloue toutes les tables de données de manière dynamique */

SCORE_TYPE **allocate_table_score_type(int nbl, int nbc) {
  SCORE_TYPE **tableau;
  int i,j;

  tableau=(SCORE_TYPE **)MyMalloc(nbl*sizeof(SCORE_TYPE *));
  if (tableau==NULL)
    return NULL;
  for (i=0;i<nbl;i++) {
    tableau[i]=(SCORE_TYPE *)MyMalloc(nbc*sizeof(SCORE_TYPE));
    if (tableau[i]==NULL) {
      for (j=0; j<i; j++) {
	MyFree((SCORE_TYPE *)tableau[j]);
      }
      return NULL;
    }
  }
  return tableau;
}

void free_table_score_type(SCORE_TYPE **tableau, int nbl) {
  int i;

  if (tableau!=NULL) {
    for (i=0;i<nbl;i++) {
      if (tableau[i]!=NULL)
	MyFree((SCORE_TYPE *)tableau[i]);
    }
    MyFree((SCORE_TYPE **)tableau);
  }
}

float **allocate_table_float(int nbl, int nbc) {
  float **tableau;
  int i,j;

  tableau=(float **)MyMalloc(nbl*sizeof(float *));
  if (tableau==NULL)
    return NULL;
  for (i=0;i<nbl;i++) {
    tableau[i]=(float *)MyMalloc(nbc*sizeof(float));
    if (tableau[i]==NULL) {
      for (j=0; j<i; j++) {
	MyFree((float *)tableau[j]);
      }
      return NULL;
    }
  }
  return tableau;
}

void free_table_float(float **tableau, int nbl) {
  int i;

  if (tableau!=NULL) {
    for (i=0;i<nbl;i++) {
      if (tableau[i]!=NULL)
	MyFree((float *)tableau[i]);
    }
    MyFree((float **)tableau);
  }
}


DllExport void free_tree_tables() {
  clean_all_trees();
/*  if (left_successor!=NULL) {
    MyFree((int *)left_successor);
    left_successor=NULL;
  }
  if (right_successor!=NULL) {
    MyFree((int *)right_successor);
    right_successor=NULL;
  }
  if (tested_attribute!=NULL) {
    MyFree((int *)tested_attribute);
    tested_attribute=NULL;
  }
  if (prediction!=NULL) {
    MyFree((int *)prediction);
    prediction=NULL;
  }*/
  if (node_weight!=NULL) {
    MyFree((float *)node_weight);
    node_weight=NULL;
  }
  if (node_size!=NULL) {
    MyFree((float *)node_size);
    node_size=NULL;
  }
/*  if (threshold!=NULL) {
    MyFree((union threshold_type *)threshold);
    threshold=NULL;
  }*/

/*  free_table_float(prediction_values,size_current_tree_table_pred);
  prediction_values=NULL;
*/
  free_table_score_type(table_score,3);
  table_score=NULL;

  free_table_score_type(table_score_symb,MAX_NUMBER_OF_SYMBOLIC_VALUES);
  table_score_symb=NULL;

#ifdef LOAD_MULTIREGR
  free_multiregr_table_score();
#endif

  size_current_tree_table=0;
  size_current_tree_table_pred=0;

}

DllExport int allocate_tree_tables(int nb_of_nodes, int nb_of_leaves, int nb_pred, int tsp) {
  free_tree_tables();

  left_successor=(int *)MyMalloc((size_t)nb_of_nodes*sizeof(int));
  if (left_successor==NULL) {
    return 0;
  }
  right_successor=(int *)MyMalloc((size_t)nb_of_nodes*sizeof(int));
  if (right_successor==NULL) {
    free_tree_tables();
    return 0; 
  }
  tested_attribute=(int *)MyMalloc((size_t)nb_of_nodes*sizeof(int));
  if (tested_attribute==NULL) {
    free_tree_tables();
    return 0;
  }

  node_weight=(float *)MyMalloc((size_t)nb_of_nodes*sizeof(float));
  if (node_weight==NULL) {
    free_tree_tables();
    return 0;
  }
  node_size=(float *)MyMalloc((size_t)nb_of_nodes*sizeof(float));
  if (node_size==NULL) {
    free_tree_tables();    
    return 0;
  }
  threshold=(union threshold_type *)MyMalloc((size_t)nb_of_nodes*sizeof(union threshold_type));
  if (threshold==NULL) {
    free_tree_tables();    
    return 0;
  }

  /* ne sert a rien si multiregr_savepred est a 1 */
  prediction=(int *)MyMalloc((size_t)nb_of_nodes*sizeof(int));
  if (prediction==NULL) {
    free_tree_tables();
    return 0;
  }

  nb_of_predictions=nb_pred;

  if (nb_pred>0) {
    prediction_values=allocate_table_float(nb_of_leaves,nb_pred);
    if (prediction_values==NULL) {
      free_tree_tables();
      return 0;
    }
    
    /* allocation de la table de score (a ne pas faire si multiregr) */
    if (tsp==1) {
      table_score=allocate_table_score_type(3,nb_pred+1);
      if (table_score==NULL) {
	free_tree_tables();
	return 0;
      }
      table_score_symb=allocate_table_score_type(MAX_NUMBER_OF_SYMBOLIC_VALUES,nb_pred+1);
      if (table_score==NULL) {
	free_tree_tables();
	return 0;
      }
    }

    size_current_tree_table_pred=nb_of_leaves;

  } else
    size_current_tree_table_pred=0;

  size_current_tree_table=nb_of_nodes;

  return 1;

}

/* initialise les poids des classes */
DllExport void init_class_weights(int cwp, float *weight_vect) {
  class_weightp=cwp;
  class_weight_vector=weight_vect;
}

DllExport void init_weight_attribute(int wap, int wa) {
  weight_attributep=wap;
  weight_attribute=wa;
}

/* initialise l'ensemble d'apprentissage (les poids et la liste du mapping) */

DllExport void init_learning_set(int *ls_vector, int length_ls_vector) {
  int i;

  /*
  for (i=0; i<length_ls_vector; i++) {
    printf("%d ", ls_vector[i]);
  }
  fflush(stdout);
  */

  if ((global_learning_set_size<length_ls_vector)||(current_learning_set_size<length_ls_vector)) {
    if (current_learning_set_size>0) {
      MyFree((int *)current_learning_set);
      MyFree((SCORE_TYPE *)object_weight);
    }
    object_weight=(SCORE_TYPE *)MyMalloc((size_t)length_ls_vector*sizeof(SCORE_TYPE));
    current_learning_set=(int *)MyMalloc((size_t)length_ls_vector*sizeof(int));
  }
    
  object_mapping=ls_vector;
  global_learning_set_size=length_ls_vector;
  current_learning_set_size=length_ls_vector;
  
  for (i=0; i<length_ls_vector; i++) {
    current_learning_set[i]=i;
    if (class_weightp) {
      object_weight[i]=class_weight_vector[getobjclass_learn(i)];
    } else if (weight_attributep) {
      object_weight[i]=getattval(object_mapping[i],weight_attribute);
    } else {
      object_weight[i]=1.0;
    }
  }
  /*
  printf("%d %d\n",weight_attributep, weight_attribute);
  for (i=0; i<length_ls_vector; i++) {
    printf("%d %f\n",i, object_weight[i]);
  }
  fflush(stdout);
  */
}

/* definit un probleme de classification */

DllExport void init_classification_problem(int *att_vect, int nb_att, int goal_class, int nb_clas) {
  goal=goal_class;
  goal_type=CLASSIFICATION;
  nb_classes=nb_clas;
  attribute_vector=att_vect;
  nb_attributes=nb_att;

  /*
  for (i=0; i<nb_attributes; i++) {
    printf("%d ", att_vect[i]);
  }
  fflush(stdout);
  */
  
#ifdef LOAD_DB_HANDLER
  if (core_table_type==SPARSE_CORE_TABLE) {
    getobjclass_learn=getobjclass_learn_sparse;
    getattval=getattval_sparse;
  } else if (core_table_type==TWO_DIM_CORE_TABLE) {
    getobjclass_learn=getobjclass_learn_2dim;
    getattval=getattval_2dim;
  } else if (core_table_type==NORMAL_CORE_TABLE) {
    getobjclass_learn=getobjclass_learn_true;
    getattval=getattval_normal;
  } else {
    printf("wrong type of core_table\n");
    fflush(stdout);
  }
#else 
  getobjclass_learn=getobjclass_learn_true;
#ifdef GETATTVAL_POINTER
  getattval=getattval_normal;
#endif
#endif

}

/* definit un probleme de regression */

DllExport void init_regression_problem(int *att_vect, int nb_att, int goal_regr) {
  goal=goal_regr;
  goal_type=REGRESSION;
  attribute_vector=att_vect;
  nb_attributes=nb_att;
  nb_classes=1;
#ifdef LOAD_DB_HANDLER
  if (core_table_type==SPARSE_CORE_TABLE) {
    getobjy_learn=getobjy_learn_sparse;
    getattval=getattval_sparse;
  } else if (core_table_type==TWO_DIM_CORE_TABLE) {
    getobjy_learn=getobjy_learn_2dim;
    getattval=getattval_2dim;
  } else if (core_table_type==NORMAL_CORE_TABLE) {
    getobjy_learn=getobjy_learn_true;
    getattval=getattval_normal;
  } else {
    printf("wrong type of core_table\n");
    fflush(stdout);
  }
#else 
  getobjy_learn=getobjy_learn_true;
#ifdef GETATTVAL_POINTER
  getattval=getattval_normal;
#endif
#endif
}

int get_current_tree_type() {
  return goal_type;
}

/* definit un probleme non supervise */

DllExport void init_unsupervised_problem(int *att_vect, int nb_att) {
  attribute_vector=att_vect;
  nb_attributes=nb_att;
  nb_classes=1;

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

}

/* initialise la methode de tri */

DllExport void init_sorting_method() {
  int size;
  if (sorting_variant==LOCAL_SORT) {
    sort_ls_vector=quicksort_ls_vector;
    separate_ls_vector=separate_ls_vector_local;
  } else if (sorting_variant==GLOBAL_SORT_PROJECTION) {
    /* preparation des matrices de tri */
    prepare_sorted_matrix(current_learning_set, global_learning_set_size);
    sort_ls_vector=sort_ls_vector_by_projection;
    separate_ls_vector=separate_ls_vector_local;
  } else if (sorting_variant=GLOBAL_SORT_REARRANGE) {
    prepare_sorted_matrix(current_learning_set, global_learning_set_size);
    printf("temp_sorted_matrix_size=%d temp_nb_obj=%d global_ls=%d current_ls=%d\n",
	   temp_sorted_matrix_size, temp_sorted_matrix_nb_obj, global_learning_set_size, current_learning_set_size);
    fflush(stdout);
    if (temp_sorted_matrix_size>=(size=nb_attributes*global_learning_set_size)) {
      copy_matrix_a_to_b(sorted_matrix, temp_sorted_matrix, size);
    } else {
      if (temp_sorted_matrix_size!=0)
	MyFree((int *)temp_sorted_matrix);
      temp_sorted_matrix_size=0;
      temp_sorted_matrix=sorted_matrix; /* au cas ou on n'utiliserait pas une methode d'ensemble */
    }
    temp_sorted_matrix_nb_obj=sorted_matrix_nb_obj;
    fflush(stdout);
    sort_ls_vector=sort_ls_vector_by_copy;
    separate_ls_vector=rearrange_ls_vector;
  }
}

/* change la methode de tri */

DllExport void set_sorting_variant(int i) {
  if ((i!=LOCAL_SORT) && (i!=GLOBAL_SORT_PROJECTION) && (i!=GLOBAL_SORT_REARRANGE)) {
    printf("Wrong value for the sorting variant (0, 1 or 2)\n");
    fflush(stdout);
  } else
    sorting_variant=i;
}

/* fonction qui recupere la taille du modele dernierement construit */

DllExport int get_last_model_complexity () {
  return (index_nodes+1);
}

/* pour fixer le score a partir du lisp */

SCORE_TYPE (*compute_classification_score_from_table)()=compute_classification_score_from_table_log;
int (*stop_splitting_criterio_classification)()=stop_splitting_criterio_classification_log;
int (*not_significant_test_classification)()=not_significant_test_classification_log;
SCORE_TYPE (*best_first_node_priority_function_class)()=best_first_node_priority_function_class_log;

DllExport void set_score_gini_classification() {
  compute_classification_score_from_table=compute_classification_score_from_table_gini;
  stop_splitting_criterio_classification=stop_splitting_criterio_classification_gini;
  best_first_node_priority_function_class=best_first_node_priority_function_class_gini;
  not_significant_test_classification=not_significant_test_classification_gini;
}

DllExport void set_score_gini_classification_unnorm() {
  compute_classification_score_from_table=compute_classification_score_from_table_gini_unnorm;
  stop_splitting_criterio_classification=stop_splitting_criterio_classification_gini;
  best_first_node_priority_function_class=best_first_node_priority_function_class_gini;
  not_significant_test_classification=not_significant_test_classification_gini;
}

DllExport void set_score_log_classification() {
  compute_classification_score_from_table=compute_classification_score_from_table_log;
  stop_splitting_criterio_classification=stop_splitting_criterio_classification_log;
  best_first_node_priority_function_class=best_first_node_priority_function_class_log;
  not_significant_test_classification=not_significant_test_classification_log;
}

DllExport void set_score_c45_classification() {
  compute_classification_score_from_table=compute_classification_score_from_table_C45;
  stop_splitting_criterio_classification=stop_splitting_criterio_classification_C45;
  best_first_node_priority_function_class=best_first_node_priority_function_class_log;
  not_significant_test_classification=not_significant_test_classification_gini;
}

/* pour changer manuellement la fonction de recherche d'un test */

DllExport void set_find_a_test_function(int i, float th, int k, int nb_tests) {

  nb_of_random_tests=nb_tests;
  random_split_score_threshold=th;
  rf_k=k;

  if (i==1) {
    find_a_split=find_a_split_at_random;
  } else if (i==2) {
    find_a_split=find_the_best_split_among_k;
  } else if (i==3) {
    find_a_split=find_a_split_at_random_2;
  } else if (i==4) {
    find_a_split=find_a_split_at_random_3;
  } else if (i==5) {
    find_a_split=find_a_split_totally_at_random;
  } else if (i==6) {
    find_a_split=find_a_split_at_random_4;
  } else if (i==0)
    find_a_split=find_the_best_split;
}

/* pour changer manuellement la fonction de recherche d'un seuil */

DllExport void set_find_a_threshold_num_function_clas(int i, int nb_rt) {
  
  nb_of_random_thresholds=nb_rt;
  
  if (i==1) {
    find_a_threshold_num=find_a_threshold_at_random_1_clas;
  } else if (i==2) {
    find_a_threshold_num=find_a_threshold_at_random_2_clas;
  } else if (i==3) {
    find_a_threshold_num=find_a_threshold_at_random_3_clas;
  } else if (i==4) {
    find_a_threshold_num=find_a_threshold_at_random_4_clas;
  } else if (i==5) {
    find_a_threshold_num=find_the_best_threshold_clas_randomized;
  } else if (i==6) {
    find_a_threshold_num=find_a_threshold_totally_at_random;
  } else if (i==7) {
    find_a_threshold_num=find_a_threshold_at_random_3_clas_sort;
  } else if (i==8) {
    find_a_threshold_num=find_a_threshold_at_random_5_clas;
  } else {
    find_a_threshold_num=find_the_best_threshold_clas;
  }
}

DllExport void set_find_a_threshold_num_function_regr(int i, int nb_rt) {
  
  nb_of_random_thresholds=nb_rt;
  
  if (i==1) {
    find_a_threshold_num=find_a_threshold_at_random_1_regr;
  } else if (i==2) {
    find_a_threshold_num=find_a_threshold_at_random_2_regr;
  } else if (i==3) {
    find_a_threshold_num=find_a_threshold_at_random_3_regr;
  } else if (i==4) {
    find_a_threshold_num=find_a_threshold_at_random_4_regr;
  } else if (i==5) {
    find_a_threshold_num=find_the_best_threshold_regr_randomized;
  } else if (i==6) {
    find_a_threshold_num=find_a_threshold_totally_at_random;
  } else if (i==7) {
    find_a_threshold_num=find_a_threshold_at_random_3_regr_sort;
  } else if (i==8) {
    find_a_threshold_num=find_a_threshold_at_random_5_regr;
  } else {
    find_a_threshold_num=find_the_best_threshold_regr;
  }
}

/* pour changer manuellement la fonction de recherche d'un seuil pour attribut symb */

DllExport void set_find_a_threshold_symb_function_clas(int i) {
  if (i==1) {
    find_a_threshold_symb=find_a_threshold_symb_at_random_1_clas;
  } else if (i==2) {
    find_a_threshold_symb=find_a_threshold_symb_totally_at_random;
  } else {
    find_a_threshold_symb=find_the_best_threshold_symb_clas;
  }
}

DllExport void set_find_a_threshold_symb_function_regr(int i) {
  if (i==1) {
    find_a_threshold_symb=find_a_threshold_symb_at_random_1_regr;
  } else if (i==2) {
    find_a_threshold_symb=find_a_threshold_symb_totally_at_random;
  } else {
    find_a_threshold_symb=find_the_best_threshold_symb_regr;
  }
}

/* set print_result */

DllExport void set_print_result(int i, int j) {
  print_result=i;
  print_detailled_result=j;
}

/* set best_first */

DllExport void set_best_first(int bf_flag, int tbf_flag, int max_tests) {
  best_first=bf_flag;
  true_best_first=tbf_flag;
  best_first_max_nb_tests=max_tests;

  if (size_tbf>0) {
    MyFree((int *)tbf_best_attribute);
    MyFree((int *)tbf_stop_splitting);
    MyFree((int *)tbf_not_significant);
    MyFree((SCORE_TYPE *)tbf_best_threshold_score);
    MyFree((SCORE_TYPE *)tbf_best_threshold_info);
    MyFree((union threshold_type  *)tbf_best_threshold);
    size_tbf=0;
  }
  
  if (tbf_flag) {
    tbf_best_attribute=(int *)MyMalloc((size_t)(max_tests+1)*sizeof(int));
    tbf_stop_splitting=(int *)MyMalloc((size_t)(max_tests+1)*sizeof(int));
    tbf_not_significant=(int *)MyMalloc((size_t)(max_tests+1)*sizeof(int));
    tbf_best_threshold_score=(SCORE_TYPE *)MyMalloc((size_t)(max_tests+1)*sizeof(SCORE_TYPE));
    tbf_best_threshold_info=(SCORE_TYPE *)MyMalloc((size_t)(max_tests+1)*sizeof(SCORE_TYPE));
    tbf_best_threshold=(union threshold_type *)MyMalloc((size_t)(max_tests+1)*sizeof(union threshold_type));    
    size_tbf=max_tests+1;
  }
  /*
  printf("max_tests=%d",max_tests);
  fflush(stdout);
  */
}

/* definit des arbres de classification */

DllExport void init_classification_trees(int n, float h, float alfa) {
  min_node_size=n;
  h_min=h;
  alfa_dt=alfa;
  summarize_vector=summarize_vector_clas;
  make_leaf_prediction=make_leaf_prediction_classification;
  stop_splitting_criterio=stop_splitting_criterio_classification;
  best_first_node_priority_function=best_first_node_priority_function_class;
  not_significant_test=not_significant_test_classification;
  compute_score_from_table=compute_classification_score_from_table;
}

DllExport void init_regression_trees(int n_min, float v_min_thres, float alfa) {
  min_node_size=n_min;
  v_min=v_min_thres;
  alfa_rt=alfa;
  summarize_vector=summarize_vector_regr;
  make_leaf_prediction=make_leaf_prediction_regression;
  stop_splitting_criterio=stop_splitting_criterio_regression;
  best_first_node_priority_function=best_first_node_priority_function_regr;
  not_significant_test=not_significant_test_regression;
  compute_score_from_table=compute_regression_score_from_table;
}

/* version non supervisee des arbres. On se sert quand meme
 * d'une sortie pour definir les predictions aux feuilles mais
 * on ne s'en sert pas du tout pour l'apprentissage
 */
DllExport void init_unsupervised_trees(int n_min) {
  min_node_size=n_min;
  summarize_vector=summarize_vector_unsupervised;
  make_leaf_prediction=make_leaf_prediction_unsupervised;
  stop_splitting_criterio=no_stop_splitting_criterio;
  best_first_node_priority_function=best_first_node_priority_function_unsupervised;
  not_significant_test=not_significant_test_if_constant_atts;
  compute_score_from_table=NULL;
}

/* fonction de test par defaut */

float *(*get_tree_prediction_vector)(int tree, int obj)=get_tree_prediction_vector_classical;

/* pour changer la fonction de test */

DllExport void set_test_by_dualpc(float lambda) {
  get_tree_prediction_vector=get_tree_prediction_vector_dualpc;
  lambda_noise=lambda;
}

DllExport void set_lambda_dualpc (float lambda) {
  lambda_noise=lambda;
}

DllExport void set_test_classical() {
  get_tree_prediction_vector=get_tree_prediction_vector_classical;
}

DllExport void set_test_without_obj() {
  get_tree_prediction_vector=get_tree_prediction_vector_without_obj;
}

/* fonction de test des arbres */

/* classification */

int test_tree_classification(int tree, int *ts_vector, int length_ts_vector) {
  int i;
  int nb_error=0;
  float error;
  
  for (i=0; i<length_ts_vector; i++) {
    if (my_best_class(get_tree_prediction_vector(tree, ts_vector[i]), nb_classes)!=getobjclass(ts_vector[i]))
      nb_error++;
  }
  
  error=(float)nb_error*1.0/length_ts_vector;
  //printf("Error rate: %f\% - %d/%d\n", error*100.0 , nb_error, length_ts_vector);
  fflush(stdout);
  return nb_error;
}

/* recupere la classification donnee par le dernier arbre courant */

DllExport int apply_current_tree_obj_clas(int obj, float *vector) {
  float *pred_vect=get_tree_prediction_vector(0, obj);
  float max_prob=-1.0, sum=0.0;
  int i, best_clas;

  for (i=0; i<nb_classes; i++) {
    sum+=(vector[i]=pred_vect[i]);
    if (pred_vect[i]>max_prob) {
      max_prob=pred_vect[i];
      best_clas=i;
    }
  }
  
  for (i=0; i<nb_classes; i++) {
    vector[i]/=sum;
  }
  
  return best_clas;
}

/* regression */

float test_tree_regression(int tree, int *ts_vector, int length_ts_vector) {
  int i;
  ERROR_TYPE sum_error=0.0;
  
  for (i=0; i<length_ts_vector; i++) {
    float err=getobjy(ts_vector[i])-(get_tree_prediction_vector(tree, ts_vector[i]))[0];
    sum_error+=(ERROR_TYPE)(err*err);
  }
  
  sum_error=sum_error/length_ts_vector;
  printf("Square error: %e\n",sum_error);
  fflush(stdout);
  return (float)sum_error;
}

DllExport float apply_current_tree_obj_regr(int obj) {
  return ((get_tree_prediction_vector(0, obj))[0]);
}


/* recupere des elements des arbres. Permet de le reconstruire du côté lisp */

int get_tested_attribute(int node) {
  return tested_attribute[node];
}

int get_left_successor(int node) {
  if (left_successor[node]==-1)
    return -1;
  else
    return node+left_successor[node];
}

int get_right_successor(int node) {
  return node+right_successor[node];
}

float get_num_att_threshold(int node) {
  return threshold[node].f;
}

void get_symb_att_subset(int node, int *boolean_vector) {
  int i;
  for (i=0; i<NB_VAL(tested_attribute[node]); i++) {
    boolean_vector[i]=BITN(threshold[node],i);
  }
}

int get_dt_leaf_prediction(int node) {
  return my_best_class(prediction_values[prediction[node]],nb_classes);
}

float get_rt_leaf_prediction(int node) {
  return prediction_values[prediction[node]][0];
}

/*********************/
/* ENSEMBLE D'ARBRES */
/*********************/


/* int nb_effective_att=20; */

/* void set_nb_effective_att(int v) { */
/*   nb_effective_att=v; */
/* } */

/* description de l'ensemble d'arbres courant */
#define MAX_NUMBER_OF_TREES 10000
int ltrees[MAX_NUMBER_OF_TREES];
float ltrees_weight[MAX_NUMBER_OF_TREES];
int current_nb_of_ensemble_terms=0;
int average_predictions_ltrees=1; /* 1 -> divide pred by the sum of weights 0-> no */
int early_stopping_ensemble=0; /* 1 -> on arrete la construction de l'ensemble au minimum
				       de l'erreur sur l'ensemble de test 
			       */

/* Différentes variantes pour la creation du LS */

/* identite */

float make_ls_vector_identity(int tree) {
  /* on ne fait rien */
  return 1.0;
}

/* bagging */

float make_ls_vector_bagging(int tree) {
  int i;

  /* on remet les poids a zero */
  for (i=0; i<global_learning_set_size; i++)
    object_weight[i]=0.0;

  /* on incremente les poids d'objets tires au hasard */
  for (i=0; i<global_learning_set_size; i++) {
    int rn=get_random_integer(global_learning_set_size);
    object_weight[rn]+=1.0;
  }
  
  /* on construit le ls en prenant les objets de poids non nuls */
  current_learning_set_size=0;
  for (i=0; i<global_learning_set_size; i++) {
    if (object_weight[i]!=0.0) {
      current_learning_set[current_learning_set_size]=i;
      current_learning_set_size++;
      if (class_weightp) {
	object_weight[i]=object_weight[i]*class_weight_vector[getobjclass_learn(i)];
      } else if (weight_attributep) { /* approximation grossiere, on devrait prendre en compte les poids
				       * directement mais ce serait tres lent 
				       */
	object_weight[i]*=getattval(object_mapping[i],weight_attribute);
      }
    }
  }
  
  return 1.0;
}

/* boosting */

#define BOOSTING_PROP_ERROR 0.5

float make_ls_vector_boosting(int tree) {
  int i, predc, truec, nb_error=0;
  SCORE_TYPE weighted_error=0.0, total_weight=0.0;
  float w_model;

  if (tree<0) {
    /* les poids sont initialises dans init_learning_set */
    /* for (i=0; i<global_learning_set_size; i++)
       object_weight[i]=1.0;
    */
    w_model=1.0;
    /* on initialise object_mark_cells si ce n'est pas fait */
    if (mark_cells_number<global_learning_set_size) {
      if (mark_cells_number!=0)
	MyFree((int *)object_mark_cells);
      object_mark_cells=(int *)MyMalloc((size_t)global_learning_set_size*sizeof(int));
      if (!object_mark_cells) {
	printf("impossible d'allouer de la memoire dans make_ls_vector_boosting\n");
	exit(0);
      }
      mark_cells_number=global_learning_set_size;
    }
    for (i=0; i<global_learning_set_size; i++)
      object_mark_cells[i]=0;
  } else {
    /* calcul l'erreur sur chaque objet */
    for (i=0; i<global_learning_set_size; i++) {
      total_weight+=object_weight[i];
      predc=my_best_class(get_tree_prediction_vector(tree, object_mapping[i]), nb_classes);
      truec=getobjclass_learn(i);
      if (predc!=truec) {
	object_mark_cells[i]=1;
	weighted_error+=object_weight[i];
	nb_error++;
      }
    }
    weighted_error/=total_weight;
    if (print_result)
      printf("Weighted error (ls) : %f (%d objects)", weighted_error, nb_error);

    /* calcul du poids */
    if (weighted_error>0.5) /* on arrete, le modele est trop mauvais */
      w_model=0.0;
    else if (weighted_error==0.0)
      w_model=-1.0;
    else {
      float w_error=weighted_error/BOOSTING_PROP_ERROR;
      float w_correct=(1.0-weighted_error)/(1.0-BOOSTING_PROP_ERROR);
      w_model=log((1.0-weighted_error)/weighted_error);
      for (i=0; i<global_learning_set_size; i++) {
	if (object_mark_cells[i]==1) {
	  object_mark_cells[i]=0;
	  object_weight[i]/=w_error;
	} else {
	  object_weight[i]/=w_correct;
	}
      }
    }
  }

  /* on construit le vecteur d'attributs en prenant les objets de poids non nuls */
  current_learning_set_size=0;
  for (i=0; i<global_learning_set_size; i++) {
    if (object_weight[i]!=0.0) {
      current_learning_set[current_learning_set_size]=i;
      current_learning_set_size++;
    }
  }

  return w_model;
}

float make_ls_vector_boosting_c45_pruned(int tree) {
  int i, predc, truec, nb_error=0;
  SCORE_TYPE weighted_error=0.0, total_weight=0.0;
  float w_model;

  if (tree<0) {
    /* les poids sont initialises dans init_learning_set */
    /* for (i=0; i<global_learning_set_size; i++)
       object_weight[i]=1.0;
    */
    w_model=1.0;
    /* on initialise object_mark_cells si ce n'est pas fait */
    if (mark_cells_number<global_learning_set_size) {
      if (mark_cells_number!=0)
	MyFree((int *)object_mark_cells);
      object_mark_cells=(int *)MyMalloc((size_t)global_learning_set_size*sizeof(int));
      if (!object_mark_cells) {
	printf("impossible d'allouer de la memoire dans make_ls_vector_boosting\n");
	exit(0);
      }
      mark_cells_number=global_learning_set_size;
    }
    for (i=0; i<global_learning_set_size; i++)
      object_mark_cells[i]=0;
  } else {
    /* on elague l'arbre avec l'algo de C45 */
    int nb_nodes_before=get_tree_nb_nodes(tree);
    int print_result_back=print_result;
    print_result=0;
    prune_tree_c45(tree);
    print_result=print_result_back;
    if (print_result) {
      printf("Pruning: C=%d->C=%d\n", nb_nodes_before, get_tree_nb_nodes(tree));
      fflush(stdout);
    }
    
    /* calcul l'erreur sur chaque objet */
    for (i=0; i<global_learning_set_size; i++) {
      total_weight+=object_weight[i];
      predc=my_best_class(get_tree_prediction_vector(tree, object_mapping[i]), nb_classes);
      truec=getobjclass_learn(i);
      if (predc!=truec) {
	object_mark_cells[i]=1;
	weighted_error+=object_weight[i];
	nb_error++;
      }
    }
    weighted_error/=total_weight;
    if (print_result)
      printf("Weighted error (ls) : %f (%d objects)", weighted_error, nb_error);

    /* calcul du poids */
    if (weighted_error>0.5) /* on arrete, le modele est trop mauvais */
      w_model=0.0;
    else if (weighted_error==0.0)
      w_model=-1.0;
    else {
      float w_error=weighted_error/BOOSTING_PROP_ERROR;
      float w_correct=(1.0-weighted_error)/(1.0-BOOSTING_PROP_ERROR);
      w_model=log((1.0-weighted_error)/weighted_error);
      for (i=0; i<global_learning_set_size; i++) {
	if (object_mark_cells[i]==1) {
	  object_mark_cells[i]=0;
	  object_weight[i]/=w_error;
	} else {
	  object_weight[i]/=w_correct;
	}
      }
    }
  }

  /* on construit le vecteur du ls en prenant les objets de poids non nuls */
  current_learning_set_size=0;
  for (i=0; i<global_learning_set_size; i++) {
    if (object_weight[i]!=0.0) {
      current_learning_set[current_learning_set_size]=i;
      current_learning_set_size++;
    }
  }

  return w_model;
}

/* version experimentale avec du dual pc pour rechercher les points mal classes */

float make_ls_vector_boosting_dualpc(int tree) {
  int i, predc, truec, nb_error=0;
  SCORE_TYPE weighted_error=0.0, total_weight=0.0;
  float w_model;

  if (tree<0) {
    for (i=0; i<global_learning_set_size; i++)
      object_weight[i]=1.0;
    w_model=1.0;
    /* on initialise object_mark_cells si ce n'est pas fait */
    if (mark_cells_number<global_learning_set_size) {
      if (mark_cells_number!=0)
	MyFree((int *)object_mark_cells);
      object_mark_cells=(int *)MyMalloc((size_t)global_learning_set_size*sizeof(int));
      if (!object_mark_cells) {
	printf("impossible d'allouer de la memoire dans make_ls_vector_boosting\n");
	exit(0);
      }
      mark_cells_number=global_learning_set_size;
    }
    for (i=0; i<global_learning_set_size; i++)
      object_mark_cells[i]=0;
    /* on calcule les std dev pour le dual PC */
    jassertfalse;
    compute_att_stddev_from_ls(/*object_mapping, global_learning_set_size, attribute_vector, nb_attributes*/);
  } else {
    /* calcul l'erreur sur chaque objet */
    for (i=0; i<global_learning_set_size; i++) {
      total_weight+=object_weight[i];
      predc=my_best_class(get_tree_prediction_vector_dualpc(tree, object_mapping[i]), nb_classes);
      truec=getobjclass_learn(i);
      if (predc!=truec) {
	object_mark_cells[i]=1;
	weighted_error+=object_weight[i];
	nb_error++;
      }
    }
    weighted_error/=total_weight;
    if (print_result)
      printf("Weighted error (ls) : %f (%d objects)", weighted_error, nb_error);

    /* calcul du poids */
    if (weighted_error>0.5) /* on arrete, le modele est trop mauvais */
      w_model=0.0;
    else if (weighted_error==0.0)
      w_model=-1.0;
    else {
      float w_error=weighted_error/BOOSTING_PROP_ERROR;
      float w_correct=(1.0-weighted_error)/(1.0-BOOSTING_PROP_ERROR);
      w_model=log((1.0-weighted_error)/weighted_error);
      for (i=0; i<global_learning_set_size; i++) {
	if (object_mark_cells[i]==1) {
	  object_mark_cells[i]=0;
	  object_weight[i]/=w_error;
	} else {
	  object_weight[i]/=w_correct;
	}
      }
    }
  }

  /* on construit le vecteur d'attributs en prenant les objets de poids non nuls */
  current_learning_set_size=0;
  for (i=0; i<global_learning_set_size; i++) {
    if (object_weight[i]!=0.0) {
      current_learning_set[current_learning_set_size]=i;
      current_learning_set_size++;
    }
  }

  return w_model;
}

/*----------------------------*/
/* MART regression (Friedman) */
/*----------------------------*/

/* squared error (residual fitting) */

ERROR_TYPE *temp_output;
int length_temp_output=0;

float getobjy_learn_temp(int index) {
  return (float)temp_output[index];
}

int temp_best_first_max_nb_tests;
float mart_mu=0.1;

float make_ls_vector_mart_regr(int tree) {
  int i;

  if (tree<0) { /* initialisation */
    for (i=0; i<global_learning_set_size; i++)
      if (weight_attributep) {
	object_weight[i]=getattval(object_mapping[i],weight_attribute);
      } else
	object_weight[i]=1.0;
    /* on initialise temp_output si ce n'est pas fait */
    if (length_temp_output<global_learning_set_size) {
      if (length_temp_output!=0)
	MyFree((ERROR_TYPE *)temp_output);
      temp_output=(ERROR_TYPE *)MyMalloc((size_t)global_learning_set_size*sizeof(ERROR_TYPE));
      if (!temp_output) {
	printf("impossible d'allouer de la memoire dans make_ls_vector_mart\n");
	exit(0);
      }
      length_temp_output=global_learning_set_size;
    }
    /* on le remplit */
    for (i=0; i<global_learning_set_size; i++)
      temp_output[i]=getobjy_learn(i);
    /* on fixe la fonction de sortie */
    getobjy_learn=getobjy_learn_temp;
    /* pour que le premier modele ait une complexite de 1 (pas de test) */
    temp_best_first_max_nb_tests=best_first_max_nb_tests;
    best_first_max_nb_tests=0;
  } else {
    
    /* met à jour l'erreur sur chaque objet */
    if (current_nb_of_ensemble_terms==0) {
      best_first_max_nb_tests=temp_best_first_max_nb_tests;
      for (i=0; i<global_learning_set_size; i++) {
	temp_output[i]-=(get_tree_prediction_vector(tree, object_mapping[i]))[0];
      }
      return 1.0;
    } else {
      for (i=0; i<global_learning_set_size; i++) {
	temp_output[i]-=mart_mu*(get_tree_prediction_vector(tree, object_mapping[i]))[0];
      }
      return mart_mu;
    }
  }

  return 1.0;

}

/*----------------------------*/
/* LOGit boost (Friedman)     */
/*----------------------------*/

/* squared error (residual fitting) */

float (*getobjy_learn_saved)(int index);

ERROR_TYPE *temp_output_f;
int length_temp_output_f=0;

float w_min=10e-15;
float z_max=10.0;

float make_ls_vector_logitboost(int tree) {
  int i;
  float p;

  if (tree<0) { /* initialisation */
    for (i=0; i<global_learning_set_size; i++)
      if (weight_attributep) {
	object_weight[i]=getattval(object_mapping[i],weight_attribute)*0.5;
      } else
	object_weight[i]=0.5;

    /* on initialise temp_output et temp_output_f si ce n'est pas fait */
    if (length_temp_output<global_learning_set_size) {
      if (length_temp_output!=0)
	MyFree((ERROR_TYPE *)temp_output);
      temp_output=(ERROR_TYPE *)MyMalloc((size_t)global_learning_set_size*sizeof(ERROR_TYPE));
      if (!temp_output) {
	printf("impossible d'allouer de la memoire dans make_ls_vector_logitboost\n");
	exit(0);
      }
      length_temp_output=global_learning_set_size;
    }
    if (length_temp_output_f<global_learning_set_size) {
      if (length_temp_output_f!=0)
	MyFree((ERROR_TYPE *)temp_output_f);
      temp_output_f=(ERROR_TYPE *)MyMalloc((size_t)global_learning_set_size*sizeof(ERROR_TYPE));
      if (!temp_output_f) {
	printf("impossible d'allouer de la memoire dans make_ls_vector_logitboost\n");
	exit(0);
      }
      length_temp_output=global_learning_set_size;
    }

    /* on les remplit */
    for (i=0; i<global_learning_set_size; i++)
      temp_output[i]=4*(getobjy_learn(i)-0.5);
    for (i=0; i<global_learning_set_size; i++)
      temp_output_f[i]=0.0;

    /* on fixe la fonction de sortie */
    getobjy_learn_saved=getobjy_learn;
    getobjy_learn=getobjy_learn_temp;

  } else {
    
    /* met à jour l'erreur sur chaque objet */
    for (i=0; i<global_learning_set_size; i++) {
      temp_output_f[i]+=mart_mu*(get_tree_prediction_vector(tree, object_mapping[i]))[0];
      p=exp(temp_output_f[i])/(1+exp(temp_output_f[i]));
      object_weight[i]=p*(1-p);
      object_weight[i]=MAX(w_min,object_weight[i]);
      temp_output[i]=(getobjy_learn_saved(i)-p)/(p*(1-p));
      temp_output[i]=MIN(MAX(temp_output[i],-z_max),z_max);
    }

    return mart_mu;
    
  }

  return 1.0;

}

/* différentes variantes pour le test des arbres */

float **test_data;
ERROR_TYPE current_error_single;
ERROR_TYPE current_error_combined;
float current_sum_weight=0.0;

/* fonction d'update pour la classification */

float update_test_data_clas(int tree, float weight, int *ts_vector, int length_ts_vector) {
  int i,j;
  int nb_error_single=0;
  int nb_error_combined=0;
  
  for (i=0; i<length_ts_vector; i++) {
    float *proba_vect=get_tree_prediction_vector(tree,ts_vector[i]);
    float max_single=-1.0;
    int index_single;
    float max_combined=-1.0;
    int index_combined;
    int good_class=getobjclass(ts_vector[i]);
    
    for (j=0; j<nb_classes; j++) {
      test_data[i][j]+=weight*proba_vect[j];
      if (proba_vect[j]>max_single) {
	max_single=proba_vect[j];
	index_single=j;
      }
      if (test_data[i][j]>max_combined) {
	max_combined=test_data[i][j];
	index_combined=j;
      }
    }
    if (index_single!=good_class)
      nb_error_single++;
    if (index_combined!=good_class)
      nb_error_combined++;
  }
  current_error_single=nb_error_single*1.0/length_ts_vector;
  current_error_combined=nb_error_combined*1.0/length_ts_vector;

  /*if (print_result)
    printf("error single %f\%  - %d/%d - error combined %f\% - %d/%d",100.0*current_error_single,
	   nb_error_single, length_ts_vector, nb_error_combined*100.0/length_ts_vector, 
	   nb_error_combined, length_ts_vector);*/
  return (nb_error_combined*100.0/length_ts_vector);
}

/* fonction d'update pour la regression */

float update_test_data_regr(int tree, float weight, int *ts_vector, int length_ts_vector) {
  int i;
  ERROR_TYPE err_combined, err_single;

  current_sum_weight+=weight;
  current_error_single=0.0;
  current_error_combined=0.0;
  
  for (i=0; i<length_ts_vector; i++) {
    float predy=(get_tree_prediction_vector(tree,ts_vector[i]))[0];
    
    test_data[i][0]+=weight*predy;
    err_single=(predy-getobjy(ts_vector[i]));
    if (average_predictions_ltrees) 
      err_combined=((test_data[i][0]/current_sum_weight)-getobjy(ts_vector[i]));
    else
      err_combined=(test_data[i][0]-getobjy(ts_vector[i]));
    current_error_single+=(err_single*err_single);
    current_error_combined+=(err_combined*err_combined);
  }

  current_error_single=current_error_single/length_ts_vector;
  current_error_combined=current_error_combined/length_ts_vector;

  if (print_result) printf("- sq.err. single %e - sq.err. combined %e",current_error_single, current_error_combined);
  return current_error_combined;
}

/* interface lisp */

float (*update_test_data)(int tree, float weight, int *ts_vector, int length_ts_vector);

DllExport void set_ensemble_method_parameters(int i, int nb_terms, int clas_regr, int test_on_test_set, int se, int sewg, int es, float mu, float zm) {
  if (i==1) {
    make_ls_vector=make_ls_vector_bagging;
    average_predictions_ltrees=1;
  } else if (i==2) {
    make_ls_vector=make_ls_vector_boosting;
    average_predictions_ltrees=1;
  } else if (i==3) {
    make_ls_vector=make_ls_vector_boosting_dualpc;
    average_predictions_ltrees=1;
  } else if (i==4) {
    if (goal_type==REGRESSION) {
      make_ls_vector=make_ls_vector_mart_regr;
    }
#ifdef LOAD_OK3 /* seulement si on utilise le module OK3 */
    else if (goal_type==DISTANCE) {
      make_ls_vector=make_ls_vector_mart_distance;
      make_leaf_prediction=make_leaf_prediction_distance_mart;
    }
#endif
#ifdef LOAD_MULTIREGR
    else if (goal_type==MULTIREGR) {
      make_ls_vector=make_ls_vector_mart_multiregr;
    }
#endif
    mart_mu=mu;
    average_predictions_ltrees=0;
  } else if (i==5) {
    make_ls_vector=make_ls_vector_boosting_c45_pruned;
    average_predictions_ltrees=1;
  } 
#ifdef LOAD_OK3
  else if (i==6) {
    make_ls_vector=make_ls_vector_mart_ok_uniregr;
    mart_mu=mu;
    average_predictions_ltrees=0;
  }
#endif  
  else if (i==7) {
    if (goal_type!=REGRESSION) {
      printf("Logit boost requires a goal-regression\n");
      fflush(stdout);
      make_ls_vector=make_ls_vector_identity;
      average_predictions_ltrees=1;
    } else {
      make_ls_vector=make_ls_vector_logitboost;
      mart_mu=mu;
      z_max=zm;
      average_predictions_ltrees=0;
    }
  } else {
    make_ls_vector=make_ls_vector_identity;
    average_predictions_ltrees=1;
  }
  
  if (clas_regr==0)
    update_test_data=update_test_data_clas;
  else
    update_test_data=update_test_data_regr;

  test_ensemble_while_growing=test_on_test_set;
  store_ensemble=se;
  save_ensemble_while_growing=sewg;

  early_stopping_ensemble=es;
  number_of_ensemble_terms=nb_terms;  
}

/* pour sauvegarder les ls */
int save_ensemble_ls=0;
int *save_ensemble_ls_vector=NULL;
float *save_ensemble_ls_weight=NULL;
int save_ensemble_ls_size[MAX_NUMBER_OF_TREES];
int save_ensemble_ls_pos=0;
int save_ensemble_ls_nb_ls=0;

DllExport void init_save_ensemble_ls(int b) {

  if (save_ensemble_ls && (save_ensemble_ls_vector!=NULL)) {
    MyFree((int *)save_ensemble_ls_vector);
    save_ensemble_ls_vector=NULL;
    MyFree((float *)save_ensemble_ls_weight);
    save_ensemble_ls_weight=NULL;
  }

  save_ensemble_ls=b;

  if (save_ensemble_ls) {
    int s=number_of_ensemble_terms*global_learning_set_size;
    save_ensemble_ls_vector=(int *)MyMalloc((size_t)s*sizeof(int));
    save_ensemble_ls_weight=(float *)MyMalloc((size_t)s*sizeof(float));
    save_ensemble_ls_pos=0;
    save_ensemble_ls_nb_ls=0;
  }
}

/* Construction de l'ensemble d'arbres */

DllExport float build_one_tree_ensemble(int *ts_vector, int length_ts_vector) {
  int i, j, t;
  int sum_complexity=0;
  float sum_error_single=0.0;
  float min_error; int index_min_error;
  float current_weight;
  int nbn=0;
  FILE *fp;

  /* on vide tout */
  clean_all_trees();
  current_nb_of_ensemble_terms=0;
  current_sum_weight=0.0;
  mean_nb_random_splits=0;
  nb_call_to_find_a_split=0;

  /* verification de la memoire */
  if (best_first)
    nbn=2*best_first_max_nb_tests+1;
  else
    nbn=(2*global_learning_set_size-1);

  if (!store_ensemble) { /* ensemble non stocke */
    if (size_current_tree_table < nbn) {
      printf("memoire trop faible pour construire un arbre\n");
      return -1.0;
    }
  } else {/* ensemble stocke en memoire totalement */
    
    if (size_current_tree_table <(number_of_ensemble_terms*nbn)) {
      printf("memoire trop faible pour construire l'ensemble d'arbres\n");
      return -1.0;
    }
    if (number_of_ensemble_terms>MAX_NUMBER_OF_TREES) {
      printf("maximum %d trees, increase table size\n",MAX_NUMBER_OF_TREES);
      return -1.0;
    } 
  }

  if (save_ensemble_while_growing) {
    fp=fopen("temp-ensemble-trees.dat", "wb");
    fwrite(&average_predictions_ltrees, sizeof(float), 1, fp);
  }
  
  /* initialisation du test */
  if (print_result && length_ts_vector>0) {
    printf("initialisation de l'ensemble de test...\n");
    fflush(stdout);
  }
  /* allocation de la matrice de test si necessaire */
  /* de la taille (length_ts_vector*nb_pred) */
  if (length_ts_vector<=0 || nb_classes<=0)
    test_ensemble_while_growing=0; /* on desactive le test si pas d'objets de test */

  if (test_ensemble_while_growing) {
    test_data=allocate_table_float(length_ts_vector,nb_classes);
    if (test_data==0) {
      printf("impossible d'allouer la memoire pour l'ensemble de test\ndesactivation du test\n");
      test_ensemble_while_growing=0;
      fflush(stdout);
    } else {
      for (i=0; i<length_ts_vector; i++) {
	for (j=0; j<nb_classes; j++) {
	  test_data[i][j]=0.0;
	}
      }
    }
  }
  
  /* initialisation de l'ensemble d'apprentissage */
  make_ls_vector(-1);
  
  /* boucle de construction */
  for (t=0; t<number_of_ensemble_terms; t++) {
    int current_tree;

    /* si on le demande, on sauve les LS */
    /* pour compute_node_subset_current_ensemble. Idealement, on devrait l'implementer
     * aussi pour le boosting en tenant compte des poids. Ca permettrait de faire des calculs
     * de variable importance plus precise.
     */
    if (save_ensemble_ls) {
      save_ensemble_ls_size[save_ensemble_ls_nb_ls]=current_learning_set_size;
      save_ensemble_ls_nb_ls++;
      for (i=0; i<current_learning_set_size; i++) {
	save_ensemble_ls_vector[save_ensemble_ls_pos]=current_learning_set[i];
	save_ensemble_ls_weight[save_ensemble_ls_pos]=object_weight[current_learning_set[i]];
	save_ensemble_ls_pos++;
      }
    }
    rtree_update_progression((size_t)t + 1);
    if (print_result) {
      printf("model %d...",t+1);
      fflush(stdout);
    }

    /* replacement of missing values */
    if (missing_values) {
      replace_missing_values();
    }

    /* prepare le tri */
    if (sorting_variant==GLOBAL_SORT_REARRANGE)
      make_sorted_matrix_copy(current_learning_set, current_learning_set_size);

    /* melange les attributes: */
    /* randomize_vector(attribute_vector, old_nb_attributes); */
    /* nb_attributes=nb_effective_att; */

    /* construction du modele */
    current_tree=build_one_tree();

    sum_complexity+=index_nodes-current_tree+1;
    current_weight=make_ls_vector(current_tree);

    if (print_result) {
      printf("(compl=%d) ", index_nodes-current_tree+1);
      fflush(stdout);
    }

    /* calcul de l'erreur */
    if (test_ensemble_while_growing) {
      CORETABLE_TYPE *temp;
      if (missing_values) {
	temp=core_table;
	core_table=backup_core_table;
      }
      update_test_data(current_tree, current_weight, ts_vector, length_ts_vector);
      if (t==0) {
	min_error=current_error_combined;
	index_min_error=0;
      } else if (current_error_combined<min_error) {
	min_error=current_error_combined;
	index_min_error=t;
      }
      sum_error_single+=current_error_single;
      if (missing_values) {
	core_table=temp;
      }
    }

    /* stockage du modele si on ne teste pas tout de suite */
    if (save_ensemble_while_growing) {
      fwrite(&current_weight,sizeof(float),1,fp);
      write_one_tree(current_tree, fp);
    }

    if (store_ensemble) {
      ltrees[t]=current_tree;
      ltrees_weight[t]=current_weight;
      current_nb_of_ensemble_terms++;
    } else
      clean_all_trees();

    if (print_result) printf("\n");
    
    if (current_weight==0.0) {
      /* on arrete, le modele precedent est trop mauvais */
      if (print_result) {
	printf("Stop (error too high)\n");
	fflush(stdout);
      } 
      t=number_of_ensemble_terms;
    } else if (current_weight<0) {
      /* on arrete, le dernier modele est parfait. Il devient le seul
	 modele */
      if (print_result) {
	printf("Stop (null error)\n");
	fflush(stdout);
      }
      t=number_of_ensemble_terms;
      if (store_ensemble) {
	current_nb_of_ensemble_terms=1;
	ltrees[0]=ltrees[t];
	ltrees_weight[0]=1.0;
      }
      sum_complexity=index_nodes-current_tree+1;
    }
  }
  context_result(T("Complexity"), sum_complexity);
  if (print_result) {
    printf("complexity: %d",sum_complexity);
    if (test_ensemble_while_growing)
      printf(", average error single=%f", sum_error_single/t);
    printf("\n");
    fflush(stdout);
    if (mean_nb_random_splits>0) {
      printf("mean number of random splits per node = %f(%ld/%ld)\n",(double)mean_nb_random_splits/nb_call_to_find_a_split,
	     mean_nb_random_splits, nb_call_to_find_a_split);
      fflush(stdout);
    }
  }
  
  /* early stopping */
  if (test_ensemble_while_growing && early_stopping_ensemble) {
    current_nb_of_ensemble_terms=index_min_error+1;
    if (print_result) {
      printf("\nEarly stopping at T=%d/%d - error=%f(/%f)\n",current_nb_of_ensemble_terms, 
	     number_of_ensemble_terms, min_error, current_error_combined);
      fflush(stdout);
    }
    current_error_combined=min_error;
  }

  if (save_ensemble_while_growing) {
    fclose(fp);
    fp=fopen("temp-ensemble-nb-trees.dat", "wb");
    fwrite(&current_nb_of_ensemble_terms, sizeof(int), 1, fp);
    fclose(fp);
  }

  if (test_ensemble_while_growing) {
    if (test_data!=NULL)
      free_table_float(test_data,length_ts_vector);
    return (float)current_error_combined;
  } else
    return -1.0;
}

/* fonction pour le test */

float make_test_data_ltrees(int *ts_vector, int length_ts_vector){
  int i, j, t;
  float sum_weight=0.0;
  
  /* initialisation du test */
  if (print_result) {
    printf("initialisation de l'ensemble de test...\n");
    fflush(stdout);
  }
  /* nb_classes vaut 1 dans le cas de la regression */
  for (i=0; i<length_ts_vector; i++) {
    for (j=0; j<nb_classes; j++) {
      test_data[i][j]=0.0;
    }
  }
  
  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    int current_tree=ltrees[t];
    float w=ltrees_weight[t];

    if (print_result) {
      printf("Testing model %d...",t+1);
      fflush(stdout);
    }
    
    /* mise a jour des donnees de test */
    for (i=0; i<length_ts_vector; i++) {
      float *proba_vect=get_tree_prediction_vector(current_tree, ts_vector[i]);
      for (j=0; j<nb_classes; j++)
	test_data[i][j]+=w*proba_vect[j];
    }
    if (print_result) {
      printf("\n"); 
      fflush(stdout);
    }
    sum_weight+=w;
  }
  return sum_weight;
}

DllExport float test_current_ltrees_clas(int* ts_vector, int length_ts_vector) {
  int i, j, nb_error_combined=0;

  /* completement idiot en fait, on peut n'allouer qu'un seul vecteur */
  test_data=allocate_table_float(length_ts_vector,nb_classes);
  if (test_data==NULL) {
    printf("Impossible d'allouer la memoire pour le test (test_current_ltrees_clas)\n");
    fflush(stdout);
    return -1.0;
  }

  make_test_data_ltrees(ts_vector, length_ts_vector);

  /* calcul de l'erreur */
  for (i=0; i<length_ts_vector; i++) {
    float max_combined=-1.0;
    int index_combined;
    for (j=0; j<nb_classes; j++) {
      if (test_data[i][j]>max_combined) {
	max_combined=test_data[i][j];
	index_combined=j;
      }
    }
    if (index_combined!=getobjclass(ts_vector[i]))
      nb_error_combined++;
  }
  
  /* affichage de l'erreur */
  /*printf("error combined %f\% - %d/%d\n", nb_error_combined*100.0/length_ts_vector,
	 nb_error_combined, length_ts_vector);*/
  fflush(stdout);

  free_table_float(test_data,length_ts_vector);
  
  return (nb_error_combined*100.0/length_ts_vector);
}

/* un petit truc pour pouvoir tester avec moins de termes */

DllExport int apply_current_ltrees_obj_clas(int obj, float *vector) {
  int i,t,best_class;
  float sum=0.0, max_proba=-1.0;
  
  for (i=0; i<nb_classes; i++) {
    vector[i]=0.0;
  }

  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    float *proba_vect=get_tree_prediction_vector(ltrees[t], obj);
    float w=ltrees_weight[t];
    for (i=0; i<nb_classes; i++)
      vector[i]+=w*proba_vect[i];
  }
  
  for (i=0; i<nb_classes; i++) {
    sum+=vector[i];
    if (vector[i]>max_proba) {
      max_proba=vector[i];
      best_class=i;
    }
  }

  for (i=0; i<nb_classes; i++)
    vector[i]/=sum;

  return best_class;
}

DllExport float test_current_ltrees_regr(int *ts_vector, int length_ts_vector) {
  int i;
  ERROR_TYPE sum_error=0.0;
  float sum_weight;

  /* completement idiot en fait, on peut n'allouer qu'un seul vecteur */
  test_data=allocate_table_float(length_ts_vector,nb_classes);
  if (test_data==NULL) {
    printf("Impossible d'allouer la memoire pour le test (test_current_ltrees_clas)\n");
    fflush(stdout);
    return -1.0;
  }

  sum_weight=make_test_data_ltrees(ts_vector, length_ts_vector);
    
  /* calcul de l'erreur */
  for (i=0; i<length_ts_vector; i++) {
    ERROR_TYPE err;
    if (average_predictions_ltrees)
      err=getobjy(ts_vector[i])-(test_data[i][0]/sum_weight);
    else
      err=getobjy(ts_vector[i])-test_data[i][0];
    sum_error+=(err*err);
  }
  
  /* affichage de l'erreur */
  sum_error=sum_error/length_ts_vector;
  printf("Square error: %e (%d objs)\n", sum_error, length_ts_vector);
  fflush(stdout);

  free_table_float(test_data,length_ts_vector);
  
  return (float)sum_error;
}

DllExport float apply_current_ltrees_obj_regr(int obj) {
  int t;
  float sum=0.0; float sum_weight=0.0;
    
  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    sum_weight+=ltrees_weight[t];
    sum+=ltrees_weight[t]*(get_tree_prediction_vector(ltrees[t], obj))[0];
  }
  
  if (average_predictions_ltrees)
    return (sum/sum_weight);
  else
    return sum;
}

DllExport float test_current_decision_tree(int* ts_vector, int length_ts_vector) {
  return ((100.0*test_tree_classification(0, ts_vector, length_ts_vector))/length_ts_vector);
}

DllExport float test_current_regression_tree(int* ts_vector, int length_ts_vector) {
  return test_tree_regression(0, ts_vector, length_ts_vector);
}

/* pour pouvoir tester les premiers arbres d'un ensemble */

DllExport void set_current_nb_of_ensemble_terms(int nb) {
  current_nb_of_ensemble_terms=nb;
}

/* pour voir le nombre d'arbres construits */
int get_current_nb_of_ensemble_terms() {
  return current_nb_of_ensemble_terms;
}

/* pour pouvoir tester un arbre comme un ensemble d'un seul arbre */

DllExport void set_one_tree_ensemble() {
  current_nb_of_ensemble_terms=1;
  ltrees[0]=0;
  ltrees_weight[0]=1.0;
}

/* pour virer l'ensemble */

void clean_all_ensemble () {
  current_nb_of_ensemble_terms=0;
}

/* renvoit au c la racine de l'arbre i de l'ensemble */

int get_root_tree_ensemble(int i) {
  return ltrees[i];
}

float get_weight_tree_ensemble(int i) {
  return ltrees_weight[i];
}


DllExport void pretty_print_tree_from_ensemble(int tree, int max_depth) {
  prettyprinttree(ltrees[tree],max_depth);
}

DllExport int get_tree_leaf_from_ensemble(int tree, int obj) {
  return get_tree_leaf(ltrees[tree],obj);
}

DllExport int get_tree_pred_nb_from_ensemble(int tree, int obj) {
  return prediction[get_tree_leaf(ltrees[tree],obj)];
}

/********
 * 1-NN *
 ********/

/* 1-NN implementation, assume that the standard deviation have already been computed */

int *knn_ls;
int knn_ls_size=0;
int normalizep=1;

DllExport void init_nn(int *ls_vector, int length_ls_vector, int norm, int print_res) {
  knn_ls=ls_vector;
  object_mapping=ls_vector;
  knn_ls_size=length_ls_vector;
  global_learning_set_size=length_ls_vector;
  normalizep=norm;
  print_result=print_res;
  if (norm)
    compute_att_stddev_from_ls();
}

int get_nn(int o) {
  int i,j=0;
  float current_min_distance=PLUS_INFINI;
  int current_nn=-1;

  for (i=0; i<knn_ls_size; i++) {
    float current_sum=0.0, d;
    int att;
    j=0;
    do {
      att=attribute_vector[j];
      if (get_att_stddev(att)!=0.0) {
	d=(getattval(o,att)-getattval(knn_ls[i],att))/get_att_stddev(att);
	current_sum+=d*d;
      }
      j++;
    } while ((j<nb_attributes) && (current_sum<current_min_distance));
    if (current_sum<current_min_distance) {
      current_min_distance=current_sum;
      current_nn=i;
    }
  }
  return current_nn;
}

int get_k_nn(int o, int k, int *lobj, float *ldist) {
  int i,j=0, pos;
  float current_min_distance=PLUS_INFINI;
  int current_nn=-1;

  for (i=0; i<k; i++) {
    ldist[i]=PLUS_INFINI;
  }

  for (i=0; i<knn_ls_size; i++) {
    float current_sum=0.0, d;
    int att;
    j=0;
    do {
      att=attribute_vector[j];
      if (normalizep && get_att_stddev(att)!=0.0) {
	d=(getattval(o,att)-getattval(knn_ls[i],att))/get_att_stddev(att);
      } else {
	d=(getattval(o,att)-getattval(knn_ls[i],att));
      }
      current_sum+=d*d;
      j++;
    } while ((j<nb_attributes) && (current_sum<current_min_distance));
    if (current_sum<current_min_distance) {
      /* insert l'objet dans la liste */
      pos=k-1;
      while ((pos-1>=0) && (ldist[pos-1]>current_sum)) {
	ldist[pos]=ldist[pos-1];
	lobj[pos]=lobj[pos-1];
	pos--;
      }
      ldist[pos]=current_sum;
      lobj[pos]=i;
      current_min_distance=ldist[k-1];
    }
  }
  return current_nn;
}

DllExport int get_1nn_class_pred(int o) {
  return (getobjclass(knn_ls[get_nn(o)]));
}

DllExport float get_1nn_regr_pred(int o) {
  return (getobjy(knn_ls[get_nn(o)]));
}

/* teste le 1-nn */

DllExport float test_1nn_clas(int *ts_vector, int length_ts_vector) {
  int i, nb_error=0;

  /* calcul de l'erreur */
  for (i=0; i<length_ts_vector; i++) {
    int class_1nn=get_1nn_class_pred(ts_vector[i]);
    if (class_1nn!=getobjclass(ts_vector[i])) {
      nb_error++;
    }
  }
  return (nb_error*100.0/length_ts_vector);
}

/* test le k-nn pour differentes valeurs de k */

DllExport float test_knn_clas(int *ts_vector, int length_ts_vector, int max_k, double *lerrors) {
  int i, j;
  int *lobj;
  float *ldist;
  
  lobj=(int *)MyMalloc((size_t)max_k*sizeof(int));
  ldist=(float *)MyMalloc((size_t)max_k*sizeof(float));

  for (i=0; i<max_k; i++) {
    lerrors[i]=0.0;
  }

  for (i=0; i<length_ts_vector; i++) {
    int obj_class=getobjclass(ts_vector[i]);
    int pred_class;

    for (j=0;j<nb_classes; j++) {
      table_score[0][j]=0.0;
    }
    get_k_nn(ts_vector[i], max_k, lobj, ldist);
    if (print_result) {
      printf(".");
      fflush(stdout);
    }
    for (j=0; j<max_k; j++) {
      table_score[0][getobjclass(knn_ls[lobj[j]])]++;
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

  MyFree((int *)lobj);
  MyFree((float *)ldist);
  return 0.f;
}

float test_1nn_regr(int *ts_vector, int length_ts_vector) {
  int i;
  ERROR_TYPE sum_error=0.0;

  /* calcul de l'erreur */
  for (i=0; i<length_ts_vector; i++) {
    ERROR_TYPE err=getobjy(ts_vector[i])-(get_1nn_regr_pred(ts_vector[i]));
    sum_error+=(err*err);
  }
  sum_error=sum_error/length_ts_vector;
  return (float)sum_error;
}

/* test le k-nn pour differentes valeurs de k */

float test_knn_regr(int *ts_vector, int length_ts_vector, int max_k, double *lerrors) {
  int i, j;
  int *lobj;
  float *ldist;
  
  lobj=(int *)MyMalloc((size_t)max_k*sizeof(int));
  ldist=(float *)MyMalloc((size_t)max_k*sizeof(float));

  for (i=0; i<max_k; i++) {
    lerrors[i]=0.0;
  }

  for (i=0; i<length_ts_vector; i++) {
    float y=getobjy(ts_vector[i]);
    ERROR_TYPE pred_regr=0.0;
    ERROR_TYPE err;

    get_k_nn(ts_vector[i], max_k, lobj, ldist);

    for (j=0; j<max_k; j++) {
      
      pred_regr+=getobjy(knn_ls[lobj[j]]);
      err=y-(pred_regr/(j+1));
      lerrors[j]+=(err*err);
    }
  }
  
  for (i=0; i<max_k; i++) {
    lerrors[i]=lerrors[i]/length_ts_vector;
  }

  MyFree((int *)lobj);
  MyFree((float *)ldist);
  return 0.f;
}

/* apply_knn_class */

int lobj_temp[50];
float ldist_temp[50];

DllExport int apply_knn_class(int obj, float *vector, int current_k) {
  int j;

  for (j=0;j<nb_classes; j++) {
    vector[j]=0.0;
  }

  get_k_nn(obj, current_k, lobj_temp, ldist_temp);

  for (j=0; j<current_k; j++) {
    vector[getobjclass(knn_ls[lobj_temp[j]])]++;
  }
  
  for (j=0; j<current_k; j++)
    vector[j]/=current_k;
  
  return my_best_class(vector, nb_classes);
}

/* compare le 1nn with euclidian distance with the 1nn with ltrees distance */

float compare_1nn_ltrees_fast(int *ts_vector, int length_ts_vector) {
  int nb_error=0;
  int i,t;

  for (i=0; i<length_ts_vector;  i++) {
    int o=ts_vector[i];
    int i_1nn=get_nn(o);
    int i_ltrees=get_nn_ltrees_fast(o);
    int max_sim_o=last_1nn_ltrees_sim;

    /*
    printf("o=%d, o_1nn=%d, o_ltrees=%d, max_sim_o=%d, o_o_1nn_sim=%d, o_o_ltrees_sim=%d 1nn_ltrees_sim=%d\n",
	   o, o_1nn, o_ltrees, max_sim_o, ensemble_similarity(o, o_1nn), ensemble_similarity(o, o_ltrees), 
	   ensemble_similarity(o_ltrees, o_1nn));
    fflush(stdout);
    */

    if (knn_ls[i_1nn]!=object_mapping[i_ltrees]) {
      int c=0;

      for (t=0; t<current_nb_of_ensemble_terms; t++) {
	if (matrix_pred[t]==matrix_pred[(i_1nn+1)*current_nb_of_ensemble_terms+t])
	  c++;
      }
      if (c!=max_sim_o)
	nb_error++;
    }
    if (print_result) {
      printf(".");
      fflush(stdout);
    }
  }
  if (print_result) {
    printf("\n");
    fflush(stdout);
  }

  return (nb_error*100.0/length_ts_vector);
}

/*******************************
 *  Importance des variables   *
 *******************************/

/* calcul l'importance des variables à partir d'un ensemble d'objets */

/* SCORE_TYPE *attribute_importance; */
/* int length_attribute_importance=0; */

DllExport double compute_ltrees_variable_importance(int *ts_vector, int length_ts_vector, SCORE_TYPE *attribute_importance) {
  int i,t;
  SCORE_TYPE sum_val=0.0;
  SCORE_TYPE sum_val2=0.0;
  SCORE_TYPE total_var_one_tree;

  /* allouer le vecteur */
  /*
    if (length_attribute_importance<nb_attributes) {
    if (length_attribute_importance>0)
    MyFree((SCORE_TYPE *) attribute_importance);
    attribute_importance=(SCORE_TYPE *)MyMalloc((size_t)nb_attributes*sizeof(SCORE_TYPE));
    }
  */

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
      compute_one_tree_variable_importance(0, ts_vector, length_ts_vector, 1.0, attribute_importance);
    sum_val2+=total_var_one_tree;
  
  } else {
    for (t=0; t<current_nb_of_ensemble_terms; t++) {
      if (missing_values) {
	replace_missing_values();      
      }

      total_var_one_tree=
	compute_one_tree_variable_importance(ltrees[t], ts_vector, length_ts_vector, ltrees_weight[t], attribute_importance);
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
	   sum_val/current_nb_of_ensemble_terms);
    fflush(stdout);
  }
  return (sum_val/current_nb_of_ensemble_terms);
}

SCORE_TYPE compute_one_tree_variable_importance(int tree, int *ts_vector, int length_ts_vector, float weight,
					  SCORE_TYPE *attribute_importance) {
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
      int borne=separate_ls_vector_vi(tested_attribute[node], threshold[node], ts_vector, start, end);

      /*
	printf("current_node=%d, left=%d, right=%d\n",node, left_successor[node], right_successor[node]);
	printf("start=%d, borne=%d, end=%d, tested attribute=%d isop=%d\n", start, borne, end, tested_attribute[node], index_stack_open_nodes);
	fflush(stdout);
      */

      /* calcul de l'importance (seulement si borne OK */
      vi=get_vi(ts_vector, start, end, borne);
      /*
	printf("start=%d borne=%d end=%d vi=%f\n",start,borne,end, vi);
	fflush(stdout);
      */

      if (node==tree) {
	if (goal_type==CLASSIFICATION) {
	  total_var=table_score[0][0]*hs;
	} else {
	  total_var=v_tot;
	}
      }
	/* mis a jour du vecteur */
      attribute_importance[attribute_position[tested_attribute[node]]]+=(weight*vi);

      /* left and right successors are put on the stack */
      index_stack_open_nodes--;
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
    }
  }
  return total_var;
}

/* 18/11/2008 */

/* pour les essais avec Yvan: on ne normalise pas et on ne calcule l'importance que d'un arbre */

DllExport void compute_oneltrees_variable_importance(int tree, int *ts_vector, int length_ts_vector, SCORE_TYPE *attribute_importance) {
  int i;
  SCORE_TYPE sum_val2=0.0;
  SCORE_TYPE total_var_one_tree;

  /* allouer le vecteur */
  /*
    if (length_attribute_importance<nb_attributes) {
    if (length_attribute_importance>0)
    MyFree((SCORE_TYPE *) attribute_importance);
    attribute_importance=(SCORE_TYPE *)MyMalloc((size_t)nb_attributes*sizeof(SCORE_TYPE));
    }
  */

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
      compute_one_tree_variable_importance(0, ts_vector, length_ts_vector, 1.0, attribute_importance);
    sum_val2+=total_var_one_tree;
  
  } else {
    int t=tree;
    if (missing_values) {
      replace_missing_values();      
    }

    total_var_one_tree=
      compute_one_tree_variable_importance(ltrees[t], ts_vector, length_ts_vector, ltrees_weight[t], attribute_importance);
    if (total_var_one_tree!=-1) {
	/* toutes les valeurs sont les memes */
      sum_val2=total_var_one_tree;
    }
  }

  /* /\* normalizing the values *\/ */
/*   sum_val=0.0; */
/*   for (i=0; i<nb_attributes; i++) { */
/*     sum_val+=attribute_importance[i]; */
/*   } */
  
/*   if (sum_val!=0.0) { /\* arbre ne se reduit pas a la racine *\/ */
/*     for (i=0; i<nb_attributes; i++) { */
/*       attribute_importance[i]/=sum_val; */
/*     } */
/*   } */
/*   if (print_result) { */
/*     printf("Total impurity(N.I(S))=%d*%f=%f\nAverage Impurity explained by trees=%f\n",  */
/* 	   length_ts_vector,  */
/* 	   sum_val2/length_ts_vector, sum_val2,  */
/* 	   sum_val/current_nb_of_ensemble_terms); */
/*     fflush(stdout); */
/*   } */
/*   return (sum_val/current_nb_of_ensemble_terms); */
}


SCORE_TYPE get_vi(int *ts_vector, int start, int end, int borne) {
  int i;
  SCORE_TYPE current_score;

  /* summarize_vector */
  for (i=0; i<=nb_classes; i++) {
    table_score[0][i]=0.0;
    table_score[1][i]=0.0;
  }
  table_score[0][2]=0.0; /* au cas ou */
  table_score[1][2]=0.0;

  for (i=start; i<=end; i++) {
    if (goal_type==CLASSIFICATION) {
      int c=getobjclass(ts_vector[i]);
      if (class_weightp) {
	table_score[0][0]+=class_weight_vector[c];
	table_score[0][c+1]+=class_weight_vector[c];	
      } else {
	table_score[0][0]++;
	table_score[0][c+1]++;
      }
    } else {
      float y=getobjy(ts_vector[i]);
      table_score[0][0]++;
      table_score[0][1]+=y;
      table_score[0][2]+=(y*y);
    }
  }
  /* calcul de l'entropie totale */
  if (goal_type==CLASSIFICATION)
    stop_splitting_criterio_classification_log();
  else
    stop_splitting_criterio_regression();

  if ((start>=borne)||(borne>end)) {
    return 0.0;
  }

  /* fill the table corresponding to the left successor */
  for (i=start; i<borne; i++) {
    if (goal_type==CLASSIFICATION) {
      int c=getobjclass(ts_vector[i]);
      if (class_weightp) {
	table_score[1][c+1]+=class_weight_vector[c];
	table_score[1][0]+=class_weight_vector[c];
      } else {
	table_score[1][c+1]++;
	table_score[1][0]++;
      }
    } else {
      float y=getobjy(ts_vector[i]);
      table_score[1][0]++;
      table_score[1][1]+=y;
      table_score[1][2]+=(y*y);
    }
  }

  /* compute the score */
  if (goal_type==CLASSIFICATION) {
    current_score=compute_classification_score_from_table_log();
    return (table_score[0][0]*info);
  } else {
    current_score=compute_regression_score_from_table();
    return (info);
  }
}

int separate_ls_vector_vi(int best_attribute, union threshold_type best_threshold, int *ls_vector, int start, int end) {
  
  while (start!=end) {
    while (start!=end && (check_test(best_attribute,
				     getattval(ls_vector[start],best_attribute), 
				     best_threshold)))
      start++;
    while (start!=end && !(check_test(best_attribute,
				      getattval(ls_vector[end],best_attribute), 
				      best_threshold)))
      end--;
    if (start!=end) { /* on inverse les deux */
      int temp;
      temp=ls_vector[start];
      ls_vector[start]=ls_vector[end];
      ls_vector[end]=temp;
      start++;
    }
  }
  /* ici, on a start=end, on renvoie la borne */
  
  if (check_test(best_attribute,getattval(ls_vector[start],best_attribute), best_threshold))
    return (start+1);
  else
    return start;
}

/************************
 * OUT-OF-BAG ESTIMATES *
 ************************/

/* estimation de l'erreur out-of-bag. Suppose bien sûr qu'on a fait
 * du bootstrap sampling et qu'on a sauvegarde le LS
 */

float test_ltrees_oob() {
  /* table qui va contenir les predictions */
  int save_pos=0,i,j,t, nb_error_combined=0;
  ERROR_TYPE sum_error=0.0;
  float **pv=allocate_table_float(global_learning_set_size,nb_classes+1);
  char *mask=(char *)MyMalloc((size_t)global_learning_set_size*sizeof(char));

  for (i=0; i<global_learning_set_size; i++) {
    for (j=0; j<nb_classes+1; j++) {
      pv[i][j]=0.0;
    }
  }
  
  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    int current_tree=ltrees[t];
    int nboob=0;
    float w=ltrees_weight[t];
    
    if (print_result) {
      printf("Testing model %d...",t+1);
      fflush(stdout);
    }
    
    /* on recherche les objects out of bag */
    for (i=0; i<global_learning_set_size; i++)
      mask[i]=1;
    for (i=0; i<save_ensemble_ls_size[t]; i++,save_pos++)
      mask[save_ensemble_ls_vector[save_pos]]=0;
      
    /* on les teste */
    for (i=0; i<global_learning_set_size; i++) {
      if (mask[i]) {
	float *proba_vect=get_tree_prediction_vector(current_tree, object_mapping[i]);
	for (j=0; j<nb_classes; j++)
	  pv[i][j]+=w*proba_vect[j];
	pv[i][nb_classes]+=w;
	nboob++;
      }
    }
    if (print_result) {
      printf("(%d oob cases)\n",nboob);
      fflush(stdout);
    }
  }
  
  /* on synthetise */
  if (goal_type==CLASSIFICATION) {
    for (i=0; i<global_learning_set_size; i++) {
      float max_combined=-1.0;
      int index_combined;
      for (j=0; j<nb_classes; j++) {
	if (pv[i][j]>max_combined) {
	  max_combined=pv[i][j];
	  index_combined=j;
	}
      }
      if (index_combined!=getobjclass(object_mapping[i]))
	nb_error_combined++;
    }
  
    /* affichage de l'erreur */
    /*printf("Out-of-bag error combined %f\% - %d/%d\n", nb_error_combined*100.0/global_learning_set_size,
	   nb_error_combined, global_learning_set_size);*/
    fflush(stdout);

    /* liberation de la memoire */
    free_table_float(pv,global_learning_set_size);

    return (nb_error_combined*100.0/global_learning_set_size);
  } else {
    sum_error=0.0;
    for (i=0; i<global_learning_set_size; i++) {
      ERROR_TYPE err;
      if (average_predictions_ltrees)
	err=getobjy(object_mapping[i])-(pv[i][0]/pv[i][nb_classes]);
      else /* on ne devrait pas rencontrer ce cas */
	err=getobjy(object_mapping[i])-pv[i][0];
      sum_error+=(err*err);
    }
  
    /* affichage de l'erreur */
    sum_error=sum_error/global_learning_set_size;
    printf("Out-of-bag error combined %f\n", sum_error);
    fflush(stdout);

    /* liberation de la memoire */
    free_table_float(pv,global_learning_set_size);
    MyFree((char *)mask);

    return sum_error;
  }
}

/* variable importance by permutation and oob (classification) */

DllExport void compute_ltrees_variable_importance_rf(SCORE_TYPE *attribute_importance) {
  int i,j,t;
  char *mask=(char *)MyMalloc((size_t)global_learning_set_size*sizeof(char));
  int *local_vector;
  int *permutation_vector;
  int save_pos;
  CORETABLE_TYPE *saved_att_val;
  SCORE_TYPE sderror, *sd;

  local_vector=(int *)MyMalloc((size_t)global_learning_set_size*sizeof(int));
  permutation_vector=(int *)MyMalloc((size_t)global_learning_set_size*sizeof(int));
  saved_att_val=(CORETABLE_TYPE *)MyMalloc((size_t)global_learning_set_size*sizeof(CORETABLE_TYPE));
  sd=(SCORE_TYPE *)MyMalloc((size_t)nb_attributes*sizeof(SCORE_TYPE));

  for (i=0; i<nb_attributes; i++) {
    attribute_importance[i]=0.0;
    sd[i]=0.0;
    attribute_position[attribute_vector[i]]=i;
  }
  save_pos=0;
  /* arbre par arbre */
  for (t=0; t<current_nb_of_ensemble_terms; t++) {
    int current_tree=ltrees[t];
    int err_no_perm=0;
    int nboob=0;

    if (print_result) {
      printf("Testing model %d...",t+1);
      fflush(stdout);
    }

    /* on cherche le oob */
    for (i=0; i<global_learning_set_size; i++)
      mask[i]=1;
    for (i=0; i<save_ensemble_ls_size[t]; i++,save_pos++)
      mask[save_ensemble_ls_vector[save_pos]]=0;
    
    for (i=0; i<global_learning_set_size; i++) {
      if (mask[i]) { 
	local_vector[nboob]=object_mapping[i];
	nboob++;
      }
    }

    if (print_result) {
      printf("(%d oob cases)\n",nboob);
      fflush(stdout);
    }


    /* on le teste avant permutation */
    for (i=0; i<nboob; i++) {
      if (my_best_class(get_tree_prediction_vector(current_tree, local_vector[i]),nb_classes)!=getobjclass(local_vector[i]))
	err_no_perm++;	
    }
    
    /*    printf("error before perm=%d\n",err_no_perm); */
    /* fflush(stdout); */

    /* attribut par attribut */
    for (i=0; i<nb_attributes; i++) {
      int remaining_obj=global_learning_set_size;
      int att=attribute_vector[i];
      int err_perm=0;
      /* on construit un vecteur de permutation du LS */
      for (j=0; j<global_learning_set_size; j++) {
	permutation_vector[j]=j;
      }
      for (j=0; j<global_learning_set_size; j++) {
	int rp=get_random_integer(remaining_obj),temp;
	remaining_obj--;
	if (remaining_obj!=0) {
	  temp=permutation_vector[rp];
	  permutation_vector[rp]=permutation_vector[remaining_obj];
	  permutation_vector[remaining_obj]=temp;
	}
      }

      /*
      for (j=0; j<global_learning_set_size; j++) {
	printf(" %d",permutation_vector[j]);
      }
      printf("\n");
      fflush(stdout);
      */

      /* on sauve les anciennes valeurs */       
      for (j=0; j<global_learning_set_size; j++) {
	saved_att_val[j]=getattval(object_mapping[j],att);
      }
      /* on permute les valeurs de l'attribut */
#ifdef LOAD_DB_HANDLER
      if (core_table_type==NORMAL_CORE_TABLE) {
	for (j=0; j<global_learning_set_size; j++) {	   
	  set_core_table_value(saved_att_val[j],object_mapping[permutation_vector[j]],att);
	}
      } else if (core_table_type=TWO_DIM_CORE_TABLE) {
	for (j=0; j<global_learning_set_size; j++) {	   
	  set_core_table_value_2dim(saved_att_val[j],object_mapping[permutation_vector[j]],att);
	}
      }
#else
      for (j=0; j<global_learning_set_size; j++) {
	core_table[att*nb_obj_in_core_table+object_mapping[permutation_vector[j]]]=(CORETABLE_TYPE)saved_att_val[j];
      }
#endif
      
      /* on teste apres permutation */	
      for (j=0; j<nboob; j++) {
	if (my_best_class(get_tree_prediction_vector(current_tree, local_vector[j]),nb_classes)!=getobjclass(local_vector[j]))
	  err_perm++;
      }
      
      /* on incremente l'importance */
      attribute_importance[i]+=(err_perm-err_no_perm)*1.0/nboob;
      sd[i]+=(err_perm-err_no_perm)*(err_perm-err_no_perm)*1.0/(nboob*nboob);
      
      /* on remet les valeurs en place */
#ifdef LOAD_DB_HANDLER
      if (core_table_type==NORMAL_CORE_TABLE) {
	for (j=0; j<global_learning_set_size; j++) {	   
	  set_core_table_value(saved_att_val[j],object_mapping[j],att);
	}
      } else if (core_table_type=TWO_DIM_CORE_TABLE) {
	for (j=0; j<global_learning_set_size; j++) {
	  set_core_table_value_2dim(saved_att_val[j],object_mapping[j],att);
	}
      }
#else
      for (j=0; j<global_learning_set_size; j++) {
	core_table[att*nb_obj_in_core_table+object_mapping[j]]=(CORETABLE_TYPE)saved_att_val[j];
      }
#endif
    }
    if (print_result) {
      printf("\n");fflush(stdout);
    }
  }

  /* on normalise */
  for (i=0; i<nb_attributes; i++) {
    attribute_importance[i]=attribute_importance[i]/current_nb_of_ensemble_terms;
    
    sderror=sqrt((sd[i]/current_nb_of_ensemble_terms-attribute_importance[i]*attribute_importance[i])/current_nb_of_ensemble_terms);
    
    if (sderror==0.0) {
      /* I'm not sure that this could happen */
      printf("sd error of variable %d is zero\n",i);
      fflush(stdout);
      attribute_importance[i]=0.0;
    } else {
      attribute_importance[i]=attribute_importance[i]/sderror;
    }
  }
  
  MyFree((int *)local_vector);
  MyFree((int *)permutation_vector);
  MyFree((CORETABLE_TYPE *)saved_att_val);
}


/****************************************
 * SAUVEGARDE ET CHARGEMENT DES MODÈLES *
 ****************************************/

/* calcule le nombre de noeud dans un arbre */

DllExport int get_tree_nb_nodes(int tree) {
  int nb_nodes=1;
  int current_node;
  
  index_stack_open_nodes=-1;
  index_stack_open_nodes++;
  stack_open_nodes[index_stack_open_nodes][0]=tree;
  
  while(index_stack_open_nodes>=0) {
    current_node=stack_open_nodes[index_stack_open_nodes][0];
    index_stack_open_nodes--;
    if (left_successor[current_node]!=-1) {
      nb_nodes+=2;
      index_stack_open_nodes++;
      stack_open_nodes[index_stack_open_nodes][0]=current_node+left_successor[current_node];
      index_stack_open_nodes++;
      stack_open_nodes[index_stack_open_nodes][0]=current_node+right_successor[current_node];
    }
  }

  return nb_nodes;

}

/* Ecrit un arbre dans le fichier (version propre a l'algorithme de construction) */

void write_one_tree(int tree, FILE *fp) {
  int nb_nodes=get_tree_nb_nodes(tree);
  int current_node;

  printf("nombre de noeuds a ecrire=%d\n", nb_nodes);
  fflush(stdout);
  /* on ecrit le nombre de noeuds */
  fwrite(&nb_nodes, sizeof(int), 1, fp);

  /* on ecrit l'info sur les noeuds */
  for (current_node=tree; current_node<tree+nb_nodes; current_node++) {
    int pred;
    /* on ecrit les info sur ce noeud dans le fichier */
    /* on ecrit 0 ou 1 selon la valeur de prediction[current_node] */
    /* deux cas: */
    if (left_successor[current_node]!=-1) {
      pred=-1;
      fwrite(&pred, sizeof(int), 1, fp);
      fwrite(&tested_attribute[current_node], sizeof(int), 1, fp);
      fwrite(&threshold[current_node], sizeof(union threshold_type), 1, fp);
      fwrite(&left_successor[current_node], sizeof(int), 1, fp);
      /* normalement, on n'a pas besoin de cette valeur */
      fwrite(&right_successor[current_node], sizeof(int), 1, fp);
    } else {
      pred=1;
      fwrite(&pred, sizeof(int), 1, fp);
      /* on ecrit les valeurs de predictions */
      fwrite(prediction_values[prediction[current_node]], sizeof(float), nb_classes, fp);
    }
  }
}

void read_one_tree(FILE *fp) {
  int nb_nodes;
  int i;
  
  /* on lit le nombre de noeuds */
  fread(&nb_nodes, sizeof(int), 1, fp);
  
  printf("nombre de noeuds a lire=%d\n", nb_nodes);
  fflush(stdout);
  
  if (index_nodes+nb_nodes>size_current_tree_table) {
    printf("not enough memory to load the tree (read_one_tree)\n");
    fflush(stdout);
  }
  /* on lit l'info sur tous les noeuds */
  for (i=0; i<nb_nodes; i++) {
    int pred;
    /* on rajoute un noeud */
    index_nodes++;
    fread(&pred, sizeof(int), 1, fp);
    if (pred==-1) {
      /* c'est un noeud test */
      prediction[index_nodes]=-1;
      fread(&tested_attribute[index_nodes], sizeof(int), 1, fp);
      fread(&threshold[index_nodes], sizeof(union threshold_type), 1, fp);
      fread(&left_successor[index_nodes], sizeof(int), 1, fp);
      fread(&right_successor[index_nodes], sizeof(int), 1, fp);
    } else {
      /* c'est une feuille */
      index_prediction_values++;
      prediction[index_nodes]=index_prediction_values;
      fread(prediction_values[index_prediction_values], sizeof(float), nb_classes, fp);
      tested_attribute[index_nodes]=-1;
      left_successor[index_nodes]=-1;
      right_successor[index_nodes]=-1;
    }
  }
}

void write_tree_to_file(int tree, char *filename) {
  FILE *fp=fopen(filename,"wb");
  write_one_tree(tree, fp);
  fclose(fp);
  printf("done");
  fflush(stdout);
}

void read_tree_from_file(char *filename) {
  FILE *fp=fopen(filename,"rb");
  clean_all_trees();
  read_one_tree(fp);
  fclose(fp);
  printf("done");
  fflush(stdout);
}

void write_ensemble_to_file(char *filename) {
  char str1[100];
  char str2[100];
  FILE *fp;
  FILE *fp_nb_trees;
  
  int i;

  strcpy(str1,filename);
  strcat(str1,"-trees.dat");
  fp=fopen(str1, "wb");
  strcpy(str2,filename);
  strcat(str2,"-nb-trees.dat");
  fp_nb_trees=fopen(str2, "wb");
  
  fwrite(&current_nb_of_ensemble_terms, sizeof(int), 1, fp_nb_trees);
  fclose(fp_nb_trees);

  /* on ecrit d'abord le nombre d'arbres, puis le poids des arbres */
  /* fwrite(&current_nb_of_ensemble_terms, sizeof(int), 1, fp); */
  fwrite(&average_predictions_ltrees, sizeof(int), 1, fp);
  /* fwrite(ltrees_weight, sizeof(float), current_nb_of_ensemble_terms, fp); */
  /* ensuite on ecrit les arbres un par un */
  for (i=0; i<current_nb_of_ensemble_terms; i++) {
    fwrite(&ltrees_weight[i],sizeof(float),1,fp);
    write_one_tree(ltrees[i], fp);
  }
  fclose(fp);
  
  printf("done");
  fflush(stdout);
}

void read_ensemble_from_file(char *filename) {
  char str1[100];
  char str2[100];
  FILE *fp;
  FILE *fp_nb_trees;
  
  int i;

  strcpy(str1,filename);
  strcat(str1,"-trees.dat");
  fp=fopen(str1, "rb");
  strcpy(str2,filename);
  strcat(str2,"-nb-trees.dat");
  fp_nb_trees=fopen(str2, "rb");

  clean_all_trees();
  
  /* on ecrit d'abord le nombre d'arbres, puis le poids des arbres */
  fread(&current_nb_of_ensemble_terms, sizeof(int), 1, fp_nb_trees);
  if (current_nb_of_ensemble_terms> MAX_NUMBER_OF_TREES) {
     printf("not enough memory to load the ensemble of trees (read_ensemble_from_file)\n");
     fflush(stdout);
     fclose(fp); fclose(fp_nb_trees);
     return;
  }

  fread(&average_predictions_ltrees, sizeof(int), 1, fp);
  /* fread(ltrees_weight, sizeof(float), current_nb_of_ensemble_terms, fp); */
  /* ensuite on ecrit les arbres un par un */
   printf("nb trees a lire=%d average_pred=%d\n", current_nb_of_ensemble_terms, average_predictions_ltrees);
  fflush(stdout);
  
  for (i=0; i<current_nb_of_ensemble_terms; i++) {
    fread(&ltrees_weight[i],sizeof(float), 1, fp);
    ltrees[i]=index_nodes+1;
    read_one_tree(fp);
  }

  fclose(fp);
  fclose(fp_nb_trees);
  
  printf("done");
  fflush(stdout);
}

DllExport void save_ensemble(char *filename) {
  write_ensemble_to_file(filename);
}

DllExport void load_ensemble(char *filename) {
  read_ensemble_from_file(filename);
}

/****************
 * TREE PRUNING *
 ****************/

/* IMPLEMENTATION OF THE MINIMAL COST PRUNING */

/* Efficient but it may require a lot of memory */

int size_current_tree_table_pruning=0;

int *parent;
int *complexity;
ERROR_TYPE *r_node;
ERROR_TYPE *r_subtree;
ERROR_TYPE *r_node_ts;
ERROR_TYPE *r_subtree_ts;
float *g_global;
float *g_local;

void free_memory_for_pruning() {
  if (size_current_tree_table_pruning>0) {
    MyFree((int *)parent);
    MyFree((int *)complexity);
    MyFree((ERROR_TYPE *)r_node);
    MyFree((ERROR_TYPE *)r_subtree);
    MyFree((ERROR_TYPE *)r_node_ts);
    MyFree((ERROR_TYPE *)r_subtree_ts);
    MyFree((float *)g_global);
    MyFree((float *)g_local);
  }
}

void allocate_memory_for_pruning(int size_requested) {
  if (size_current_tree_table_pruning<size_requested) {
    free_memory_for_pruning();
    parent=(int *)MyMalloc((size_t)size_requested*sizeof(int));
    complexity=(int *)MyMalloc((size_t)size_requested*sizeof(int));
    r_node=(ERROR_TYPE *)MyMalloc((size_t)size_requested*sizeof(ERROR_TYPE));
    r_subtree=(ERROR_TYPE *)MyMalloc((size_t)size_requested*sizeof(ERROR_TYPE));
    r_node_ts=(ERROR_TYPE *)MyMalloc((size_t)size_requested*sizeof(ERROR_TYPE));
    r_subtree_ts=(ERROR_TYPE *)MyMalloc((size_t)size_requested*sizeof(ERROR_TYPE));
    g_global=(float *)MyMalloc((size_t)size_requested*sizeof(float));
    g_local=(float *)MyMalloc((size_t)size_requested*sizeof(float));
    
    if (!g_local) {
      printf("impossible d'allouer de la memoire dans allocate_memory_for_pruning\n");
      exit(0);
    }
    size_current_tree_table_pruning=size_requested;
  }
}

/* Version of make_leaf_prediction with the storage of the size of the learning
   sample and of the best class */

int make_leaf_prediction_classification_pruning() {
  
  int i;
  index_prediction_values++;

  for (i=0; i<nb_classes; i++) {
    prediction_values[index_prediction_values][i] =(float)(table_score[0][i+1]/table_score[0][0]);
  }
  /* we add the number of cases and the majority class at the end of the prediction vector */
  prediction_values[index_prediction_values][nb_classes]=table_score[0][0];
  prediction_values[index_prediction_values][nb_classes+1]=my_best_class(prediction_values[index_prediction_values], nb_classes);

  return index_prediction_values;
}

/* ATTENTION LES PREDICTIONS DEVRAIENT ÊTRE EN DOUBLE PRECISION EN REGRESSION */

int make_leaf_prediction_regression_pruning() {

  index_prediction_values++;
  
  prediction_values[index_prediction_values][0]=(float)(table_score[0][1]/table_score[0][0]);
  /* we replace the variance by the sum of y^2 */
  prediction_values[index_prediction_values][1]=(float)(table_score[0][2]/table_score[0][0]);
  /* we add the number of cases at the end of the prediction vector */
  prediction_values[index_prediction_values][2]=table_score[0][0];

  return index_prediction_values;
}

/* Different way to defines the total reliability of a terminal node */

/* Total number of error, classification (CART) */
/* input: vector[i]=proportion of class i in this node
          vector[nb_classes]=N total
*/

ERROR_TYPE compute_r_pruning_classification_error(float *vector) {
  int i;
  int bc=vector[nb_classes+1];
  ERROR_TYPE sum_err=0.0;
  for (i=0; i<nb_classes; i++) {
    if (i!=bc)
      sum_err+=vector[i];
  }
  sum_err*=vector[nb_classes];

  /* sum_err = total number of misclassified cases in the node, assuming this
   *           node is terminal 
   */
  return sum_err;
}

/* Total information, classification (Wehenkel) */
/* To do */

ERROR_TYPE compute_r_pruning_classification_info(float *vector) {
  return 0.0;
}

/* Variance, regression (CART) */
/* input: vector[0]=y mean
          vector[1]=y^2 mean
	  vector[2]=N
*/

ERROR_TYPE compute_r_pruning_regression(float *vector) {
  ERROR_TYPE squared_mean=vector[0]*vector[0];
  ERROR_TYPE sum_err=vector[2]*(vector[1]-squared_mean);
  
  /* sum_err = total variance in the node */
  return sum_err;
}

ERROR_TYPE (*compute_r_pruning)(float *vector)=compute_r_pruning_classification_error;

/* combine two prediction vector */

void combine_pred_vector_clas(float *vector, float *vector_l, float *vector_r) {
  int i;
  double proba_l, proba_r;

  vector[nb_classes]=vector_l[nb_classes]+vector_r[nb_classes];
  proba_l=vector_l[nb_classes]/vector[nb_classes];
  proba_r=vector_r[nb_classes]/vector[nb_classes];

  for (i=0; i<nb_classes; i++) {
    vector[i]=proba_l*vector_l[i]+proba_r*vector_r[i];
  }
  vector[nb_classes+1]=my_best_class(vector, nb_classes);
}

void combine_pred_vector_regr(float *vector, float *vector_l, float *vector_r) {
  double proba_l, proba_r;

  vector[2]=vector_l[2]+vector_r[2];
  proba_l=vector_l[2]/vector[2];
  proba_r=vector_r[2]/vector[2];

  vector[0]=proba_l*vector_l[0]+proba_r*vector_r[0];
  vector[1]=proba_l*vector_l[1]+proba_r*vector_r[1];
}

void (*combine_pred_vector)(float *vector, float *vector_l, float *vector_r)=combine_pred_vector_clas;

/* Compute the error for one object */

ERROR_TYPE compute_local_error_classification(int obj, float *vector) {
  int temp=(int)vector[nb_classes+1];
  if (temp==getobjclass(obj))
    return 0.0;
  else
    return 1.0;
}

ERROR_TYPE compute_local_error_regression(int obj, float *vector) {
  double temp=(getobjy(obj)-vector[0]);
  return temp*temp;
}

ERROR_TYPE (*compute_local_error)(int obj, float *vector)=compute_local_error_classification;

/* test_tree_and_init_r_node_ts(int first_node, int *ts_vector, int length_ts_vector) */

void test_tree_and_init_r_node_ts(int first_node, int last_node, int *ts_vector, int length_ts_vector) {
  int i, t;
  int current_node;
  
  /* initialization of r_node_ts */
  for (t=first_node; t<=last_node; t++) {
    r_node_ts[t]=0.0;
  }
  /* update the values of r_node_ts */
  for (i=0; i<length_ts_vector; i++) {
    int o=ts_vector[i];
    current_node=first_node;
    r_node_ts[current_node]+=compute_local_error(o, prediction_values[prediction[current_node]]);
    while(left_successor[current_node]!=-1) {
      if (check_test(tested_attribute[current_node],
		     getattval(o, tested_attribute[current_node]), 
		     threshold[current_node]))
	current_node+=left_successor[current_node];
      else
	current_node+=right_successor[current_node];
      r_node_ts[current_node]+=compute_local_error(o, prediction_values[prediction[current_node]]);
    }
  }
  
  /* update the values of r_subtree_ts */
  for (t=last_node; t>=first_node; t--) {
    if (tested_attribute[t]==-1)
      r_subtree_ts[t]=r_node_ts[t];
    else
      r_subtree_ts[t]=r_subtree_ts[t+left_successor[t]]+r_subtree_ts[t+right_successor[t]];
  }
}


/* Build the sequence of pruned tree 
 * (follows the algorithm of Breiman et al in the CART book, page 294) 
 */

/* ATTENTION: diviser les valeurs de r_node[t] par total_nb_cases */
/* C'est indispensable pour l'élagage par validation croisée ??? */

float *alfa_k;
ERROR_TYPE *error_k;

void build_alfa_sequence_and_test(int first_node, int last_node, int ls_size, int *ps_vector, int length_ps_vector) {
  int t, k, left, right;
  float alfa, epsilon;
  
  /* create the two vectors alfa_k and error_k to store the sequences */
  alfa_k=(float *)MyMalloc((size_t)(last_node-first_node+2)*sizeof(float));
  if (!alfa_k) {
    printf("impossible d'allouer de la memoire dans build_alfa_sequence_and_test\n");
    fflush(stdout);
    exit(0);
  }
  if (length_ps_vector>0)
    error_k=(ERROR_TYPE *)MyMalloc((size_t)(last_node-first_node+2)*sizeof(ERROR_TYPE));
  
  /* initialisation of the different data associated to each node */
  
  for (t=last_node; t>=first_node; t--) {
    if (tested_attribute[t]==-1) {
      complexity[t]=1;
      r_subtree[t]=(r_node[t]=compute_r_pruning(prediction_values[prediction[t]]));
      g_global[t]=(float)PLUS_INFINI;
    } else {
      left=t+left_successor[t]; right=t+right_successor[t];
      parent[left]=t;
      parent[right]=t;
      complexity[t]=complexity[left]+complexity[right];
      r_subtree[t]=r_subtree[left]+r_subtree[right];

      /* create a prediction vector for this node */
      index_prediction_values++;
      prediction[t]=index_prediction_values;
      combine_pred_vector(prediction_values[index_prediction_values], 
			  prediction_values[prediction[left]], prediction_values[prediction[right]]);
      r_node[t]=compute_r_pruning(prediction_values[index_prediction_values]);

      g_local[t]=(r_node[t]-r_subtree[t])/(complexity[t]-1);
      /* g_global[t]=min(g_local[t], g_global[left], g_global[right]); */
      if ((g_local[t]<=g_global[left]) && (g_local[t]<=g_global[right]))
	g_global[t]=g_local[t];
      else if (g_global[left]<=g_global[right])
	g_global[t]=g_global[left];
      else
	g_global[t]=g_global[right];
    }
  }

  /* test of the tree and initialization of r_node_ts and r_subtree_ts */
  if (length_ps_vector>0)
    test_tree_and_init_r_node_ts(first_node, last_node, ps_vector, length_ps_vector);

  /* display all node information */
/*   for (t=first_node; t<=last_node; t++) { */
/*     int left=left_successor[t]==-1? -1: t+left_successor[t]; */
/*     int right=right_successor[t]==-1? -1: t+right_successor[t]; */
    
/*     printf("t=%d l=%d r=%d p=%d r_n=%f r_st=%f r_n_ts%f r_st_ts%f\n",  */
/* 	   t, left, right, parent[t], r_node[t], r_subtree[t], r_node_ts[t], r_subtree_ts[t]); */
/*     printf("  g_local=%f g_global=%f\n  pred: ", g_local[t], g_global[t]); */
/*     for (i=0; i<nb_classes+2; i++) { */
/*       printf(" %f", prediction_values[prediction[t]][i]); */
/*     } */
/*     printf("\n"); */
/*     fflush(stdout); */
/*   } */
  
  /* at this point, we could remove the prediction values */
  /* index_prediction_values=old_index_prediction_values; */
    
  /* main loop */
  k=0;
  alfa=0.0;
  epsilon=0.000000000001; /* should be taken to a small positive number times r_node[first_node] */
  
  for (;;) {
    /*
      printf("g_global[first_node]=%e, alfa=%e epsilon=%e\n", g_global[first_node], alfa, epsilon);
      fflush(stdout);
    */
    if (g_global[first_node]>alfa+epsilon) {
      if (print_result) {
	printf("k=%d nb_leafs=%d alfa_k=%f error_ls=%f error_ts=%f\n", 
	       k, complexity[first_node], alfa, r_subtree[first_node], r_subtree_ts[first_node]);
	fflush(stdout);
      }
      alfa_k[k]=alfa;
      if (length_ps_vector>0)
	error_k[k]=r_subtree_ts[first_node];
      alfa=g_global[first_node];
      k=k+1;
    }
    if (complexity[first_node]==1)
      break;
    t=first_node;
    while (g_global[t]<g_local[t]-epsilon) {
      if (g_global[t]==g_global[t+left_successor[t]])
	t+=left_successor[t];
      else
	t+=right_successor[t];
    }
    /* prune this node */
    complexity[t]=1;
    g_local[t]=alfa; /* g_local stores a value such that if alfa in [g_local;+infinity[, this node must be pruned */
    r_subtree[t]=r_node[t];
    r_subtree_ts[t]=r_node_ts[t];
    g_global[t]=(float)PLUS_INFINI;
    /* update the values along the branch leading to this node */
    while (t>first_node) {
      t=parent[t];
      left=t+left_successor[t]; right=t+right_successor[t];
      complexity[t]=complexity[left]+complexity[right];
      r_subtree[t]=r_subtree[left]+r_subtree[right];
      if (length_ps_vector>0) 
	r_subtree_ts[t]=r_subtree_ts[left]+r_subtree_ts[right];
      g_local[t]=(r_node[t]-r_subtree[t])/(complexity[t]-1);
      if ((g_local[t]<=g_global[left]) && (g_local[t]<=g_global[right]))
	g_global[t]=g_local[t];
      else if (g_global[left]<=g_global[right])
	g_global[t]=g_global[left];
      else
	g_global[t]=g_global[right];
    }
  }
  /* we close the vector */
  alfa_k[k]=-1.0;
}

/* Prune the current tree from an independent pruning set */

DllExport void prune_current_tree_from_pruning_set(int *ps_vector, int length_ps_vector) {
  int k, t;
  float best_alfa;
  ERROR_TYPE min_error;

  /* alloue de la memoire */
  allocate_memory_for_pruning(index_nodes+1);
  
  /* build the sequence of trees */
  build_alfa_sequence_and_test(0, index_nodes, current_learning_set_size, ps_vector, length_ps_vector);

  /* select the best tree among this sequence */
  best_alfa=alfa_k[0];
  min_error=error_k[0];
  k=1;
  while (alfa_k[k]!=-1.0) {
    if (error_k[k]<=min_error) {
      best_alfa=alfa_k[k];
      min_error=error_k[k];
    }
    k++;
  }
  printf("Best alfa=%f -> Error minimum=%f\n", best_alfa, min_error);
  fflush(stdout);
  /* prune the tree */
  for (t=0; t<=index_nodes; t++) {
    if (best_alfa>=g_local[t]) {
      tested_attribute[t]=-1;
      left_successor[t]=-1;
      right_successor[t]=-1;
    }
  }
  
  /* free memory */
  MyFree(alfa_k); MyFree(error_k);
  /* on pourrait aussi faire: 
  free_memory_for_pruning() 
  */
}

/* Prune the current tree by cross-validation
 * Assume that the LS is in current_learning_set and that all the weights are 1
 */

int number_of_cv_partitions=10;

DllExport void set_number_of_cv_partitions(int nb_cv) {
  printf("old value=%d\n", number_of_cv_partitions);
  fflush(stdout);
  number_of_cv_partitions=nb_cv;
}

DllExport int get_number_of_cv_partitions() {
  return number_of_cv_partitions;
}

#ifdef _WIN32

int find_pos(float *vect, float thres) {
  int l=0;
  while((vect[l+1]!=-1.0)&&(vect[l+1]<=thres))
			l++;
  return l;

}

#endif

DllExport void prune_current_tree_by_cross_validation() {
  int i, j, k, l, t;
  int first_node=0, last_node=index_nodes;
  int current_tree;
  int part_size=(int)ceil(current_learning_set_size*1.0/number_of_cv_partitions);
  int *ls_vector=current_learning_set;
  int *ts_vector; int length_ts_vector;
  int length_ls_vector=current_learning_set_size;
  float best_alfa;
  ERROR_TYPE best_error;
  float **alfa_list_cv;
  ERROR_TYPE **error_list_cv;
  
  /* init some variables */
  alfa_list_cv=(float **)MyMalloc((size_t)number_of_cv_partitions*sizeof(float *));
  error_list_cv=(ERROR_TYPE **)MyMalloc((size_t)number_of_cv_partitions*sizeof(ERROR_TYPE *));
  if (!alfa_list_cv || !error_list_cv) {
    printf("impossible d'allouer de la memoire dans prune_tree_by_cross_validation\n");
    fflush(stdout);
    exit(0);
  }
  
  /* allocate memory */
  /* suppose que les arbres obtenus pas CV sont plus petits que l'arbre initial
   * C'est toujours le cas avec les arbres classiques qui sont visés par l'élagage
   * Pour être sûr, on peut mettre size_current_tree_table
   */
  allocate_memory_for_pruning((number_of_cv_partitions+1)*(index_nodes+1));
  
  /* randomize the ls vector */
  randomize_vector(ls_vector, length_ls_vector);
  
  /* create a new vector */
  if ((part_size*number_of_cv_partitions)>=length_ls_vector) {
    ts_vector=(int *)MyMalloc((size_t)part_size*sizeof(int));
  } else {
    ts_vector=(int *)MyMalloc((size_t)(length_ls_vector-(number_of_cv_partitions-1)*part_size)*sizeof(int));
  }
  current_learning_set=(int *)MyMalloc((size_t)length_ls_vector*sizeof(int));
  if (!ts_vector || !current_learning_set) {
    printf("impossible d'allouer de la memoire dans prune_tree_by_cross_validation\n");
    fflush(stdout);
    exit(0);
  }
  
  /* loop for building the sequences */
  for(i=0; i<number_of_cv_partitions; i++) {
    int borne_inf_test=i*part_size;
    int borne_sup_test=(i+1)*part_size;
    
    if (print_result) {
      printf("Arbre %d:\n",i);
      fflush(stdout);
    }
      
    /* correstion si pas multiple de 10 */
    if ((borne_sup_test>length_ls_vector) || (i==number_of_cv_partitions-1)) {
      borne_sup_test=length_ls_vector;
    }
    
    /* fullfill the learning set vector */
    current_learning_set_size=0;
    length_ts_vector=0;
    for (j=0; j<length_ls_vector; j++) {
      if ((j<borne_inf_test) || (j>=borne_sup_test)) {
	current_learning_set[current_learning_set_size]=ls_vector[j];
	current_learning_set_size++;
      } else {
	ts_vector[length_ts_vector]=object_mapping[ls_vector[j]];
	length_ts_vector++;
      }
    }
    if (print_result) {
      printf("nombre d'objets %d borne_inf %d borne sup %d ts %d\n", 
	     current_learning_set_size, borne_inf_test, borne_sup_test, length_ts_vector);
      fflush(stdout);
    }

    /* build the tree */
    current_tree=build_one_tree();
    /* build the sequence */
    build_alfa_sequence_and_test(current_tree, index_nodes, current_learning_set_size, 
				 ts_vector, length_ts_vector);
    /* store it */
    alfa_list_cv[i]=alfa_k;
    error_list_cv[i]=error_k;
  }

  if (print_result) {
    printf("Pruning large tree\n");
    fflush(stdout);
  }

  /* build the sequence for the large tree */
  build_alfa_sequence_and_test(first_node, last_node, length_ls_vector, NULL, 0);

  /* estimate the error for each alfa value */
  k=0;
  best_error=PLUS_INFINI;
  while (alfa_k[k]!=-1.0) {
    ERROR_TYPE current_error=0.0;
    float current_alfa;

    if (alfa_k[k+1]==-1) {
      current_alfa=PLUS_INFINI; 
    } else {
      current_alfa=sqrt(alfa_k[k]*alfa_k[k+1]);
    }
    /* look for the error estimate of each tree for the current alfa value */
    for (i=0; i<number_of_cv_partitions; i++) {
      l=0;

#ifdef _WIN32
      l=find_pos(alfa_list_cv[i],current_alfa);
#else
      while((alfa_list_cv[i][l+1]!=-1.0)&&(alfa_list_cv[i][l+1]<=current_alfa))
			l++;
#endif

      current_error+=error_list_cv[i][l];
    }

    if (print_result) {
      printf("alfa_k=%f current_alfa=%f error_cv=%f\n", alfa_k[k], current_alfa, current_error);
      fflush(stdout);
    }

    if (current_error<=best_error) {
      best_error=current_error;
      best_alfa=alfa_k[k];
    }
    k++;
  }

  if (print_result) {
    printf("Best alfa = %f Best error = %f\n", best_alfa, best_error);
    fflush(stdout);
  }
    
  /* prune the tree according to best_alfa */
  for (t=first_node; t<=last_node; t++) {
    if (best_alfa>=g_local[t]) {
      tested_attribute[t]=-1;
      left_successor[t]=-1;
      right_successor[t]=-1;
    }
  }

  /* clean the memory */
  MyFree(current_learning_set);
  MyFree(ts_vector);
  current_learning_set_size=length_ls_vector;
  current_learning_set=ls_vector;
  index_nodes=last_node;
  for(i=0; i<number_of_cv_partitions; i++) {
    MyFree(alfa_list_cv[i]);
    MyFree(error_list_cv[i]);
  }
  MyFree(alfa_list_cv);
  MyFree(error_list_cv);
  MyFree(alfa_k);
  /* on pourrait aussi faire: 
  free_memory_for_pruning() 
  */
}

/* interface with lisp */

DllExport void init_pruning_classification() {
  make_leaf_prediction=make_leaf_prediction_classification_pruning;
  compute_r_pruning=compute_r_pruning_classification_error;
  combine_pred_vector=combine_pred_vector_clas;
  compute_local_error=compute_local_error_classification;
}

DllExport void init_pruning_regression() {
  make_leaf_prediction=make_leaf_prediction_regression_pruning;
  compute_r_pruning=compute_r_pruning_regression;
  combine_pred_vector=combine_pred_vector_regr;
  compute_local_error=compute_local_error_regression;
}

/* ERROR BASED PRUNING DE QUINLAN (C4.5) */

/* implementation largement inspirée du code de C4.5 */
/* implementation par recurrence */

/* suppose que l'arbre est au début de la table (O à index_nodes) et que le vecteur current_learning_set contient
 * les objets de l'ensemble d'apprentissage
 */

float c45_cf=0.25; /* valeur par défaut de Quinlan */

DllExport void set_c45_cf(float cf) {
  c45_cf=cf;
  printf("c45_cf=%f\n",c45_cf);
  fflush(stdout);
}

/* tiré du code de C4.5, calcule une estimation de l'erreur a partir de l'erreur sur le LS  */

float Val[] = {  0,  0.001, 0.005, 0.01, 0.05, 0.10, 0.20, 0.40, 1.00},
      Dev[] = {4.0,  3.09,  2.58,  2.33, 1.65, 1.28, 0.84, 0.25, 0.00};


DllExport float adderrs(float n, float e) {
  float Coeff=0;

  float Val0, Pr;

  if ( ! Coeff ) {
	/*  Compute and retain the coefficient value, interpolating from
	    the values in Val and Dev  */
    int i;
    i = 0;
    while ( c45_cf > Val[i] ) i++;
    Coeff = Dev[i-1] +
      (Dev[i] - Dev[i-1]) * (c45_cf - Val[i-1]) /(Val[i] - Val[i-1]);
    Coeff = Coeff * Coeff;
  }
  
  if ( e < 1E-6 ) {
    return n * (1 - exp(log(c45_cf) / n));
  } else if ( e < 0.9999 ) {
    Val0 = n * (1 - exp(log(c45_cf) / n));
    return Val0 + e * (adderrs(n, 1.0) - Val0);
  } else if ( e + 0.5 >= n ) {
    return 0.67 * (n - e);
  } else {
    Pr = (e + 0.5 + Coeff/2
	  + sqrt(Coeff * ((e + 0.5) * (1 - (e + 0.5)/n) + Coeff/4)) )
      / (n + Coeff);
    return (n*Pr-e);
  }
}
      
float estimate_error_and_prune_c45(int node, int start, int end, int update) {
  int best_c, borne, max_branch;
  float est_error, est_error_pruned, est_error_branch, n, e;

  /* calcul un summary du LS courant */
  summarize_vector_clas(current_learning_set, start, end);
  
  /* calcul de l'erreur si on supprime le sous-arbre */
  best_c=my_best_class_double(table_score[0]+1,nb_classes);
  n=table_score[0][0];
  e=table_score[0][0]-table_score[0][best_c+1];
  est_error_pruned=adderrs(n,e)+e;

  /* si c'est une feuille, on arrête */
  if (left_successor[node]==-1) {
    return est_error_pruned;
  }

  /* on separe le ls */
  borne=separate_ls_vector(tested_attribute[node], threshold[node], current_learning_set, start, end);
  /* on calcule la somme des erreurs des arbres elagués à droite et à gauche (par recurrence en elaguant les deux parties) */
  est_error=estimate_error_and_prune_c45(node+left_successor[node],start,borne-1,update);
  est_error+=estimate_error_and_prune_c45(node+right_successor[node],borne,end,update);

  if (update==0) {
    return est_error;
  }

  /* on regarde ensuite la branche la plus peuplée */
  if ((borne-start)>(end-borne+1)) { /* a gauche */
    max_branch=node+left_successor[node];
  } else { /* a droite */  
    max_branch=node+right_successor[node];
  }
  est_error_branch=estimate_error_and_prune_c45(max_branch,start,end,0);

  /* ensuite, on decide */
  if (print_result) {
    printf("node=%d, true_error=%f/%f, est_error_pruned=%f, est_error=%f, est_error_branch=%f\n", 
	   node, e, n, est_error_pruned, est_error, est_error_branch);
    fflush(stdout);
  }
  
  if ((est_error_pruned <= est_error_branch+0.1)&&
      (est_error_pruned <= est_error+0.1)) {
    /* on elague */
    if (print_result) {
      printf("-> this node becomes a leaf\n");
      fflush(stdout);
    }
    /* il a ete modifie, il faut le recalculer */
    summarize_vector_clas(current_learning_set, start, end);
    prediction[node]=make_leaf_prediction_classification();
    left_successor[node]=-1; right_successor[node]=-1;
    return est_error_pruned;
  } else if (est_error_branch <= est_error + 0.1) {
    /* on supprime le test et on rattache ce noeud au suivant */
    if (print_result) {
      printf("-> the maximum branch is taken\n");
      fflush(stdout);
    }
    tested_attribute[node]=tested_attribute[max_branch];
    threshold[node]=threshold[max_branch];
    left_successor[node]=max_branch+left_successor[max_branch]-node;
    right_successor[node]=max_branch+right_successor[max_branch]-node;
    prediction[node]=prediction[max_branch]; /* pas necessaire normalement */

    est_error=estimate_error_and_prune_c45(node,start,end,1); /* cree les nouvelles predictions */
    /*
      if (est_error!=est_error_branch) {
      printf("Probleme c4.5: est_error=%f, est_error_branch=%f\n",est_error, est_error_branch);
      fflush(stdout);
      }
    */

    return est_error;
  } else {
    if (print_result) {
      printf("-> nothing change\n");
      fflush(stdout);
    }
    /* on ne fait rien */
    return est_error;
  }

}

DllExport void prune_current_tree_c45(float confidence_factor) {

  c45_cf=confidence_factor; /* par defaut 0.25 */

  estimate_error_and_prune_c45(0, 0, current_learning_set_size-1, 1);
}

void prune_tree_c45(int tree) {
  estimate_error_and_prune_c45(tree, 0, current_learning_set_size-1,1);
}

/*******************************
 * EXPERIMENT ON OUTPUT UPDATE *
 *******************************/

/* cette partie est complétement expérimentale et pourrait être retirée */

/* 1 we build a set of trees over some proportion of the learning sample
 * 2 we update the output of the other cases from the last value and with some
 *   learning rate
 * 3 we repeat from 1 until ...
 * 4 we build a model from the whole learning sample using the last value of the ouput
 */

/* parameters */
/* learning rate, ls_prop, number_of_iterations... */

float output_correction_learning_rate=0.1;
float output_correction_ls_prop=0.7;
int output_correction_nb_iterations=20;
int output_correction_nb_cv=10;

ERROR_TYPE *temp_output2;
int length_temp_output2=0;

float getobjy_learn_temp2(int index) {
  return (float)temp_output2[index];
}

/* assume that the ensemble methods is parametrized */

void build_models_with_output_correction(int *ls_vector, int length_ls_vector, int *ts_vector, int length_ts_vector) {
  int length_current_ls=(int)(output_correction_ls_prop*length_ls_vector);
  int i,j;
  int old_print_result=print_result;

  /* initialization */
  if (length_temp_output2<length_ls_vector) {
    if (length_temp_output2!=0)
	MyFree((int *)temp_output2);
    temp_output2=(ERROR_TYPE *)MyMalloc((size_t)length_ls_vector*sizeof(ERROR_TYPE));
    if (!temp_output) {
      printf("impossible d'allouer de la memoire dans build_models_with_output_correction\n");
      exit(0);
    }
    length_temp_output2=length_ls_vector;
  }
  for (i=0; i<length_ls_vector; i++)
    temp_output2[i]=getobjy(ls_vector[i]);

  test_ensemble_while_growing=0;
  store_ensemble=1;
  print_result=0;

  for (i=0; i<output_correction_nb_iterations; i++) {
    printf("Model %d",i);
    fflush(stdout);

    /* random sampling of length_current_ls cases */
    for (j=0; j<length_current_ls; j++) {
      /* select a random position */
      int rn=get_random_integer(length_ls_vector-j);
      /* swap the object and the output */
      int temp=ls_vector[j];
      float temp_out=temp_output2[j];
      ls_vector[j]=ls_vector[j+rn];
      ls_vector[j+rn]=temp;
      temp_output2[j]=temp_output2[j+rn];
      temp_output2[j+rn]=temp_out;
    }
    init_learning_set(ls_vector, length_current_ls);
    
    /* construction of the model */
    getobjy_learn=getobjy_learn_temp2;
    build_one_tree_ensemble(NULL, 0);

    /* update of the output */
    for (j=length_current_ls; j<length_ls_vector; j++) {
      float output_ltrees=apply_current_ltrees_obj_regr(ls_vector[j]);
      temp_output2[j]=(1-output_correction_learning_rate)*temp_output2[j]+output_correction_learning_rate*output_ltrees;
    }

    /* test the current model */
    printf("\nLS:");
    test_current_ltrees_regr(ls_vector, length_ls_vector);
    printf("TS: ");
    test_current_ltrees_regr(ts_vector, length_ts_vector);
  }

  /* build the final model */  
  init_learning_set(ls_vector, length_ls_vector);
  test_ensemble_while_growing=1;
  print_result=old_print_result;
  getobjy_learn=getobjy_learn_temp2;
  build_one_tree_ensemble(ts_vector, length_ts_vector);
}

void build_models_with_output_correction_cv(int *ls_vector, int length_ls_vector, int *ts_vector, int length_ts_vector) {
  int part_size=(int)ceil(length_ls_vector*1.0/output_correction_nb_cv);
  int length_current_ls=((output_correction_nb_cv-1)*part_size);
  int i,j,t, temp, rn;
  int old_print_result=print_result;
  int old_test_while_growing=test_ensemble_while_growing;
  float tempf;
  ERROR_TYPE *temp_output3;
  ERROR_TYPE sum_error1;
  ERROR_TYPE sum_error2;

  /* initialization */
  if (length_temp_output2<length_ls_vector) {
    if (length_temp_output2!=0)
	MyFree((int *)temp_output2);
    temp_output2=(ERROR_TYPE *)MyMalloc((size_t)length_ls_vector*sizeof(ERROR_TYPE));
    if (!temp_output2) {
      printf("impossible d'allouer de la memoire dans build_models_with_output_correction\n");
      exit(0);
    }
    length_temp_output2=length_ls_vector;
  }
  temp_output3=(ERROR_TYPE *)MyMalloc((size_t)length_ls_vector*sizeof(ERROR_TYPE));
  if (!temp_output3) {
    printf("impossible d'allouer de la memoire dans build_models_with_output_correction\n");
    exit(0);
  }

  for (i=0; i<length_ls_vector; i++) {
    temp_output2[i]=getobjy(ls_vector[i]);
  }

  test_ensemble_while_growing=0;
  store_ensemble=1;
  print_result=0;
  
  if (old_test_while_growing) {
    printf("Without correction:");
    init_learning_set(ls_vector, length_ls_vector);
    getobjy_learn=getobjy_learn_temp2;
    build_one_tree_ensemble(ts_vector, length_ts_vector);
    printf("TS: ");
    test_current_ltrees_regr(ts_vector, length_ts_vector);
  }

  for (i=0; i<output_correction_nb_iterations; i++) {
    printf("Model %d",i);
    fflush(stdout);
    
    /* randomize le ls et temp_output */
    for (j=0; j<length_ls_vector; j++) {
      rn=get_random_integer(length_ls_vector-j);
      temp=ls_vector[j];
      tempf=temp_output2[j];
      ls_vector[j]=ls_vector[j+rn];
      ls_vector[j+rn]=temp;
      temp_output2[j]=temp_output2[j+rn];
      temp_output2[j+rn]=tempf;
    }
    
    /* construit les nb_cv modeles (assume that length_ls_vector=nb_cv*part_size) */
    for (t=0; t<output_correction_nb_cv; t++) {
      int borne_inf_test=t*part_size;
      int borne_sup_test=(t+1)*part_size;
      /* put the test set at the end */
      int pos=(t==output_correction_nb_cv-1)?0:((output_correction_nb_cv-1)*part_size);
      for (j=borne_inf_test; j<borne_sup_test; j++) {
	temp=ls_vector[j];
	ls_vector[j]=ls_vector[pos];
	ls_vector[pos]=temp;
	tempf=temp_output2[j];
	temp_output2[j]=temp_output2[pos];
	temp_output2[pos]=tempf;
	tempf=temp_output3[j];
	temp_output3[j]=temp_output3[pos];
	temp_output3[pos]=tempf;
	pos++;
      }
      
      init_learning_set(ls_vector, length_current_ls);

      /* construction of the model */
      getobjy_learn=getobjy_learn_temp2;
      build_one_tree_ensemble(NULL, 0);

      /* store the output in memory */
      for (j=length_current_ls; j<length_ls_vector; j++) {
	temp_output3[j]=apply_current_ltrees_obj_regr(ls_vector[j]);
      }
      printf("."); fflush(stdout);
    }

    /* update the outputs and compute the error */
    sum_error1=0.0;
    sum_error2=0.0;
    for (j=0; j<length_ls_vector; j++) {
      float err1=(temp_output3[j]-getobjy(ls_vector[j]));
      float err2=(temp_output3[j]-temp_output2[j]);
      sum_error1+=(ERROR_TYPE)(err1*err1);
      sum_error2+=(ERROR_TYPE)(err2*err2);
      temp_output2[j]=(1-output_correction_learning_rate)*temp_output2[j]+
	output_correction_learning_rate*temp_output3[j];
    }
    
    /* test the current model */
    if (old_test_while_growing) {
      init_learning_set(ls_vector, length_ls_vector);
      getobjy_learn=getobjy_learn_temp2;
      build_one_tree_ensemble(ts_vector, length_ts_vector);
      printf("LS_true: %e LS_temp: %e ",sum_error1/length_ls_vector, sum_error2/length_ls_vector);
      printf("TS: ");
      test_current_ltrees_regr(ts_vector, length_ts_vector);
    }
  }

  /* build the final model */  
  init_learning_set(ls_vector, length_ls_vector);
  test_ensemble_while_growing=1;
  print_result=old_print_result;
  getobjy_learn=getobjy_learn_temp2;
  build_one_tree_ensemble(ts_vector, length_ts_vector);
}

void set_output_correction_parameters(int nb_it, float lr, float prop, int cross_val_nb) {
  output_correction_learning_rate=lr;
  output_correction_nb_iterations=nb_it;
  output_correction_ls_prop=prop;
  output_correction_nb_cv=cross_val_nb;
}

/******************
 * MISSING VALUES *
 ******************/

/* simple solution: we create a new dataset with random values at each iteration
 * of the ensemble
 */

/* LEARNING */

void copy_core_table() {
  int max_att=goal;
  int i,size;

  backup_core_table=core_table;

  /* recherche l'indice max d'un attribute */
  for (i=0; i<nb_attributes; i++) {
    if (attribute_vector[i]>max_att)
      max_att=attribute_vector[i];
  }
  size=nb_obj_in_core_table*(max_att+1);
  if (size>new_core_table_missing_size) {
    new_core_table_missing=(CORETABLE_TYPE *)MyMalloc((size_t)size*sizeof(float));
    if (!new_core_table_missing) {
      printf("impossible d'allouer de la memoire dans copy_core_table\n");
      exit(0);
    }
    new_core_table_missing_size=size;
  }

  core_table=new_core_table_missing;
  
  /* on remplit le goal qui ne changera pas (sauf si on veut faire du unlabeled training plus tard) */
  for (i=0; i<global_learning_set_size; i++) {
    int index_val=goal*nb_obj_in_core_table+object_mapping[i];
    core_table[index_val]=backup_core_table[index_val];
  }

}

/* remplace les valeurs manquantes, en choisissant une valeur au hasard 
 * entre le min et le max ?
 */
void replace_missing_values_at_random() {
  int i,j;
  /* variables par variables */
  if (print_result) {
    printf("MISSING AT RANDOM");
    fflush(stdout);
  }
  for (i=0; i<nb_attributes; i++) {
    int att=attribute_vector[i];
    if (SYMBOLICp(att)) { /* symbolic variable */
      int nbv=NB_VAL(att);
      for (j=0; j<global_learning_set_size; j++) {
	int index_val=att*nb_obj_in_core_table+object_mapping[j];
	if (backup_core_table[index_val]==MISSING_VALUES_SYMB) {
	  /* suppose que MISSING_VALUES_SYMB est egal à 0.0 */
	  core_table[index_val]=(CORETABLE_TYPE)1.0+get_random_integer(nbv-1);
	} else {
	  core_table[index_val]=backup_core_table[index_val];
	}
      }
    } else { /* numerical variable */
      float min=MISSING_VALUES_NUM;
      float max=MISSING_VALUES_NUM;
      /* on recherche le min et le max */
      for (j=0; j<global_learning_set_size; j++) {
	float val;
	if ((val=(float)backup_core_table[att*nb_obj_in_core_table+object_mapping[j]])!=MISSING_VALUES_NUM) {
	  if (min==MISSING_VALUES_NUM) {
	    min=val;
	    max=val;
	  } else if (val<min)
	    min=val;
	  else if (val>max)
	    max=val;
	}
      }
      /* on remplit la table */
      for (j=0; j<global_learning_set_size; j++) {
	int index_val=att*nb_obj_in_core_table+object_mapping[j];
	if (backup_core_table[index_val]==MISSING_VALUES_NUM) {
	  core_table[index_val]=min+(max-min)*get_random_float();
	} else {
	  core_table[index_val]=backup_core_table[index_val];
	}
      }
    }
  }   
  if (print_result) {
    printf("(OK)");
    fflush(stdout);
  }
}

/* replace les valeurs numeriques par la mediane et les valeurs symboliques
 * par la valeur la plus probable
 */

void replace_missing_values_by_median() {
  int i,j,index_att,k;
  /* variables par variables */
  for (i=0; i<nb_attributes; i++) {
    int att=attribute_vector[i];
    index_att=att*nb_obj_in_core_table;
    if (SYMBOLICp(att)) { /* symbolic variable */
      int nb_val=NB_VAL(att);
      int max_val;
      float max_weight;
      /* calcul des proportions et de la valeur la plus probable */
      for (j=0; j<nb_val; j++)
	table_score_symb[j][0]=0;
      
      for (j=0; j<global_learning_set_size; j++) {
	float v=(float)backup_core_table[index_att+object_mapping[j]];
	if (v!=MISSING_VALUES_SYMB) {
	  table_score_symb[(int)v][0]++;
	}
      }
      max_weight=0.0;
      for (j=0; j<nb_val; j++) {
	if (table_score_symb[j][0]>max_weight) {
	  max_weight=table_score_symb[j][0];
	  max_val=j;
	}
      }

      for (j=0; j<global_learning_set_size; j++) {
	int index_val=att*nb_obj_in_core_table+object_mapping[j];
	if (backup_core_table[index_val]==MISSING_VALUES_SYMB) {
	  core_table[index_val]=(CORETABLE_TYPE)max_val;
	} else {
	  core_table[index_val]=backup_core_table[index_val];
	}
      }
    } else { /* numerical variable */
      CORETABLE_TYPE *temp;
      float median;
      /* on recherche la mediane (recherche par un bete tri) */
      int *temp_vector=(int *)MyMalloc((size_t)global_learning_set_size*sizeof(int));
      if (!temp_vector) {
	printf("Impossible d'allouer de la memoire dans replace_missing_values_by_median");
	fflush(stdout);
	exit(0);
      }
      k=0; 
      for (j=0; j<global_learning_set_size; j++) {
	if ((backup_core_table[index_att+object_mapping[j]])!=MISSING_VALUES_NUM) {
	  temp_vector[k]=j;
	  k++;
	}
      }
      /* on trie */
      temp=core_table;
      core_table=backup_core_table;
      quicksort_ls_vector(temp_vector,0,k-1,att);
      core_table=temp;
      
      /* calcule la mediane */
      if ((k%2)==1) {
	median=backup_core_table[index_att+object_mapping[temp_vector[(int)(k-1)/2]]];
      } else {
	median=0.5*(backup_core_table[index_att+object_mapping[temp_vector[(int)(k/2-1)]]]
		    +backup_core_table[index_att+object_mapping[temp_vector[(int)(k/2)]]]);
      }
      
      /* on remplit la table */
      for (j=0; j<global_learning_set_size; j++) {
	int index_val=index_att+object_mapping[j];
	if (backup_core_table[index_val]==MISSING_VALUES_NUM) {
	  core_table[index_val]=(CORETABLE_TYPE)median;
	} else {
	  core_table[index_val]=backup_core_table[index_val];
	}
      }
      /* on vire le vecteur */
      MyFree((int *)temp_vector);
    }
  }
}

/* pour debugger */

DllExport void display_data_matrix() {
  int i,j;
  for (j=0; j<global_learning_set_size; j++) {
    for (i=0; i<nb_attributes; i++) {
      printf("%d",(int)backup_core_table[attribute_vector[i]*nb_obj_in_core_table+object_mapping[j]]);
    }
    printf("-%f",backup_core_table[goal*nb_obj_in_core_table+object_mapping[j]]);
    printf("\n");
    for (i=0; i<nb_attributes; i++) {
      printf("%d",(int)core_table[attribute_vector[i]*nb_obj_in_core_table+object_mapping[j]]);
    }
    printf("-%f",core_table[goal*nb_obj_in_core_table+object_mapping[j]]);
    printf("\n\n");
  }

  
  fflush(stdout);
}

DllExport void display_data_matrix_float() {
  int i,j;
  for (j=0; j<global_learning_set_size; j++) {
    for (i=0; i<nb_attributes; i++) {
      printf("%f ", (float)backup_core_table[attribute_vector[i]*nb_obj_in_core_table+object_mapping[j]]);
    }
    printf("-%f",(float)backup_core_table[goal*nb_obj_in_core_table+object_mapping[j]]);
    printf("\n");
    for (i=0; i<nb_attributes; i++) {
      printf("%f ",(float)core_table[attribute_vector[i]*nb_obj_in_core_table+object_mapping[j]]);
    }
    printf("-%f",(float)core_table[goal*nb_obj_in_core_table+object_mapping[j]]);
    printf("\n\n");
  }

  
  fflush(stdout);
}

/* remplace les valeurs manquantes en construisant iterativement des arbres */

void replace_missing_values_from_previous_ensemble() {
  int i,j,a,k,t;
  int *temp_vector1;
  float *temp_vector2; 

  /* vecteur de proximite */
  temp_vector1=(int *)MyMalloc((size_t)global_learning_set_size*sizeof(int));
  temp_vector2=(float *)MyMalloc((size_t)global_learning_set_size*sizeof(float));
  if ((!temp_vector1)||(!temp_vector2)) {
    printf("Impossible d'allouer de la memoire dans replace_missing_values_from_previous_ensemble");
    fflush(stdout);
    exit(0);
  }

  if (print_result) {
    printf("Compute prediction matrix...");
    fflush(stdout);
  }
  compute_matrix_pred();
  if (print_result) {
    printf("OK\n");
    fflush(stdout);
  }

  for (i=0; i<global_learning_set_size; i++) {
    int nb_neighbors=0;
    int i_pm=(i+1)*current_nb_of_ensemble_terms;
    printf(".");fflush(stdout);
    /* calcul des proximite */
    for (j=0; j<global_learning_set_size; j++) {
      int j_pm=(j+1)*current_nb_of_ensemble_terms;
      int current_sum=0;
      for (t=0;t<current_nb_of_ensemble_terms; t++) {
	if (matrix_pred[i_pm+t]==matrix_pred[j_pm+t]) {
	  current_sum++;
	}
      }
      if (current_sum>0) {
	temp_vector1[nb_neighbors]=j;
	temp_vector2[nb_neighbors]=1.0*current_sum/current_nb_of_ensemble_terms;
	nb_neighbors++;
      }
    }
    /* printf("\n");fflush(stdout); */
    /* calcul des valeurs manquantes */
    for (a=0; a<nb_attributes; a++) {
      int att=attribute_vector[a];
      int index_att=att*nb_obj_in_core_table;
      if (SYMBOLICp(att) && (backup_core_table[index_att+object_mapping[i]]==MISSING_VALUES_SYMB)) { 
	/* attribut symbolique */
	int nb_val=NB_VAL(att);
	int max_val;
	float max_nb;
	for (k=0; k<nb_val; k++)
	  table_score_symb[k][0]=0;
	for (k=0; k<nb_neighbors; k++) {
	  float v=(float)backup_core_table[index_att+object_mapping[temp_vector1[k]]];
	  if (v!=MISSING_VALUES_SYMB) {
	    table_score_symb[(int)v][0]+=temp_vector2[k];
	  }
	}
	max_nb=0.0;
	for (k=0; k<nb_val; k++) {
	  /* printf("%f ",table_score_symb[k][0]); */
	  if (table_score_symb[k][0]>max_nb) {
	    max_nb=table_score_symb[k][0];
	    max_val=k;
	  }
	}
	/* 
	   printf("\nmax_val=%d\n",max_val);
	   fflush(stdout);
	*/
	/* remplacement de la valeur */
	if (max_nb!=0.0) /* sinon, on ne remplace pas */
	  core_table[index_att+object_mapping[i]]=(CORETABLE_TYPE)max_val;
      } else if (backup_core_table[index_att+object_mapping[i]]==MISSING_VALUES_NUM) {
	/* attribut numerique */
	double sum=0.0;
	double sum_weight=0.0;
	
	for (k=0; k<nb_neighbors; k++) {
	  float v=backup_core_table[index_att+object_mapping[temp_vector1[k]]];
	  if (v!=MISSING_VALUES_NUM) {
	    sum+=(temp_vector2[k]*v);
	    sum_weight+=temp_vector2[k];
	  }
	}
	
	/* remplacement */
	if (sum_weight>0.0) /* sinon, on ne remplace pas */
	  core_table[index_att+object_mapping[i]]=(CORETABLE_TYPE)sum/sum_weight;
      }
    }
  }
  
  /* on vire le vecteur */
  MyFree((float *)temp_vector1);
  MyFree((float *)temp_vector2);
  
}

/* ne fait rien du tout */

void do_nothing() {
}    

/* TEST */

/* to test with missing attribute values */

float *get_tree_prediction_vector_missing(int tree, int obj) {
  int i;
  float proba;
  
  node_weight[tree]=1.0;
  index_stack_nodes=-1;
  index_stack_nodes++;
  stack_nodes[index_stack_nodes]=tree;

  /* initialisation du vecteur */
  for (i=0; i<nb_classes; i++) {
    test_prediction_vector[i]=0.0;
  }
  
  /* propagation dans l'arbre */
  while (index_stack_nodes>=0) {
    int current_node, left, right;

    /* on retire le noeud au sommet de la pile */
    current_node=stack_nodes[index_stack_nodes];
    index_stack_nodes--;

    if (left_successor[current_node]==-1) { /* leaf */
      int index_proba=prediction[current_node];
      if ((proba=node_weight[current_node])!=0.0) {
	
	for (i=0; i<nb_classes; i++) {
	  test_prediction_vector[i]+=(proba*prediction_values[index_proba][i]);
	}
      }
    } else {
      /* on calcule les probas d'aller a droite et a gauche */
      float attval=getattval(obj,tested_attribute[current_node]);
     
      if ((SYMBOLICp(tested_attribute[current_node]) && attval==MISSING_VALUES_SYMB)
	  || (NUMERICALp(tested_attribute[current_node]) && attval==MISSING_VALUES_NUM)) {
	/* missing values */
	
	left=current_node+left_successor[current_node];
	right=current_node+right_successor[current_node];

	index_stack_nodes++;
	stack_nodes[index_stack_nodes]=left;
	node_weight[left]=node_weight[current_node]*node_size[left]/node_size[current_node];
	
	index_stack_nodes++;
	stack_nodes[index_stack_nodes]=right;
	node_weight[right]=node_weight[current_node]*node_size[right]/node_size[current_node];
      } else {
	if (check_test(tested_attribute[current_node],attval,threshold[current_node])) {
	  left=current_node+left_successor[current_node];
	  index_stack_nodes++;
	  stack_nodes[index_stack_nodes]=left;
	  node_weight[left]=node_weight[current_node];
	} else {
	  right=current_node+right_successor[current_node];
	  index_stack_nodes++;
	  stack_nodes[index_stack_nodes]=right;
	  node_weight[right]=node_weight[current_node];
	}
      }
    }
  }

  return test_prediction_vector;
}

DllExport void init_missing_values(int mv, int method, int param) {
  int i,temp_test_ensemble_while_growing;

#ifdef LOAD_DB_HANDLER
  if (mv && core_table_type!=NORMAL_CORE_TABLE) {
    printf("Sorry, missing values are not handled with sparse and two-dimensional core xtable\n");
    fflush(stdout);
    missing_values=0;
    return;    
  }
#endif

  missing_values=mv;
  if (missing_values) {
    copy_core_table();
    if (method==0) {
      /* on ne fait le remplacement qu'une seule fois par la mediane */
      if (print_result) {
	printf("Replace missing values by median\n");
	fflush(stdout);
      }
      replace_missing_values_by_median();
      replace_missing_values=do_nothing;

      /*
      for (i=0; i<global_learning_set_size; i++) {
	for (j=0;j<nb_attributes;j++) {
	  printf("%d ", (int)backup_core_table[attribute_vector[j]*nb_obj_in_core_table+object_mapping[i]]);
	}
	printf("\n");
      }
      fflush(stdout);
      */
    } else if (method==1) {
      replace_missing_values=replace_missing_values_at_random;
    } else if (method==2) {
      temp_test_ensemble_while_growing=test_ensemble_while_growing;
      if (print_result) {
	printf("ITERATIVE REPLACEMENT OF MISSING VALUES\n");
	fflush(stdout);
      }

      test_ensemble_while_growing=0;
      get_tree_prediction_vector=get_tree_prediction_vector_classical;
      replace_missing_values_by_median();
      replace_missing_values=do_nothing;

      for (i=0; i<param; i++) {
	if (print_result) {
	  printf("Iteration (%d/%d):\n\n", i+1, param);
	  fflush(stdout);
	}
	build_one_tree_ensemble(NULL, 0);
	replace_missing_values_from_previous_ensemble();
	clean_all_trees();
      }
      replace_missing_values=do_nothing;
      get_tree_prediction_vector=get_tree_prediction_vector_missing;
      test_ensemble_while_growing=temp_test_ensemble_while_growing;
    }
    get_tree_prediction_vector=get_tree_prediction_vector_missing;
  }
}

/* pour mes essais */

CORETABLE_TYPE *temp_core_table;
int temp_core_table_size=0;

DllExport void introduce_missing_values(float p, int in_ls, int in_ts) {
  int max_att=goal;
  int i,j,size,index_val;

  /* recherche l'indice max des attribut */
  for (i=0; i<nb_attributes; i++) {
    if (attribute_vector[i]>max_att)
      max_att=attribute_vector[i];
  }
  size=nb_obj_in_core_table*(max_att+1);
  if (size>temp_core_table_size) {
    temp_core_table=(CORETABLE_TYPE *)MyMalloc((size_t)size*sizeof(float));
    if (!temp_core_table) {
      printf("impossible d'allouer de la memoire dans copy_core_table\n");
      exit(0);
    }
    temp_core_table_size=size;
  }
  
  /* on remplit (toute) la table (pour le test)*/
  index_val=goal*nb_obj_in_core_table;
  for (i=index_val; i<index_val+nb_obj_in_core_table; i++) {
    temp_core_table[i]=core_table[i];
  }

  for(j=0; j<nb_attributes; j++) {
    int index_val_att=attribute_vector[j]*nb_obj_in_core_table;
    /* TOUT */
    if (in_ts) {
      for (i=index_val_att; i<index_val_att+nb_obj_in_core_table; i++) {
	if (get_random_float()<p) {
	  if (SYMBOLICp(attribute_vector[j]))
	    temp_core_table[i]=(CORETABLE_TYPE)MISSING_VALUES_SYMB;
	  else
	    temp_core_table[i]=(CORETABLE_TYPE)MISSING_VALUES_NUM;
	} else {
	  temp_core_table[i]=core_table[i];
	}
      }
    } else {
      for (i=index_val_att; i<index_val_att+nb_obj_in_core_table; i++) {
	temp_core_table[i]=core_table[i];
      }
    }
    /* LS */
    if (!in_ls) {
      index_val_att=attribute_vector[j]*nb_obj_in_core_table;
      for (i=0; i<global_learning_set_size; i++) {
	index_val=index_val_att+object_mapping[i];
	temp_core_table[index_val]=core_table[index_val];
      }
    } else {
      index_val_att=attribute_vector[j]*nb_obj_in_core_table;
      for (i=0; i<global_learning_set_size; i++) {
	index_val=index_val_att+object_mapping[i];
	if (get_random_float()<p) {
	  if (SYMBOLICp(attribute_vector[j]))
	    temp_core_table[index_val]=(CORETABLE_TYPE)MISSING_VALUES_SYMB;
	  else
	    temp_core_table[index_val]=(CORETABLE_TYPE)MISSING_VALUES_NUM;
	} else {
	  temp_core_table[index_val]=core_table[index_val];
	}
      } 
    }
  }
  
  core_table=temp_core_table;
}


/* quelques petits essais: */

void printmodifarray(int *vect, int n, char *str) {
  int i;
  printf("%s",str);
  for (i=0;i<n;i++) 
    vect[i]=i;
}

void printarray(int *vect, int n) {
  int i;
  for (i=0;i<n;i++) 
    printf("%d ",vect[i]);
  printf("\n");
  fflush(stdout);
}

/* pour tester un seul arbre de l'ensemble (pour les tests avec Yvan) */

DllExport void apply_current_oneltrees_obj_clas(int tree, int obj, float *vector) {
  int i;
  float *proba_vect;
  float w;
  
  for (i=0; i<nb_classes; i++) {
    vector[i]=0.0;
  }

  proba_vect=get_tree_prediction_vector(ltrees[tree], obj);
  w=ltrees_weight[tree];
  for (i=0; i<nb_classes; i++)
    vector[i]+=w*proba_vect[i];
  
/*   for (i=0; i<nb_classes; i++) { */
/*     sum+=vector[i]; */
/*     if (vector[i]>max_proba) { */
/*       max_proba=vector[i]; */
/*       best_class=i; */
/*     } */
/*   } */

/*
  for (i=0; i<nb_classes; i++)
    vector[i]/=sum;

  return best_class;
*/

}

#endif //!FILE_TREE_MODEL_C


/* MACRO, CONSTANTES */

#define VALUE_PI 3.141592653589793
#define PLUS_INFINI FLT_MAX
#define MOINS_INFINI -FLT_MAX

/* precision des differentes fonction de calcul */

#define PRECISION double
/* allocation dynamique -> ces limites sont supprimees */
/* #define MAX_NUMBER_OF_NODES 1000500 */
/* #define MAX_NUMBER_OF_PROBABILITY 500100 */
/* #define MAX_NUMBER_OF_PREDICTION_VALUES 50 nombre de classes max aussi */

#define MAX_OPEN_NODES 15000
#define MAX_TEST_CASES 20000

#define SCORE_TYPE double
/* permet de definir la core_table en char pour les grosses BDs */
#define CORETABLE_TYPE float
/* #define CORETABLE_TYPE unsigned char */

#define ERROR_TYPE double
#define MAX_NUMBER_OF_ATTRIBUTES 300000

/* permet de definir une nouvelle fonction getattval *
 * Paramétrable au cas où ça ralentirait les temps de calcul 
 * (a vérifier) */
#define GETATTVAL_POINTER 1

/* on s'assure que GETATTVAL_POINTER est défini
 * si on veut charger SANDC et DB_HANDLER
 */

#if defined(LOAD_DB_HANDLER) && defined(LOAD_SANDC)
#ifndef GETATTVAL_POINTER
#define GETATTVAL_POINTER
#endif
#endif

/* pour les variables symboliques */

#define MAX_NUMBER_OF_SYMBOLIC_VALUES 1024
#define MAX_NUMBER_OF_SYMBOLIC_VALUES_DIV_32 32 /* C'est à dire maximum 128 valeurs */

#define BITN(x,n) (x.i[n/32]>>(n%32))%2
#define SET_BITN(x,n) x.i[n/32]|=(1<<(n%32))

#define SYMBOLICp(att) attribute_descriptors[att]>0
#define NUMERICALp(att) attribute_descriptors[att]==0
#define NB_VAL(att) attribute_descriptors[att]

/* type de problème: */

#define CLASSIFICATION 0
#define REGRESSION 1
#define DISTANCE 2
#define MULTIREGR 3

/* valeurs manquantes */

#define MISSING_VALUES_NUM -10000.0
#define MISSING_VALUES_SYMB 0.0

/* type de tri */

#define LOCAL_SORT 0
#define GLOBAL_SORT_PROJECTION 1
#define GLOBAL_SORT_REARRANGE 2

#define MAX(x,y) (x<y? y: x)
#define MIN(x,y) (x<y? x: y)

/* type de BD */

#define SPARSE_CORE_TABLE 0  /* bd sparse */
#define NORMAL_CORE_TABLE 1  /* bd sous la forme d un gros vecteur (seul format pour lequel tout fonctionne) */
#define TWO_DIM_CORE_TABLE 2 /* bd sous la forme d une table a deux dimensions */

/* pour la compilation sous windows */

#ifdef _WIN32
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif

/* redefinition du malloc (pour l'interface matlab) */
#define MyMalloc malloc
#define MyFree free
#define MyFreeAndNull(x) free(x); x = NULL

/* definition de types */

union threshold_type {
  unsigned int i[MAX_NUMBER_OF_SYMBOLIC_VALUES_DIV_32];
  float f;
};

/* typedef float PredVect[MAX_NUMBER_OF_PREDICTION_VALUES]; */

/* prototype de fonctions */

CORETABLE_TYPE *backup_core_table;
CORETABLE_TYPE *new_core_table_missing;
int new_core_table_missing_size=0;
void (*replace_missing_values)();

/* fonction qui renvoie la sortie correspondant a l'objet */
int getobjclass_learn_true(int index);

float getobjy_learn_true(int index);

/* par defaut */
int (*getobjclass_learn)(int obj)=getobjclass_learn_true;
float (*getobjy_learn)(int obj)=getobjy_learn_true;


//int (*getobjclass_learn)(int obj);
//float (*getobjy_learn)(int obj);
#ifdef GETATTVAL_POINTER
float (*getattval)(int obj, int att);
#endif

/* fonction qui calcule des stats pour le vecteur et qui place le resultat dans table_score */
void (*summarize_vector)(int *vector, int start, int end);
/* fonction qui cree un vecteur de prediction */
int (*make_leaf_prediction)();
/* fonction qui renvoie true si on doit arreter la construction */
int (*stop_splitting_criterio)();
/* fonction d'evaluation pour le best_first */
SCORE_TYPE (*best_first_node_priority_function)();
/* fonction qui renvoie true si le test choisi n'est pas significatif */
int (*not_significant_test)();
/* fonction qui calcule le score a partir d'une table de score */
SCORE_TYPE (*compute_score_from_table)();
/* une fonction qui recherche un test pour splitter le noeud */
void (*find_a_split)(int *ls_vector, int start, int end);
/* une fonction qui recherche un seuil pour un attribut numerique */
void (*find_a_threshold_num)(int att, int *ls_vector, int start, int end);
/* une fonction qui recherche un seuil pour un attribut symbolique */
void (*find_a_threshold_symb)(int att, int *ls_vector, int start, int end);

void write_one_tree(int tree, FILE *fp);

/* Sorting function */

void quicksort_ls_vector(int ls_vector[], int start, int end, int att);
int separate_ls_vector_local(int best_attribute, union threshold_type best_threshold, int *ls_vector, int start, int end);

/* generique */
DllExport int build_one_tree();
void find_a_split_at_random(int *ls_vector, int start, int end);
void find_a_threshold(int att, int *ls_vector, int start, int end);
int check_test(int att, float val, union threshold_type threshold);

/* classification */
SCORE_TYPE compute_classification_score_from_table_log();
SCORE_TYPE compute_classification_score_from_table_gini();
int stop_splitting_criterio_classification_log();
int stop_splitting_criterio_classification_gini();
SCORE_TYPE best_first_node_priority_function_class_log();
SCORE_TYPE best_first_node_priority_function_class_gini();
int not_significant_test_classification_log();
int not_significant_test_classification_gini();
void summarize_vector_clas(int *vector, int start, int end);
int make_leaf_prediction_classification();
int test_tree_classification(int tree, int *ts_vector, int length_ts_vector);
void find_a_threshold_at_random_1_clas(int att, int *ls_vector, int start, int end);
void find_a_threshold_at_random_2_clas(int att, int *ls_vector, int start, int end);
void find_a_threshold_at_random_3_clas(int att, int *ls_vector, int start, int end);
void find_a_threshold_at_random_4_clas(int att, int *ls_vector, int start, int end);
void prune_tree_c45(int tree);

/* regression */
SCORE_TYPE compute_regression_score_from_table();
int stop_splitting_criterio_regression();
SCORE_TYPE best_first_node_priority_function_regr();
int not_significant_test_regression();
void summarize_vector_regr(int *vector, int start, int end);
int make_leaf_prediction_regression();
void find_a_threshold_at_random_1_regr(int att, int *ls_vector, int start, int end);
void find_a_threshold_at_random_2_regr(int att, int *ls_vector, int start, int end);
void find_a_threshold_at_random_3_regr(int att, int *ls_vector, int start, int end);
void find_a_threshold_at_random_4_regr(int att, int *ls_vector, int start, int end);

float test_tree_regression(int tree, int *ts_vector, int length_ts_vector);

/* utilitaires */
DllExport int get_random_integer(int max_val);
float get_random_normal();
void get_random_subset_non_empty_full(union threshold_type *rs, int max_val);
DllExport float get_random_float();
int my_best_class(float *proba_vector, int nb_classes);
float chi_square(int ddl, float conf);

#ifdef GETATTVAL_POINTER
float getattval_normal(int obj, int att);
#else
float getattval(int obj, int att);
#endif

int getobjclass(int obj);
float getobjy(int obj);

void print_bit_repr(int a);
void printtablescore();
void printtablescoresymb(int nb_v, int nb_c);
void printlsvector(int *ls_vector, int start, int end);
void printmatrix(int *matrix, int nb_row, int nb_col);
void printlsvectoratt(int *ls_vector, int start, int end, int att);
float get_att_stddev(int att);
DllExport void compute_att_stddev_from_ls();

DllExport double compute_ltrees_variable_importance(int *ts_vector, int length_ts_vector, SCORE_TYPE *attribute_importance);
SCORE_TYPE compute_one_tree_variable_importance(int tree, int *ts_vector, int length_ts_vector, float weight, SCORE_TYPE *attribute_importance);
SCORE_TYPE get_vi(int *ts_vector, int start, int end, int borne);
int separate_ls_vector_vi(int best_attribute, union threshold_type best_threshold, int *ls_vector, int start, int end);

/* interface */
DllExport void init_db1(CORETABLE_TYPE *core_t, int nb_obj, int *att_desc);
DllExport void init_db2(CORETABLE_TYPE *core_t, int nb_obj, int *att_desc);
DllExport void clean_all_trees();
void clean_all_ensemble();
DllExport void set_score_gini_classification();
DllExport void set_score_log_classification();
void set_random_threshold_method(int i);
DllExport void init_classification_problem(int *att_vect, int nb_att, int goal_class, int nb_clas);
DllExport void init_regression_problem(int *att_vect, int nb_att, int goal_regr);
DllExport void init_sorting_method();

float make_ls_vector_bagging(int tree);
float make_ls_vector_boosting(int tree);
float make_ls_vector_identity(int tree);
float make_ls_vector_mart_regr(int tree);
float make_ls_vector_logitboost(int tree);

float update_test_data_clas(int tree, float weight, int *ts_vector, int length_ts_vector);
float update_test_data_regr(int tree, float weight, int *ts_vector, int length_ts_vector);

DllExport int get_tree_nb_nodes(int tree);

/* interface particulière avec d'autres modules */

/* fonctions et variables provenant de tree-kernel.c et nécessaire à tree-model.c */

extern int last_1nn_ltrees_sim;
extern float **matrix_pred;
DllExport void compute_matrix_pred();

/* tree-kernel.c */

DllExport int get_nn_ltrees_fast(int o);

/* tree-distance.c */

#ifdef LOAD_OK3

float make_ls_vector_mart_distance(int tree);
int make_leaf_prediction_distance_mart();
float make_ls_vector_mart_ok_uniregr(int tree);

#endif

/* tree-multiregr.c */

#ifdef LOAD_MULTIREGR

void free_multiregr_table_score();
float make_ls_vector_mart_multiregr(int tree);

#endif

/* db-handler.c */

#ifdef LOAD_DB_HANDLER

extern int core_table_type;
float getattval_sparse(int obj, int att);
int getobjclass_learn_sparse(int index);
float getobjy_learn_sparse(int index);
int getobjclass_sparse(int obj);
float getobjy_sparse(int obj);

extern CORETABLE_TYPE **core_table_2dim;
float getattval_2dim(int obj, int att);
int getobjclass_learn_2dim(int index);
float getobjy_learn_2dim(int index);
int getobjclass_2dim(int obj);
float getobjy_2dim(int obj);
void set_core_table_value(float val, int obj, int att);
void set_core_table_value_2dim(float val, int obj, int att);
#endif

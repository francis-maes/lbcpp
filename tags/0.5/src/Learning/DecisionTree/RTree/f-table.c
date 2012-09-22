
#ifndef FILE_TREE_TABLE_C
# define FILE_TREE_TABLE_C

/* pour faire un f-test en regression */

/* tables reprises de http://www.stat.auckland.ac.nz/~wild/ChanceEnc/ */

/* colonnes=df2=N-k o� N est le nombre total d'objet dans le noeud et k le nombre de successeurs
 *  lignes=df1=k-1 o� k est le nombre de successeurs
 */

/* 10 % */

const float f_table_10[37][17]={{0,1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 20, 30, 60, 1000},
{1, 39.9, 49.5, 53.6, 55.8, 57.2, 58.2, 58.9, 59.4, 59.9, 60.2, 60.7, 61.2, 61.7, 62.3, 62.8, 63.3},
{2, 8.53, 9.00, 9.16, 9.24, 9.29, 9.33, 9.35, 9.37, 9.38, 9.39, 9.41, 9.42, 9.44, 9.46, 9.47, 9.49},
{3, 5.54, 5.46, 5.39, 5.34, 5.31, 5.28, 5.27, 5.25, 5.24, 5.23, 5.22, 5.20, 5.18, 5.17, 5.15, 5.13},
{4, 4.54, 4.32, 4.19, 4.11, 4.05, 4.01, 3.98, 3.95, 3.94, 3.92, 3.90, 3.87, 3.84, 3.82, 3.79, 3.76},
{5, 4.06, 3.78, 3.62, 3.52, 3.45, 3.40, 3.37, 3.34, 3.32, 3.30, 3.27, 3.24, 3.21, 3.17, 3.14, 3.10},
{6, 3.78, 3.46, 3.29, 3.18, 3.11, 3.05, 3.01, 2.98, 2.96, 2.94, 2.90, 2.87, 2.84, 2.80, 2.76, 2.72},
{7, 3.59, 3.26, 3.07, 2.96, 2.88, 2.83, 2.78, 2.75, 2.72, 2.70, 2.67, 2.63, 2.59, 2.56, 2.51, 2.47},
{8, 3.46, 3.11, 2.92, 2.81, 2.73, 2.67, 2.62, 2.59, 2.56, 2.54, 2.50, 2.46, 2.42, 2.38, 2.34, 2.29},
{9, 3.36, 3.01, 2.81, 2.69, 2.61, 2.55, 2.51, 2.47, 2.44, 2.42, 2.38, 2.34, 2.30, 2.25, 2.21, 2.16},
{10, 3.29, 2.92, 2.73, 2.61, 2.52, 2.46, 2.41, 2.38, 2.35, 2.32, 2.28, 2.24, 2.20, 2.16, 2.11, 2.06},
{11, 3.23, 2.86, 2.66, 2.54, 2.45, 2.39, 2.34, 2.30, 2.27, 2.25, 2.21, 2.17, 2.12, 2.08, 2.03, 1.97},
{12, 3.18, 2.81, 2.61, 2.48, 2.39, 2.33, 2.28, 2.24, 2.21, 2.19, 2.15, 2.10, 2.06, 2.01, 1.96, 1.90},
{13, 3.14, 2.76, 2.56, 2.43, 2.35, 2.28, 2.23, 2.20, 2.16, 2.14, 2.10, 2.05, 2.01, 1.96, 1.90, 1.85},
{14, 3.10, 2.73, 2.52, 2.39, 2.31, 2.24, 2.19, 2.15, 2.12, 2.10, 2.05, 2.01, 1.96, 1.91, 1.86, 1.80},
{15, 3.07, 2.70, 2.49, 2.36, 2.27, 2.21, 2.16, 2.12, 2.09, 2.06, 2.02, 1.97, 1.92, 1.87, 1.82, 1.76},
{16, 3.05, 2.67, 2.46, 2.33, 2.24, 2.18, 2.13, 2.09, 2.06, 2.03, 1.99, 1.94, 1.89, 1.84, 1.78, 1.72},
{17, 3.03, 2.64, 2.44, 2.31, 2.22, 2.15, 2.10, 2.06, 2.03, 2.00, 1.96, 1.91, 1.86, 1.81, 1.75, 1.69},
{18, 3.01, 2.62, 2.42, 2.29, 2.20, 2.13, 2.08, 2.04, 2.00, 1.98, 1.93, 1.89, 1.84, 1.78, 1.72, 1.66},
{19, 2.99, 2.61, 2.40, 2.27, 2.18, 2.11, 2.06, 2.02, 1.98, 1.96, 1.91, 1.86, 1.81, 1.76, 1.70, 1.63},
{20, 2.97, 2.59, 2.38, 2.25, 2.16, 2.09, 2.04, 2.00, 1.96, 1.94, 1.89, 1.84, 1.79, 1.74, 1.68, 1.61},
{21, 2.96, 2.57, 2.36, 2.23, 2.14, 2.08, 2.02, 1.98, 1.95, 1.92, 1.87, 1.83, 1.78, 1.72, 1.66, 1.59},
{22, 2.95, 2.56, 2.35, 2.22, 2.13, 2.06, 2.01, 1.97, 1.93, 1.90, 1.86, 1.81, 1.76, 1.70, 1.64, 1.57},
{23, 2.94, 2.55, 2.34, 2.21, 2.11, 2.05, 1.99, 1.95, 1.92, 1.89, 1.84, 1.80, 1.74, 1.69, 1.62, 1.55},
{24, 2.93, 2.54, 2.33, 2.19, 2.10, 2.04, 1.98, 1.94, 1.91, 1.88, 1.83, 1.78, 1.73, 1.67, 1.61, 1.53},
{25, 2.92, 2.53, 2.32, 2.18, 2.09, 2.02, 1.97, 1.93, 1.89, 1.87, 1.82, 1.77, 1.72, 1.66, 1.59, 1.52},
{26, 2.91, 2.52, 2.31, 2.17, 2.08, 2.01, 1.96, 1.92, 1.88, 1.86, 1.81, 1.76, 1.71, 1.65, 1.58, 1.50},
{27, 2.90, 2.51, 2.30, 2.17, 2.07, 2.00, 1.95, 1.91, 1.87, 1.85, 1.80, 1.75, 1.70, 1.64, 1.57, 1.49},
{28, 2.89, 2.50, 2.29, 2.16, 2.06, 2.00, 1.94, 1.90, 1.87, 1.84, 1.79, 1.74, 1.69, 1.63, 1.56, 1.48},
{29, 2.89, 2.50, 2.28, 2.15, 2.06, 1.99, 1.93, 1.89, 1.86, 1.83, 1.78, 1.73, 1.68, 1.62, 1.55, 1.47},
{30, 2.88, 2.49, 2.28, 2.14, 2.05, 1.98, 1.93, 1.88, 1.85, 1.82, 1.77, 1.72, 1.67, 1.61, 1.54, 1.46},
{40, 2.84, 2.44, 2.23, 2.09, 2.00, 1.93, 1.87, 1.83, 1.79, 1.76, 1.71, 1.66, 1.61, 1.54, 1.47, 1.38},
{60, 2.79, 2.39, 2.18, 2.04, 1.95, 1.87, 1.82, 1.77, 1.74, 1.71, 1.66, 1.60, 1.54, 1.48, 1.40, 1.29},
{80, 2.77, 2.37, 2.15, 2.02, 1.92, 1.85, 1.79, 1.75, 1.71, 1.68, 1.63, 1.57, 1.51, 1.44, 1.36, 1.24},
{100, 2.76, 2.36, 2.14, 2.00, 1.91, 1.83, 1.78, 1.73, 1.69, 1.66, 1.61, 1.56, 1.49, 1.42, 1.34, 1.21},
{120, 2.75, 2.35, 2.13, 1.99, 1.90, 1.82, 1.77, 1.72, 1.68, 1.65, 1.60, 1.55, 1.48, 1.41, 1.32, 1.19},
{1000, 2.71, 2.30, 2.08, 1.94, 1.85, 1.77, 1.72, 1.67, 1.63, 1.60, 1.55, 1.49, 1.42, 1.34, 1.24, 1.00}};

/*, 5%, */

const float f_table_5[37][17]={{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 20, 30, 60, 1000},
{1, 161, 199, 215, 224, 230, 233, 236, 238, 240, 241, 243, 245, 248, 250, 252, 254},
{2, 18.5, 19.0, 19.2, 19.2, 19.3, 19.3, 19.4, 19.4, 19.4, 19.4, 19.4, 19.4, 19.4, 19.5, 19.5, 19.5},
{3, 10.1, 9.55, 9.28, 9.12, 9.01, 8.94, 8.89, 8.85, 8.81, 8.79, 8.74, 8.70, 8.66, 8.62, 8.57, 8.53},
{4, 7.71, 6.94, 6.59, 6.39, 6.26, 6.16, 6.09, 6.04, 6.00, 5.96, 5.91, 5.86, 5.80, 5.75, 5.69, 5.63},
{5, 6.61, 5.79, 5.41, 5.19, 5.05, 4.95, 4.88, 4.82, 4.77, 4.74, 4.68, 4.62, 4.56, 4.50, 4.43, 4.36},
{6, 5.99, 5.14, 4.76, 4.53, 4.39, 4.28, 4.21, 4.15, 4.10, 4.06, 4.00, 3.94, 3.87, 3.81, 3.74, 3.67},
{7, 5.59, 4.74, 4.35, 4.12, 3.97, 3.87, 3.79, 3.73, 3.68, 3.64, 3.57, 3.51, 3.44, 3.38, 3.30, 3.23},
{8, 5.32, 4.46, 4.07, 3.84, 3.69, 3.58, 3.50, 3.44, 3.39, 3.35, 3.28, 3.22, 3.15, 3.08, 3.01, 2.93},
{9, 5.12, 4.26, 3.86, 3.63, 3.48, 3.37, 3.29, 3.23, 3.18, 3.14, 3.07, 3.01, 2.94, 2.86, 2.79, 2.71},
{10, 4.96, 4.10, 3.71, 3.48, 3.33, 3.22, 3.14, 3.07, 3.02, 2.98, 2.91, 2.85, 2.77, 2.70, 2.62, 2.54},
{11, 4.84, 3.98, 3.59, 3.36, 3.20, 3.09, 3.01, 2.95, 2.90, 2.85, 2.79, 2.72, 2.65, 2.57, 2.49, 2.40},
{12, 4.75, 3.89, 3.49, 3.26, 3.11, 3.00, 2.91, 2.85, 2.80, 2.75, 2.69, 2.62, 2.54, 2.47, 2.38, 2.30},
{13, 4.67, 3.81, 3.41, 3.18, 3.03, 2.92, 2.83, 2.77, 2.71, 2.67, 2.60, 2.53, 2.46, 2.38, 2.30, 2.21},
{14, 4.60, 3.74, 3.34, 3.11, 2.96, 2.85, 2.76, 2.70, 2.65, 2.60, 2.53, 2.46, 2.39, 2.31, 2.22, 2.13},
{15, 4.54, 3.68, 3.29, 3.06, 2.90, 2.79, 2.71, 2.64, 2.59, 2.54, 2.48, 2.40, 2.33, 2.25, 2.16, 2.07},
{16, 4.49, 3.63, 3.24, 3.01, 2.85, 2.74, 2.66, 2.59, 2.54, 2.49, 2.42, 2.35, 2.28, 2.19, 2.11, 2.01},
{17, 4.45, 3.59, 3.20, 2.96, 2.81, 2.70, 2.61, 2.55, 2.49, 2.45, 2.38, 2.31, 2.23, 2.15, 2.06, 1.96},
{18, 4.41, 3.55, 3.16, 2.93, 2.77, 2.66, 2.58, 2.51, 2.46, 2.41, 2.34, 2.27, 2.19, 2.11, 2.02, 1.92},
{19, 4.38, 3.52, 3.13, 2.90, 2.74, 2.63, 2.54, 2.48, 2.42, 2.38, 2.31, 2.23, 2.16, 2.07, 1.98, 1.88},
{20, 4.35, 3.49, 3.10, 2.87, 2.71, 2.60, 2.51, 2.45, 2.39, 2.35, 2.28, 2.20, 2.12, 2.04, 1.95, 1.84},
{21, 4.32, 3.47, 3.07, 2.84, 2.68, 2.57, 2.49, 2.42, 2.37, 2.32, 2.25, 2.18, 2.10, 2.01, 1.92, 1.81},
{22, 4.30, 3.44, 3.05, 2.82, 2.66, 2.55, 2.46, 2.40, 2.34, 2.30, 2.23, 2.15, 2.07, 1.98, 1.89, 1.78},
{23, 4.28, 3.42, 3.03, 2.80, 2.64, 2.53, 2.44, 2.37, 2.32, 2.27, 2.20, 2.13, 2.05, 1.96, 1.86, 1.76},
{24, 4.26, 3.40, 3.01, 2.78, 2.62, 2.51, 2.42, 2.36, 2.30, 2.25, 2.18, 2.11, 2.03, 1.94, 1.84, 1.73},
{25, 4.24, 3.39, 2.99, 2.76, 2.60, 2.49, 2.40, 2.34, 2.28, 2.24, 2.16, 2.09, 2.01, 1.92, 1.82, 1.71},
{26, 4.23, 3.37, 2.98, 2.74, 2.59, 2.47, 2.39, 2.32, 2.27, 2.22, 2.15, 2.07, 1.99, 1.90, 1.80, 1.69},
{27, 4.21, 3.35, 2.96, 2.73, 2.57, 2.46, 2.37, 2.31, 2.25, 2.20, 2.13, 2.06, 1.97, 1.88, 1.79, 1.67},
{28, 4.20, 3.34, 2.95, 2.71, 2.56, 2.45, 2.36, 2.29, 2.24, 2.19, 2.12, 2.04, 1.96, 1.87, 1.77, 1.65},
{29, 4.18, 3.33, 2.93, 2.70, 2.55, 2.43, 2.35, 2.28, 2.22, 2.18, 2.10, 2.03, 1.94, 1.85, 1.75, 1.64},
{30, 4.17, 3.32, 2.92, 2.69, 2.53, 2.42, 2.33, 2.27, 2.21, 2.16, 2.09, 2.01, 1.93, 1.84, 1.74, 1.62},
{40, 4.08, 3.23, 2.84, 2.61, 2.45, 2.34, 2.25, 2.18, 2.12, 2.08, 2.00, 1.92, 1.84, 1.74, 1.64, 1.51},
{60, 4.00, 3.15, 2.76, 2.53, 2.37, 2.25, 2.17, 2.10, 2.04, 1.99, 1.92, 1.84, 1.75, 1.65, 1.53, 1.39},
{80, 3.96, 3.11, 2.72, 2.49, 2.33, 2.21, 2.13, 2.06, 2.00, 1.95, 1.88, 1.79, 1.70, 1.60, 1.48, 1.32},
{100, 3.94, 3.09, 2.70, 2.46, 2.31, 2.19, 2.10, 2.03, 1.97, 1.93, 1.85, 1.77, 1.68, 1.57, 1.45, 1.28},
{120, 3.92, 3.07, 2.68, 2.45, 2.29, 2.18, 2.09, 2.02, 1.96, 1.91, 1.83, 1.75, 1.66, 1.55, 1.43, 1.25},
{1000, 3.84, 3.00, 2.60, 2.37, 2.21, 2.10, 2.01, 1.94, 1.88, 1.83, 1.75, 1.67, 1.57, 1.46, 1.32, 1.00}};

/*, 1%, */

const float f_table_1[37][17]={{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 20, 30, 60, 1000},
{1, 4052, 4999, 5403, 5624, 5763, 5858, 5928, 5981, 6022, 6055, 6106, 6157, 6208, 6260, 6313, 6365},
{2, 98.5, 99.0, 99.2, 99.2, 99.3, 99.3, 99.4, 99.4, 99.4, 99.4, 99.4, 99.4, 99.4, 99.5, 99.5, 99.5},
{3, 34.1, 30.8, 29.5, 28.7, 28.2, 27.9, 27.7, 27.5, 27.3, 27.2, 27.1, 26.9, 26.7, 26.5, 26.3, 26.1},
{4, 21.2, 18.0, 16.7, 16.0, 15.5, 15.2, 15.0, 14.8, 14.7, 14.5, 14.4, 14.2, 14.0, 13.8, 13.7, 13.5},
{5, 16.3, 13.3, 12.1, 11.4, 11.0, 10.7, 10.5, 10.3, 10.2, 10.1, 9.89, 9.72, 9.55, 9.38, 9.20, 9.02},
{6, 13.7, 10.9, 9.78, 9.15, 8.75, 8.47, 8.26, 8.10, 7.98, 7.87, 7.72, 7.56, 7.40, 7.23, 7.06, 6.88},
{7, 12.2, 9.55, 8.45, 7.85, 7.46, 7.19, 6.99, 6.84, 6.72, 6.62, 6.47, 6.31, 6.16, 5.99, 5.82, 5.65},
{8, 11.3, 8.65, 7.59, 7.01, 6.63, 6.37, 6.18, 6.03, 5.91, 5.81, 5.67, 5.52, 5.36, 5.20, 5.03, 4.86},
{9, 10.6, 8.02, 6.99, 6.42, 6.06, 5.80, 5.61, 5.47, 5.35, 5.26, 5.11, 4.96, 4.81, 4.65, 4.48, 4.31},
{10, 10.0, 7.56, 6.55, 5.99, 5.64, 5.39, 5.20, 5.06, 4.94, 4.85, 4.71, 4.56, 4.41, 4.25, 4.08, 3.91},
{11, 9.65, 7.21, 6.22, 5.67, 5.32, 5.07, 4.89, 4.74, 4.63, 4.54, 4.40, 4.25, 4.10, 3.94, 3.78, 3.60},
{12, 9.33, 6.93, 5.95, 5.41, 5.06, 4.82, 4.64, 4.50, 4.39, 4.30, 4.16, 4.01, 3.86, 3.70, 3.54, 3.36},
{13, 9.07, 6.70, 5.74, 5.21, 4.86, 4.62, 4.44, 4.30, 4.19, 4.10, 3.96, 3.82, 3.66, 3.51, 3.34, 3.17},
{14, 8.86, 6.51, 5.56, 5.04, 4.69, 4.46, 4.28, 4.14, 4.03, 3.94, 3.80, 3.66, 3.51, 3.35, 3.18, 3.00},
{15, 8.68, 6.36, 5.42, 4.89, 4.56, 4.32, 4.14, 4.00, 3.89, 3.80, 3.67, 3.52, 3.37, 3.21, 3.05, 2.87},
{16, 8.53, 6.23, 5.29, 4.77, 4.44, 4.20, 4.03, 3.89, 3.78, 3.69, 3.55, 3.41, 3.26, 3.10, 2.93, 2.75},
{17, 8.40, 6.11, 5.19, 4.67, 4.34, 4.10, 3.93, 3.79, 3.68, 3.59, 3.46, 3.31, 3.16, 3.00, 2.83, 2.65},
{18, 8.29, 6.01, 5.09, 4.58, 4.25, 4.01, 3.84, 3.71, 3.60, 3.51, 3.37, 3.23, 3.08, 2.92, 2.75, 2.57},
{19, 8.18, 5.93, 5.01, 4.50, 4.17, 3.94, 3.77, 3.63, 3.52, 3.43, 3.30, 3.15, 3.00, 2.84, 2.67, 2.49},
{20, 8.10, 5.85, 4.94, 4.43, 4.10, 3.87, 3.70, 3.56, 3.46, 3.37, 3.23, 3.09, 2.94, 2.78, 2.61, 2.42},
{21, 8.02, 5.78, 4.87, 4.37, 4.04, 3.81, 3.64, 3.51, 3.40, 3.31, 3.17, 3.03, 2.88, 2.72, 2.55, 2.36},
{22, 7.95, 5.72, 4.82, 4.31, 3.99, 3.76, 3.59, 3.45, 3.35, 3.26, 3.12, 2.98, 2.83, 2.67, 2.50, 2.31},
{23, 7.88, 5.66, 4.76, 4.26, 3.94, 3.71, 3.54, 3.41, 3.30, 3.21, 3.07, 2.93, 2.78, 2.62, 2.45, 2.26},
{24, 7.82, 5.61, 4.72, 4.22, 3.90, 3.67, 3.50, 3.36, 3.26, 3.17, 3.03, 2.89, 2.74, 2.58, 2.40, 2.21},
{25, 7.77, 5.57, 4.68, 4.18, 3.85, 3.63, 3.46, 3.32, 3.22, 3.13, 2.99, 2.85, 2.70, 2.54, 2.36, 2.17},
{26, 7.72, 5.53, 4.64, 4.14, 3.82, 3.59, 3.42, 3.29, 3.18, 3.09, 2.96, 2.81, 2.66, 2.50, 2.33, 2.13},
{27, 7.68, 5.49, 4.60, 4.11, 3.78, 3.56, 3.39, 3.26, 3.15, 3.06, 2.93, 2.78, 2.63, 2.47, 2.29, 2.10},
{28, 7.64, 5.45, 4.57, 4.07, 3.75, 3.53, 3.36, 3.23, 3.12, 3.03, 2.90, 2.75, 2.60, 2.44, 2.26, 2.06},
{29, 7.60, 5.42, 4.54, 4.04, 3.73, 3.50, 3.33, 3.20, 3.09, 3.00, 2.87, 2.73, 2.57, 2.41, 2.23, 2.03},
{30, 7.56, 5.39, 4.51, 4.02, 3.70, 3.47, 3.30, 3.17, 3.07, 2.98, 2.84, 2.70, 2.55, 2.39, 2.21, 2.01},
{40, 7.31, 5.18, 4.31, 3.83, 3.51, 3.29, 3.12, 2.99, 2.89, 2.80, 2.66, 2.52, 2.37, 2.20, 2.02, 1.80},
{60, 7.08, 4.98, 4.13, 3.65, 3.34, 3.12, 2.95, 2.82, 2.72, 2.63, 2.50, 2.35, 2.20, 2.03, 1.84, 1.60},
{80, 6.96, 4.88, 4.04, 3.56, 3.26, 3.04, 2.87, 2.74, 2.64, 2.55, 2.42, 2.27, 2.12, 1.94, 1.75, 1.49},
{100, 6.90, 4.82, 3.98, 3.51, 3.21, 2.99, 2.82, 2.69, 2.59, 2.50, 2.37, 2.22, 2.07, 1.89, 1.69, 1.43},
{120, 6.85, 4.79, 3.95, 3.48, 3.17, 2.96, 2.79, 2.66, 2.56, 2.47, 2.34, 2.19, 2.03, 1.86, 1.66, 1.38},
{1000, 6.63, 4.61, 3.78, 3.32, 3.02, 2.80, 2.64, 2.51, 2.41, 2.32, 2.18, 2.04, 1.88, 1.70, 1.47, 1.00}};

/* fonction de recherche du seuil dans la table */

float f_test_threshold(int df1, int df2, float a) {
  int i,col,row;

  /* recherche la colonne */
  i=0;
  do {
    i++;
  } while ((i<17) && (f_table_1[0][i]<=df1));
  col=i-1;
  
  /* recherche ligne */
  i=0;
  do {
    i++;
  } while ((i<37) && (f_table_1[i][0]<=df2));
  row=i-1;
  /*
  printf("row=%d, col=%d alpha=%f, %f, %f, %f\n",row, col, a, f_table_1[row][col], f_table_5[row][col], f_table_10[row][col]);
  fflush(stdout);
  */

  /* renvoit la valeur */
  if (a>=1.0) {
    return 0.0;
  } if (a>=0.1) {
    return f_table_10[row][col];
  } else if (a>=0.05) {
    return f_table_5[row][col];
  } else if (a>=1.0) {
    return f_table_1[row][col];
  }
  return 0.f;
}

#endif //!FILE_TREE_TABLE_C

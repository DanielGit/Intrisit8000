
#include "config.h"
#ifdef __MINIOS__
#include <mplaylib.h>
#else
#include <stdio.h>
#include <stdlib.h>
#endif
#ifdef JZ4750_OPT
#include "jz4750_ipu_regops.h"
#else
#include "jz4740_ipu_regops.h"
#endif
#include "jz47_iputype.h"

extern struct JZ47_IPU_MOD jz47_ipu_module;
extern int jz47_cpu_type;

int sinxdivx_table_8[(2<<9)+1];
int sinxdivx_table_init = 0;

static struct Ratio_n2m ipu_ratio_table[647] = {
    {1.000000, 1, 1},{0.500000, 1, 2},{0.333333, 1, 3},{0.250000, 1, 4},
    {0.200000, 1, 5},{0.166667, 1, 6},{0.142857, 1, 7},{0.125000, 1, 8},
    {0.111111, 1, 9},{0.100000, 1, 10},{0.090909, 1, 11},{0.083333, 1, 12},
    {0.076923, 1, 13},{0.071429, 1, 14},{0.066667, 1, 15},{0.062500, 1, 16},
    {0.058824, 1, 17},{0.055556, 1, 18},{0.052632, 1, 19},{0.050000, 1, 20},
    {0.047619, 1, 21},{0.045455, 1, 22},{0.043478, 1, 23},{0.041667, 1, 24},
    {0.040000, 1, 25},{0.038462, 1, 26},{0.037037, 1, 27},{0.035714, 1, 28},
    {0.034483, 1, 29},{0.033333, 1, 30},{0.032258, 1, 31},{0.031250, 1, 32},
    {2.000000, 2, 1},{0.666667, 2, 3},{0.400000, 2, 5},{0.285714, 2, 7},
    {0.222222, 2, 9},{0.181818, 2, 11},{0.153846, 2, 13},{0.133333, 2, 15},
    {0.117647, 2, 17},{0.105263, 2, 19},{0.095238, 2, 21},{0.086957, 2, 23},
    {0.080000, 2, 25},{0.074074, 2, 27},{0.068966, 2, 29},{0.064516, 2, 31},
    {3.000000, 3, 1},{1.500000, 3, 2},{0.750000, 3, 4},{0.600000, 3, 5},
    {0.428571, 3, 7},{0.375000, 3, 8},{0.300000, 3, 10},{0.272727, 3, 11},
    {0.230769, 3, 13},{0.214286, 3, 14},{0.187500, 3, 16},{0.176471, 3, 17},
    {0.157895, 3, 19},{0.150000, 3, 20},{0.136364, 3, 22},{0.130435, 3, 23},
    {0.120000, 3, 25},{0.115385, 3, 26},{0.107143, 3, 28},{0.103448, 3, 29},
    {0.096774, 3, 31},{0.093750, 3, 32},{4.000000, 4, 1},{1.333333, 4, 3},
    {0.800000, 4, 5},{0.571429, 4, 7},{0.444444, 4, 9},{0.363636, 4, 11},
    {0.307692, 4, 13},{0.266667, 4, 15},{0.235294, 4, 17},{0.210526, 4, 19},
    {0.190476, 4, 21},{0.173913, 4, 23},{0.160000, 4, 25},{0.148148, 4, 27},
    {0.137931, 4, 29},{0.129032, 4, 31},{5.000000, 5, 1},{2.500000, 5, 2},
    {1.666667, 5, 3},{1.250000, 5, 4},{0.833333, 5, 6},{0.714286, 5, 7},
    {0.625000, 5, 8},{0.555556, 5, 9},{0.454545, 5, 11},{0.416667, 5, 12},
    {0.384615, 5, 13},{0.357143, 5, 14},{0.312500, 5, 16},{0.294118, 5, 17},
    {0.277778, 5, 18},{0.263158, 5, 19},{0.238095, 5, 21},{0.227273, 5, 22},
    {0.217391, 5, 23},{0.208333, 5, 24},{0.192308, 5, 26},{0.185185, 5, 27},
    {0.178571, 5, 28},{0.172414, 5, 29},{0.161290, 5, 31},{0.156250, 5, 32},
    {6.000000, 6, 1},{1.200000, 6, 5},{0.857143, 6, 7},{0.545455, 6, 11},
    {0.461538, 6, 13},{0.352941, 6, 17},{0.315789, 6, 19},{0.260870, 6, 23},
    {0.240000, 6, 25},{0.206897, 6, 29},{0.193548, 6, 31},{7.000000, 7, 1},
    {3.500000, 7, 2},{2.333333, 7, 3},{1.750000, 7, 4},{1.400000, 7, 5},
    {1.166667, 7, 6},{0.875000, 7, 8},{0.777778, 7, 9},{0.700000, 7, 10},
    {0.636364, 7, 11},{0.583333, 7, 12},{0.538462, 7, 13},{0.466667, 7, 15},
    {0.437500, 7, 16},{0.411765, 7, 17},{0.388889, 7, 18},{0.368421, 7, 19},
    {0.350000, 7, 20},{0.318182, 7, 22},{0.304348, 7, 23},{0.291667, 7, 24},
    {0.280000, 7, 25},{0.269231, 7, 26},{0.259259, 7, 27},{0.241379, 7, 29},
    {0.233333, 7, 30},{0.225806, 7, 31},{0.218750, 7, 32},{8.000000, 8, 1},
    {2.666667, 8, 3},{1.600000, 8, 5},{1.142857, 8, 7},{0.888889, 8, 9},
    {0.727273, 8, 11},{0.615385, 8, 13},{0.533333, 8, 15},{0.470588, 8, 17},
    {0.421053, 8, 19},{0.380952, 8, 21},{0.347826, 8, 23},{0.320000, 8, 25},
    {0.296296, 8, 27},{0.275862, 8, 29},{0.258065, 8, 31},{9.000000, 9, 1},
    {4.500000, 9, 2},{2.250000, 9, 4},{1.800000, 9, 5},{1.285714, 9, 7},
    {1.125000, 9, 8},{0.900000, 9, 10},{0.818182, 9, 11},{0.692308, 9, 13},
    {0.642857, 9, 14},{0.562500, 9, 16},{0.529412, 9, 17},{0.473684, 9, 19},
    {0.450000, 9, 20},{0.409091, 9, 22},{0.391304, 9, 23},{0.360000, 9, 25},
    {0.346154, 9, 26},{0.321429, 9, 28},{0.310345, 9, 29},{0.290323, 9, 31},
    {0.281250, 9, 32},{10.000000, 10, 1},{3.333333, 10, 3},{1.428571, 10, 7},
    {1.111111, 10, 9},{0.909091, 10, 11},{0.769231, 10, 13},{0.588235, 10, 17},
    {0.526316, 10, 19},{0.476190, 10, 21},{0.434783, 10, 23},{0.370370, 10, 27},
    {0.344828, 10, 29},{0.322581, 10, 31},{11.000000, 11, 1},{5.500000, 11, 2},
    {3.666667, 11, 3},{2.750000, 11, 4},{2.200000, 11, 5},{1.833333, 11, 6},
    {1.571429, 11, 7},{1.375000, 11, 8},{1.222222, 11, 9},{1.100000, 11, 10},
    {0.916667, 11, 12},{0.846154, 11, 13},{0.785714, 11, 14},{0.733333, 11, 15},
    {0.687500, 11, 16},{0.647059, 11, 17},{0.611111, 11, 18},{0.578947, 11, 19},
    {0.550000, 11, 20},{0.523810, 11, 21},{0.478261, 11, 23},{0.458333, 11, 24},
    {0.440000, 11, 25},{0.423077, 11, 26},{0.407407, 11, 27},{0.392857, 11, 28},
    {0.379310, 11, 29},{0.366667, 11, 30},{0.354839, 11, 31},{0.343750, 11, 32},
    {12.000000, 12, 1},{2.400000, 12, 5},{1.714286, 12, 7},{1.090909, 12, 11},
    {0.923077, 12, 13},{0.705882, 12, 17},{0.631579, 12, 19},{0.521739, 12, 23},
    {0.480000, 12, 25},{0.413793, 12, 29},{0.387097, 12, 31},{13.000000, 13, 1},
    {6.500000, 13, 2},{4.333333, 13, 3},{3.250000, 13, 4},{2.600000, 13, 5},
    {2.166667, 13, 6},{1.857143, 13, 7},{1.625000, 13, 8},{1.444444, 13, 9},
    {1.300000, 13, 10},{1.181818, 13, 11},{1.083333, 13, 12},{0.928571, 13, 14},
    {0.866667, 13, 15},{0.812500, 13, 16},{0.764706, 13, 17},{0.722222, 13, 18},
    {0.684211, 13, 19},{0.650000, 13, 20},{0.619048, 13, 21},{0.590909, 13, 22},
    {0.565217, 13, 23},{0.541667, 13, 24},{0.520000, 13, 25},{0.481481, 13, 27},
    {0.464286, 13, 28},{0.448276, 13, 29},{0.433333, 13, 30},{0.419355, 13, 31},
    {0.406250, 13, 32},{14.000000, 14, 1},{4.666667, 14, 3},{2.800000, 14, 5},
    {1.555556, 14, 9},{1.272727, 14, 11},{1.076923, 14, 13},{0.933333, 14, 15},
    {0.823529, 14, 17},{0.736842, 14, 19},{0.608696, 14, 23},{0.560000, 14, 25},
    {0.518519, 14, 27},{0.482759, 14, 29},{0.451613, 14, 31},{15.000000, 15, 1},
    {7.500000, 15, 2},{3.750000, 15, 4},{2.142857, 15, 7},{1.875000, 15, 8},
    {1.363636, 15, 11},{1.153846, 15, 13},{1.071429, 15, 14},{0.937500, 15, 16},
    {0.882353, 15, 17},{0.789474, 15, 19},{0.681818, 15, 22},{0.652174, 15, 23},
    {0.576923, 15, 26},{0.535714, 15, 28},{0.517241, 15, 29},{0.483871, 15, 31},
    {0.468750, 15, 32},{16.000000, 16, 1},{5.333333, 16, 3},{3.200000, 16, 5},
    {2.285714, 16, 7},{1.777778, 16, 9},{1.454545, 16, 11},{1.230769, 16, 13},
    {1.066667, 16, 15},{0.941176, 16, 17},{0.842105, 16, 19},{0.761905, 16, 21},
    {0.695652, 16, 23},{0.640000, 16, 25},{0.592593, 16, 27},{0.551724, 16, 29},
    {0.516129, 16, 31},{17.000000, 17, 1},{8.500000, 17, 2},{5.666667, 17, 3},
    {4.250000, 17, 4},{3.400000, 17, 5},{2.833333, 17, 6},{2.428571, 17, 7},
    {2.125000, 17, 8},{1.888889, 17, 9},{1.700000, 17, 10},{1.545455, 17, 11},
    {1.416667, 17, 12},{1.307692, 17, 13},{1.214286, 17, 14},{1.133333, 17, 15},
    {1.062500, 17, 16},{0.944444, 17, 18},{0.894737, 17, 19},{0.850000, 17, 20},
    {0.809524, 17, 21},{0.772727, 17, 22},{0.739130, 17, 23},{0.708333, 17, 24},
    {0.680000, 17, 25},{0.653846, 17, 26},{0.629630, 17, 27},{0.607143, 17, 28},
    {0.586207, 17, 29},{0.566667, 17, 30},{0.548387, 17, 31},{0.531250, 17, 32},
    {18.000000, 18, 1},{3.600000, 18, 5},{2.571429, 18, 7},{1.636364, 18, 11},
    {1.384615, 18, 13},{1.058824, 18, 17},{0.947368, 18, 19},{0.782609, 18, 23},
    {0.720000, 18, 25},{0.620690, 18, 29},{0.580645, 18, 31},{19.000000, 19, 1},
    {9.500000, 19, 2},{6.333333, 19, 3},{4.750000, 19, 4},{3.800000, 19, 5},
    {3.166667, 19, 6},{2.714286, 19, 7},{2.375000, 19, 8},{2.111111, 19, 9},
    {1.900000, 19, 10},{1.727273, 19, 11},{1.583333, 19, 12},{1.461538, 19, 13},
    {1.357143, 19, 14},{1.266667, 19, 15},{1.187500, 19, 16},{1.117647, 19, 17},
    {1.055556, 19, 18},{0.950000, 19, 20},{0.904762, 19, 21},{0.863636, 19, 22},
    {0.826087, 19, 23},{0.791667, 19, 24},{0.760000, 19, 25},{0.730769, 19, 26},
    {0.703704, 19, 27},{0.678571, 19, 28},{0.655172, 19, 29},{0.633333, 19, 30},
    {0.612903, 19, 31},{0.593750, 19, 32},{20.000000, 20, 1},{6.666667, 20, 3},
    {2.857143, 20, 7},{2.222222, 20, 9},{1.818182, 20, 11},{1.538462, 20, 13},
    {1.176471, 20, 17},{1.052632, 20, 19},{0.952381, 20, 21},{0.869565, 20, 23},
    {0.740741, 20, 27},{0.689655, 20, 29},{0.645161, 20, 31},{21.000000, 21, 1},
    {10.500000, 21, 2},{5.250000, 21, 4},{4.200000, 21, 5},{2.625000, 21, 8},
    {2.100000, 21, 10},{1.909091, 21, 11},{1.615385, 21, 13},{1.312500, 21, 16},
    {1.235294, 21, 17},{1.105263, 21, 19},{1.050000, 21, 20},{0.954545, 21, 22},
    {0.913043, 21, 23},{0.840000, 21, 25},{0.807692, 21, 26},{0.724138, 21, 29},
    {0.677419, 21, 31},{0.656250, 21, 32},{22.000000, 22, 1},{7.333333, 22, 3},
    {4.400000, 22, 5},{3.142857, 22, 7},{2.444444, 22, 9},{1.692308, 22, 13},
    {1.466667, 22, 15},{1.294118, 22, 17},{1.157895, 22, 19},{1.047619, 22, 21},
    {0.956522, 22, 23},{0.880000, 22, 25},{0.814815, 22, 27},{0.758621, 22, 29},
    {0.709677, 22, 31},{23.000000, 23, 1},{11.500000, 23, 2},{7.666667, 23, 3},
    {5.750000, 23, 4},{4.600000, 23, 5},{3.833333, 23, 6},{3.285714, 23, 7},
    {2.875000, 23, 8},{2.555556, 23, 9},{2.300000, 23, 10},{2.090909, 23, 11},
    {1.916667, 23, 12},{1.769231, 23, 13},{1.642857, 23, 14},{1.533333, 23, 15},
    {1.437500, 23, 16},{1.352941, 23, 17},{1.277778, 23, 18},{1.210526, 23, 19},
    {1.150000, 23, 20},{1.095238, 23, 21},{1.045455, 23, 22},{0.958333, 23, 24},
    {0.920000, 23, 25},{0.884615, 23, 26},{0.851852, 23, 27},{0.821429, 23, 28},
    {0.793103, 23, 29},{0.766667, 23, 30},{0.741935, 23, 31},{0.718750, 23, 32},
    {24.000000, 24, 1},{4.800000, 24, 5},{3.428571, 24, 7},{2.181818, 24, 11},
    {1.846154, 24, 13},{1.411765, 24, 17},{1.263158, 24, 19},{1.043478, 24, 23},
    {0.960000, 24, 25},{0.827586, 24, 29},{0.774194, 24, 31},{25.000000, 25, 1},
    {12.500000, 25, 2},{8.333333, 25, 3},{6.250000, 25, 4},{4.166667, 25, 6},
    {3.571429, 25, 7},{3.125000, 25, 8},{2.777778, 25, 9},{2.272727, 25, 11},
    {2.083333, 25, 12},{1.923077, 25, 13},{1.785714, 25, 14},{1.562500, 25, 16},
    {1.470588, 25, 17},{1.388889, 25, 18},{1.315789, 25, 19},{1.190476, 25, 21},
    {1.136364, 25, 22},{1.086957, 25, 23},{1.041667, 25, 24},{0.961538, 25, 26},
    {0.925926, 25, 27},{0.892857, 25, 28},{0.862069, 25, 29},{0.806452, 25, 31},
    {0.781250, 25, 32},{26.000000, 26, 1},{8.666667, 26, 3},{5.200000, 26, 5},
    {3.714286, 26, 7},{2.888889, 26, 9},{2.363636, 26, 11},{1.733333, 26, 15},
    {1.529412, 26, 17},{1.368421, 26, 19},{1.238095, 26, 21},{1.130435, 26, 23},
    {1.040000, 26, 25},{0.962963, 26, 27},{0.896552, 26, 29},{0.838710, 26, 31},
    {27.000000, 27, 1},{13.500000, 27, 2},{6.750000, 27, 4},{5.400000, 27, 5},
    {3.857143, 27, 7},{3.375000, 27, 8},{2.700000, 27, 10},{2.454545, 27, 11},
    {2.076923, 27, 13},{1.928571, 27, 14},{1.687500, 27, 16},{1.588235, 27, 17},
    {1.421053, 27, 19},{1.350000, 27, 20},{1.227273, 27, 22},{1.173913, 27, 23},
    {1.080000, 27, 25},{1.038462, 27, 26},{0.964286, 27, 28},{0.931035, 27, 29},
    {0.870968, 27, 31},{0.843750, 27, 32},{28.000000, 28, 1},{9.333333, 28, 3},
    {5.600000, 28, 5},{3.111111, 28, 9},{2.545455, 28, 11},{2.153846, 28, 13},
    {1.866667, 28, 15},{1.647059, 28, 17},{1.473684, 28, 19},{1.217391, 28, 23},
    {1.120000, 28, 25},{1.037037, 28, 27},{0.965517, 28, 29},{0.903226, 28, 31},
    {29.000000, 29, 1},{14.500000, 29, 2},{9.666667, 29, 3},{7.250000, 29, 4},
    {5.800000, 29, 5},{4.833333, 29, 6},{4.142857, 29, 7},{3.625000, 29, 8},
    {3.222222, 29, 9},{2.900000, 29, 10},{2.636364, 29, 11},{2.416667, 29, 12},
    {2.230769, 29, 13},{2.071429, 29, 14},{1.933333, 29, 15},{1.812500, 29, 16},
    {1.705882, 29, 17},{1.611111, 29, 18},{1.526316, 29, 19},{1.450000, 29, 20},
    {1.380952, 29, 21},{1.318182, 29, 22},{1.260870, 29, 23},{1.208333, 29, 24},
    {1.160000, 29, 25},{1.115385, 29, 26},{1.074074, 29, 27},{1.035714, 29, 28},
    {0.966667, 29, 30},{0.935484, 29, 31},{0.906250, 29, 32},{30.000000, 30, 1},
    {4.285714, 30, 7},{2.727273, 30, 11},{2.307692, 30, 13},{1.764706, 30, 17},
    {1.578947, 30, 19},{1.304348, 30, 23},{1.034483, 30, 29},{0.967742, 30, 31},
    {31.000000, 31, 1},{15.500000, 31, 2},{10.333333, 31, 3},{7.750000, 31, 4},
    {6.200000, 31, 5},{5.166667, 31, 6},{4.428571, 31, 7},{3.875000, 31, 8},
    {3.444444, 31, 9},{3.100000, 31, 10},{2.818182, 31, 11},{2.583333, 31, 12},
    {2.384615, 31, 13},{2.214286, 31, 14},{2.066667, 31, 15},{1.937500, 31, 16},
    {1.823529, 31, 17},{1.722222, 31, 18},{1.631579, 31, 19},{1.550000, 31, 20},
    {1.476190, 31, 21},{1.409091, 31, 22},{1.347826, 31, 23},{1.291667, 31, 24},
    {1.240000, 31, 25},{1.192308, 31, 26},{1.148148, 31, 27},{1.107143, 31, 28},
    {1.068966, 31, 29},{1.033333, 31, 30},{0.968750, 31, 32},{32.000000, 32, 1},
    {10.666667, 32, 3},{6.400000, 32, 5},{4.571429, 32, 7},{3.555556, 32, 9},
    {2.909091, 32, 11},{2.461539, 32, 13},{2.133333, 32, 15},{1.882353, 32, 17},
    {1.684211, 32, 19},{1.523810, 32, 21},{1.391304, 32, 23},{1.280000, 32, 25},
    {1.185185, 32, 27},{1.103448, 32, 29},{1.032258, 32, 31},
};

#if 0
int init_ipu_ratio_table ()
{
  int i, j, cnt;
  float r, min, diff;

  // orig table, first calculate
  for (i = 1; i <= (IPU_LUT_LEN); i++)
    for (j = 1; j <= (IPU_LUT_LEN); j++)
    {
      ipu_ratio_table [(i - 1) * (IPU_LUT_LEN) + j - 1].ratio = i / (float)j;
      ipu_ratio_table [(i - 1) * (IPU_LUT_LEN) + j - 1].n = i;
      ipu_ratio_table [(i - 1) * (IPU_LUT_LEN) + j - 1].m = j;
    }

#if 0
// Eliminate the ratio greater than 1:2
  for (i = 0; i < (IPU_LUT_LEN) * (IPU_LUT_LEN); i++)
    if (ipu_ratio_table[i].ratio < 0.4999)
      ipu_ratio_table[i].n = ipu_ratio_table[i].m = -1;
#endif

// eliminate the same ratio
  for (i = 0; i < (IPU_LUT_LEN) * (IPU_LUT_LEN); i++)
    for (j = i + 1; j < (IPU_LUT_LEN) * (IPU_LUT_LEN); j++)
    {
      diff = ipu_ratio_table[i].ratio - ipu_ratio_table[j].ratio;
      if (diff > -0.001 && diff < 0.001)
      {
        ipu_ratio_table[j].n = -1;
        ipu_ratio_table[j].m = -1;
      }
    }

// reorder ipu_ratio_table
  cnt = 0;
  for (i = 0; i < (IPU_LUT_LEN) * (IPU_LUT_LEN); i++)
    if (ipu_ratio_table[i].n != -1)
    {
      if (cnt != i)
        ipu_ratio_table[cnt] = ipu_ratio_table[i];
      cnt++;
    }
  ipu_rtable_len = cnt;

#if 0
  printf ("static struct Ratio2m ipu_ratio_table[%d] = {\n    ", ipu_rtable_len);
  for (i = 0; i < ipu_rtable_len; i++)
  {
    printf("{%f, %d, %d},", ipu_ratio_table[i].ratio,ipu_ratio_table[i].n,ipu_ratio_table[i].m);
    if ((i + 1) % 4 == 0)
      printf ("\n    ");
  }
  printf ("};\n");
#endif

  return 0;
}
#endif


int resize_out_cal (int insize, int outsize, int srcN, int dstM, int upScale)
{
  int tmp, calsize;
  float tmp2;

  /* Calculate the output size followed by the SPEC introduce.  */
  tmp = (float)(insize - 1) * dstM / (float)srcN;
  tmp2 = tmp * srcN / (float)dstM;
  calsize = (int)tmp + (upScale ? 1 : 0);

  if (tmp2 != insize - 1)
    calsize++;

  return calsize;
}

int find_ipu_ratio_factor (float ratio, int lut_len)
{
  int i, sel;
  float diff, min;

  sel = 0;
  min = 100;
  diff = min;

  /* must <= 32 or <= 20.  */
  if (ratio > lut_len)
    ratio = lut_len;

  for (i = 0; i < (sizeof(ipu_ratio_table) / sizeof(ipu_ratio_table[0])); i++)
  {
    if (ipu_ratio_table[i].m <= lut_len && ipu_ratio_table[i].n <= lut_len) 
    {
      diff = (ipu_ratio_table[i].ratio - ratio);
      if (diff < 0)
        diff = -diff;
    }

    if (diff < min)
    {
      min = diff;
      sel = i;
    }
  }
  printf ("resize: sel = %d, srcN = %d, dstM = %d\n", sel, ipu_ratio_table[sel].n, ipu_ratio_table[sel].m); 
  return sel;
}

/* Following codes is written depend on SPECS, please read the SPEC document for details.  */
int resize_lut_calc (int srcN, int dstM, int upScale, unsigned int coef[], float fixed_point_coef)
{
  int i, j, t, x, in, out, out0, off;
  float w_coef, factor, factor2;
  int coef_raw[32], coef_prev;

  if (upScale)
  {
    for (i = 0, t = 0; i < dstM; i++)
    {
      factor = (float) (i * srcN) / (float) dstM;
      factor2 = factor - (int)factor;
      w_coef = 1.0 - factor2;
      coef_raw[i] = ((unsigned int)(fixed_point_coef * w_coef));

      // calculate in & out
      in = 0;
      if (t <= factor)
      {
        in = 1;
        t++;
      }

      if (jz47_cpu_type == 4760)
      {
        if (i != 0)
          coef[i - 1] = (coef_raw[i - 1] << 6) | (in << 1);
      }
      else
        coef[i] = (coef_raw[i] << 2) | 0x1 | (in << 1);
    }
    if (jz47_cpu_type == 4760)
      coef[i - 1] = (coef_raw[i - 1] << 6) | (1 << 1);
    j = i;
  }
  else
  {
    off = 0;
    coef_prev = 0;
    for (i = 0, j = 0, t = 0, x = 0; i < srcN; i++)
    {
      factor = (float)(t * srcN + 1) / (float)dstM;

      if (((int)factor - i) >= 1)
        coef[i] = 0;
      else if (factor - i == 0)
      {
        coef[i] = (unsigned int)(fixed_point_coef * 1.0);
        t++;
      }
      else
      {
        factor2 = (float)(t * srcN) / (float)dstM;
        factor = factor - (int)factor2;
        w_coef = 1.0 - factor;
        coef[i] = (unsigned int) (fixed_point_coef * w_coef);
        t++;
      }
      // calculate in & out
      out = 1;
      if (((x * srcN + 1) / dstM - i) >= 1)
        out = 0;
      else
        x++;

      // Followed SPEC, m=1 is the spcial case.
      if (dstM == 1)
      {
        coef[i] = (i == 0) ? ((int)fixed_point_coef / 2) : 0x0;
        out = (i == 0) ? 1 : 0;
      }

      // setting coef
      if (jz47_cpu_type == 4760)
      {
        if (out && i != 0)
          coef[j++] = (coef_prev << 6) | (off << 1);

        if (out)
          coef_prev = coef[i];

        off = (out) ? 1 : off + 1;
      }
      else
        coef[i] = (coef[i] << 2) | 0x2 | out;
      if (i == 0)
        out0 = out;
    }
    if (jz47_cpu_type == 4760 && out0)
      coef[j++] = (coef_prev << 6) | (off << 1);
  }
  return j;
}

static void caculate_bicube_coef_table(unsigned int bicube_coef[], unsigned int coef[], int idx);

int jz47_calc_resize_para ()
{
  int srcN, dstM, resize_w, resize_h, srcW, srcH;
  int width_resize_enable, height_resize_enable;
  int Wsel, Hsel, lut_len;
  float fixpoint_lut_coef;

  /* Get shape parameters.  */
  resize_w = jz47_ipu_module.act_w;
  resize_h = jz47_ipu_module.act_h;
  srcW = jz47_ipu_module.srcW;
  srcH = jz47_ipu_module.srcH;

  /* Use different coefs for different CPU.  */
  if (jz47_cpu_type == 4740)
  {
    lut_len = 20;
    fixpoint_lut_coef = 128.0;
  }
  else
  {
    lut_len = 32;
    fixpoint_lut_coef = 512.0;
  }

  // enable resize
  width_resize_enable  = (srcW != resize_w) ? 1 : 0;
  height_resize_enable = (srcH != resize_h) ? 1 : 0;

  // set some parameters when needn't do resize since jz4740 
  jz47_ipu_module.resize_para.outW = srcW;
  jz47_ipu_module.resize_para.outH = srcH;
  jz47_ipu_module.resize_para.width_up = 1;
  jz47_ipu_module.resize_para.height_up = 1;
  jz47_ipu_module.resize_para.width_lut_size = 1;
  jz47_ipu_module.resize_para.height_lut_size = 1;

  jz47_ipu_module.resize_para.width_resize_enable  =  width_resize_enable;
  jz47_ipu_module.resize_para.height_resize_enable = height_resize_enable;

  // calc width resize lut
  if (width_resize_enable)
  {
    int idx;
    int width_up = (resize_w >= srcW) ? 1 : 0;
    
    Wsel = find_ipu_ratio_factor (((float)(srcW - 1)) / (resize_w - 1 - width_up), lut_len);
    srcN = ipu_ratio_table[Wsel].n;
    dstM = ipu_ratio_table[Wsel].m;
    jz47_ipu_module.resize_para.outW = resize_out_cal (srcW, resize_w, srcN, dstM, width_up);
    idx = resize_lut_calc (srcN, dstM, width_up, jz47_ipu_module.resize_para.width_lut_coef, fixpoint_lut_coef);

    /* set the parameters.  */
    jz47_ipu_module.resize_para.Wsel = Wsel;
    jz47_ipu_module.resize_para.width_up = width_up;
    jz47_ipu_module.resize_para.width_lut_size = width_up ? dstM : srcN;
    if (jz47_cpu_type == 4760)
    {
      jz47_ipu_module.resize_para.width_lut_size = idx;
      if (jz47_ipu_module.rsize_algorithm)     /* 0: liner, 1: bicube, 2: biliner.  */
        caculate_bicube_coef_table(jz47_ipu_module.resize_para.width_bicube_lut_coef, 
                                   jz47_ipu_module.resize_para.width_lut_coef, idx);
    }
  }

  // calc width resize lut
  if (height_resize_enable)
  {
    int idx;
    int height_up = (resize_h >= srcH) ? 1 : 0;
    
    Hsel = find_ipu_ratio_factor (((float)(srcH - 1)) / (resize_h - 1 - height_up), lut_len);
    srcN = ipu_ratio_table[Hsel].n;
    dstM = ipu_ratio_table[Hsel].m;
    jz47_ipu_module.resize_para.outH = resize_out_cal (srcH, resize_h, srcN, dstM, height_up);
    idx = resize_lut_calc (srcN, dstM, height_up, jz47_ipu_module.resize_para.height_lut_coef, fixpoint_lut_coef);

    /* set the parameters.  */
    jz47_ipu_module.resize_para.Hsel = Hsel;
    jz47_ipu_module.resize_para.height_up = height_up;
    jz47_ipu_module.resize_para.height_lut_size = height_up ? dstM : srcN;
    if (jz47_cpu_type == 4760)
    {
      jz47_ipu_module.resize_para.height_lut_size = idx;
      if (jz47_ipu_module.rsize_algorithm)     /* 0: liner, 1: bicube, 2: biliner.  */
        caculate_bicube_coef_table(jz47_ipu_module.resize_para.height_bicube_lut_coef, 
                                   jz47_ipu_module.resize_para.height_lut_coef, idx);
    }
  }
  return 0;
}

// following function is used for bicube algorithm
//----------------------------------------------------------

static double sinxdivx(double x, float cube_level)
{
  //calculate sin(x*PI)/(x*PI)
  const float a = cube_level;//a can be selected : -2, -1, -0.75, -0.5
  //double B = 0.0;
  //double C=  0.6;
  if ( x < 0) x = -x;
  double x2 = x*x;
  double x3= x*x*x;
  if (x<=1)
    //return ((12-9*B-6*C)*x*x*x + (-18+12*B+6*C)*x*x + 6-2*B);
    return (a+2)*x3 - (a+3)*x2+1;
  else if ( x<=2 )
    //return ((-B-6*C)*x*x*x + (6*B+30*C)*x*x + (-12*B-48*C)*x +8*B+24*C);
    return a*x3 - (5*a)*x2 + (8*a)*x - 4*a;
  else
    return 0;
}

static void init_sinxdivx_table()
{
  int i = 0 ;
  for (i = 0 ; i < (2<<9); i++)
    sinxdivx_table_8[i] = (int)(0.5 + 512 * sinxdivx(i * (1.0/512), jz47_ipu_module.rsize_bicube_level));
}

static void caculate_bicube_coef_table(unsigned int bicube_coef[], unsigned int coef[], int idx)
{
  int i;
  int u_8, off;

#ifdef JZ4750_OPT
  /* initial the sinxdivx table.  */
  if (!sinxdivx_table_init)
  {
    init_sinxdivx_table();
    sinxdivx_table_init = 1;
  }

  if (jz47_ipu_module.rsize_algorithm == 1)      /* 1: bicube, 2: biliner.  */
  {
    for (i = 0 ; i < idx; i++)
    {
      int au_8[4];

      off = coef[i] & 0x3f;
      u_8 = 512 - (coef[i] >> 6);
      bicube_coef[i*2 + 0] = ((sinxdivx_table_8[(1<<9) + u_8] & W_CUBE_COEF0_MSK) << W_CUBE_COEF0_SFT)
                             | ((sinxdivx_table_8[u_8] & W_CUBE_COEF1_MSK) << W_CUBE_COEF1_SFT);
      bicube_coef[i*2 + 1] = ((sinxdivx_table_8[(1<<9)-u_8] & W_CUBE_COEF0_MSK) << W_CUBE_COEF0_SFT)
                             | ((sinxdivx_table_8[(2<<9)-u_8] & W_CUBE_COEF1_MSK) << W_CUBE_COEF1_SFT)
                             | off;
    }
  }
  else     /* 2 biliner */
  {
    for ( i = 0 ; i <idx; i++ )
    {
      int av_8[4];
      int sum = 0;
      double ratio = 0 ;

      off = coef[i] & 0x3f;
      u_8 = 512 - (coef[i] >> 6);
      av_8[0] = (1<<9)+u_8;
      av_8[1] = u_8;
      av_8[2] = (1<<9)-u_8;
      av_8[3] = (2<<9)-u_8;

      av_8[0] = (1<<10) - av_8[0];
      av_8[1] = (1<<10) - av_8[1];
      av_8[2] = (1<<10) - av_8[2];
      av_8[3] = (1<<10) - av_8[3];

      sum = av_8[0] + av_8[1] + av_8[2] + av_8[3];
      ratio = (1<<9)/(double)sum ;

      bicube_coef[i*2 + 0] = ((((int)(av_8[0] * ratio))  & W_CUBE_COEF0_MSK) << W_CUBE_COEF0_SFT)
                             |((((int)(av_8[1] * ratio)) & W_CUBE_COEF1_MSK) << W_CUBE_COEF1_SFT);
      bicube_coef[i*2 + 1] = ((((int)(av_8[2] * ratio))  & W_CUBE_COEF0_MSK) << W_CUBE_COEF0_SFT)
                             |((((int)(av_8[3] * ratio)) & W_CUBE_COEF1_MSK) << W_CUBE_COEF1_SFT)
                             | off;
    }
  }
#endif
}



  



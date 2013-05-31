/*
 * Copyright (c) 2012 David Brandt
 *  All rights reserved.
 *
 * Licensed under: CC BY-NC-SA 3.0 (full license in file LICENSE)
 * 
 * You are free:
 * - to Share - to copy, distribute and transmit the work
 * - to Remix - to adapt the work
 * 
 * Under the following conditions:
 * - Attribution - You must attribute the work in the manner specified by
 *   the author or licensor (but not in any way that suggests that
 *   they endorse you or your use of the work).
 * 
 * - Noncommercial - You may not use this work for commercial purposes.
 * 
 * - Share Alike - If you alter, transform, or build upon this work, you may
 *   distribute the resulting work only under the same or similar license
 *   to this one.
 */

#include <avr/io.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "config.h"
#include "adc.h"
#include "mcan.h"
#include "pec.h"
#include "slave.h"
#include "spi.h"
#include "spi_adc.h"
#include "uart.h"
#include "timer.h"
#include "charger.h"
#include "relai.h"
#include "intermediate.h"
#include "balancing.h"volatile unsigned char slave_configs[NUMSLAVES][6];
volatile unsigned char slave_read_configs[NUMSLAVES][6];
volatile unsigned char slave_diags[NUMSLAVES][2];

volatile unsigned int cell_voltages[NUMCELLS];

volatile unsigned int cell_temps[NUMTEMPS];
volatile unsigned int cell_temps_raw[NUMTEMPS];

volatile unsigned int slave_fsm[NUMSLAVES];
volatile unsigned int slave_fsm_raw[NUMSLAVES];

volatile uint32_t batt_fsm_volt_copy;
volatile uint32_t batt_fsm_volt;

volatile unsigned int ltc_temps[NUMSLAVES];
volatile unsigned char slave_sent_cmd;volatile unsigned char i, j, ii, jj;
volatile unsigned int slave_errors;

volatile unsigned int slave_cycle_ms = 0;
volatile unsigned int slave_adc_time = 0;

volatile unsigned char verify_errors = 0;

/*
This lookup table can be created with this code snippet:
#include <stdio.h>
#include <math.h>
 * 
int main()
{
  float rn = 10000.0f;
  float b = 4100.0f;
  float tn = 298.15f;
  float r1 = 10000.0f;
  int i;

  printf("const PROGMEM unsigned int temp_curve[] = {\n");
  for(i=0; i<255 ;i++)
  {
    float temp = ((float)i/2) + 233.15f;
    float e = (b / temp) - (b / tn);
    float r2 = rn * exp(e);
    float u2 = (3.065f * r2) / (r1 + r2);
    float adc = ((u2*1000) / 1.5) + 512;
    printf("  %d%s // Temp: %fK Rntc: %fOhm Untc: %fmV\n", (int)adc, (i!=254?",":""), temp, r2, u2);
  }
  printf("};\n");
  return 0;
}
*/

const unsigned int temp_curve[] = {
  2512, // Temp: 233.149994K Rntc: 462368.656250Ohm Untc: 3.000114mV
  2510, // Temp: 233.649994K Rntc: 445292.968750Ohm Untc: 2.997681mV
  2508, // Temp: 234.149994K Rntc: 428915.812500Ohm Untc: 2.995169mV
  2507, // Temp: 234.649994K Rntc: 413207.187500Ohm Untc: 2.992577mV
  2505, // Temp: 235.149994K Rntc: 398137.656250Ohm Untc: 2.989903mV
  2503, // Temp: 235.649994K Rntc: 383677.687500Ohm Untc: 2.987144mV
  2501, // Temp: 236.149994K Rntc: 369800.750000Ohm Untc: 2.984300mV
  2499, // Temp: 236.649994K Rntc: 356481.437500Ohm Untc: 2.981367mV
  2497, // Temp: 237.149994K Rntc: 343694.968750Ohm Untc: 2.978343mV
  2495, // Temp: 237.649994K Rntc: 331417.687500Ohm Untc: 2.975227mV
  2493, // Temp: 238.149994K Rntc: 319628.375000Ohm Untc: 2.972017mV
  2491, // Temp: 238.649994K Rntc: 308304.843750Ohm Untc: 2.968709mV
  2488, // Temp: 239.149994K Rntc: 297427.312500Ohm Untc: 2.965302mV
  2486, // Temp: 239.649994K Rntc: 286976.812500Ohm Untc: 2.961793mV
  2484, // Temp: 240.149994K Rntc: 276934.687500Ohm Untc: 2.958181mV
  2481, // Temp: 240.649994K Rntc: 267283.187500Ohm Untc: 2.954463mV
  2479, // Temp: 241.149994K Rntc: 258006.468750Ohm Untc: 2.950637mV
  2476, // Temp: 241.649994K Rntc: 249087.828125Ohm Untc: 2.946700mV
  2473, // Temp: 242.149994K Rntc: 240512.328125Ohm Untc: 2.942651mV
  2470, // Temp: 242.649994K Rntc: 232265.734375Ohm Untc: 2.938486mV
  2468, // Temp: 243.149994K Rntc: 224334.421875Ohm Untc: 2.934204mV
  2465, // Temp: 243.649994K Rntc: 216704.515625Ohm Untc: 2.929802mV
  2462, // Temp: 244.149994K Rntc: 209363.671875Ohm Untc: 2.925278mV
  2459, // Temp: 244.649994K Rntc: 202300.046875Ohm Untc: 2.920629mV
  2455, // Temp: 245.149994K Rntc: 195502.328125Ohm Untc: 2.915853mV
  2452, // Temp: 245.649994K Rntc: 188959.328125Ohm Untc: 2.910949mV
  2449, // Temp: 246.149994K Rntc: 182660.406250Ohm Untc: 2.905912mV
  2445, // Temp: 246.649994K Rntc: 176595.687500Ohm Untc: 2.900741mV
  2442, // Temp: 247.149994K Rntc: 170755.796875Ohm Untc: 2.895434mV
  2438, // Temp: 247.649994K Rntc: 165131.375000Ohm Untc: 2.889989mV
  2434, // Temp: 248.149994K Rntc: 159713.546875Ohm Untc: 2.884402mV
  2431, // Temp: 248.649994K Rntc: 154494.390625Ohm Untc: 2.878671mV
  2427, // Temp: 249.149994K Rntc: 149465.734375Ohm Untc: 2.872796mV
  2423, // Temp: 249.649994K Rntc: 144620.062500Ohm Untc: 2.866772mV
  2419, // Temp: 250.149994K Rntc: 139949.921875Ohm Untc: 2.860599mV
  2414, // Temp: 250.649994K Rntc: 135448.140625Ohm Untc: 2.854272mV
  2410, // Temp: 251.149994K Rntc: 131108.187500Ohm Untc: 2.847791mV
  2406, // Temp: 251.649994K Rntc: 126923.976562Ohm Untc: 2.841153mV
  2401, // Temp: 252.149994K Rntc: 122888.781250Ohm Untc: 2.834356mV
  2396, // Temp: 252.649994K Rntc: 118997.304688Ohm Untc: 2.827398mV
  2392, // Temp: 253.149994K Rntc: 115243.789062Ohm Untc: 2.820277mV
  2387, // Temp: 253.649994K Rntc: 111622.500000Ohm Untc: 2.812991mV
  2382, // Temp: 254.149994K Rntc: 108128.828125Ohm Untc: 2.805538mV
  2377, // Temp: 254.649994K Rntc: 104757.484375Ohm Untc: 2.797915mV
  2372, // Temp: 255.149994K Rntc: 101503.843750Ohm Untc: 2.790122mV
  2366, // Temp: 255.649994K Rntc: 98363.445312Ohm Untc: 2.782156mV
  2361, // Temp: 256.149994K Rntc: 95331.851562Ohm Untc: 2.774015mV
  2355, // Temp: 256.649994K Rntc: 92404.968750Ohm Untc: 2.765698mV
  2350, // Temp: 257.149994K Rntc: 89578.875000Ohm Untc: 2.757204mV
  2344, // Temp: 257.649994K Rntc: 86849.656250Ohm Untc: 2.748530mV
  2338, // Temp: 258.149994K Rntc: 84213.625000Ohm Untc: 2.739676mV
  2332, // Temp: 258.649994K Rntc: 81667.335938Ohm Untc: 2.730639mV
  2326, // Temp: 259.149994K Rntc: 79207.484375Ohm Untc: 2.721419mV
  2320, // Temp: 259.649994K Rntc: 76830.726562Ohm Untc: 2.712014mV
  2313, // Temp: 260.149994K Rntc: 74534.039062Ohm Untc: 2.702424mV
  2307, // Temp: 260.649994K Rntc: 72314.414062Ohm Untc: 2.692647mV
  2300, // Temp: 261.149994K Rntc: 70169.054688Ohm Untc: 2.682683mV
  2293, // Temp: 261.649994K Rntc: 68095.132812Ohm Untc: 2.672530mV
  2286, // Temp: 262.149994K Rntc: 66090.070312Ohm Untc: 2.662188mV
  2279, // Temp: 262.649994K Rntc: 64151.332031Ohm Untc: 2.651656mV
  2272, // Temp: 263.149994K Rntc: 62276.589844Ohm Untc: 2.640934mV
  2265, // Temp: 263.649994K Rntc: 60463.378906Ohm Untc: 2.630023mV
  2257, // Temp: 264.149994K Rntc: 58709.511719Ohm Untc: 2.618919mV
  2250, // Temp: 264.649994K Rntc: 57012.882812Ohm Untc: 2.607625mV
  2242, // Temp: 265.149994K Rntc: 55371.406250Ohm Untc: 2.596141mV
  2234, // Temp: 265.649994K Rntc: 53783.089844Ohm Untc: 2.584465mV
  2227, // Temp: 266.149994K Rntc: 52246.062500Ohm Untc: 2.572599mV
  2219, // Temp: 266.649994K Rntc: 50758.480469Ohm Untc: 2.560544mV
  2210, // Temp: 267.149994K Rntc: 49318.566406Ohm Untc: 2.548298mV
  2202, // Temp: 267.649994K Rntc: 47924.664062Ohm Untc: 2.535864mV
  2194, // Temp: 268.149994K Rntc: 46575.179688Ohm Untc: 2.523243mV
  2185, // Temp: 268.649994K Rntc: 45268.441406Ohm Untc: 2.510434mV
  2176, // Temp: 269.149994K Rntc: 44003.066406Ohm Untc: 2.497440mV
  2168, // Temp: 269.649994K Rntc: 42777.546875Ohm Untc: 2.484261mV
  2159, // Temp: 270.149994K Rntc: 41590.484375Ohm Untc: 2.470898mV
  2150, // Temp: 270.649994K Rntc: 40440.566406Ohm Untc: 2.457354mV
  2141, // Temp: 271.149994K Rntc: 39326.527344Ohm Untc: 2.443630mV
  2131, // Temp: 271.649994K Rntc: 38247.117188Ohm Untc: 2.429729mV
  2122, // Temp: 272.149994K Rntc: 37201.132812Ohm Untc: 2.415651mV
  2112, // Temp: 272.649994K Rntc: 36187.441406Ohm Untc: 2.401400mV
  2103, // Temp: 273.149994K Rntc: 35204.902344Ohm Untc: 2.386976mV
  2093, // Temp: 273.649994K Rntc: 34252.500000Ohm Untc: 2.372384mV
  2083, // Temp: 274.149994K Rntc: 33329.203125Ohm Untc: 2.357625mV
  2073, // Temp: 274.649994K Rntc: 32434.039062Ohm Untc: 2.342702mV
  2063, // Temp: 275.149994K Rntc: 31566.017578Ohm Untc: 2.327619mV
  2053, // Temp: 275.649994K Rntc: 30724.246094Ohm Untc: 2.312377mV
  2043, // Temp: 276.149994K Rntc: 29907.859375Ohm Untc: 2.296981mV
  2032, // Temp: 276.649994K Rntc: 29115.998047Ohm Untc: 2.281433mV
  2022, // Temp: 277.149994K Rntc: 28347.832031Ohm Untc: 2.265737mV
  2011, // Temp: 277.649994K Rntc: 27602.619141Ohm Untc: 2.249897mV
  2001, // Temp: 278.149994K Rntc: 26879.558594Ohm Untc: 2.233917mV
  1990, // Temp: 278.649994K Rntc: 26177.910156Ohm Untc: 2.217798mV
  1979, // Temp: 279.149994K Rntc: 25497.033203Ohm Untc: 2.201548mV
  1968, // Temp: 279.649994K Rntc: 24836.185547Ohm Untc: 2.185168mV
  1957, // Temp: 280.149994K Rntc: 24194.728516Ohm Untc: 2.168663mV
  1946, // Temp: 280.649994K Rntc: 23572.041016Ohm Untc: 2.152038mV
  1935, // Temp: 281.149994K Rntc: 22967.503906Ohm Untc: 2.135297mV
  1924, // Temp: 281.649994K Rntc: 22380.542969Ohm Untc: 2.118444mV
  1912, // Temp: 282.149994K Rntc: 21810.578125Ohm Untc: 2.101484mV
  1901, // Temp: 282.649994K Rntc: 21257.074219Ohm Untc: 2.084422mV
  1890, // Temp: 283.149994K Rntc: 20719.494141Ohm Untc: 2.067262mV
  1878, // Temp: 283.649994K Rntc: 20197.337891Ohm Untc: 2.050010mV
  1867, // Temp: 284.149994K Rntc: 19690.107422Ohm Untc: 2.032670mV
  1855, // Temp: 284.649994K Rntc: 19197.335938Ohm Untc: 2.015247mV
  1843, // Temp: 285.149994K Rntc: 18718.556641Ohm Untc: 1.997746mV
  1832, // Temp: 285.649994K Rntc: 18253.318359Ohm Untc: 1.980172mV
  1820, // Temp: 286.149994K Rntc: 17801.222656Ohm Untc: 1.962531mV
  1808, // Temp: 286.649994K Rntc: 17361.849609Ohm Untc: 1.944827mV
  1796, // Temp: 287.149994K Rntc: 16934.771484Ohm Untc: 1.927066mV
  1784, // Temp: 287.649994K Rntc: 16519.650391Ohm Untc: 1.909253mV
  1772, // Temp: 288.149994K Rntc: 16116.087891Ohm Untc: 1.891394mV
  1760, // Temp: 288.649994K Rntc: 15723.733398Ohm Untc: 1.873493mV
  1749, // Temp: 289.149994K Rntc: 15342.233398Ohm Untc: 1.855556mV
  1737, // Temp: 289.649994K Rntc: 14971.259766Ohm Untc: 1.837589mV
  1725, // Temp: 290.149994K Rntc: 14610.497070Ohm Untc: 1.819597mV
  1713, // Temp: 290.649994K Rntc: 14259.610352Ohm Untc: 1.801583mV
  1701, // Temp: 291.149994K Rntc: 13918.318359Ohm Untc: 1.783556mV
  1689, // Temp: 291.649994K Rntc: 13586.322266Ohm Untc: 1.765518mV
  1676, // Temp: 292.149994K Rntc: 13263.345703Ohm Untc: 1.747477mV
  1664, // Temp: 292.649994K Rntc: 12949.109375Ohm Untc: 1.729436mV
  1652, // Temp: 293.149994K Rntc: 12643.354492Ohm Untc: 1.711402mV
  1640, // Temp: 293.649994K Rntc: 12345.820312Ohm Untc: 1.693379mV
  1628, // Temp: 294.149994K Rntc: 12056.264648Ohm Untc: 1.675372mV
  1616, // Temp: 294.649994K Rntc: 11774.455078Ohm Untc: 1.657387mV
  1604, // Temp: 295.149994K Rntc: 11500.142578Ohm Untc: 1.639428mV
  1593, // Temp: 295.649994K Rntc: 11233.120117Ohm Untc: 1.621500mV
  1581, // Temp: 296.149994K Rntc: 10973.166992Ohm Untc: 1.603609mV
  1569, // Temp: 296.649994K Rntc: 10720.078125Ohm Untc: 1.585758mV
  1557, // Temp: 297.149994K Rntc: 10473.655273Ohm Untc: 1.567954mV
  1545, // Temp: 297.649994K Rntc: 10233.687500Ohm Untc: 1.550200mV
  1533, // Temp: 298.149994K Rntc: 10000.000000Ohm Untc: 1.532500mV
  1521, // Temp: 298.649994K Rntc: 9772.403320Ohm Untc: 1.514860mV
  1510, // Temp: 299.149994K Rntc: 9550.724609Ohm Untc: 1.497283mV
  1498, // Temp: 299.649994K Rntc: 9334.777344Ohm Untc: 1.479774mV
  1486, // Temp: 300.149994K Rntc: 9124.417969Ohm Untc: 1.462337mV
  1475, // Temp: 300.649994K Rntc: 8919.479492Ohm Untc: 1.444977mV
  1463, // Temp: 301.149994K Rntc: 8719.792969Ohm Untc: 1.427696mV
  1452, // Temp: 301.649994K Rntc: 8525.218750Ohm Untc: 1.410499mV
  1440, // Temp: 302.149994K Rntc: 8335.614258Ohm Untc: 1.393390mV
  1429, // Temp: 302.649994K Rntc: 8150.825684Ohm Untc: 1.376372mV
  1418, // Temp: 303.149994K Rntc: 7970.726074Ohm Untc: 1.359448mV
  1407, // Temp: 303.649994K Rntc: 7795.178223Ohm Untc: 1.342623mV
  1395, // Temp: 304.149994K Rntc: 7624.056641Ohm Untc: 1.325900mV
  1384, // Temp: 304.649994K Rntc: 7457.232422Ohm Untc: 1.309281mV
  1373, // Temp: 305.149994K Rntc: 7294.586426Ohm Untc: 1.292769mV
  1362, // Temp: 305.649994K Rntc: 7136.005859Ohm Untc: 1.276368mV
  1352, // Temp: 306.149994K Rntc: 6981.371582Ohm Untc: 1.260081mV
  1341, // Temp: 306.649994K Rntc: 6830.576660Ohm Untc: 1.243910mV
  1330, // Temp: 307.149994K Rntc: 6683.517090Ohm Untc: 1.227857mV
  1319, // Temp: 307.649994K Rntc: 6540.084961Ohm Untc: 1.211926mV
  1309, // Temp: 308.149994K Rntc: 6400.182617Ohm Untc: 1.196118mV
  1298, // Temp: 308.649994K Rntc: 6263.709473Ohm Untc: 1.180436mV
  1288, // Temp: 309.149994K Rntc: 6130.572754Ohm Untc: 1.164881mV
  1278, // Temp: 309.649994K Rntc: 6000.683594Ohm Untc: 1.149457mV
  1268, // Temp: 310.149994K Rntc: 5873.950195Ohm Untc: 1.134164mV
  1258, // Temp: 310.649994K Rntc: 5750.293457Ohm Untc: 1.119005mV
  1247, // Temp: 311.149994K Rntc: 5629.620605Ohm Untc: 1.103980mV
  1238, // Temp: 311.649994K Rntc: 5511.854004Ohm Untc: 1.089092mV
  1228, // Temp: 312.149994K Rntc: 5396.921387Ohm Untc: 1.074342mV
  1218, // Temp: 312.649994K Rntc: 5284.737793Ohm Untc: 1.059732mV
  1208, // Temp: 313.149994K Rntc: 5175.236816Ohm Untc: 1.045262mV
  1199, // Temp: 313.649994K Rntc: 5068.338379Ohm Untc: 1.030934mV
  1189, // Temp: 314.149994K Rntc: 4963.979004Ohm Untc: 1.016748mV
  1180, // Temp: 314.649994K Rntc: 4862.093262Ohm Untc: 1.002706mV
  1171, // Temp: 315.149994K Rntc: 4762.607422Ohm Untc: 0.988809mV
  1162, // Temp: 315.649994K Rntc: 4665.468750Ohm Untc: 0.975057mV
  1152, // Temp: 316.149994K Rntc: 4570.603027Ohm Untc: 0.961449mV
  1143, // Temp: 316.649994K Rntc: 4477.961426Ohm Untc: 0.947989mV
  1135, // Temp: 317.149994K Rntc: 4387.477539Ohm Untc: 0.934675mV
  1126, // Temp: 317.649994K Rntc: 4299.101074Ohm Untc: 0.921509mV
  1117, // Temp: 318.149994K Rntc: 4212.769531Ohm Untc: 0.908489mV
  1109, // Temp: 318.649994K Rntc: 4128.439941Ohm Untc: 0.895617mV
  1100, // Temp: 319.149994K Rntc: 4046.052734Ohm Untc: 0.882892mV
  1092, // Temp: 319.649994K Rntc: 3965.559326Ohm Untc: 0.870315mV
  1083, // Temp: 320.149994K Rntc: 3886.911865Ohm Untc: 0.857886mV
  1075, // Temp: 320.649994K Rntc: 3810.060303Ohm Untc: 0.845603mV
  1067, // Temp: 321.149994K Rntc: 3734.963379Ohm Untc: 0.833469mV
  1059, // Temp: 321.649994K Rntc: 3661.570068Ohm Untc: 0.821480mV
  1051, // Temp: 322.149994K Rntc: 3589.841553Ohm Untc: 0.809639mV
  1043, // Temp: 322.649994K Rntc: 3519.733154Ohm Untc: 0.797943mV
  1036, // Temp: 323.149994K Rntc: 3451.207520Ohm Untc: 0.786394mV
  1028, // Temp: 323.649994K Rntc: 3384.219482Ohm Untc: 0.774990mV
  1021, // Temp: 324.149994K Rntc: 3318.731201Ohm Untc: 0.763730mV
  1013, // Temp: 324.649994K Rntc: 3254.708740Ohm Untc: 0.752614mV
  1006, // Temp: 325.149994K Rntc: 3192.110107Ohm Untc: 0.741642mV
  999, // Temp: 325.649994K Rntc: 3130.903564Ohm Untc: 0.730812mV
  992, // Temp: 326.149994K Rntc: 3071.052002Ohm Untc: 0.720124mV
  985, // Temp: 326.649994K Rntc: 3012.525879Ohm Untc: 0.709577mV
  978, // Temp: 327.149994K Rntc: 2955.283936Ohm Untc: 0.699170mV
  971, // Temp: 327.649994K Rntc: 2899.304199Ohm Untc: 0.688903mV
  964, // Temp: 328.149994K Rntc: 2844.547363Ohm Untc: 0.678774mV
  957, // Temp: 328.649994K Rntc: 2790.987061Ohm Untc: 0.668782mV
  951, // Temp: 329.149994K Rntc: 2738.594482Ohm Untc: 0.658926mV
  944, // Temp: 329.649994K Rntc: 2687.339355Ohm Untc: 0.649206mV
  938, // Temp: 330.149994K Rntc: 2637.194336Ohm Untc: 0.639620mV
  932, // Temp: 330.649994K Rntc: 2588.130615Ohm Untc: 0.630167mV
  925, // Temp: 331.149994K Rntc: 2540.127441Ohm Untc: 0.620846mV
  919, // Temp: 331.649994K Rntc: 2493.152588Ohm Untc: 0.611656mV
  913, // Temp: 332.149994K Rntc: 2447.184082Ohm Untc: 0.602596mV
  907, // Temp: 332.649994K Rntc: 2402.198242Ohm Untc: 0.593664mV
  901, // Temp: 333.149994K Rntc: 2358.172119Ohm Untc: 0.584860mV
  896, // Temp: 333.649994K Rntc: 2315.078613Ohm Untc: 0.576181mV
  890, // Temp: 334.149994K Rntc: 2272.898438Ohm Untc: 0.567627mV
  884, // Temp: 334.649994K Rntc: 2231.610107Ohm Untc: 0.559197mV
  879, // Temp: 335.149994K Rntc: 2191.193115Ohm Untc: 0.550890mV
  873, // Temp: 335.649994K Rntc: 2151.623047Ohm Untc: 0.542703mV
  868, // Temp: 336.149994K Rntc: 2112.882324Ohm Untc: 0.534636mV
  863, // Temp: 336.649994K Rntc: 2074.951904Ohm Untc: 0.526688mV
  857, // Temp: 337.149994K Rntc: 2037.813232Ohm Untc: 0.518856mV
  852, // Temp: 337.649994K Rntc: 2001.444336Ohm Untc: 0.511141mV
  847, // Temp: 338.149994K Rntc: 1965.829468Ohm Untc: 0.503540mV
  842, // Temp: 338.649994K Rntc: 1930.951416Ohm Untc: 0.496051mV
  837, // Temp: 339.149994K Rntc: 1896.791626Ohm Untc: 0.488675mV
  832, // Temp: 339.649994K Rntc: 1863.333984Ohm Untc: 0.481409mV
  828, // Temp: 340.149994K Rntc: 1830.562500Ohm Untc: 0.474253mV
  823, // Temp: 340.649994K Rntc: 1798.461670Ohm Untc: 0.467204mV
  818, // Temp: 341.149994K Rntc: 1767.014771Ohm Untc: 0.460261mV
  814, // Temp: 341.649994K Rntc: 1736.207153Ohm Untc: 0.453424mV
  809, // Temp: 342.149994K Rntc: 1706.024536Ohm Untc: 0.446690mV
  805, // Temp: 342.649994K Rntc: 1676.452881Ohm Untc: 0.440059mV
  801, // Temp: 343.149994K Rntc: 1647.477173Ohm Untc: 0.433529mV
  796, // Temp: 343.649994K Rntc: 1619.085571Ohm Untc: 0.427099mV
  792, // Temp: 344.149994K Rntc: 1591.262207Ohm Untc: 0.420767mV
  788, // Temp: 344.649994K Rntc: 1563.996094Ohm Untc: 0.414532mV
  784, // Temp: 345.149994K Rntc: 1537.274780Ohm Untc: 0.408393mV
  780, // Temp: 345.649994K Rntc: 1511.083618Ohm Untc: 0.402349mV
  776, // Temp: 346.149994K Rntc: 1485.413574Ohm Untc: 0.396398mV
  772, // Temp: 346.649994K Rntc: 1460.252197Ohm Untc: 0.390539mV
  768, // Temp: 347.149994K Rntc: 1435.588135Ohm Untc: 0.384771mV
  764, // Temp: 347.649994K Rntc: 1411.407959Ohm Untc: 0.379091mV
  761, // Temp: 348.149994K Rntc: 1387.703857Ohm Untc: 0.373500mV
  757, // Temp: 348.649994K Rntc: 1364.464355Ohm Untc: 0.367997mV
  753, // Temp: 349.149994K Rntc: 1341.679077Ohm Untc: 0.362578mV
  750, // Temp: 349.649994K Rntc: 1319.337402Ohm Untc: 0.357244mV
  746, // Temp: 350.149994K Rntc: 1297.429565Ohm Untc: 0.351993mV
  743, // Temp: 350.649994K Rntc: 1275.946289Ohm Untc: 0.346825mV
  739, // Temp: 351.149994K Rntc: 1254.879761Ohm Untc: 0.341737mV
  736, // Temp: 351.649994K Rntc: 1234.218750Ohm Untc: 0.336728mV
  733, // Temp: 352.149994K Rntc: 1213.954712Ohm Untc: 0.331798mV
  729, // Temp: 352.649994K Rntc: 1194.080322Ohm Untc: 0.326946mV
  726, // Temp: 353.149994K Rntc: 1174.584961Ohm Untc: 0.322169mV
  723, // Temp: 353.649994K Rntc: 1155.463013Ohm Untc: 0.317467mV
  720, // Temp: 354.149994K Rntc: 1136.704468Ohm Untc: 0.312839mV
  717, // Temp: 354.649994K Rntc: 1118.301636Ohm Untc: 0.308284mV
  714, // Temp: 355.149994K Rntc: 1100.248169Ohm Untc: 0.303800mV
  711, // Temp: 355.649994K Rntc: 1082.534546Ohm Untc: 0.299387mV
  708, // Temp: 356.149994K Rntc: 1065.156006Ohm Untc: 0.295044mV
  705, // Temp: 356.649994K Rntc: 1048.103394Ohm Untc: 0.290768mV
  703, // Temp: 357.149994K Rntc: 1031.369995Ohm Untc: 0.286560mV
  700, // Temp: 357.649994K Rntc: 1014.949280Ohm Untc: 0.282418mV
  697, // Temp: 358.149994K Rntc: 998.835754Ohm Untc: 0.278341mV
  694, // Temp: 358.649994K Rntc: 983.021118Ohm Untc: 0.274329mV
  692, // Temp: 359.149994K Rntc: 967.500244Ohm Untc: 0.270380mV
  689, // Temp: 359.649994K Rntc: 952.266235Ohm Untc: 0.266492mV
  687 // Temp: 360.149994K Rntc: 937.314087Ohm Untc: 0.262667mV
};

unsigned char slave_tempfromadc(unsigned int adc)
{
	unsigned char i;
	
	for(i=0; i<0xFF ;i++)
	{
		if( adc > temp_curve[i] )
		{
			return i;
		}
	}
	return 0xFF;
}

#define spi_append(x) spi_buffer[spi_buffer_size++] = x; update_pec(x)
#define spi_pec() spi_buffer[spi_buffer_size++] = pecvoid slave_init(void){
	slave_errors = 0;
	SET_OUTPUT(SLAVECS);
	SET(SLAVECS);}void slave_build_command(char cmd)
{
	_delay_us(5);
	spi_buffer_size = 2;
	slave_sent_cmd = spi_buffer[0] = cmd;	reset_pec();	update_pec(cmd);	spi_buffer[1] = pec;
}

void slave_set_gpios(char gpio)
{
	for(ii=0; ii<NUMSLAVES ;ii++)
	{
		slave_set_gpio(ii, gpio);
	}
}

void slave_set_cdcs(char cdc)
{
	for(ii=0; ii<NUMSLAVES ;ii++)
	{
		slave_set_cdc(ii, cdc);
	}
}

void slave_set_wdts(char wdt)
{
	for(ii=0; ii<NUMSLAVES ;ii++)
	{
		slave_set_wdt(ii, wdt);
	}
}

void slave_set_lvlpls(char lvlpl)
{
	for(ii=0; ii<NUMSLAVES ;ii++)
	{
		slave_set_lvlpl(ii, lvlpl);
	}
}

void slave_set_cell10s(char cell10)
{
	for(ii=0; ii<NUMSLAVES ;ii++)
	{
		slave_set_cell10(ii, cell10);
	}
}

void slave_set_vuvs(char vuv)
{
	for(ii=0; ii<NUMSLAVES ;ii++)
	{
		slave_set_vuv(ii, vuv);
	}
}

void slave_set_vovs(char vov)
{
	for(ii=0; ii<NUMSLAVES ;ii++)
	{
		slave_set_vov(ii, vov);
	}
}

void slave_set_balancings(unsigned int balancing)
{
	for(ii=0; ii<NUMSLAVES ;ii++)
	{
		slave_set_balancing(ii, balancing);
	}
}

void slave_set_maskints(unsigned int maskint)
{
	for(ii=0; ii<NUMSLAVES ;ii++)
	{
		slave_set_maskint(ii, maskint);
	}
}

void slave_set_balance_cell(unsigned char cell, unsigned char balance)
{
	unsigned char slave;
	slave = cell / 12;
	cell %= 12;
	
	if( cell < 8 )
	{
		if( balance )
		{
			slave_configs[slave][1] |= (1 << cell);
		}
		else
		{
			slave_configs[slave][1] &= ~(1 << cell);
		}
	}
	else
	{
		cell -= 8;
		if( balance )
		{
			slave_configs[slave][2] |= (1 << cell);
		}
		else
		{
			slave_configs[slave][2] &= ~(1 << cell);
		}
	}
}

void slave_write_config(void)
{
	static unsigned char i;
	slave_build_command(SLAVE_CMD_WRITE_CONFIG);
	for(i=NUMSLAVES; i>0 ;i--)
	{
		reset_pec();
		spi_append(slave_configs[i-1][0]);
		spi_append(slave_configs[i-1][1]);
		spi_append(slave_configs[i-1][2]);
		spi_append(slave_configs[i-1][3]);
		spi_append(slave_configs[i-1][4]);
		spi_append(slave_configs[i-1][5]);
		spi_pec();
	}
	RESET(SLAVECS);	spi_start_transfer();
}

void slave_start_diag(void)
{
	slave_build_command(SLAVE_CMD_DIAG);
	RESET(SLAVECS);	spi_start_transfer();
	slave_adc_time = SLAVE_WAIT_DIAG;
}

void slave_read_diag(void)
{
	slave_build_command(SLAVE_CMD_READ_DIAG);	spi_buffer_size += 3 * NUMSLAVES; // 2 data + 1 pec	RESET(SLAVECS);	spi_start_transfer();
}

void slave_start_cell_conv(char what)
{
	if( what == 0 )
		what = SLAVE_CMD_CONV_CELLS;
	slave_build_command(what);
	RESET(SLAVECS);	spi_start_transfer();
	slave_adc_time = SLAVE_WAIT_CELL_MEASURE;
}

void slave_start_temp_conv(void)
{
	slave_build_command(SLAVE_CMD_CONV_TEMP);
	RESET(SLAVECS);	spi_start_transfer();
	slave_adc_time = SLAVE_WAIT_TEMP_MEASURE;
}

void slave_read_temp(void)
{
	slave_build_command(SLAVE_CMD_READ_TEMP);	spi_buffer_size += 6 * NUMSLAVES; // 5 data + 1 pec	RESET(SLAVECS);	spi_start_transfer();
}

void slave_read_flags(void)
{
	slave_build_command(SLAVE_CMD_READ_FLAGS);	spi_buffer_size += 4 * NUMSLAVES; // 3 data + 1 pec	RESET(SLAVECS);	spi_start_transfer();
}

void slave_read_cellvoltages(void)
{
	slave_build_command(SLAVE_CMD_READ_VOLTAGES);	spi_buffer_size += 19 * NUMSLAVES; // 18 data + 1 pec	RESET(SLAVECS);	spi_start_transfer();
}

void slave_read_config(void)
{
	slave_build_command(SLAVE_CMD_READ_CONFIG);	spi_buffer_size += 7 * NUMSLAVES; // 6 data + 1 pec	RESET(SLAVECS);	spi_start_transfer();
}

static inline unsigned char slave_spi_loop(void)
{
	static unsigned char stmp = 0;
	static char etmp[9];
	static unsigned char verified_ok;
	
	if( spi_idle() )
	{
		if( slave_state == 0 )
		{
			for(i=0; i<NUMSLAVES ;i++)
			{
				slave_configs[i][0] = SLAVE_DEFAULT_CONFIG | (stmp << 5);
				slave_configs[i][1] = 0;
				slave_configs[i][2] = 0;
				slave_configs[i][3] = 0;
				slave_configs[i][4] = 0;
				slave_configs[i][5] = 0;
			}
			
			if( (++stmp) == 4 )
			{
				stmp = 0;
			}
			
			if( battery_state & STATE_BALANCING )
				balancing_set();
			
			slave_errors = 0;
			slave_write_config();
			slave_state = 10;
			return 1;
		}
		else if( slave_state == 10 )
		{
			slave_errors = 0;
			slave_read_config();
			slave_state = 15;
		}
		else if( slave_state == 20 )
		{
			slave_errors = 0;
			slave_start_cell_conv(0);
			slave_state = 25;
			return 1;
		}
		else if( slave_state == 30 )
		{
			slave_errors = 0;
			slave_start_temp_conv();
			slave_state = 35;
			return 1;
		}
	}
	else if( spi_done() )
	{
		if(! slave_adc_active() )
		{
			if( slave_parse_command() )
			{
				if( slave_state == 15 )
				{
					//TOGGLE(LED_R);
					verified_ok = 1;
					
					for(i=0; i<NUMSLAVES ;i++)
					{
						// check if WDTB is high and CDC is set to 1
						if( (slave_read_configs[i][0] & 0x81) != 0x81 )
						{
							verified_ok = 0;
						}
					}
					
					if( verified_ok )
					{
						slave_state = 20;
					}
					else
					{
						// debug
						sprintf(etmp, "%02x %02x %02d", slave_configs[i][0], slave_read_configs[i][0], i);
						status(etmp);
						
						// end debug
						if( (verify_errors++) > SLAVE_MAX_ERRORS )
						{
							sprintf(etmp, "SL VC %02d", i);
							error(etmp);
							return 1;
						}
						else
						{
							slave_state = 0;
						}
					}
				}
				else if( slave_state == 25 )
				{
					slave_state = 30;
				}
				else if( slave_state == 35 )
				{
					slave_state = 100;
				}
				return 1;
			}
		}
	}
	return 0;
}

// this variable is used often
volatile unsigned char current_id = 0;

// time (in ms) spent in over/undervoltage
volatile unsigned int slave_uv_time;
volatile unsigned int slave_ov_time;
// helper for uv/ov detection
volatile unsigned char slave_uv_detected = 0;
volatile unsigned char slave_ov_detected = 0;
// min/max/avg variables
volatile unsigned int slave_max_cell_volt = 0;
volatile unsigned char slave_max_cell = 0;
volatile unsigned int slave_min_cell_volt = 0xFFFF;
volatile unsigned char slave_min_cell = 0;
volatile unsigned int slave_avg_cell_volt = 0;
// temporary variables
static uint32_t slave_avg_tmp = 0;
volatile unsigned int slave_max_cell_volt_tmp = 0;
volatile unsigned char slave_max_cell_tmp = 0;
volatile unsigned int slave_min_cell_volt_tmp = 0xFFFF;
volatile unsigned char slave_min_cell_tmp = 0;

static inline void slave_check_cell_voltages_init(void)
{
	// reset all variables for the next step
	current_id = 0;
	slave_uv_detected = 0;
	slave_ov_detected = 0;
	slave_avg_tmp = 0;
	slave_min_cell_volt_tmp = 0xFFFF;
	slave_min_cell_tmp = 0;
	slave_max_cell_volt_tmp = 0;
	slave_max_cell_tmp = 0;
}

static inline void slave_check_cell_voltages(void)
{
	slave_avg_tmp += cell_voltages[current_id];
	
	if( cell_voltages[current_id] < slave_min_cell_volt_tmp )
	{
		slave_min_cell_volt_tmp = cell_voltages[current_id];
		slave_min_cell_tmp = current_id;
	}
	
	if( cell_voltages[current_id] > slave_max_cell_volt_tmp )
	{
		slave_max_cell_volt_tmp = cell_voltages[current_id];
		slave_max_cell_tmp = current_id;
	}
	
	// maximum voltage cutoff
	if( cell_voltages[current_id] > SLAVE_VOLTAGE_MAX )
	{
		slave_ov_detected = 1;
		if( slave_ov_time == 0 )
			slave_ov_time = 1;
		else if( slave_ov_time > SLAVE_OV_TIME )
		{
			sprintf(etmp, "OV  %04d", current_id + 1);
			error(etmp);
		}
	}
	
	if( cell_voltages[current_id] < SLAVE_VOLTAGE_WARN )
	{
		if( (battery_state & STATE_WARNING) == 0 )
		{
			sprintf(etmp, "WRNUV%03d", current_id);
			status(etmp);
		}
		warning_time(SLAVE_VOLTAGE_WARN_TIME);
	}
	
	// minimum voltage cutoff
	if( cell_voltages[current_id] < SLAVE_VOLTAGE_MIN && (battery_state & STATE_TRACTIVE) )
	{
		slave_uv_detected = 1;
		if( slave_uv_time == 0 )
		{
			slave_uv_time = 1;
			current_id++;
			mcan_send_message(CAN_VOLTAGE_WARN, 1, (char *)&current_id);
			current_id--;
		}
		else if( slave_uv_time > SLAVE_UV_TIME )
		{
			sprintf(etmp, "UV  %04d", current_id + 1);
			error(etmp);
		}
	}
	
	current_id++;
	if( current_id == NUMCELLS )
	{
		slave_state = 210;
		
		slave_max_cell = slave_max_cell_tmp;
		slave_max_cell_volt = slave_max_cell_volt_tmp;
		slave_min_cell = slave_min_cell_tmp;
		slave_min_cell_volt = slave_min_cell_volt_tmp;
		slave_avg_tmp /= NUMCELLS;
		slave_avg_cell_volt = slave_avg_tmp;
		
		if( slave_ov_detected == 0 )
		{
			slave_ov_time = 0;
		}
		
		if( slave_uv_detected == 0 )
		{
			slave_uv_time = 0;
		}
	}
}


// helper
volatile unsigned char slave_ot_detected = 0;
volatile unsigned int slave_ot_time;
// temp min/max/avg
volatile unsigned int slave_max_temp_val = 0;
volatile unsigned char slave_max_temp = 0;
volatile unsigned int slave_min_temp_val = 0xFF;
volatile unsigned char slave_min_temp = 0;
volatile unsigned int slave_avg_temp_val = 0;
// temporary variables
volatile unsigned int slave_max_temp_val_tmp = 0;
volatile unsigned char slave_max_temp_tmp = 0;
volatile unsigned int slave_min_temp_val_tmp = 0xFF;
volatile unsigned char slave_min_temp_tmp = 0;

static inline void slave_check_temps_init(void)
{
	// reset all variables for the next step
	current_id = 0;
	slave_ot_detected = 0;
	slave_avg_tmp = 0;
	slave_min_temp_val_tmp = 0xFF;
	slave_min_temp_tmp = 0;
	slave_max_temp_val_tmp = 0;
	slave_max_temp_tmp = 0;
}

static inline void slave_check_temps(void)
{
	slave_avg_tmp += cell_temps[current_id];
	
	if( cell_temps[current_id] < slave_min_temp_val_tmp )
	{
		slave_min_temp_val_tmp = cell_temps[current_id];
		slave_min_temp_tmp = current_id;
	}
	
	if( cell_temps[current_id] > slave_max_temp_val_tmp )
	{
		slave_max_temp_val_tmp = cell_temps[current_id];
		slave_max_temp_tmp = current_id;
	}
	
	if( cell_temps[current_id] > SLAVE_TEMPERATURE_MAX )
	{
		slave_ot_detected = 1;
		if( slave_ot_time == 0 )
			slave_ot_time = 1;
		else if( slave_ot_time > SLAVE_OT_TIME )
		{
			sprintf(etmp, "OT  %04d", current_id + 1);
			error(etmp);
		}
	}
	
	if( cell_temps[current_id] > SLAVE_TEMPERATURE_WARN )
	{
		if( (battery_state & STATE_WARNING) == 0 )
		{
			sprintf(etmp, "WRNOT%03d", current_id);
			status(etmp);
		}
		warning_time(SLAVE_TEMPERATURE_WARN_TIME);
	}
	
	current_id++;
	if( current_id == NUMTEMPS )
	{
		slave_state = 250;
		
		slave_max_temp = slave_max_temp_tmp;
		slave_max_temp_val = slave_max_temp_val_tmp;
		slave_min_temp = slave_min_temp_tmp;
		slave_min_temp_val = slave_min_temp_val_tmp;
		slave_avg_tmp /= NUMTEMPS;
		slave_avg_temp_val = slave_avg_tmp;
		
		if( slave_ot_detected == 0 )
		{
			slave_ot_time = 0;
		}
	}
}

unsigned char slave_loop(void)
{
	static unsigned char current_id = 0;
	
	if( slave_state < 100 )
	{
		return slave_spi_loop();
	}
	else if( slave_state == 100 )
	{
		if( slave_get_gpio(i) == 2 )
		{
			// calculate temperatues for each cell
			cell_temps[current_id] = slave_tempfromadc(cell_temps_raw[current_id]);
			current_id++;
		}
		else
		{
			current_id = 0;
			slave_state = 150;
		}
		
		if( current_id == NUMTEMPS )
		{
			current_id = 0;
			slave_state = 150;
		}
	}
	else if( slave_state == 150 )
	{
		static uint32_t temp32_tmp;
		// calculate full stack measurements
		// k = R2 / (R1+R2)
		// U2 = (A - 512) * 1.5
		// U2 = (U * R2) / (R1 + R2)
		// U2 * 352500 = U * 22500
		// (A - 512) * 1.5 * 352500 / 22500 = U
		// U = (A - 512) * 528750 / 22500
		// U = (A - 512) * 23,5
		// U = (A - 512) * 235 / 10
		temp32_tmp = slave_fsm_raw[current_id];
		temp32_tmp -= 512;
		temp32_tmp *= 258;
		temp32_tmp /= 10;
		slave_fsm[current_id] = temp32_tmp;
		current_id++;
		
		if( current_id == NUMSLAVES )
		{
			slave_state = 160;
			batt_fsm_volt = 0;
			current_id = 0;
		}
	}
	else if( slave_state == 160 )
	{
		batt_fsm_volt += slave_fsm[current_id];
		current_id++;
		if( current_id == NUMSLAVES )
		{
			// all measured data is now in a usable format
			slave_state = 170;
			current_id = 0;
		}
	}
	else if( slave_state == 170 )
	{
		batt_fsm_volt_copy = batt_fsm_volt;
		slave_state = 200;
		
		slave_check_cell_voltages_init();
	}
	else if( slave_state == 200 )
	{
		slave_check_cell_voltages();
	}
	else if( slave_state == 210 )
	{
		slave_check_temps_init();
		slave_state = 220;
	}
	else if( slave_state == 220 )
	{
		slave_check_temps();
	}
	else if( slave_state == 250 )
	{
#ifdef CAN_MEASURE_CYCLE_MS
		mcan_send_message(CAN_MEASURE_CYCLE_MS, 2, (char*)&slave_cycle_ms);
#endif
		if( slave_cycle_ms > SLAVE_STATUS_CYCLE_LENGTH )
		{
			sprintf(etmp, "CYC %04d", slave_cycle_ms);
			status(etmp);
		}
		slave_cycle_ms = 0;
		slave_state = 0;
		verify_errors = 0;
		can_send_data = 1;
		//TOGGLE(LED_B);
	}
	return 0;
}

// return value
// 0 -> nothing happened, continue calling slave_parse_command until 1 returned
// 1 -> this step is done, do next stepunsigned char slave_parse_command(void){
	static unsigned char tmp, i;
	static char etmp[9];
	
	SET(SLAVECS);
	_delay_us(500);
	spi_status = SPI_IDLE;
		switch(slave_sent_cmd)	{
		case SLAVE_CMD_WRITE_CONFIG:
		{
			return 1;
		}
		break;		case SLAVE_CMD_READ_CONFIG:		{
			j = 2;
			
			for(i=0; i<NUMSLAVES ;i++)
			{				reset_pec();				update_pec(spi_buffer[j++]);				update_pec(spi_buffer[j++]);				update_pec(spi_buffer[j++]);				update_pec(spi_buffer[j++]);				update_pec(spi_buffer[j++]);				update_pec(spi_buffer[j++]);				// check for errors				if( spi_buffer[j++] != pec )				{
					// fatal error! pec failed
					if( (++slave_errors) > SLAVE_MAX_ERRORS )
					{
						sprintf(etmp, "SE R %03d", i);
						error(etmp);
						return 1;
					}
					else
					{
						slave_read_config();
						return 0;
					}				}				else				{
					j -= 7;					slave_read_configs[i][0] = spi_buffer[j++];
					slave_read_configs[i][1] = spi_buffer[j++];
					slave_read_configs[i][2] = spi_buffer[j++];
					slave_read_configs[i][3] = spi_buffer[j++];
					slave_read_configs[i][4] = spi_buffer[j++];
					slave_read_configs[i][5] = spi_buffer[j++];
					j++;				}
			}
			slave_errors = 0;
			return 1;		}		break;
		case SLAVE_CMD_DIAG:
		{
			slave_read_diag();
			return 0;
		}
		break;
		case SLAVE_CMD_READ_DIAG:
		{
			j = 2;
			for(i=0; i<NUMSLAVES ;i++)
			{				reset_pec();				update_pec(spi_buffer[j++]);				update_pec(spi_buffer[j++]);				// check for errors				if( spi_buffer[j++] != pec )				{					// fatal error! pec failed
					if( (++slave_errors) > SLAVE_MAX_ERRORS )
					{
						sprintf(etmp, "SE D %03d", i);
						error(etmp);
						return 1;
					}
					else
					{
						slave_read_diag();
						return 0;
					}				}				else				{
					j -= 3;
					slave_diags[i][0] = spi_buffer[j++];
					slave_diags[i][1] = spi_buffer[j++];
					j++;				}
			}
			slave_errors = 0;
			return 1;
		}
		break;
/*		case SLAVE_CMD_READ_FLAGS:
		{
			j = 2;
			for(i=0; i<NUMSLAVES ;i++)
			{				reset_pec();				update_pec(spi_buffer[j++]);				update_pec(spi_buffer[j++]);
				update_pec(spi_buffer[j++]);				// check for errors				if( spi_buffer[j++] != pec )				{					// fatal error! pec failed
					if( (++slave_errors) > SLAVE_MAX_ERRORS )
					{
					 * sprintf(etmp, "SE F %03d", i);
						error(etmp);
						return 1;
					}
					else
					{
						slave_read_flags();
						return 0;
					}				}				else				{
					j -= 4;
					slave_ouv[i] = spi_buffer[j++];
					slave_ouv[i] |= (uint32_t)spi_buffer[j++] << 8;
					slave_ouv[i] |= (uint32_t)spi_buffer[j++] << 16;
					j++;				}
			}
			slave_errors = 0;
			return 1;
		}
		break;*/
		case SLAVE_CMD_CONV_CELLS:
		case SLAVE_CMD_OWCONV_CELLS:
		{
			slave_read_cellvoltages();
			return 0;
		}
		break;
		case SLAVE_CMD_READ_VOLTAGES:
		{
			j = 2;
			tmp = 0;
			for(i=0; i<NUMSLAVES ;i++)
			{				reset_pec();				update_pec(spi_buffer[j++]);				update_pec(spi_buffer[j++]);
				update_pec(spi_buffer[j++]);
				update_pec(spi_buffer[j++]);				update_pec(spi_buffer[j++]);
				update_pec(spi_buffer[j++]);
				update_pec(spi_buffer[j++]);				update_pec(spi_buffer[j++]);
				update_pec(spi_buffer[j++]);
				update_pec(spi_buffer[j++]);				update_pec(spi_buffer[j++]);
				update_pec(spi_buffer[j++]);
				update_pec(spi_buffer[j++]);				update_pec(spi_buffer[j++]);
				update_pec(spi_buffer[j++]);
				update_pec(spi_buffer[j++]);				update_pec(spi_buffer[j++]);
				update_pec(spi_buffer[j++]);				// check for errors				if( spi_buffer[j++] != pec )				{					// fatal error! pec failed
					if( (++slave_errors) > SLAVE_MAX_ERRORS )
					{
						sprintf(etmp, "SE V %03d", i);
						error(etmp);
						return 1;
					}
					else
					{
						slave_read_cellvoltages();
						return 0;
					}				}				else				{
					j -= 19;
					
					cell_voltages[tmp++] = (spi_buffer[j] | ((spi_buffer[j+1] & 0x0F) << 8));
					cell_voltages[tmp++] = (((spi_buffer[j+1] >> 4) & 0x0F) | (spi_buffer[j+2] << 4));
					
					cell_voltages[tmp++] = (spi_buffer[j+3] | ((spi_buffer[j+4] & 0x0F) << 8));
					cell_voltages[tmp++] = (((spi_buffer[j+4] >> 4) & 0x0F) | (spi_buffer[j+5] << 4));
					
					cell_voltages[tmp++] = (spi_buffer[j+6] | ((spi_buffer[j+7] & 0x0F) << 8));
					cell_voltages[tmp++] = (((spi_buffer[j+7] >> 4) & 0x0F) | (spi_buffer[j+8] << 4));
					
					cell_voltages[tmp++] = (spi_buffer[j+9] | ((spi_buffer[j+10] & 0x0F) << 8));
					cell_voltages[tmp++] = (((spi_buffer[j+10] >> 4) & 0x0F) | (spi_buffer[j+11] << 4));
					
					cell_voltages[tmp++] = (spi_buffer[j+12] | ((spi_buffer[j+13] & 0x0F) << 8));
					cell_voltages[tmp++] = (((spi_buffer[j+13] >> 4) & 0x0F) | (spi_buffer[j+14] << 4));
					
					cell_voltages[tmp++] = (spi_buffer[j+15] | ((spi_buffer[j+16] & 0x0F) << 8));
					cell_voltages[tmp++] = (((spi_buffer[j+16] >> 4) & 0x0F) | (spi_buffer[j+17] << 4));
					j += 19;				}
			}
			slave_errors = 0;
			return 1;
		}
		break;
		case SLAVE_CMD_CONV_TEMP:
		{
			slave_read_temp();
			return 0;
		}
		break;
		case SLAVE_CMD_READ_TEMP:
		{
			j = 2;
			for(i=0; i<NUMSLAVES ;i++)
			{				reset_pec();				update_pec(spi_buffer[j++]);				update_pec(spi_buffer[j++]);
				update_pec(spi_buffer[j++]);
				update_pec(spi_buffer[j++]);
				update_pec(spi_buffer[j++]);				// check for errors				if( spi_buffer[j++] != pec )				{					// fatal error! pec failed
					if( (++slave_errors) > SLAVE_MAX_ERRORS )
					{
						sprintf(etmp, "SE T %03d", i);
						error(etmp);
						return 1;
					}
					else
					{
						slave_read_temp();
						return 0;
					}				}				else				{
					j -= 6;
					
					if( slave_get_gpio(i) == 0 )
					{
						tmp = (i * 6);
						cell_temps_raw[tmp+4] = spi_buffer[j+0] | ((spi_buffer[j+1] & 0x0F) << 8);
						cell_temps_raw[tmp+2] = ((spi_buffer[j+1] & 0xF0) >> 4) | (spi_buffer[j+2] << 4);
					}
					else if( slave_get_gpio(i) == 1 )
					{
						tmp = (i * 6);
						cell_temps_raw[tmp+5] = spi_buffer[j+0] | ((spi_buffer[j+1] & 0x0F) << 8);
						cell_temps_raw[tmp] = ((spi_buffer[j+1] & 0xF0) >> 4) | (spi_buffer[j+2] << 4);
					}
					else if( slave_get_gpio(i) == 2 )
					{
						tmp = (i * 6);
						cell_temps_raw[tmp+3] = spi_buffer[j+0] | ((spi_buffer[j+1] & 0x0F) << 8);
						cell_temps_raw[tmp+1] = ((spi_buffer[j+1] & 0xF0) >> 4) | (spi_buffer[j+2] << 4);
					}
					else if( slave_get_gpio(i) == 3 )
					{
						slave_fsm_raw[i] = spi_buffer[j+0] | ((spi_buffer[j+1] & 0x0F) << 8);
					}
					
					ltc_temps[i] = spi_buffer[j+3] | ((spi_buffer[j+4] & 0x0F) << 8);
					j += 6;				}
			}
			slave_errors = 0;
			
			return 1;
		}
		break;		default:		{			// FATAL ERROR!			error("SL UNKOW");
			return 1;		}		break;	}
	return 0;}

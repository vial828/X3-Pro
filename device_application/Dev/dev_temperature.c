#include "stratos_defs.h"
#include "kernel.h"
#include "log.h"
#include "dev_adc.h"
#include "dev_temperature.h"
#include "usr_cmd.h"
#include "dev_gauge_bq27z561r2.h"

#define ABS_ZERO  -274
#define ABS_HOT   1000
#define MAX_ADC   4096
#define MIN_ADC   0


typedef struct
{
    int16_t adc_calue;
    int16_t  temp_value;
}temp_map_t;

const static temp_map_t coil_temp_map[] = {
    {3897, -40}, {3885, -39}, {3874, -38}, {3861, -37}, {3848, -36},
    {3837, -35}, {3823, -34}, {3809, -33}, {3794, -32}, {3778, -31},
    {3764, -30}, {3747, -29}, {3730, -28}, {3712, -27}, {3693, -26},
    {3676, -25}, {3656, -24}, {3636, -23}, {3614, -22}, {3592, -21},
    {3572, -20}, {3549, -19}, {3525, -18}, {3501, -17}, {3475, -16},
    {3452, -15}, {3426, -14}, {3399, -13}, {3371, -12}, {3342, -11},
    {3313, -10}, {3286, -9}, {3255, -8}, {3224, -7}, {3193, -6},
    {3160, -5}, {3130, -4}, {3097, -3}, {3063, -2}, {3028, -1},
    {2993, 0}, {2960, 1}, {2924, 2}, {2888, 3}, {2851, 4},
    {2814, 5}, {2779, 6}, {2742, 7}, {2704, 8}, {2666, 9},
    {2627, 10}, {2591, 11}, {2552, 12}, {2513, 13}, {2474, 14},
    {2435, 15}, {2397, 16}, {2358, 17}, {2318, 18}, {2279, 19},
    {2241, 20}, {2202, 21}, {2163, 22}, {2125, 23}, {2086, 24},
    {2048, 25}, {2010, 26}, {1972, 27}, {1935, 28}, {1897, 29},
    {1860, 30}, {1823, 31}, {1787, 32}, {1751, 33}, {1715, 34},
    {1679, 35}, {1644, 36}, {1610, 37}, {1577, 38}, {1543, 39},
    {1509, 40}, {1477, 41}, {1445, 42}, {1414, 43}, {1383, 44},
    {1350, 45}, {1320, 46}, {1291, 47}, {1262, 48}, {1234, 49},
    {1204, 50}, {1176, 51}, {1150, 52}, {1123, 53}, {1098, 54},
    {1070, 55}, {1045, 56}, {1021, 57}, {997, 58}, {974, 59},
    {949, 60}, {927, 61}, {905, 62}, {884, 63}, {863, 64},
    {842, 65}, {822, 66}, {802, 67}, {783, 68}, {764, 69},
    {746, 70}, {728, 71}, {711, 72}, {694, 73}, {677, 74},
    {661, 75}, {645, 76}, {630, 77}, {615, 78}, {600, 79},
    {586, 80}, {572, 81}, {558, 82}, {545, 83}, {532, 84},
    {519, 85}, {507, 86}, {495, 87}, {483, 88}, {472, 89},
    {461, 90}, {450, 91}, {439, 92}, {429, 93}, {419, 94},
    {409, 95}, {399, 96}, {390, 97}, {381, 98}, {372, 99},
    {363, 100}, {355, 101}, {347, 102}, {339, 103}, {331, 104},
    {324, 105}, {316, 106}, {309, 107}, {302, 108}, {295, 109},
    {289, 110}, {282, 111}, {276, 112}, {270, 113}, {264, 114},
    {258, 115}, {252, 116}, {246, 117}, {241, 118}, {236, 119},
    {231, 120}, {225, 121}, {221, 122}, {216, 123}, {211, 124},
    {207, 125},

};

/*
const static temp_map_t usb_coldJunc_coil1TempB_temp_map[] = {
	{	199	   	,	-40	},
	{	259		,	-35	},
	{	332		,	-30	},
	{	420		,	-25	},
	{	524		,	-20	},
	{	644		,	-15	},
	{	783		,	-10	},
	{	936		,	-5	},
	{	966		,	-4	},
	{	999		,	-3	},
	{	1033	,	-2	},
	{	1068	,	-1	},
	{	1103	,	0	},
	{	1136	,	1	},
	{	1172	,	2	},
	{	1208	,	3	},
	{	1245	,	4	},
	{	1282	,	5	},
	{	1317	,	6	},
	{	1354	,	7	},
	{	1392	,	8	},
	{	1430	,	9	},
	{	1469	,	10	},
	{	1505	,	11	},
	{	1544	,	12	},
	{	1583	,	13	},
	{	1622	,	14	},
	{	1661	,	15	},
	{	1699	,	16	},
	{	1738	,	17	},
	{	1778	,	18	},
	{	1817	,	19	},
	{	1855	,	20	},
	{	1894	,	21	},
	{	1933	,	22	},
	{	1971	,	23	},
	{	2010	,	24	},
	{	2048	,	25	},
	{	2086	,	26	},
	{	2124	,	27	},
	{	2161	,	28	},
	{	2199	,	29	},
	{	2236	,	30	},
	{	2273	,	31	},
	{	2309	,	32	},
	{	2345	,	33	},
	{	2381	,	34	},
	{	2417	,	35	},
	{	2452	,	36	},
	{	2486	,	37	},
	{	2519	,	38	},
	{	2553	,	39	},
	{	2587	,	40	},
	{	2619	,	41	},
	{	2651	,	42	},
	{	2682	,	43	},
	{	2713	,	44	},
	{	2746	,	45	},
	{	2776	,	46	},
	{	2805	,	47	},
	{	2834	,	48	},
	{	2862	,	49	},
	{	2892	,	50	},
	{	2920	,	51	},
	{	2946	,	52	},
	{	2973	,	53	},
	{	2998	,	54	},
	{	3026	,	55	},
	{	3051	,	56	},
	{	3075	,	57	},
	{	3099	,	58	},
	{	3122	,	59	},
	{	3147	,	60	},
	{	3169	,	61	},
	{	3191	,	62	},
	{	3212	,	63	},
	{	3233	,	64	},
	{	3254	,	65	},
	{	3274	,	66	},
	{	3294	,	67	},
	{	3313	,	68	},
	{	3332	,	69	},
	{	3350	,	70	},
	{	3368	,	71	},
	{	3385	,	72	},
	{	3402	,	73	},
	{	3419	,	74	},
	{	3435	,	75	},
	{	3451	,	76	},
	{	3466	,	77	},
	{	3481	,	78	},
	{	3496	,	79	},
	{	3510	,	80	},
	{	3577	,	85	},
	{	3635	,	90	},
	{	3687	,	95	},
	{	3733	,	100	},
	{	3772	,	105	},
	{	3807	,	110	},
	{	3838	,	115	},
	{	3865	,	120	},
	{	3889	,	125	}
};
*/
//  cold_junc ----used bat_temp data
const static temp_map_t cold_junc_temp_map[] = {
    {3897, -40}, {3885, -39}, {3874, -38}, {3861, -37}, {3848, -36},
    {3837, -35}, {3823, -34}, {3809, -33}, {3794, -32}, {3778, -31},
    {3764, -30}, {3747, -29}, {3730, -28}, {3712, -27}, {3693, -26},
    {3676, -25}, {3656, -24}, {3636, -23}, {3614, -22}, {3592, -21},
    {3572, -20}, {3549, -19}, {3525, -18}, {3501, -17}, {3475, -16},
    {3452, -15}, {3426, -14}, {3399, -13}, {3371, -12}, {3342, -11},
    {3313, -10}, {3286, -9}, {3255, -8}, {3224, -7}, {3193, -6},
    {3160, -5}, {3130, -4}, {3097, -3}, {3063, -2}, {3028, -1},
    {2993, 0}, {2960, 1}, {2924, 2}, {2888, 3}, {2851, 4},
    {2814, 5}, {2779, 6}, {2742, 7}, {2704, 8}, {2666, 9},
    {2627, 10}, {2591, 11}, {2552, 12}, {2513, 13}, {2474, 14},
    {2435, 15}, {2397, 16}, {2358, 17}, {2318, 18}, {2279, 19},
    {2241, 20}, {2202, 21}, {2163, 22}, {2125, 23}, {2086, 24},
    {2048, 25}, {2010, 26}, {1972, 27}, {1935, 28}, {1897, 29},
    {1860, 30}, {1823, 31}, {1787, 32}, {1751, 33}, {1715, 34},
    {1679, 35}, {1644, 36}, {1610, 37}, {1577, 38}, {1543, 39},
    {1509, 40}, {1477, 41}, {1445, 42}, {1414, 43}, {1383, 44},
    {1350, 45}, {1320, 46}, {1291, 47}, {1262, 48}, {1234, 49},
    {1204, 50}, {1176, 51}, {1150, 52}, {1123, 53}, {1098, 54},
    {1070, 55}, {1045, 56}, {1021, 57}, {997, 58}, {974, 59},
    {949, 60}, {927, 61}, {905, 62}, {884, 63}, {863, 64},
    {842, 65}, {822, 66}, {802, 67}, {783, 68}, {764, 69},
    {746, 70}, {728, 71}, {711, 72}, {694, 73}, {677, 74},
    {661, 75}, {645, 76}, {630, 77}, {615, 78}, {600, 79},
    {586, 80}, {572, 81}, {558, 82}, {545, 83}, {532, 84},
    {519, 85}, {507, 86}, {495, 87}, {483, 88}, {472, 89},
    {461, 90}, {450, 91}, {439, 92}, {429, 93}, {419, 94},
    {409, 95}, {399, 96}, {390, 97}, {381, 98}, {372, 99},
    {363, 100}, {355, 101}, {347, 102}, {339, 103}, {331, 104},
    {324, 105}, {316, 106}, {309, 107}, {302, 108}, {295, 109},
    {289, 110}, {282, 111}, {276, 112}, {270, 113}, {264, 114},
    {258, 115}, {252, 116}, {246, 117}, {241, 118}, {236, 119},
    {231, 120}, {225, 121}, {221, 122}, {216, 123}, {211, 124},
    {207, 125},

    };


//usb_temp,bat_temp ----used bat_temp data
const static temp_map_t bat_temp_map[] = {
    {3897, -40}, {3885, -39}, {3874, -38}, {3861, -37}, {3848, -36},
    {3837, -35}, {3823, -34}, {3809, -33}, {3794, -32}, {3778, -31},
    {3764, -30}, {3747, -29}, {3730, -28}, {3712, -27}, {3693, -26},
    {3676, -25}, {3656, -24}, {3636, -23}, {3614, -22}, {3592, -21},
    {3572, -20}, {3549, -19}, {3525, -18}, {3501, -17}, {3475, -16},
    {3452, -15}, {3426, -14}, {3399, -13}, {3371, -12}, {3342, -11},
    {3313, -10}, {3286, -9}, {3255, -8}, {3224, -7}, {3193, -6},
    {3160, -5}, {3130, -4}, {3097, -3}, {3063, -2}, {3028, -1},
    {2993, 0}, {2960, 1}, {2924, 2}, {2888, 3}, {2851, 4},
    {2814, 5}, {2779, 6}, {2742, 7}, {2704, 8}, {2666, 9},
    {2627, 10}, {2591, 11}, {2552, 12}, {2513, 13}, {2474, 14},
    {2435, 15}, {2397, 16}, {2358, 17}, {2318, 18}, {2279, 19},
    {2241, 20}, {2202, 21}, {2163, 22}, {2125, 23}, {2086, 24},
    {2048, 25}, {2010, 26}, {1972, 27}, {1935, 28}, {1897, 29},
    {1860, 30}, {1823, 31}, {1787, 32}, {1751, 33}, {1715, 34},
    {1679, 35}, {1644, 36}, {1610, 37}, {1577, 38}, {1543, 39},
    {1509, 40}, {1477, 41}, {1445, 42}, {1414, 43}, {1383, 44},
    {1350, 45}, {1320, 46}, {1291, 47}, {1262, 48}, {1234, 49},
    {1204, 50}, {1176, 51}, {1150, 52}, {1123, 53}, {1098, 54},
    {1070, 55}, {1045, 56}, {1021, 57}, {997, 58}, {974, 59},
    {949, 60}, {927, 61}, {905, 62}, {884, 63}, {863, 64},
    {842, 65}, {822, 66}, {802, 67}, {783, 68}, {764, 69},
    {746, 70}, {728, 71}, {711, 72}, {694, 73}, {677, 74},
    {661, 75}, {645, 76}, {630, 77}, {615, 78}, {600, 79},
    {586, 80}, {572, 81}, {558, 82}, {545, 83}, {532, 84},
    {519, 85}, {507, 86}, {495, 87}, {483, 88}, {472, 89},
    {461, 90}, {450, 91}, {439, 92}, {429, 93}, {419, 94},
    {409, 95}, {399, 96}, {390, 97}, {381, 98}, {372, 99},
    {363, 100}, {355, 101}, {347, 102}, {339, 103}, {331, 104},
    {324, 105}, {316, 106}, {309, 107}, {302, 108}, {295, 109},
    {289, 110}, {282, 111}, {276, 112}, {270, 113}, {264, 114},
    {258, 115}, {252, 116}, {246, 117}, {241, 118}, {236, 119},
    {231, 120}, {225, 121}, {221, 122}, {216, 123}, {211, 124},
    {207, 125},

};

/*
const static temp_map_t bat_board_temp[] = {
	{	3897	,	-40	},
	{	3837	,	-35	},
	{	3764	,	-30	},
	{	3676	,	-25	},
	{	3572	,	-20	},
	{	3452	,	-15	},
	{	3313	,	-10	},
	{	3160	,	-5	},
	{	3130	,	-4	},
	{	3097	,	-3	},
	{	3063	,	-2	},
	{	3028	,	-1	},
	{	2993	,	0	},
	{	2960	,	1	},
	{	2924	,	2	},
	{	2888	,	3	},
	{	2851	,	4	},
	{	2814	,	5	},
	{	2779	,	6	},
	{	2742	,	7	},
	{	2704	,	8	},
	{	2666	,	9	},
	{	2627	,	10	},
	{	2591	,	11	},
	{	2552	,	12	},
	{	2513	,	13	},
	{	2474	,	14	},
	{	2435	,	15	},
	{	2397	,	16	},
	{	2358	,	17	},
	{	2318	,	18	},
	{	2279	,	19	},
	{	2241	,	20	},
	{	2202	,	21	},
	{	2163	,	22	},
	{	2125	,	23	},
	{	2086	,	24	},
	{	2048	,	25	},
	{	2010	,	26	},
	{	1972	,	27	},
	{	1935	,	28	},
	{	1897	,	29	},
	{	1860	,	30	},
	{	1823	,	31	},
	{	1787	,	32	},
	{	1751	,	33	},
	{	1715	,	34	},
	{	1679	,	35	},
	{	1644	,	36	},
	{	1610	,	37	},
	{	1577	,	38	},
	{	1543	,	39	},
	{	1509	,	40	},
	{	1477	,	41	},
	{	1445	,	42	},
	{	1414	,	43	},
	{	1383	,	44	},
	{	1350	,	45	},
	{	1320	,	46	},
	{	1291	,	47	},
	{	1262	,	48	},
	{	1234	,	49	},
	{	1204	,	50	},
	{	1176	,	51	},
	{	1150	,	52	},
	{	1123	,	53	},
	{	1098	,	54	},
	{	1070	,	55	},
	{	1045	,	56	},
	{	1021	,	57	},
	{	997		,	58	},
	{	974		,	59	},
	{	949		,	60	},
	{	927		,	61	},
	{	905		,	62	},
	{	884		,	63	},
	{	863		,	64	},
	{	842		,	65	},
	{	822		,	66	},
	{	802		,	67	},
	{	783		,	68	},
	{	764		,	69	},
	{	746		,	70	},
	{	728		,	71	},
	{	711		,	72	},
	{	694		,	73	},
	{	677		,	74	},
	{	661		,	75	},
	{	645		,	76	},
	{	630		,	77	},
	{	615		,	78	},
	{	600		,	79	},
	{	586		,	80	},
	{	519		,	85	},
	{	461		,	90	},
	{	409		,	95	},
	{	363		,	100	},
	{	324		,	105	},
	{	289		,	110	},
	{	258		,	115	},
	{	231		,	120	},
	{	207		,	125	}
};
*/


/*
const static temp_map_t tc_temp_map[] = {
{0,     0},{9,    1},{18,    2},{27,     3},{36,    4},{45,    5},{53,    6},{62,    7},{71,   8},{80,    9},
{89,   10},{98,  11},{107,  12},{116,   13},{125,  14},{134,  15},{144,  16},{152,  17},{161, 18},{171,  19},
{180,  20},{189,  21},{198,  22},{207,  23},{216,  24},{225,  25},{234,  26},{243,  27},{253, 28},{262,  29},
{271,  30},{280,  31},{289,  32},{298,  33},{308,  34},{317,  35},{326,  36},{335,  37},{344, 38},{354,  39},
{363,  40},{372,  41},{381,  42},{391,  43},{400,  44},{409,  45},{419,  46},{428,  47},{437,  48},{446,  49},
{456,  50},{465,  51},{474,  52},{484,  53},{493,  54},{502,  55},{512,  56},{521,  57},{530,  58},{540,  59},
{549,  60},{559,  61},{568,  62},{577,  63},{587,  64},{596,  65},{606,  66},{615,  67},{625,  68},{634,  69},
{643,  70},{653,  71},{662,  72},{672,  73},{681,  74},{691,  75},{700,  76},{710,  77},{719,  78},{729,  79},
{738,  80},{747,  81},{757,  82},{767,  83},{776,  84},{786,  85},{795,  86},{805,  87},{814,  88},{824,  89},
{833,  90},{843,  91},{852,  92},{862,  93},{871,  94},{881,  95},{891,  96},{900,  97},{910,  98},{919,  99},
{929, 100},{938, 101},{948, 102},{958, 103},{967, 104},{977, 105},{986, 106},{996, 107},{1006,108},{1015,109},
{1025,110},{1034,111},{1044,112},{1054,113},{1063,114},{1073,115},{1083,116},{1092,117},{1102,118},{1112,119},
{1121,120},{1131,121},{1141,122},{1150,123},{1160,124},{1170,125},{1179,126},{1189,127},{1199,128},{1208,129},
{1218,130},{1228,131},{1237,132},{1247,133},{1257,134},{1266,135},{1276,136},{1286,137},{1296,138},{1305,139},
{1315,140},{1325,141},{1334,142},{1344,143},{1354,144},{1363,145},{1373,146},{1383,147},{1393,148},{1402,149},
{1412,150},{1422,151},{1431,152},{1441,153},{1451,154},{1461,155},{1470,156},{1480,157},{1490,158},{1500,159},
{1509,160},{1519,161},{1529,162},{1539,163},{1548,164},{1558,165},{1568,166},{1578,167},{1588,168},{1597,169},
{1607,170},{1617,171},{1626,172},{1636,173},{1646,174},{1656,175},{1666,176},{1675,177},{1685,178},{1695,179},
{1705,180},{1714,181},{1724,182},{1734,183},{1744,184},{1754,185},{1763,186},{1773,187},{1783,188},{1793,189},
{1802,190},{1812,191},{1822,192},{1832,193},{1842,194},{1851,195},{1861,196},{1871,197},{1881,198},{1890,199},
{1900,200},{1910,201},{1920,202},{1930,203},{1939,204},{1949,205},{1959,206},{1969,207},{1979,208},{1988,209},
{1998,210},{2008,211},{2018,212},{2028,213},{2037,214},{2047,215},{2057,216},{2067,217},{2076,218},{2086,219},
{2096,220},{2106,221},{2116,222},{2125,223},{2135,224},{2145,225},{2155,226},{2165,227},{2174,228},{2184,229},
{2194,230},{2204,231},{2214,232},{2223,233},{2233,234},{2243,235},{2253,236},{2262,237},{2272,238},{2282,239},
{2292,240},{2302,241},{2311,242},{2321,243},{2331,244},{2341,245},{2350,246},{2360,247},{2370,248},{2380,249},
{2390,250},{2400,251},{2409,252},{2419,253},{2429,254},{2439,255},{2448,256},{2458,257},{2468,258},{2478,259},
{2487,260},{2497,261},{2507,262},{2517,263},{2527,264},{2536,265},{2546,266},{2556,267},{2566,268},{2575,269},
{2585,270},{2595,271},{2605,272},{2615,273},{2624,274},{2634,275},{2644,276},{2654,277},{2664,278},{2673,279},
{2683,280},{2693,281},{2703,282},{2712,283},{2722,284},{2732,285},{2742,286},{2751,287},{2761,288},{2771,289},
{2781,290},{2791,291},{2800,292},{2810,293},{2820,294},{2829,295},{2839,296},{2849,297},{2859,298},{2869,299},
{2878,300},{2888,301},{2898,302},{2908,303},{2917,304},{2927,305},{2937,306},{2947,307},{2956,308},{2966,309},
{2976,310},{2986,311},{2995,312},{3005,313},{3015,314},{3025,315},{3034,316},{3044,317},{3054,318},{3064,319},
{3073,320},{3083,321},{3093,322},{3103,323},{3112,324},{3122,325},{3132,326},{3142,327},{3151,328},{3161,329},
{3171,330},{3180,331},{3190,332},{3200,333},{3210,334},{3219,335},{3229,336},{3239,337},{3249,338},{3258,339},
{3268,340},{3278,341},{3288,342},{3297,343},{3307,344},{3317,345},{3327,346},{3336,347},{3346,348},{3356,349},
{3365,350},{3375,351},{3385,352},{3395,353},{3404,354},{3414,355},{3424,356},{3434,357},{3443,358},{3453,359},
{3463,360},{3472,361},{3482,362},{3492,363},{3502,364},{3511,365},{3521,366},{3531,367},{3540,368},{3550,369},
{3560,370},{3570,371},{3579,372},{3589,373},{3599,374},{3609,375},{3618,376},{3628,377},{3638,378},{3647,379},
{3657,380},{3667,381},{3677,382},{3686,383},{3696,384},{3706,385},{3716,386},{3725,387},{3735,388},{3745,389},
{3754,390},{3764,391},{3774,392},{3784,393},{3793,394},{3803,395},{3813,396},{3823,397},{3832,398},{3842,399},
{3852,400},{3861,401},{3871,402},{3881,403},{3891,404},{3900,405},{3910,406},{3920,407},{3929,408},{3939,409},
{3949,410},{3959,411},{3968,412},{3978,413},{3988,414},{3998,415},{4007,416},{4017,417},{4027,418},{4036,419},
{4046,420},{4056,421},{4066,422},{4075,423},{4085,424},{4095,425}
};
*/

const static temp_map_t tc_temp_map[] = {
    {-340, -40}, {-332, -39}, {-323, -38}, {-315, -37}, {-307, -36}, {-298, -35}, {-290, -34}, {-282, -33}, {-273, -32}, {-265, -31},
    {-257, -30}, {-248, -29}, {-240, -28}, {-232, -27}, {-223, -26}, {-215, -25}, {-206, -24}, {-198, -23}, {-189, -22}, {-181, -21},
    {-172, -20}, {-164, -19}, {-155, -18}, {-147, -17}, {-138, -16}, {-130, -15}, {-121, -14}, {-113, -13}, {-104, -12}, {-95, -11},
    {-87, -10}, {-78, -9}, {-69, -8}, {-61, -7}, {-52, -6}, {-44, -5}, {-35, -4}, {-26, -3}, {-18, -2}, {-9, -1},
    {0, 0}, {9, 1}, {18, 2}, {26, 3}, {35, 4}, {44, 5}, {53, 6}, {61, 7}, {70, 8}, {79, 9},
    {88, 10}, {97, 11}, {106, 12}, {114, 13}, {123, 14}, {132, 15}, {141, 16}, {150, 17}, {159, 18}, {168, 19},
    {177, 20}, {186, 21}, {194, 22}, {203, 23}, {212, 24}, {221, 25}, {230, 26}, {239, 27}, {248, 28}, {257, 29},
    {266, 30}, {275, 31}, {284, 32}, {293, 33}, {302, 34}, {311, 35}, {320, 36}, {330, 37}, {339, 38}, {348, 39},
    {357, 40}, {366, 41}, {375, 42}, {384, 43}, {393, 44}, {402, 45}, {411, 46}, {421, 47}, {430, 48}, {439, 49},
    {448, 50}, {457, 51}, {466, 52}, {476, 53}, {485, 54}, {494, 55}, {503, 56}, {512, 57}, {521, 58}, {531, 59},
    {540, 60}, {549, 61}, {558, 62}, {568, 63}, {577, 64}, {586, 65}, {595, 66}, {605, 67}, {614, 68}, {623, 69},
    {633, 70}, {642, 71}, {651, 72}, {660, 73}, {670, 74}, {679, 75}, {688, 76}, {698, 77}, {707, 78}, {716, 79},
    {726, 80}, {735, 81}, {744, 82}, {754, 83}, {763, 84}, {772, 85}, {782, 86}, {791, 87}, {800, 88}, {810, 89},
    {819, 90}, {829, 91}, {838, 92}, {847, 93}, {857, 94}, {866, 95}, {876, 96}, {885, 97}, {894, 98}, {904, 99},
    {913, 100}, {923, 101}, {932, 102}, {941, 103}, {951, 104}, {960, 105}, {970, 106}, {979, 107}, {989, 108}, {998, 109},
    {1008, 110}, {1017, 111}, {1027, 112}, {1036, 113}, {1045, 114}, {1055, 115}, {1064, 116}, {1074, 117}, {1083, 118}, {1093, 119},
    {1102, 120}, {1112, 121}, {1121, 122}, {1131, 123}, {1140, 124}, {1150, 125}, {1159, 126}, {1169, 127}, {1178, 128}, {1188, 129},
    {1197, 130}, {1207, 131}, {1216, 132}, {1226, 133}, {1236, 134}, {1245, 135}, {1255, 136}, {1264, 137}, {1274, 138}, {1283, 139},
    {1293, 140}, {1302, 141}, {1312, 142}, {1321, 143}, {1331, 144}, {1340, 145}, {1350, 146}, {1359, 147}, {1369, 148}, {1379, 149},
    {1388, 150}, {1398, 151}, {1407, 152}, {1417, 153}, {1427, 154}, {1436, 155}, {1446, 156}, {1455, 157}, {1465, 158}, {1474, 159},
    {1484, 160}, {1494, 161}, {1503, 162}, {1513, 163}, {1522, 164}, {1532, 165}, {1541, 166}, {1551, 167}, {1561, 168}, {1570, 169},
    {1580, 170}, {1589, 171}, {1599, 172}, {1609, 173}, {1618, 174}, {1628, 175}, {1637, 176}, {1647, 177}, {1657, 178}, {1666, 179},
    {1676, 180}, {1685, 181}, {1695, 182}, {1705, 183}, {1714, 184}, {1724, 185}, {1733, 186}, {1743, 187}, {1753, 188}, {1762, 189},
    {1772, 190}, {1781, 191}, {1791, 192}, {1801, 193}, {1810, 194}, {1820, 195}, {1830, 196}, {1839, 197}, {1849, 198}, {1858, 199},
    {1868, 200}, {1878, 201}, {1887, 202}, {1897, 203}, {1907, 204}, {1916, 205}, {1926, 206}, {1935, 207}, {1945, 208}, {1955, 209},
    {1964, 210}, {1974, 211}, {1984, 212}, {1993, 213}, {2003, 214}, {2012, 215}, {2022, 216}, {2032, 217}, {2041, 218}, {2051, 219},
    {2060, 220}, {2070, 221}, {2080, 222}, {2089, 223}, {2099, 224}, {2109, 225}, {2118, 226}, {2128, 227}, {2138, 228}, {2147, 229},
    {2157, 230}, {2166, 231}, {2176, 232}, {2186, 233}, {2195, 234}, {2205, 235}, {2215, 236}, {2224, 237}, {2234, 238}, {2243, 239},
    {2253, 240}, {2263, 241}, {2272, 242}, {2282, 243}, {2292, 244}, {2301, 245}, {2311, 246}, {2320, 247}, {2330, 248}, {2340, 249},
    {2349, 250}, {2359, 251}, {2368, 252}, {2378, 253}, {2388, 254}, {2397, 255}, {2407, 256}, {2417, 257}, {2426, 258}, {2436, 259},
    {2445, 260}, {2455, 261}, {2465, 262}, {2474, 263}, {2484, 264}, {2494, 265}, {2503, 266}, {2513, 267}, {2522, 268}, {2532, 269},
    {2542, 270}, {2551, 271}, {2561, 272}, {2570, 273}, {2580, 274}, {2590, 275}, {2599, 276}, {2609, 277}, {2619, 278}, {2628, 279},
    {2638, 280}, {2647, 281}, {2657, 282}, {2667, 283}, {2676, 284}, {2686, 285}, {2695, 286}, {2705, 287}, {2715, 288}, {2724, 289},
    {2734, 290}, {2743, 291}, {2753, 292}, {2763, 293}, {2772, 294}, {2782, 295}, {2791, 296}, {2801, 297}, {2810, 298}, {2820, 299},
    {2830, 300}, {2839, 301}, {2849, 302}, {2858, 303}, {2868, 304}, {2878, 305}, {2887, 306}, {2897, 307}, {2906, 308}, {2916, 309},
    {2926, 310}, {2935, 311}, {2945, 312}, {2954, 313}, {2964, 314}, {2973, 315}, {2983, 316}, {2993, 317}, {3002, 318}, {3012, 319},
    {3021, 320}, {3031, 321}, {3041, 322}, {3050, 323}, {3060, 324}, {3069, 325}, {3079, 326}, {3088, 327}, {3098, 328}, {3108, 329},
    {3117, 330}, {3127, 331}, {3136, 332}, {3146, 333}, {3155, 334}, {3165, 335}, {3175, 336}, {3184, 337}, {3194, 338}, {3203, 339},
    {3213, 340}, {3223, 341}, {3232, 342}, {3242, 343}, {3251, 344}, {3261, 345}, {3270, 346}, {3280, 347}, {3289, 348}, {3299, 349},
    {3308, 350}, {3318, 351}, {3328, 352}, {3337, 353}, {3347, 354}, {3356, 355}, {3366, 356}, {3376, 357}, {3385, 358}, {3395, 359},
    {3404, 360}, {3414, 361}, {3423, 362}, {3433, 363}, {3442, 364}, {3452, 365}, {3462, 366}, {3471, 367}, {3481, 368}, {3490, 369},
    {3500, 370}, {3509, 371}, {3519, 372}, {3528, 373}, {3538, 374}, {3547, 375}, {3557, 376}, {3567, 377}, {3576, 378}, {3586, 379},
    {3595, 380}, {3605, 381}, {3614, 382}, {3624, 383}, {3634, 384}, {3643, 385}, {3653, 386}, {3662, 387}, {3672, 388}, {3681, 389},
    {3691, 390}, {3701, 391}, {3710, 392}, {3720, 393}, {3729, 394}, {3739, 395}, {3748, 396}, {3758, 397}, {3767, 398}, {3777, 399},
    {3786, 400}, {3796, 401}, {3806, 402}, {3815, 403}, {3825, 404}, {3834, 405}, {3844, 406}, {3853, 407}, {3863, 408}, {3873, 409},
    {3882, 410}, {3892, 411}, {3901, 412}, {3911, 413}, {3920, 414}, {3930, 415}, {3940, 416}, {3949, 417}, {3959, 418}, {3968, 419},
    {3978, 420}, {3987, 421}, {3997, 422}, {4006, 423}, {4016, 424}, {4026, 425},
};


physical_value_s phy_value = {0};

/*************************************************************************************************
  * @brief    : The value in the structure phy_value is used to
                simulate the physical value obtained by the device through adc
                phy_value_get_pos decides which values will be simulated
  * @return   : phy_value.phy_val_get_pos
*************************************************************************************************/
uint16_t dev_get_phy_val_pos(void)
{
    return phy_value.phy_val_get_pos;
}

/*************************************************************************************************
  * @brief    : Get one of the simulated values
  * @param1   : Which physical value will be returned
  * @return   : bat_temp/bat_v/zone1_temp...
*************************************************************************************************/
int16_t dev_get_phy_val(physical_value_e phy_val_e)
{
    switch(phy_val_e)
    {
        case BAT_TEMP_E:
            return phy_value.bat_temp;
        case BAT_V_E:
            return phy_value.bat_mv;
        case ZONE1_TEMP_E:
            return phy_value.zone1_temp;
        case ZONE2_TEMP_E:
            return phy_value.zone2_temp;
        case USB_TEMP_E:
            return phy_value.usb_temp;
        case I_SENSE_E:
            return phy_value.i_sense_ma;
        case COIL_TEMP_E:
            return phy_value.coil_temp;
        case COIL_JUNC_TEMP_E:
            return phy_value.coil_junc_temp;
        case PWM_DAC_E:
            return phy_value.pwm_dac;
        case TC1_TEMP_E:
            return phy_value.tc1_temp;
        case TC2_TEMP_E:
            return phy_value.tc2_temp;
        case BAT_ID_E:
            return phy_value.bat_id;
        case GAS_SOC_E:
            return phy_value.gas_soc;
        default:
            break;
    }
    return 0xFFFF;
}

/*************************************************************************************************
  * @brief    : charge convert ntc ohm to temperature
  * @return   : temperature
*************************************************************************************************/
int16_t dev_convert_ntc_ohm(const cic_ntc_map_t *pmap, uint16_t len, float ohm)
{
    uint16_t i;
    for(i=0; i<len; i++){
        if(ohm == pmap[i].ntc_ohm){
            return pmap[i].temp_value;
        }else if(ohm < pmap[i].ntc_ohm){
            if(i == 0){
                return 1000;    //ABS_HOT
            }else{
                return pmap[i].temp_value;
            }
        }
    }

    return -273;//ABS_ZERO
}

/*************************************************************************************************
 * @brief    : Store heating profile in buffer
 * @param    : temperature map pointer
 * @param    : temperature map lenght
 * @param    : adc value
 * @return   : temperature
*************************************************************************************************/
static float convert_map(const temp_map_t *pmap, uint16_t len, uint16_t adc)
{
    int16_t i;

	/*look up the temperature table*/
    for(i=0; i<len; i++){
        if(adc == pmap[i].adc_calue){
            return (pmap[i].temp_value);
        }else if(adc > pmap[i].adc_calue){
            if(i == 0){ /*adc is too small out of temperature table*/
                return (ABS_ZERO);//absolute zero
            }else{/*liner calculation*/
                int16_t tl, th;
                uint16_t al, ah;
                tl = pmap[i-1].temp_value;
                th = pmap[i].temp_value;

                al = pmap[i-1].adc_calue;
                ah = pmap[i].adc_calue;

                return (((float)adc-al)*(th-tl)/(ah-al)+tl);
            }
        }
    }
    /*adc value is too big out of temperature table*/
    return (ABS_HOT); //absolute hot
}
static float convert_map2(const temp_map_t *pmap, uint16_t len, float adc)
{
    int16_t i;

	/*look up the temperature table*/
    for(i=0; i<len; i++){
        if(adc == pmap[i].adc_calue){
            return (pmap[i].temp_value);
        }else if(adc < pmap[i].adc_calue){
            if(i == 0){ /*adc is too small out of temperature table*/
                return (ABS_ZERO);//absolute zero
            }else{/*liner calculation*/
                int16_t tl, th;
                int16_t al, ah;
                tl = pmap[i-1].temp_value;
                th = pmap[i].temp_value;

                al = pmap[i-1].adc_calue;
                ah = pmap[i].adc_calue;

                return (((float)adc-al)*(th-tl)/(ah-al)+tl);
            }
        }
    }
    /*adc value is too big out of temperature table*/
    return (ABS_HOT); //absolute hot
}

static float convert_map_temp_to_adc(const temp_map_t *pmap, uint16_t len, float temp)
{
    int16_t i;
	/*look up the temperature table*/
    for(i=0; i<len; i++){
        if(temp == pmap[i].temp_value){
            return (pmap[i].adc_calue);
        }else if(temp < pmap[i].temp_value){
            if(i == 0){ /*temp is too low*/
                return MAX_ADC;//max adc
            }else{/*liner calculation*/
                int16_t tl, th;
                int16_t al, ah;
                tl = pmap[i-1].temp_value;
                th = pmap[i].temp_value;

                al = pmap[i-1].adc_calue;
                ah = pmap[i].adc_calue;

                return (((float)temp-tl)*(ah-al)/(th-tl)+al);
            }
        }
    }
    /*temp value is too high out of temperature table*/
    return (MIN_ADC); //min_adc
}

/*************************************************************************************************
 * @brief    : get coil temperature 
 * @return   : temperature
*************************************************************************************************/
float dev_get_coil_temp(void)
{
    uint16_t adc;
    static uint16_t adc_1 = 0xffff; //store previous value for rapidly get temp
    static float temp_1;

    /*get cli set physis value*/	
    if(dev_get_phy_val_pos() & (1<<COIL_TEMP_E))
    {
        temp_1 = dev_get_phy_val(COIL_TEMP_E);
    }
    else
    {
        adc = dev_get_adc(COIL_TEMP);
        if(adc != adc_1){
            adc_1 = adc;
            temp_1 = convert_map(coil_temp_map, ARRAY_SIZE_OF(coil_temp_map), adc);
        }
    }
    return temp_1;
}


/*
float get_coil_board_temp(void)
{
	uint16_t adc;
	static uint16_t adc_1 = 0xffff; //for first
	static float temp_1;
	
    adc = dev_get_adc(COIL_TEMPB);
    if(adc != adc_1){
        adc_1 = adc;
        temp_1 = convert_map(usb_coldJunc_coil1TempB_temp_map, ARRAY_SIZE_OF(usb_coldJunc_coil1TempB_temp_map), adc);
    }
    return ABS_ZERO;
	
}
*/

/*************************************************************************************************
 * @brief    : get junc temperature 
 * @return   : temperature
*************************************************************************************************/
float dev_get_cold_junc_temp(void)
{
    uint16_t adc;
    static uint16_t adc_s = 0xFFFF; //store previous value for rapidly get temp
    static float temp_s;

    /*get cli set physis value*/	
    if(dev_get_phy_val_pos() & (1<<COIL_JUNC_TEMP_E))
    {
        temp_s = dev_get_phy_val(COIL_JUNC_TEMP_E);
    }
    else
    {
        adc = dev_get_adc(COLD_JUNC);
        if(adc != adc_s){
            adc_s = adc;
            temp_s = convert_map(cold_junc_temp_map, ARRAY_SIZE_OF(bat_temp_map), adc);
        }
    }
    return temp_s;
}

/*************************************************************************************************
 * @brief    : get usb temperature 
 * @return   : temperature
*************************************************************************************************/
float dev_get_usb_temp(void)
{
    uint16_t adc;
    static uint16_t adc_s = 0xFFFF;
    static float temp_s;

    /*get cli set physis value*/	
    if(dev_get_phy_val_pos() & (1<<USB_TEMP_E))
    {
        temp_s = dev_get_phy_val(USB_TEMP_E);
    }
    else
    {
        adc = dev_get_adc(USB_TEMP);
        if(adc != adc_s){
            adc_s = adc;
            temp_s = convert_map(bat_temp_map, ARRAY_SIZE_OF(bat_temp_map), adc);
        }
    }
    return temp_s;
//  return convert_map(usb_temp_map, ARRAY_SIZE_OF(usb_temp_map), adc);
}

/*************************************************************************************************
 * @brief    : get battery temperature 
 * @return   : temperature
*************************************************************************************************/
float dev_get_bat_temp(void)
{
    uint16_t adc;
    static uint16_t adc_s = 0xFFFF;
    static float temp_s;

    /*get cli set physis value*/	
    if(dev_get_phy_val_pos() & (1<<BAT_TEMP_E))
    {
        temp_s = dev_get_phy_val(BAT_TEMP_E);
    }
    else
    {
        adc = dev_get_adc(BAT_TEMP);
        if(adc != adc_s){
            adc_s = adc;
            temp_s = convert_map(bat_temp_map, ARRAY_SIZE_OF(bat_temp_map), adc);
        }
    }
    return temp_s;
//  return convert_map(bat_temp_map, ARRAY_SIZE_OF(bat_temp_map), adc);
}

/*
float get_bat_board_temp(void)
{
	uint16_t adc;
	static uint16_t adc_s = 0xFFFF;
	static float temp_s;
	
	adc = dev_get_adc(BAT_TEMP_1);
	if(adc != adc_s){
		adc_s = adc;
		temp_s = convert_map(bat_board_temp, ARRAY_SIZE_OF(bat_board_temp), adc);
	}
	return temp_s;	
//	return convert_map(bat_temp_map, ARRAY_SIZE_OF(bat_temp_map), adc);
}
*/

#if 1
/*************************************************************************************************
 * @brief    : get tc temperature 
 * @param    : zone1 or zone2 
 * @return   : temperature
*************************************************************************************************/
float dev_get_tc_temp(uint8_t num)
{
    float adc;
    uint16_t len;
    uint16_t i;

    static float adc_1 = 0;
    static float adc_2 = 0;
    static float temp_1 = 0;
    static float temp_2 = 0;

    /*get cli set physis value*/    
    if(dev_get_phy_val_pos() & (1<<TC1_TEMP_E))
    {
        temp_1 = dev_get_phy_val(TC1_TEMP_E);
        if(num == 1)
        {
            return temp_1;
        }
    }

    /*get cli set physis value*/    
    if(dev_get_phy_val_pos() & (1<<TC2_TEMP_E))
    {
        temp_2 = dev_get_phy_val(TC2_TEMP_E);
        if(num == 2)
        {
            return temp_2;
        }
    }

    len = ARRAY_SIZE_OF(tc_temp_map);
//  adc = (num == 1 ? dev_get_adc(TC1) : dev_get_adc(TC2));
    //LOGD("tc adc:%d\r\n", adc);
    if(num == 1){
        adc = dev_get_adc(TC1);
        if(adc == adc_1){
            return temp_1;
        }
        adc_1 = adc;
        return temp_1 = convert_map2(tc_temp_map, ARRAY_SIZE_OF(tc_temp_map), adc);
    }else{
        adc = dev_get_adc(TC2);
        if(adc == adc_2){
            return temp_2;
        }
        adc_2 = adc;
        return temp_2 = convert_map2(tc_temp_map, ARRAY_SIZE_OF(tc_temp_map), adc);
    }
}


#else
#define TCx_Calcu     ((float)(425.0f / 4095.0f))
int16_t dev_get_tc_temp(uint8_t num)
{
	uint16_t adc;
	
	if(num == 1){
		adc = dev_get_adc(TC1);
	}else{
		adc = dev_get_adc(TC2);
	}
	
	return adc * ((float)(425.0f / 4095.0f));
}
#endif

/*************************************************************************************************
 * @brief    : get zone temperature 
 * @param    : zone1 or zone2 
 * @return   : temperature
*************************************************************************************************/
float dev_get_zone_temp(uint8_t num)
{
    float adc;
    uint16_t len;
    uint16_t i;
    uint16_t cold_i;
    float cold_adc;
    float cold_temp;

    static float zone_adc_1 = 0;
    static float zone_adc_2 = 0;
    static float zone_temp_1 = 0;
    static float zone_temp_2 = 0;

    /*get cli set physis value*/
    if(dev_get_phy_val_pos() & (1<<ZONE1_TEMP_E))
    {
        zone_temp_1 = dev_get_phy_val(ZONE1_TEMP_E);
        if(num == 1)
        {
            return zone_temp_1;
        }
    }

    /*get cli set physis value*/        
    if(dev_get_phy_val_pos() & (1<<ZONE2_TEMP_E))
    {
        zone_temp_2 = dev_get_phy_val(ZONE2_TEMP_E);
        if(num == 2)
        {
            return zone_temp_2;
        }
    }

    len = ARRAY_SIZE_OF(tc_temp_map);

    /*calculate COLD_JUNC from temp to adc*/

    cold_temp = dev_get_adc_result()->cold_junc_temp;
    cold_adc=convert_map_temp_to_adc(tc_temp_map, ARRAY_SIZE_OF(tc_temp_map), cold_temp);
    if(num == 1){
        adc = dev_get_adc(TC1)+ cold_adc;
        if(adc == zone_adc_1)
        {
            return zone_temp_1;
        }
        zone_adc_1 = adc;
    }else{
        adc = dev_get_adc(TC2)+ cold_adc;
        if(adc == zone_adc_2)
        {
            return zone_temp_2;
        }
        zone_adc_2 = adc;
    }
    if(num == 1){
        zone_temp_1 = convert_map2(tc_temp_map, ARRAY_SIZE_OF(tc_temp_map), adc);
        return zone_temp_1;
    }else{
        zone_temp_2 = convert_map2(tc_temp_map, ARRAY_SIZE_OF(tc_temp_map), adc);
        return zone_temp_2;
    }
//    return ((adc-L)<=(H-adc) ? tc_temp_map[i-1].temp_value : tc_temp_map[i].temp_value);
}

void dev_print_all_adc(void)
{
	LOGD("--------------------------------------\r\n");
	LOGD("item    |  adc  |   temp  |  voltage  |\r\n");
	LOGD("bat_temp| %04d  |   %3.1fC  |  %0.3fV   |\r\n", dev_get_adc(BAT_TEMP), dev_get_bat_temp(), dev_get_voltage(BAT_TEMP));
	LOGD("usb_temp| %04d  |   %3.1fC  |  %0.3fV   |\r\n", dev_get_adc(USB_TEMP), dev_get_usb_temp(), dev_get_voltage(USB_TEMP));
	//LOGD("coil 1  | %04d  |   %03dC  |  %0.3fV   |\r\n", dev_get_adc(COIL_1_TEMP), dev_get_coil_temp(1), dev_get_voltage(COIL_1_TEMP));
	//LOGD("coil 2  | %04d  |   %03dC  |  %0.3fV   |\r\n", dev_get_adc(COIL_2_TEMP), dev_get_coil_temp(2), dev_get_voltage(COIL_2_TEMP));
    LOGD("coil    | %04d  |   %3.1fC  |  %0.3fV   |\r\n", dev_get_adc(COIL_TEMP), dev_get_coil_temp(), dev_get_voltage(COIL_TEMP));
//    LOGD("coilb   | %04d  |   %3.1fC  |  %0.3fV   |\r\n", dev_get_adc(COIL_TEMPB), get_coil_board_temp(), dev_get_voltage(COIL_TEMPB));
	LOGD("junc    | %04d  |   %3.1fC  |  %0.3fV   |\r\n", dev_get_adc(COLD_JUNC), dev_get_cold_junc_temp(), dev_get_voltage(COLD_JUNC));
	LOGD("TC1     | %04d  |    %0.3fC  |  %0.3fV   |\r\n",dev_get_adc(TC1), dev_get_tc_temp(1), dev_get_voltage(TC1));
	LOGD("TC2     | %04d  |    %0.3fC  |  %0.3fV   |\r\n",dev_get_adc(TC2), dev_get_tc_temp(2), dev_get_voltage(TC2));
	//LOGD("vbus    | %04d  |         |  %0.3fV   |\r\n", dev_get_adc(VBUS_VOLT),get_vbus_volt());
	LOGD("vbat    | %04d  |         |  %0.3fV   |\r\n", dev_get_adc(VBAT_VOLT),dev_get_vbat_volt());
	LOGD("i sense | %04d  |   %0.3fA|  %0.3fV   |\r\n",dev_get_adc(I_SENSE), dev_get_i_sense(), dev_get_voltage(I_SENSE));
//    LOGD("bat_tem1| %04d  |   %3.1fC  |  %0.3fV   |\r\n", dev_get_adc(BAT_TEMP_1), get_bat_board_temp(), dev_get_voltage(BAT_TEMP_1));
   // LOGD("i bat   | %04d  |   %0.3fA|  %0.3fV   |\r\n",dev_get_adc(I_BAT), get_i_bat(), dev_get_voltage(I_BAT));
}
float dev_get_bat_temp_gauge(void)
{
        float temp = 0.0;
        uint8_t gauge_temp[2] = {0};
        Dev_BQ27Z561R2_ReadRegBits(0x06,gauge_temp,2);
        temp =(gauge_temp[0]+gauge_temp[1]*256)/10.0 - 273;
        return temp;
        //LOGD("charge_proc temp: %d\r\n",temp);
}

#ifdef TEST_ADC
//static timer_t * temp_test_timer;
static ptimer_t temp_test_timer;
static uint16_t test_adc = 0;

static void cb_test_timer(const ptimer_t tm)
{
	dev_print_all_adc();
	LOGD("--------------------------------------\r\n");
	LOGD("item    |  adc  |   temp  |  voltage  |\r\n");
	LOGD("bat_temp| %04d  |   %0.3fC  |  %0.3fV   |\r\n", dev_get_adc(BAT_TEMP), dev_get_bat_temp(), dev_get_voltage(BAT_TEMP));
	LOGD("usb_temp| %04d  |   %0.3fC  |  %0.3fV   |\r\n", dev_get_adc(USB_TEMP), dev_get_usb_temp(), dev_get_voltage(USB_TEMP));
	//LOGD("coil 1  | %04d  |   %03dC  |  %0.3fV   |\r\n", dev_get_adc(COIL_1_TEMP), dev_get_coil_temp(1), dev_get_voltage(COIL_1_TEMP));
	//LOGD("coil 2  | %04d  |   %03dC  |  %0.3fV   |\r\n", dev_get_adc(COIL_2_TEMP), dev_get_coil_temp(2), dev_get_voltage(COIL_2_TEMP));
    LOGD("coil    | %04d  |   %0.3fC  |  %0.3fV   |\r\n", dev_get_adc(COIL_TEMP), dev_get_coil_temp(), dev_get_voltage(COIL_TEMP));
//    LOGD("coilb   | %04d  |   %03dC  |  %0.3fV   |\r\n", dev_get_adc(COIL_TEMPB), get_coil_board_temp(), dev_get_voltage(COIL_TEMPB));
	LOGD("junc    | %04d  |   %0.3fC  |  %0.3fV   |\r\n", dev_get_adc(COLD_JUNC), dev_get_cold_junc_temp(), dev_get_voltage(COLD_JUNC));
	LOGD("TC1     | %04d  |   %0.3fC  |  %0.3fV   |\r\n",dev_get_adc(TC1), dev_get_tc_temp(1), dev_get_voltage(TC1));
	LOGD("TC2     | %04d  |   %0.3fC  |  %0.3fV   |\r\n",dev_get_adc(TC2), dev_get_tc_temp(2), dev_get_voltage(TC2));
	//LOGD("vbus    | %04d  |         |  %0.3fV   |\r\n", dev_get_adc(VBUS_VOLT),get_vbus_volt());
	LOGD("vbat    | %04d  |         |  %0.3fV   |\r\n", dev_get_adc(VBAT_VOLT),dev_get_vbat_volt());
	LOGD("i sense | %04d  |   %0.3fA|  %0.3fV   |\r\n",dev_get_adc(I_SENSE), dev_get_i_sense(), dev_get_voltage(I_SENSE));
//    LOGD("bat_tem1| %04d  |   %03dC  |  %0.3fV   |\r\n", dev_get_adc(BAT_TEMP_1), get_bat_board_temp(), dev_get_voltage(BAT_TEMP_1));
    //LOGD("i bat   | %04d  |   %0.3fA|  %0.3fV   |\r\n",dev_get_adc(I_BAT), get_i_bat(), dev_get_voltage(I_BAT));
}

void test_convert_map(void)
{
    //TIMER_SAFE_RESET(temp_test_timer, 1000, TIMER_OPT_PERIOD, cb_test_timer, NULL);
    temp_test_timer = bat_timer_reset_ext(temp_test_timer, "temp_test_timer", 1000, TIMER_OPT_PERIOD, cb_test_timer);
    bat_timer_start(temp_test_timer, portMAX_DELAY);

}

//static void cb_test_timer(const timer_t *tm, void* param)
//{
//	if(test_adc < cold_junc_map[ARRAY_SIZE_OF(cold_junc_map)-1].adc_calue){
//		LOGD("adc=%d, temp=%d\r\n", test_adc, convert_map(cold_junc_map, 
//			ARRAY_SIZE_OF(cold_junc_map), test_adc));
//		test_adc++;
//	}else{
//		TIMER_SAFE_DELETE(temp_test_timer);
//	}
//}

//void test_convert_map(void)
//{
//	test_adc = cold_junc_map[0].adc_calue - 1;
//	TIMER_SAFE_RESET(temp_test_timer, 5, TIMER_OPT_PERIOD, cb_test_timer, NULL);
//}

#endif

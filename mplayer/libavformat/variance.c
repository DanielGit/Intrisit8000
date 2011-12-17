
#define MAX_STD_TIMEBASES (60*12+5)

extern float time_adj_data[];
static double error __attribute__ ((aligned (8)));

void variance_cal (double *res, double *dur)
{
   int i;
   for(i=1; i<MAX_STD_TIMEBASES; i++){
     int ticks = (int)(*dur * time_adj_data[i] + 0.5);
     error = *dur - ticks/time_adj_data[i];
     res[i] += error * error;
   }
}


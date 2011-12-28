
static double error __attribute__ ((aligned (8)));

void variance_cal (double *res, double *dur, int cnt)
{
	int i; 
  for(i=1; i<cnt; i++){
    int framerate= get_std_framerate(i);
    int ticks= (int)(*dur*framerate/(1001*12) + 0.5);
    
    error= *dur - ticks*1001*12/(double)framerate;
    res[i] += error*error;
  }
}


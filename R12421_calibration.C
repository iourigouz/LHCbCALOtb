
void R12421_calibration(double gain){
  
  const char* pmnames[]={
    "HA0532","HA0543","HA0524",
    "HA0529","HA0498","HA0488",
    "HA0542","HA0527","HA0539",
    "HA2655","HA2643","HA2651",
    "HA2641","HA2649","HA2635",
    "HA2632","HA2647","HA2645",
    "HA2631"};
  
  int hvchan[]={
    2,7,18,
    6,8,12,
    0,14,13,
    10,16,17,
    3,1,11,
    15,5,4,
    9};
  
  double G0[]={
    336395.2,207728.2,438054.7,396150.0,250767.7,652464.4,336312.7,
    437816.0,257121.0,741194.6,192712.8,206566.5,181444.6,262041.9,
    298180.9,232986.7,708913.7,286902.7,224388.1};
  
  double alpha[]={
    8.04465,8.01546,8.20366,8.09879,8.11895,7.91313,7.99629,
    8.21371,8.04914,8.01010,7.80848,7.95127,7.84793,8.03659,
    7.95498,7.93391,7.96753,7.97771,7.97410};
  
  int npmt=sizeof(G0)/sizeof(double);
  
  for(int chan=0; chan<npmt; ++chan){
    int ind;
    for(int j=0; j<npmt; ++j){
      if(chan==hvchan[j])ind=j;
    }
    //printf("%s %5.3f\n",pmnames[i], pow( (gain/G0[i]), (1.0/alpha[i]) ) );
    const char* nam=pmnames[ind];
    double g0=G0[ind];
    double al=alpha[ind];
    double hv=pow( gain/g0, 1.0/al );
    printf("%2.2d %s %9.1f %8.5f %5.3f\n",chan,nam,g0,al,hv);
  }
}

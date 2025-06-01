const float ref_speeds[6] = {16,44,54,90,88,88};
const float ref_revs[6] = {2000,3500,3000,4000,2500,2500};

float ratio[6];

void init_ratios()
{
  for(int i = 0; i<6; i++)
  {
    ratio[i] = ref_revs[i] /ref_speeds[i]; 
  }
}



int  calculate_gear(float vss, float rpm)
{
  int result = UNDEFINED_GEAR;

  float ratio_in = rpm /vss;
  DEBUG_PORT.println(ratio_in);


  for (int i = 0; i < 6; i++) {
    float lower_bound = ratio[i] * (float)0.90;
    float upper_bound = ratio[i] * (float)1.10;

    if (ratio_in >= lower_bound && ratio_in <= upper_bound) {
      result =  i;  // found a match
      break;
    }
  }

    return result+1;
}

 

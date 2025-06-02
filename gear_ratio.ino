const float ref_speeds[6] = {16,44,54,90,88,88};
const float ref_revs[6] = {2000,3500,3000,4000,2500,2500};



const float ratios_up_bound[6] = {0.00815, 0.01285, 0.01785, 0.0224, 0.02695, 0.0348};
const float rations_dw_boud[6] = {0.00615, 0.01085, 0.01585, 0.0204, 0.02495, 0.0328};


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

  if(vss < 10)
  {
    return result;
  }

  float ratio_in = vss / rpm;
  DEBUG_PORT.println(ratio_in);


  for (int i = 0; i < 6; i++) {
    float lower_bound = rations_dw_boud[i] ;
    float upper_bound = ratios_up_bound[i] ;

    if (ratio_in >= lower_bound && ratio_in <= upper_bound) {
      result =  i;  // found a match
      break;
    }
  }

    return result+1;
}

 

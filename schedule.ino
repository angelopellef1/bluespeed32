/**
 * @file scheduler.ino.cpp
 * @brief Schedules the requestes
 * 
 * 
 * @author AngeloP
 * @date 2025-06-01
 * @version 1.0
 */

#define REQUEST_N   PID_N
bool released = true;

/**
 * tack_ticker
 * Containts init values
ENG_RPM,
SPEED,
GEAR_C,
OIL,
COOLANT,
FUEL_CUSTOM,
    PID_N
 */
 //modififiend scheduler ticks for test on oil and custom header
 const uint32_t task_ticker[REQUEST_N] = 
 {
    1,
    1,
    1,
    60,
    5,
    120,
 };

 /**
  * tack_ticker_cnt
  * live counters of tick
  */
uint32_t task_ticker_cnt[REQUEST_N]; 

void Scheduler_Init()
{
    for(int i = 0; i< REQUEST_N; i++)
    {
        task_ticker_cnt[i] = task_ticker[i];
    }
}

void Scheduler_Lock()
{
    released = false;
}

void Scheduler_release(void)
{
    released = true;
}

obd_pid_states Scheduler_task_calculate(obd_pid_states current_task)
{
    obd_pid_states ret_task = current_task;

    if(released)
    {
        for(int tasks = 0; tasks < REQUEST_N; tasks++)
        {
            if(task_ticker_cnt[tasks] > 0)
            {
                task_ticker_cnt[tasks]--;
            }
            else
            {
                ret_task = (obd_pid_states)tasks;
                task_ticker_cnt[tasks] = task_ticker[tasks];
                released = false;
                break;
            }
        }
    }

    return ret_task;
    
}
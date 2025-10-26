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
FUEL_CONSUMPTION,
PID_N
 */
const uint32_t TASK_TICKER[REQUEST_N] = 
 {
    1,
    1,
    1,
    60,
    5,
    90,
    60,
 };

/**
* First round task ticker
*/
 const uint32_t TASK_TICKER_FIRST[REQUEST_N] = 
 {
    1,
    1,
    1,
    3,
    3,
    5,
    5,
 };

 /**
  * tack_ticker_cnt
  * live counters of tick
  */
uint32_t task_ticker_cnt[REQUEST_N]; 

/**
 * task_ticker_start_cnt
 * counters only executed at start
 * for firsts requests
 */
uint32_t task_ticker_start_cnt[REQUEST_N]; 

void Scheduler_Init()
{
    for(int i = 0; i< REQUEST_N; i++)
    {
        task_ticker_cnt[i] = TASK_TICKER_FIRST[i];
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
                task_ticker_cnt[tasks] = TASK_TICKER[tasks];
                released = false;
                break;
            }
        }
    }

    return ret_task;
    
}
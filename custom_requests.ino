#define DEBUG true
#include "ELMduino.h"



req_states req_state = REQ_OK;
req_stages req_stage = STEP_CHANGE_HEADER;

char command[20] = { '\0' };

int nb_query_state = SEND_COMMAND;
int cycles_count = 0;
const int cycles_max = 20; 

uint8_t hexCharToValue(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else {
    return 0xFF; // Invalid input
  }
}

req_states obdcustom_subaru_oil( float * value)
{
    req_state = REQ_E_WAIT;
    String response, result;


    switch (req_stage)
    {
        case STEP_CHANGE_HEADER:
            req_state = REQ_E_WAIT;

            if (myELM327.nb_rx_state != ELM_GETTING_MSG)
            {
                strcpy(command, "AT SH 7E0");

                myELM327.sendCommand_Blocking(command);
            }
            
           
            if (myELM327.nb_rx_state == ELM_SUCCESS)
            {
                req_stage = STEP_PID;
            }
            else if ((myELM327.nb_rx_state == ELM_NO_RESPONSE) || (myELM327.nb_rx_state == ELM_GENERAL_ERROR) )
            {
                req_state = REQ_E_FAIL;
                Serial.println("fail in header change");

            }
            else
            {
                //Serial.println(myELM327.nb_rx_state);
            }
            
        break;
    
        case STEP_PID:

            if (nb_query_state == SEND_COMMAND)         // We are ready to send a new command
            {
#ifndef SIMULATION_OBD
                myELM327.sendCommand("2101");         // Send the custom PID commnad
#else
                myELM327.sendCommand_fake("2101");         // Send the custom PID commnad
#endif
                nb_query_state = WAITING_RESP;          // Set the query state so we are waiting for response
            }
            else if (nb_query_state == WAITING_RESP)    // Our query has been sent, check for a response
            {
#ifndef SIMULATION_OBD
                myELM327.get_response();
#else
                myELM327.get_response_test();                // Each time through the loop we will check again
#endif
            }
            
            if (myELM327.nb_rx_state == ELM_SUCCESS)    // Our response is fully received, let's get our data
            {      

                //if (myELM327.recBytes > 10)
                {
                        byte rawValue = hexCharToValue(myELM327.payload[71]); 
                        Serial.println("by71 " + String(myELM327.payload[71]));
                        byte rawValue2 = hexCharToValue(myELM327.payload[72]);
                        Serial.println("by71 " + String(myELM327.payload[72]));

                        byte conValue = rawValue<<4 | rawValue2;
                        *value = (float)conValue - 40.0;        // Print the adjusted value
                        nb_query_state = SEND_COMMAND;          // Reset the query state for the next command
                        req_stage = STEP_RESTORE_HEADER;
                 }

            }
            else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
            {                                           // If state == ELM_GETTING_MSG, response is not yet complete. Restart the loop.
                nb_query_state = SEND_COMMAND;          // Reset the query state for the next command
                req_stage = STEP_RESTORE_HEADER;
                myELM327.printError();
                //delay(5000);                            // Wait 5 seconds until we query again
            }
            else
            {
                // If state == ELM_GETTING_MSG, response is not yet complete.
            }
        break;

        case STEP_RESTORE_HEADER:
            if (myELM327.nb_rx_state != ELM_GETTING_MSG)
            {
                myELM327.sendCommand_Blocking("AT SH 7DF");  // Restore default header
            }

            if (myELM327.nb_rx_state == ELM_SUCCESS)
            {
                req_stage = STEP_CHANGE_HEADER; //reset machine
                req_state = REQ_OK;
            }

           
        break;

        
        default:
        break;
    }


    return req_state;

}

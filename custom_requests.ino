#define DEBUG true
#include "ELMduino.h"



req_states req_state = REQ_OK;
req_stages req_stage = STEP_CHANGE_HEADER;

char command[20] = { '\0' };

int nb_query_state = SEND_COMMAND;
int cycles_count = 0;
const int cycles_max = 20; 

uint8_t rawValue,rawValue2;
int16_t conValue;

static int bytepos = 87; //found from real world testing

int test_dec_bytepos(void)
{
    bytepos--;
    return bytepos;
}

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

            if (nb_query_state == SEND_COMMAND) 
            {
                myELM327.sendCommand("2101"); 
                nb_query_state = WAITING_RESP;         
            }
            else if (nb_query_state == WAITING_RESP) 
            {
                myELM327.get_response(); 
            }
            
            if (myELM327.nb_rx_state == ELM_SUCCESS)
            {      

                if (myELM327.recBytes > 10)
                {
                    /*Byte 14 of python script analisys
                        These are char
                        7E8     10 1F 61 01 64 00 46 02
                        7E8     21 88 29 4C 4C 64 51 13
                        7E8     22 1D 00 00 24 0C 2A 54
                        7E8     23 22 21 00 FF 27 BD 3B
                        7E8     24 3D 37 AA>4C<00 00 00
                        But elmduino lib counts all byte including \r 
                        So
                        7E8101F610164004602r
                        7E82188294C4C645113r    36 and 37 are chars of byte value 51 taken as example
                        7E8221D0000240C2A54r
                        7E823222100FF27BD3Br
                        7E8243D37AA4C000000r
                    */
                        byte rawValue = hexCharToValue(myELM327.payload[92]); 
                        byte rawValue2 = hexCharToValue(myELM327.payload[93]);
                        byte conValue = (rawValue<<4)&0xF0 ;
                        conValue |= (0x0F & rawValue2);
                        conValue -= 40;
                        *value = (float)conValue;        
                        nb_query_state = SEND_COMMAND;          
                        req_stage = STEP_RESTORE_HEADER;
                 }

            }
            else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
            {                                                       // If state == ELM_GETTING_MSG, response is not yet complete. Restart the loop.
                nb_query_state = SEND_COMMAND;                      // Reset the query state for the next command
                req_stage = STEP_RESTORE_HEADER;
                myELM327.printError();
                //delay(5000);                                       // Wait 5 seconds until we query again
            }
            else
            {
                // If state == ELM_GETTING_MSG, response is not yet complete.
            }
        break;

        case STEP_RESTORE_HEADER:
            if (myELM327.nb_rx_state != ELM_GETTING_MSG)
            {
                myELM327.sendCommand_Blocking("AT SH 7DF");         // Restore default header
            }

            if (myELM327.nb_rx_state == ELM_SUCCESS)
            {
                req_stage = STEP_CHANGE_HEADER;                     //reset machine
                req_state = REQ_OK;
            }

           
        break;

        
        default:
        break;
    }


    return req_state;

}

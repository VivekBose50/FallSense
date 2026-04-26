#include "event_handler.h"

void handle_event(fall_event_t event){
    
    switch(event)
    {
        case EVENT_FREE_FALL:
            log_info("EVENT: FREE FALL");
            break;

        case EVENT_IMPACT:
            log_info("EVENT: IMPACT");
            break;

        case EVENT_TIMEOUT:
            log_info("EVENT: TIMEOUT");
            break;

        case EVENT_FALL_CONFIRMED:
            log_info("EVENT: FALL CONFIRMED");
            break;

        case EVENT_NONE:
            log_info("EVENT: NONE");
            break;
    }
}
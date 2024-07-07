#ifndef UTILS_H_CMD_BLINKER
#define UTILS_H_CMD_BLINKER

#include <ctype.h>
#include "defines.h"

void info_message(command_message_t* message, char* message_str) {
    uint8_t* args = (uint8_t*)pvPortMalloc(sizeof(char)*(strlen(message_str)+1)); // Null character included
    strcpy((char*)args, message_str);
    message->data = args;
}

void process_command(char* read_buffer) {
    char* params_string = strchr(read_buffer, ' ');
    params_string++;  // We dont' want the space.
    command_message_t message;
    xEventGroupSetBits(task_eventgroup_handle, ALL_TASKS_QUEUE_READY);
    if (strstr(read_buffer, "led") != NULL) {
        // Handling the arguments in case of an `led` command
        xEventGroupClearBits(task_eventgroup_handle, LED_TASK_QUEUE_READY);
        xEventGroupClearBits(task_eventgroup_handle, TRANSMIT_TASK_QUEUE_READY);
        
        char* start_ptr = params_string;
        int val = 0;
        while (!isdigit((int)*start_ptr)) {
            start_ptr++;
        }
        char* end_ptr = start_ptr;
        while (isdigit((int)*end_ptr)) {
            val *= 10;
            val += *end_ptr-'0';
            end_ptr++;
        }
        ESP_LOGI(TAG, "LED args: %d", val);
        message.led_tp = val;
        if (val == 0) {
            info_message(&message, "LED turned off");
        }
        else if (val == 1) {
            info_message(&message, "LED turned on");
        }
        else {
            char blink_message[50];
            sprintf(blink_message, "LED blinking with  half time period of %d ms", val);
            info_message(&message, blink_message);
        }
    } else if (strstr(read_buffer, "echo") != NULL) {
        // Transmit the echo message
        xEventGroupClearBits(task_eventgroup_handle, TRANSMIT_TASK_QUEUE_READY);
        uint8_t* args = (uint8_t*)pvPortMalloc(sizeof(char)*(strlen(params_string)+1)); // Null character included
        strcpy((char*)args, params_string);
        message.led_tp = 0;
        message.data = args;
        ESP_LOGI(TAG, "echo message ready");
    } else {
        xEventGroupClearBits(task_eventgroup_handle, TRANSMIT_TASK_QUEUE_READY);
        message.led_tp = 0;
        info_message(&message, "Invalid command received");
    }

    if (xQueueSend(global_queue_handle, &message, 100) == pdTRUE) {
        ESP_LOGI(TAG, "Task added to the queue successfully");
    } else {
        ESP_LOGE(TAG, "Failed to send message");
    }
}

#endif
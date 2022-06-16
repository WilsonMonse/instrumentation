/*
 *  FM RADIO FOR STM32F746G
 *  Using Seeed RDA5807M Module
 *  
 *  Made by Lucas Godefroy LP MECSE
 *  June 2022
 */

#include "lv_area.h"
#include "mbed.h"
#include "lvgl/lvgl.h"
#include "hal_stm_lvgl/tft/tft.h"
#include "hal_stm_lvgl/touchpad/touchpad.h" 
#include "lv_slider.h"
#include "RDA5807M.h"

#define LVGL_TICK   10      // Time tick value for lvgl in ms (1-10msa)
#define TICKER_TIME 10ms    // modified to miliseconds

RDA5807M radio(PB_9, PB_8); // sda - scl STM32F746G
Ticker ticker;              // Ticker for lvgl                    

static void slider_event_cb(lv_event_t * e); // Event handler
static lv_obj_t * slider_label;

// Label set in global to be used in the event handler:
   lv_obj_t * labelTunning;
   lv_obj_t * slider;

// ----------------------------------------------------------------
// Callbacks for the event handler:

// Callback when button 'SeekUp' is pressed, allow the FM radio to
// seek higher frequency:

static void event_handlerSeekUp(lv_event_t* event)
{
    lv_event_code_t code = lv_event_get_code(event);
    if(code == LV_EVENT_CLICKED) 
    {
        lv_label_set_text(labelTunning, "Tunning running...");
        radio.SeekUp();
        radio.ProcessData();
        lv_slider_set_value(slider, radio.freq/1000, LV_ANIM_OFF);
        lv_label_set_text(labelTunning, "Tuned");
    }
}

// Callback when button 'SeekDown' is pressed, allow the FM radio to
// seek lower frequency:

static void event_handlerSeekDown(lv_event_t* event)
{
    lv_event_code_t code = lv_event_get_code(event);
    if(code == LV_EVENT_CLICKED) 
    {
        lv_label_set_text(labelTunning, "Tunning running...");
        radio.SeekDown();
        radio.ProcessData();
        lv_slider_set_value(slider, radio.freq/1000, LV_ANIM_OFF);
        lv_label_set_text(labelTunning, "Tuned");
    }
}

// Slider callback, permit to update the radio frequency:

static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    char buf[8];
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    radio.Frequency((float)lv_slider_get_value(slider)); // Updating the frequency on the radio
}

// ----------------------------------------------------------------
// Main function of the program:

// Ticker function, specific to LVGL:
void lv_ticker_func(){
    lv_tick_inc(LVGL_TICK); 
}

// main() runs in its own thread in the OS
int main()
{   
    char signalStr[21]; // String receiving the signal strengh value
    char freqStr[21];

    printf("LVGL-"); 
    lv_init();  
    tft_init();                                             //Initialize diplay 
    touchpad_init();                                        //Initialize touchpad
    ticker.attach(callback(&lv_ticker_func),TICKER_TIME);   //Attach callback to ticker

    // Placing global label:
    labelTunning = lv_label_create(lv_scr_act());
    lv_obj_align(labelTunning, LV_ALIGN_TOP_MID, 0, 25);
    lv_label_set_text(labelTunning, " ");

    // Initializing the FM Radio with volume at 15
    radio.Reset();      // Power up and init the radio                                          
    radio.Volume (15);
    radio.Frequency (101.90);

    // Label to display signal strengh:
    lv_obj_t * signalLabel;
    signalLabel = lv_label_create(lv_scr_act());
    lv_obj_align(signalLabel, LV_ALIGN_BOTTOM_MID, 0, -50);

    // Button to seek Higher frequency:
    lv_obj_t * labelSeekUpButton;
    lv_obj_t * buttonSeekUp = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(buttonSeekUp, event_handlerSeekUp, LV_EVENT_ALL, NULL);
    lv_obj_align(buttonSeekUp, LV_ALIGN_LEFT_MID, 0, -25);
    labelSeekUpButton = lv_label_create(buttonSeekUp);
    lv_label_set_text(labelSeekUpButton, "Seek Up");

    // Button to seek Lower Frequency:
    lv_obj_t * labelSeekDownButton;
    lv_obj_t * buttonSeekDown = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(buttonSeekDown, event_handlerSeekDown, LV_EVENT_ALL, NULL);
    lv_obj_align(buttonSeekDown, LV_ALIGN_LEFT_MID, 0, +25);
    labelSeekUpButton = lv_label_create(buttonSeekDown);
    lv_label_set_text(labelSeekDownButton, "Seek Down");
    
    // Slider for setting up the desired frequency
    slider = lv_slider_create(lv_scr_act());
    lv_slider_set_range(slider, 87.00 , 108.00);
    lv_obj_center(slider);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Label below the slider displaying the frequency
    slider_label = lv_label_create(lv_scr_act());
    lv_label_set_text(slider_label, "Select Desired Frequency");
    lv_slider_set_value(slider, 9856, LV_ANIM_OFF);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    while (true){
        lv_task_handler(); 
        thread_sleep_for(LVGL_TICK); 

        // Updating the signal strengh:
        sprintf (signalStr,"Signal Strengh: %d",radio.signal);
        lv_label_set_text (signalLabel, signalStr);

        sprintf (freqStr,"Freq: %d kHz",((int)radio.freq));
        lv_label_set_text (slider_label, freqStr);

        radio.ProcessData();
    }
}
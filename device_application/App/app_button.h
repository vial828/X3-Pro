#ifndef __BUTTON_H
#define __BUTTON_H

typedef enum{
    btn_down = 0,
    btn_up = 1,
    btn_max = 2,
}btn_status_e;


typedef enum{
    door_close = 0,
    door_base,
    door_boost,
    door_error,
}hall_status_e;


btn_status_e app_read_button(void);
void button_reset(void);
void app_button_init(void);
void app_button_task(void);
hall_status_e app_get_hall_door_status(void);
void app_hall_door_init(void);
void app_hall_door_task(void);

#endif


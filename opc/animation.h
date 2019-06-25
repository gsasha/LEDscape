#ifndef LEDSCAPE_OPC_ANIMATION_H
#define LEDSCAPE_OPC_ANIMATION_H

typedef struct {
} animation_state_t;

void init_animation_state(animation_state_t *animation_state);
void start_animation_thread(animation_state_t* animation_state);
void stop_animation_thread(animation_state_t* animation_state);

void set_animation_mode_all(animation_state_t *animation_state,
                            char *animation_mode);

void set_animation_mode_strip(animation_state_t *animation_state, int strip,
                              char *animation_mode);

void animation_disable_all(animation_state_t *animation_state, double seconds);

void animation_disable_strip(animation_state_t *animation_state, double seconds,
                             int strip);

#endif // LEDSCAPE_OPC_ANIMATION_H

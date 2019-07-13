#ifndef LEDSCAPE_OPC_ANIMATION_H
#define LEDSCAPE_OPC_ANIMATION_H

#include <pthread.h>
#include <sys/time.h>

#include "opc/server-config.h"

#ifdef __cplusplus
extern "C" {
#endif

struct animation_state_t;
typedef struct animation_state_t animation_state_t;

animation_state_t *create_animation_state(server_config_t *server_config);

void start_animation_thread(animation_state_t* animation_state);
void join_animation_thread(animation_state_t* animation_state);

void set_animation_mode_all(animation_state_t *animation_state,
                            char *animation_mode);

void set_animation_mode_strip(animation_state_t *animation_state, int strip,
                              char *animation_mode);

void animation_disable_all(animation_state_t *animation_state, double seconds);

void animation_disable_strip(animation_state_t *animation_state, double seconds,
                             int strip);
#ifdef __cplusplus
}
#endif


#endif // LEDSCAPE_OPC_ANIMATION_H

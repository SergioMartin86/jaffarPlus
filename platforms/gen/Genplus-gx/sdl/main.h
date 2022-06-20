
#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdint.h>

#define MAX_INPUTS 8

extern __thread  int debug_on;
extern __thread  int log_error;
extern int sdl_input_update(void);

#ifdef __cplusplus
extern "C" {
#endif

extern __thread  uint16_t jaffarInput;
void initSDLWindow();
void sdl_video_update();

#ifdef __cplusplus
}
#endif

#endif /* _MAIN_H_ */

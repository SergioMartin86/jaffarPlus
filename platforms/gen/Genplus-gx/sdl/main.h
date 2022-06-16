
#ifndef _MAIN_H_
#define _MAIN_H_

#define MAX_INPUTS 8

extern int debug_on;
extern int log_error;
extern int sdl_input_update(void);

#ifdef __cplusplus
extern "C" {
#endif

void initSDLWindow();
void sdl_video_update();

#ifdef __cplusplus
}
#endif

#endif /* _MAIN_H_ */

#ifndef _ERROR_H_
#define _ERROR_H_

/* Function prototypes */
#ifdef __cplusplus
extern "C" {
#endif
void error_init(void);
void error_shutdown(void);
void error(char *format, ...);
#ifdef __cplusplus
}
#endif
#endif /* _ERROR_H_ */


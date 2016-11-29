#ifndef __TIME_H__
#define __TIME_H__

#define HZ	100

// to send end of interruption from newly created thread
#define PIT_IRQ     0

void time_setup(void);

#endif /*__TIME_H__*/

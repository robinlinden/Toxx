#ifndef FILTER_AUDIO_H
#define FILTER_AUDIO_H

#include <stdbool.h>

#ifdef AUDIO_FILTERING
#include <filter_audio.h>
#else
#include <stdint.h>
typedef uint8_t Filter_Audio;
#endif

extern Filter_Audio *f_a;

bool filter_audio_check(void);

#endif

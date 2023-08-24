#pragma once
#ifndef _TOUCH_H_
#define _TOUCH_H_

#if ENABLE_TOUCH_FT5216

#include "libHidDisplayDesc.h"

void touch_start (const int intPin);
int touch_isPressed ();
int touch_process (touch_t *touch);


#endif

#endif

/*
 * Copyright 2024, cafeina <cafeina@world>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef __STARTUP_DEFS_H
#define __STARTUP_DEFS_H

#include <SupportDefs.h>

#define kAppName "StartUp"
#define kAppSignature "application/x-vnd.Haiku-StartUp"
#define kAppVersionStr "0.3.1"
#define kAppHomePage "https://codeberg.org/cafeina/StartUp"

#if defined(DEBUG) || defined(_DEBUG)
#define __trace(x, ...) fprintf(stderr, kAppName " @ %s: " x, __func__, ##__VA_ARGS__)
#else
#define __trace(x, ...)
#endif


#endif /* __STARTUP_DEFS_H */

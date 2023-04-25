/*
 *
 * KIAVC versioning.
 *
 * Author: Lorenzo Miniero (lminiero@gmail.com)
 *
 */

#ifndef __KIAVC_VERSION_H
#define __KIAVC_VERSION_H

#define KIAVC_VERSION_MAJOR		0
#define KIAVC_VERSION_MINOR		1
#define KIAVC_VERSION_PATCH		0

#define KIAVC_VERSION_STRINGIFY_(major, minor, patch) #major "." #minor "." #patch
#define KIAVC_VERSION_STRINGIFY(major, minor, patch) KIAVC_VERSION_STRINGIFY_(major, minor, patch)

#define KIAVC_VERSION_STRING KIAVC_VERSION_STRINGIFY(KIAVC_VERSION_MAJOR, KIAVC_VERSION_MINOR, KIAVC_VERSION_PATCH)

#endif

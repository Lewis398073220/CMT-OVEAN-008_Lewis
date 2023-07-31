/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.6 */

#ifndef PB_FILES_MEDIA_PB_H_INCLUDED
#define PB_FILES_MEDIA_PB_H_INCLUDED
#include <pb_0_4_6.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Enum definitions */
typedef enum _MediaControl { 
    MediaControl_PLAY = 0, 
    MediaControl_PAUSE = 1, 
    MediaControl_NEXT = 2, 
    MediaControl_PREVIOUS = 3, 
    MediaControl_PLAY_PAUSE = 4 
} MediaControl;

/* Struct definitions */
typedef struct _IssueMediaControl { 
    MediaControl control;
} IssueMediaControl;


/* Helper constants for enums */
#define _MediaControl_MIN MediaControl_PLAY
#define _MediaControl_MAX MediaControl_PLAY_PAUSE
#define _MediaControl_ARRAYSIZE ((MediaControl)(MediaControl_PLAY_PAUSE+1))


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define IssueMediaControl_init_default           {_MediaControl_MIN}
#define IssueMediaControl_init_zero              {_MediaControl_MIN}

/* Field tags (for use in manual encoding/decoding) */
#define IssueMediaControl_control_tag            1

/* Struct field encoding specification for nanopb */
#define IssueMediaControl_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UENUM,    control,           1)
#define IssueMediaControl_CALLBACK NULL
#define IssueMediaControl_DEFAULT NULL

extern const pb_msgdesc_t IssueMediaControl_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define IssueMediaControl_fields &IssueMediaControl_msg

/* Maximum encoded size of messages (where known) */
#define IssueMediaControl_size                   2

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

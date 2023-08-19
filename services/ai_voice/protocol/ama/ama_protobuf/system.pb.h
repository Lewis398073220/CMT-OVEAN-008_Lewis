/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.6 */

#ifndef PB_FILES_SYSTEM_PB_H_INCLUDED
#define PB_FILES_SYSTEM_PB_H_INCLUDED
#include <pb_0_4_6.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Enum definitions */
typedef enum _ResetConnection_ResetReason { 
    ResetConnection_ResetReason_UNKNOWN = 0, 
    ResetConnection_ResetReason_AAP_REFUSED_MAX_CONNECTIONS_REACHED = 1 
} ResetConnection_ResetReason;

/* Struct definitions */
typedef struct _GetLocales { 
    char dummy_field;
} GetLocales;

typedef struct _KeepAlive { 
    char dummy_field;
} KeepAlive;

typedef struct _RemoveDevice { 
    char dummy_field;
} RemoveDevice;

typedef struct _LaunchApp { 
    char app_id[255];
} LaunchApp;

typedef struct _Locale { 
    char name[8];
} Locale;

typedef struct _ResetConnection { 
    uint32_t timeout;
    bool force_disconnect;
    ResetConnection_ResetReason reset_reason;
} ResetConnection;

typedef struct _SynchronizeSettings { 
    uint32_t timestamp_hi;
    uint32_t timestamp_lo;
} SynchronizeSettings;

typedef struct _Locales { 
    pb_size_t supported_locales_count;
    Locale supported_locales[32];
    bool has_current_locale;
    Locale current_locale;
} Locales;

typedef struct _SetLocale { 
    bool has_locale;
    Locale locale;
} SetLocale;


/* Helper constants for enums */
#define _ResetConnection_ResetReason_MIN ResetConnection_ResetReason_UNKNOWN
#define _ResetConnection_ResetReason_MAX ResetConnection_ResetReason_AAP_REFUSED_MAX_CONNECTIONS_REACHED
#define _ResetConnection_ResetReason_ARRAYSIZE ((ResetConnection_ResetReason)(ResetConnection_ResetReason_AAP_REFUSED_MAX_CONNECTIONS_REACHED+1))


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define ResetConnection_init_default             {0, 0, _ResetConnection_ResetReason_MIN}
#define SynchronizeSettings_init_default         {0, 0}
#define KeepAlive_init_default                   {0}
#define RemoveDevice_init_default                {0}
#define Locale_init_default                      {""}
#define Locales_init_default                     {0, {Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default, Locale_init_default}, false, Locale_init_default}
#define GetLocales_init_default                  {0}
#define SetLocale_init_default                   {false, Locale_init_default}
#define LaunchApp_init_default                   {""}
#define ResetConnection_init_zero                {0, 0, _ResetConnection_ResetReason_MIN}
#define SynchronizeSettings_init_zero            {0, 0}
#define KeepAlive_init_zero                      {0}
#define RemoveDevice_init_zero                   {0}
#define Locale_init_zero                         {""}
#define Locales_init_zero                        {0, {Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero, Locale_init_zero}, false, Locale_init_zero}
#define GetLocales_init_zero                     {0}
#define SetLocale_init_zero                      {false, Locale_init_zero}
#define LaunchApp_init_zero                      {""}

/* Field tags (for use in manual encoding/decoding) */
#define LaunchApp_app_id_tag                     1
#define Locale_name_tag                          1
#define ResetConnection_timeout_tag              1
#define ResetConnection_force_disconnect_tag     2
#define ResetConnection_reset_reason_tag         3
#define SynchronizeSettings_timestamp_hi_tag     1
#define SynchronizeSettings_timestamp_lo_tag     2
#define Locales_supported_locales_tag            1
#define Locales_current_locale_tag               2
#define SetLocale_locale_tag                     1

/* Struct field encoding specification for nanopb */
#define ResetConnection_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UINT32,   timeout,           1) \
X(a, STATIC,   SINGULAR, BOOL,     force_disconnect,   2) \
X(a, STATIC,   SINGULAR, UENUM,    reset_reason,      3)
#define ResetConnection_CALLBACK NULL
#define ResetConnection_DEFAULT NULL

#define SynchronizeSettings_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UINT32,   timestamp_hi,      1) \
X(a, STATIC,   SINGULAR, UINT32,   timestamp_lo,      2)
#define SynchronizeSettings_CALLBACK NULL
#define SynchronizeSettings_DEFAULT NULL

#define KeepAlive_FIELDLIST(X, a) \

#define KeepAlive_CALLBACK NULL
#define KeepAlive_DEFAULT NULL

#define RemoveDevice_FIELDLIST(X, a) \

#define RemoveDevice_CALLBACK NULL
#define RemoveDevice_DEFAULT NULL

#define Locale_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, STRING,   name,              1)
#define Locale_CALLBACK NULL
#define Locale_DEFAULT NULL

#define Locales_FIELDLIST(X, a) \
X(a, STATIC,   REPEATED, MESSAGE,  supported_locales,   1) \
X(a, STATIC,   OPTIONAL, MESSAGE,  current_locale,    2)
#define Locales_CALLBACK NULL
#define Locales_DEFAULT NULL
#define Locales_supported_locales_MSGTYPE Locale
#define Locales_current_locale_MSGTYPE Locale

#define GetLocales_FIELDLIST(X, a) \

#define GetLocales_CALLBACK NULL
#define GetLocales_DEFAULT NULL

#define SetLocale_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, MESSAGE,  locale,            1)
#define SetLocale_CALLBACK NULL
#define SetLocale_DEFAULT NULL
#define SetLocale_locale_MSGTYPE Locale

#define LaunchApp_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, STRING,   app_id,            1)
#define LaunchApp_CALLBACK NULL
#define LaunchApp_DEFAULT NULL

extern const pb_msgdesc_t ResetConnection_msg;
extern const pb_msgdesc_t SynchronizeSettings_msg;
extern const pb_msgdesc_t KeepAlive_msg;
extern const pb_msgdesc_t RemoveDevice_msg;
extern const pb_msgdesc_t Locale_msg;
extern const pb_msgdesc_t Locales_msg;
extern const pb_msgdesc_t GetLocales_msg;
extern const pb_msgdesc_t SetLocale_msg;
extern const pb_msgdesc_t LaunchApp_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define ResetConnection_fields &ResetConnection_msg
#define SynchronizeSettings_fields &SynchronizeSettings_msg
#define KeepAlive_fields &KeepAlive_msg
#define RemoveDevice_fields &RemoveDevice_msg
#define Locale_fields &Locale_msg
#define Locales_fields &Locales_msg
#define GetLocales_fields &GetLocales_msg
#define SetLocale_fields &SetLocale_msg
#define LaunchApp_fields &LaunchApp_msg

/* Maximum encoded size of messages (where known) */
#define GetLocales_size                          0
#define KeepAlive_size                           0
#define LaunchApp_size                           257
#define Locale_size                              9
#define Locales_size                             363
#define RemoveDevice_size                        0
#define ResetConnection_size                     10
#define SetLocale_size                           11
#define SynchronizeSettings_size                 12

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

/***************************************************************************
 *
 * Copyright 2015-2022 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __BT_COMMON_DEFINE_H__
#define __BT_COMMON_DEFINE_H__
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "cmsis_os.h"
#include "hal_aud.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "bt_sys_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Baisc Type
 */

typedef unsigned char  byte;
typedef unsigned char  uint8;
typedef unsigned char  uint8_t;
typedef signed   char  int8;
typedef unsigned short uint16;
typedef signed   short int16;
typedef unsigned int   uint32;
typedef signed   int   int32;

#ifndef BOOL_DEFINED
typedef unsigned int BOOL;
#endif

#ifndef __U32_TYPE
#define __U32_TYPE
typedef unsigned int U32;
#endif

#ifndef __U16_TYPE
#define __U16_TYPE
typedef unsigned short U16;
#endif

#ifndef __U8_TYPE
#define __U8_TYPE
typedef unsigned char U8;
#endif

typedef int S32;
typedef short S16;
typedef char S8;

#ifndef U32_PTR_DEFINED
typedef U32 U32_PTR;
#define U32_PTR_DEFINED
#endif

#ifndef __I32_TYPE
#define __I32_TYPE
typedef unsigned long I32;
#endif

#if XA_INTEGER_SIZE == 4
#ifndef __I16_TYPE
#define __I16_TYPE
typedef unsigned long I16;
#endif
#ifndef __I8_TYPE
#define __I8_TYPE
typedef unsigned long I8;
#endif

#elif XA_INTEGER_SIZE == 2
typedef unsigned short I16;
typedef unsigned short I8;
#elif XA_INTEGER_SIZE == 1
typedef unsigned short I16;
typedef unsigned char I8;
#else
#error No XA_INTEGER_SIZE specified!
#endif

typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TimeT
typedef U32 TimeT;
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAILURE
#define FAILURE 1
#endif

#ifndef INPROGRESS
#define INPROGRESS 2
#endif

#define OUTOFMEMORY 3

#define BUSYSTATUS 4

#define SENDFAILED 5

#define EINVAHNDLE 6

#define EINVACHNNL 7

#define EINVASTATE 8

static inline uint32_t co_round_size(uint32_t a)
{
    return ((a + 3) / 4) * 4;
}

/**
 * byte order
 */

#define HOST_OS_LITTLE_ENDIAN 1

#define CO_UINT16_SPLIT_TO_LE(n) ((uint8_t)((n) & 0xff)), ((uint8_t)(((n) >> 8) & 0xff))

#define CO_UINT32_SPLIT_TO_LE(n) ((uint8_t)((n) & 0xff)), ((uint8_t)(((n) >> 8) & 0xff)), ((uint8_t)(((n) >> 16) & 0xff)), ((uint8_t)(((n) >> 24) & 0xff))

#define CO_UINT16_SPLIT_TO_BE(n) ((uint8_t)(((n) >> 8) & 0xff)), ((uint8_t)((n) & 0xff))

#define CO_UINT32_SPLIT_TO_BE(n) ((uint8_t)(((n) >> 24) & 0xff)), ((uint8_t)(((n) >> 16) & 0xff)), ((uint8_t)(((n) >> 8) & 0xff)), ((uint8_t)((n) & 0xff))

#if HOST_OS_LITTLE_ENDIAN
static inline uint16_t co_host_to_uint16_le(uint16_t n) { return n; }
static inline uint16_t co_uint16_le_to_host(uint16_t n) { return n; }
static inline uint32_t co_host_to_uint32_le(uint32_t n) { return n; }
static inline uint32_t co_uint32_le_to_host(uint32_t n) { return n; }
#else
static inline uint16_t co_host_to_uint16_le(uint16_t n) {
    return ((n & 0xff) << 8) | (n >> 8);
}
static inline uint32_t co_host_to_uint32_le(uint32_t n) {
    return ((n & 0xff) << 24) | ((n & 0xff00) << 8) | ((n & 0xff0000) >> 8) | ((n & 0xff000000) >> 24);
}
static inline uint16_t co_uint16_le_to_host(uint16_t n) {
    return co_host_to_uint16_le(n);
}
static inline uint32_t co_uint32_le_to_host(uint32_t n) {
    return co_host_to_uint32_le(n);
}
#endif

/**
 * list entry
 */

typedef struct _list_entr {
    struct _list_entr *Flink;
    struct _list_entr *Blink;
    unsigned int resv;
} list_entry_t;

#define initialize_list_head(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead) )

#define initialize_list_entry(Entry) (\
    (Entry)->Flink = (Entry)->Blink = 0 )

#define is_entry_available(Entry) (\
    ((Entry)->Flink == 0))

#define is_list_empty(ListHead) (\
    ((ListHead)->Flink == (ListHead)))

#define get_head_list(ListHead) (ListHead)->Flink

#define get_tail_list(ListHead) (ListHead)->Blink

#define get_next_node(Node)     (Node)->Flink

#define get_prior_node(Node)    (Node)->Blink

#define is_node_connected(n) (((n)->Blink->Flink == (n)) && ((n)->Flink->Blink == (n)))
BOOL is_list_circular(list_entry_t * list);

#define list_assert(exp) (ASSERT(exp, "%s %s, %d\n", #exp, __func__, __LINE__))

void _insert_tail_list(list_entry_t * head, list_entry_t * entry);

#define insert_tail_list(a, b) (list_assert(is_list_circular(a)), \
                            _insert_tail_list(a, b), \
                            list_assert(is_list_circular(a)))

void insert_head_list(list_entry_t * head, list_entry_t * entry);
void _insert_head_list(list_entry_t * head, list_entry_t * entry);

#define insert_head_list(a, b) (list_assert(is_list_circular(a)), \
                            _insert_head_list(a, b), \
                            list_assert(is_list_circular(a)))

list_entry_t *remove_head_list(list_entry_t * head);
list_entry_t *_remove_head_list(list_entry_t * head);

#define remove_head_list(a) (list_assert(is_list_circular(a)), \
                            _remove_head_list(a))

void remove_entry_list(list_entry_t * entry);
BOOL is_node_on_list(list_entry_t * head, list_entry_t * node);
U8 get_list_number(list_entry_t * head);
BOOL is_list_circular(list_entry_t * list);
void move_list(list_entry_t * dest, list_entry_t * src);

#define iterate_list_safe(head, cur, next, type) \
    for ( (cur) = (type) get_head_list(head) ; \
          (next) = (type) get_next_node(&(cur)->node), \
            (cur) != (type) (head); \
          (cur) = (next))

#define iterate_list(head, cur, type) \
    for ( (cur) = (type) get_head_list(&(head)) ; \
          (cur) != (type) &(head); \
          (cur) = (type) get_next_node(&(cur)->node) )

#ifndef OFFSETOF
#define OFFSETOF(type, member) ((unsigned int) &((type *)0)->member)
#endif

#ifndef CONTAINER_OF
#define CONTAINER_OF(ptr, type, member) ((type *)( (char *)ptr - OFFSETOF(type,member) ))
#endif

U32 be_to_host32(const U8* ptr);

#define STR_BE32(buff,num) ( (((U8*)buff)[0] = (U8) ((num)>>24)),  \
                              (((U8*)buff)[1] = (U8) ((num)>>16)),  \
                              (((U8*)buff)[2] = (U8) ((num)>>8)),   \
                              (((U8*)buff)[3] = (U8) (num)) )

#define STR_BE16(buff,num) ( (((U8*)buff)[0] = (U8) ((num)>>8)),    \
                              (((U8*)buff)[1] = (U8) (num)) )

#define BEtoHost16(ptr)  (U16)( ((U16) *((U8*)(ptr)) << 8) | \
                                ((U16) *((U8*)(ptr)+1)) )

#define BEtoHost32(ptr)  (U32)( ((U32) *((U8*)(ptr)) << 24)   | \
                                ((U32) *((U8*)(ptr)+1) << 16) | \
                                ((U32) *((U8*)(ptr)+2) << 8)  | \
                                ((U32) *((U8*)(ptr)+3)) )

/* Store value into a buffer in Little Endian format */
#define StoreLE16(buff,num) ( ((buff)[1] = (U8) ((num)>>8)),    \
                              ((buff)[0] = (U8) (num)) )

#define StoreLE32(buff,num) ( ((buff)[3] = (U8) ((num)>>24)),  \
                              ((buff)[2] = (U8) ((num)>>16)),  \
                              ((buff)[1] = (U8) ((num)>>8)),   \
                              ((buff)[0] = (U8) (num)) )

/* Store value into a buffer in Big Endian format */
#define StoreBE16(buff,num) ( ((buff)[0] = (U8) ((num)>>8)),    \
                              ((buff)[1] = (U8) (num)) )

#define StoreBE32(buff,num) ( ((buff)[0] = (U8) ((num)>>24)),  \
                              ((buff)[1] = (U8) ((num)>>16)),  \
                              ((buff)[2] = (U8) ((num)>>8)),   \
                              ((buff)[3] = (U8) (num)) )

#define LEtoHost16(ptr)  (U16)(((U16) *((U8*)(ptr)+1) << 8) | \
        (U16) *((U8*)(ptr)))

/**
 * list node
 */

struct list_node {
    struct list_node *next;
    struct list_node *prev;
};

#define DEF_LIST_HEAD(head) \
    struct list_node head = { &(head), &(head) }

#define INIT_LIST_HEAD(head) do { \
    (head)->next = (head); (head)->prev = (head); \
} while (0)

void colist_addto_head(struct list_node *n, struct list_node *head);
void colist_addto_tail(struct list_node *n, struct list_node *head);
void colist_insert_after(struct list_node *n, struct list_node *head);
void colist_delete(struct list_node *entry);
int colist_is_node_on_list(struct list_node *list, struct list_node *node);
int colist_item_count(struct list_node *list);
struct list_node *colist_get_head(struct list_node *head);
int colist_is_list_empty(struct list_node *head);

#define colist_structure(ptr, type, member) \
    ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

#define colist_iterate(pos, head) \
    for (int __list__node__iter__cnt = ((pos = (head)->next), 0); \
         pos != (head); \
         (pos = pos->next), ITER_CHK(__list__node__iter__cnt))

#define colist_iterate_prev(pos, head) \
    for (int __list__node__iter__cnt = ((pos = (head)->prev), 0); \
         pos != (head); \
         (pos = pos->prev), ITER_CHK(__list__node__iter__cnt))

#define colist_iterate_safe(pos, n, head) \
    for (int __list__node__iter__cnt = ((pos = (head)->next), (n = pos->next), 0); \
         pos != (head); \
         (pos = n), (n = pos->next), ITER_CHK(__list__node__iter__cnt))

#define colist_iterate_entry(pos, type, head, member) \
    for (int __list__node__iter__cnt = ((pos = colist_structure((head)->next, type, member)), 0); \
         &pos->member != (head); \
         (pos = colist_structure(pos->member.next, type, member)), ITER_CHK(__list__node__iter__cnt))

#define colist_iterate_entry_safe(pos, n, type, head, member) \
    for (int __list__node__iter__cnt = ((pos = colist_structure((head)->next, type, member)), \
         (n = colist_structure(pos->member.next, type, member)), 0); \
         &pos->member != (head); \
         (pos = n), (n = colist_structure(n->member.next, type, member)), ITER_CHK(__list__node__iter__cnt))

static inline void ITERM_ASSERT(uintptr_t count)
{
    ASSERT(0, "list too much loop %d", count);
}

#define COLIST_MAX_NODES_ON_LIST 1000
#define ITER_CHK(count) (count>COLIST_MAX_NODES_ON_LIST?ITERM_ASSERT(count):(void)(count++))

struct single_link_node_t {
    struct single_link_node_t *next;
};

struct single_link_head_t {
    struct single_link_node_t head;
    struct single_link_node_t *tail;
    uint32_t size;
};

#define DEF_SINGLE_LINK_HEAD(head) \
    struct single_link_head_t head = {{(struct single_link_node_t *)&(head)}, (struct single_link_node_t *)&(head), 0}

#define INIT_SINGLE_LINK_HEAD(list) do { \
    (list)->tail = (list)->head.next = &(list)->head; (list)->size = 0; \
} while (0)

#define colist_single_link_iterate(pos, list) \
    for (uintptr_t __link__node_next_item = ((pos = (list)->head.next), (uintptr_t)(pos->next)), __list__node__iter__cnt = 0; \
         pos != &((list)->head); \
         (pos = (struct single_link_node_t *)__link__node_next_item), \
         (__link__node_next_item = (uintptr_t)(pos->next)), ITER_CHK(__list__node__iter__cnt))

void colist_single_link_push_head(struct single_link_node_t *new_node, struct single_link_head_t *list);
void colist_single_link_push_tail(struct single_link_node_t *new_node, struct single_link_head_t *list);
void colist_single_link_push_after(struct single_link_node_t *new_node, struct single_link_node_t *exist_node, struct single_link_head_t *list);
void colist_single_link_delete_entry(struct single_link_head_t *list, struct single_link_node_t *entry);
struct single_link_node_t *colist_single_link_delete_entry_after(struct single_link_head_t *list, struct single_link_node_t *entry);
struct single_link_node_t *colist_single_link_delete_head(struct single_link_head_t *list);
int colist_single_link_is_node_on_list(struct single_link_head_t *list, struct single_link_node_t *node);
uint32_t colist_single_link_size(struct single_link_head_t *list);
struct single_link_node_t *colist_single_link_get_head(struct single_link_head_t *list);
struct single_link_node_t *colist_single_link_pop_head(struct single_link_head_t *list);
int colist_single_link_is_list_empty(struct single_link_head_t *list);


/**
 * BLE Debug
 */

const char *DebugMask2Prefix(uint8_t mask);

#define DEBUG_PREFIX_FORMAT         "%s<%s> "
#define DEBUG_SUFFIX_FORMAT         "\n"

#define LOG_HCI_TAG  "[BLE HCI]: "
#define LOG_L2C_TAG  "[BLE L2C]: "
#define LOG_ATT_TAG  "[BLE ATT]: "
#define LOG_GATT_TAG "[BLE GATT]: "
#define LOG_SMP_TAG  "[BLE SMP]: "

#define LOG_GAP_TAG "[BLE GAP]: "
#define LOG_APP_TAG "[BLE APP]: "
#define LOG_BLE_TAG "[BLE]: "
#define LOG_HTP_TAG "[BLE HTP]: "

#define GAP_ERROR   1
#define GAP_OUT     2
#define GATT_ERROR  3
#define GATT_OUT    4
#define ATT_ERROR   5
#define ATT_OUT     6
#define L2C_ERROR   7
#define L2C_OUT     8
#define HCI_ERROR   9
#define HCI_OUT     10
#define SMP_ERROR   11
#define SMP_OUT     12
#define APP_ERROR   13
#define APP_OUT     14
#define PRF_HT_ERROR    15
#define PRF_HT_OUT      16
#define BLE_ERROR       30
#define BLE_OUT         31

#define DebugOut(mask, str, ...)                             \
    do                                                       \
    {                                                        \
        const char *prefix = NULL;                           \
        prefix             = DebugMask2Prefix(mask);         \
        TRACE(1, DEBUG_PREFIX_FORMAT, prefix, __FUNCTION__); \
        TRACE(1, str, ##__VA_ARGS__);                        \
    } while (0)

#if DEBUG_BLE_HCI_DBG
#define BLE_HCI_DBG(str,...)     DebugOut(HCI_OUT, str, ##__VA_ARGS__)
#define BLE_HCI_ERR(str,...)     DebugOut(HCI_ERROR, str, ##__VA_ARGS__)
#define BLE_HCI_FUNC_ENTER()     TRACE(LOG_BLE_TAG"%s line: %d +++\n", __FUNCTION__, __LINE__)
#define BLE_HCI_FUNC_LEAVE()     TRACE(LOG_BLE_TAG"%s line: %d ---\n", __FUNCTION__, __LINE__)
#else
#define BLE_HCI_DBG(str,...)
#define BLE_HCI_ERR(str,...)     DebugOut(HCI_ERROR, str, ##__VA_ARGS__)
#define BLE_HCI_FUNC_ENTER()
#define BLE_HCI_FUNC_LEAVE()
#endif

#if DEBUG_BLE_GAP_DBG
#define BLE_GAP_DBG(str,...)     DebugOut(GAP_OUT, str, ##__VA_ARGS__)
#define BLE_GAP_ERR(str,...)     DebugOut(GAP_ERROR, str, ##__VA_ARGS__)
#define BLE_GAP_FUNC_ENTER()     TRACE(LOG_GAP_TAG"%s line: %d +++\n", __FUNCTION__, __LINE__)
#define BLE_GAP_FUNC_LEAVE()     TRACE(LOG_GAP_TAG"%s line: %d ---\n", __FUNCTION__, __LINE__)
#else
#define BLE_GAP_DBG(str,...)
#define BLE_GAP_ERR(str,...)     DebugOut(GAP_ERROR, str, ##__VA_ARGS__)
#define BLE_GAP_FUNC_ENTER()
#define BLE_GAP_FUNC_LEAVE()
#endif

#if DEBUG_BLE_GATT_DBG
#define BLE_GATT_DBG(str,...)    DebugOut(GATT_OUT, str, ##__VA_ARGS__)
#define BLE_GATT_ERR(str,...)    DebugOut(GATT_ERROR, str, ##__VA_ARGS__)
#define BLE_GATT_FUNC_ENTER()    TRACE(LOG_GATT_TAG"%s line: %d +++\n", __FUNCTION__, __LINE__)
#define BLE_GATT_FUNC_LEAVE()    TRACE(LOG_GATT_TAG"%s line: %d ---\n", __FUNCTION__, __LINE__)
#else
#define BLE_GATT_DBG(str,...)
#define BLE_GATT_ERR(str,...)    DebugOut(GATT_ERROR, str, ##__VA_ARGS__)
#define BLE_GATT_FUNC_ENTER()
#define BLE_GATT_FUNC_LEAVE()
#endif

#if DEBUG_BLE_ATT_DBG
#define BLE_ATT_DBG(str,...)     DebugOut(ATT_OUT, str, ##__VA_ARGS__)
#define BLE_ATT_ERR(str,...)     DebugOut(ATT_ERROR, str, ##__VA_ARGS__)
#define BLE_ATT_FUNC_ENTER()     TRACE(LOG_ATT_TAG"%s line: %d +++\n", __FUNCTION__, __LINE__)
#define BLE_ATT_FUNC_LEAVE()     TRACE(LOG_ATT_TAG"%s line: %d ---\n", __FUNCTION__, __LINE__)
#else
#define BLE_ATT_DBG(str,...)
#define BLE_ATT_ERR(str,...)     DebugOut(ATT_ERROR, str, ##__VA_ARGS__)
#define BLE_ATT_FUNC_ENTER()
#define BLE_ATT_FUNC_LEAVE()
#endif

#if DEBUG_BLE_L2C_DBG
#define BLE_L2C_DBG(str,...)     DebugOut(L2C_OUT, str, ##__VA_ARGS__)
#define BLE_L2C_ERR(str,...)     DebugOut(L2C_ERROR, str, ##__VA_ARGS__)
#define BLE_L2C_FUNC_ENTER()     TRACE(LOG_L2C_TAG"%s line: %d +++\n", __FUNCTION__, __LINE__)
#define BLE_L2C_FUNC_LEAVE()     TRACE(LOG_L2C_TAG"%s line: %d ---\n", __FUNCTION__, __LINE__)
#else
#define BLE_L2C_DBG(str,...)
#define BLE_L2C_ERR(str,...)     DebugOut(L2C_ERROR, str, ##__VA_ARGS__)
#define BLE_L2C_FUNC_ENTER()
#define BLE_L2C_FUNC_LEAVE()
#endif

#if DEBUG_BLE_SMP_DBG
#define BLE_SMP_DBG(str,...)     DebugOut(SMP_OUT, str, ##__VA_ARGS__)
#define BLE_SMP_ERR(str,...)     DebugOut(SMP_ERROR, str, ##__VA_ARGS__)
#define BLE_SMP_FUNC_ENTER()     TRACE(LOG_SMP_TAG"%s line: %d +++\n", __FUNCTION__, __LINE__)
#define BLE_SMP_FUNC_LEAVE()     TRACE(LOG_SMP_TAG"%s line: %d ---\n", __FUNCTION__, __LINE__)
#else
#define BLE_SMP_DBG(str,...) 
#define BLE_SMP_ERR(str,...)     DebugOut(SMP_ERROR, str, ##__VA_ARGS__)
#define BLE_SMP_FUNC_ENTER()
#define BLE_SMP_FUNC_LEAVE()
#endif

#if DEBUG_BLE_APP_DBG
#define BLE_APP_DBG(str,...)     DebugOut(APP_OUT, str, ##__VA_ARGS__)
#define BLE_APP_ERR(str,...)     DebugOut(APP_ERROR, str, ##__VA_ARGS__)
#define BLE_APP_FUNC_ENTER()     TRACE(LOG_APP_TAG"%s line: %d +++\n", __FUNCTION__, __LINE__)
#define BLE_APP_FUNC_LEAVE()     TRACE(LOG_APP_TAG"%s line: %d ---\n", __FUNCTION__, __LINE__)
#else
#define BLE_APP_DBG(str,...)
#define BLE_APP_ERR(str,...)     DebugOut(APP_ERROR, str, ##__VA_ARGS__)
#define BLE_APP_FUNC_ENTER()
#define BLE_APP_FUNC_LEAVE()
#endif

#if DEBUG_BLE_PRF_DBG
#define BLE_PRF_HP_DBG(str,...)  DebugOut(PRF_HT_OUT, str, ##__VA_ARGS__)
#define BLE_PRF_HP_ERR(str,...)  DebugOut(PRF_HT_ERROR, str, ##__VA_ARGS__)
#define BLE_PRF_HP_FUNC_ENTER()  TRACE(LOG_HTP_TAG"%s line: %d +++\n", __FUNCTION__, __LINE__)
#define BLE_PRF_HP_FUNC_LEAVE()  TRACE(LOG_HTP_TAG"%s line: %d ---\n", __FUNCTION__, __LINE__)
#else
#define BLE_PRF_HP_DBG(str,...)
#define BLE_PRF_HP_ERR(str,...)  DebugOut(PRF_HT_ERROR, str, ##__VA_ARGS__)
#define BLE_PRF_HP_FUNC_ENTER()
#define BLE_PRF_HP_FUNC_LEAVE()
#endif

#if DEBUG_BLE_BLE_DBG
#define BLE_DBG(str,...)         DebugOut(BLE_OUT, str, ##__VA_ARGS__)
#define BLE_ERR(str,...)         DebugOut(BLE_ERROR, str, ##__VA_ARGS__)
#define BLE_FUNC_ENTER()         TRACE(LOG_BLE_TAG"%s line: %d +++\n", __FUNCTION__, __LINE__)
#define BLE_FUNC_LEAVE()         TRACE(LOG_BLE_TAG"%s line: %d ---\n", __FUNCTION__, __LINE__)
#define BLE_DUMP8(x,y,z)         DUMP8(x,y,z)
#else
#define BLE_DBG(str,...)
#define BLE_ERR(str,...)         DebugOut(BLE_ERROR, str, ##__VA_ARGS__)
#define BLE_FUNC_ENTER() 
#define BLE_FUNC_LEAVE()
#define BLE_DUMP8(x,y,z)
#endif

/**
 * error code
 */

typedef uint8_t btif_error_code_t;

#define BTIF_BEC_NO_ERROR             0x00
#define BTIF_BEC_UNKNOWN_HCI_CMD      0x01
#define BTIF_BEC_NO_CONNECTION        0x02
#define BTIF_BEC_HARDWARE_FAILURE     0x03
#define BTIF_BEC_PAGE_TIMEOUT         0x04
#define BTIF_BEC_AUTHENTICATE_FAILURE 0x05
#define BTIF_BEC_MISSING_KEY          0x06
#define BTIF_BEC_MEMORY_FULL          0x07
#define BTIF_BEC_CONNECTION_TIMEOUT   0x08
#define BTIF_BEC_MAX_CONNECTIONS      0x09
#define BTIF_BEC_MAX_SCO_CONNECTIONS  0x0a
#define BTIF_BEC_ACL_ALREADY_EXISTS   0x0b
#define BTIF_BEC_COMMAND_DISALLOWED   0x0c
#define BTIF_BEC_LIMITED_RESOURCE     0x0d
#define BTIF_BEC_SECURITY_ERROR       0x0e
#define BTIF_BEC_PERSONAL_DEVICE      0x0f
#define BTIF_BEC_HOST_TIMEOUT         0x10
#define BTIF_BEC_UNSUPPORTED_FEATURE  0x11
#define BTIF_BEC_INVALID_HCI_PARM     0x12
#define BTIF_BEC_USER_TERMINATED      0x13
#define BTIF_BEC_LOW_RESOURCES        0x14
#define BTIF_BEC_POWER_OFF            0x15
#define BTIF_BEC_LOCAL_TERMINATED     0x16
#define BTIF_BEC_REPEATED_ATTEMPTS    0x17
#define BTIF_BEC_PAIRING_NOT_ALLOWED  0x18
#define BTIF_BEC_UNKNOWN_LMP_PDU      0x19
#define BTIF_BEC_UNSUPPORTED_REMOTE   0x1a
#define BTIF_BEC_SCO_OFFSET_REJECT    0x1b
#define BTIF_BEC_SCO_INTERVAL_REJECT  0x1c
#define BTIF_BEC_SCO_AIR_MODE_REJECT  0x1d
#define BTIF_BEC_INVALID_LMP_PARM     0x1e
#define BTIF_BEC_UNSPECIFIED_ERR      0x1f
#define BTIF_BEC_UNSUPPORTED_LMP_PARM 0x20
#define BTIF_BEC_ROLE_CHG_NOT_ALLOWED 0x21
#define BTIF_BEC_LMP_RESPONSE_TIMEOUT 0x22
#define BTIF_BEC_LMP_TRANS_COLLISION  0x23
#define BTIF_BEC_LMP_PDU_NOT_ALLOWED  0x24
#define BTIF_BEC_ENCRYP_MODE_NOT_ACC  0x25
#define BTIF_BEC_UNIT_KEY_USED        0x26
#define BTIF_BEC_QOS_NOT_SUPPORTED    0x27
#define BTIF_BEC_INSTANT_PASSED       0x28
#define BTIF_BEC_PAIR_UNITKEY_NO_SUPP 0x29
#define BTIF_BEC_NOT_FOUND            0xf1
#define BTIF_BEC_REQUEST_CANCELLED    0xf2
#define BTIF_BEC_INVALID_SDP_PDU      0xd1
#define BTIF_BEC_SDP_DISCONNECT       0xd2
#define BTIF_BEC_SDP_NO_RESOURCES     0xd3
#define BTIF_BEC_SDP_INTERNAL_ERR     0xd4
#define BTIF_BEC_STORE_LINK_KEY_ERR   0xe0
#define BTIF_BEC_BT_LINK_REAL_DISCONNECTED 0xb8
#define BTIF_BEC_BT_CANCEL_PAGE       0xb9

//BES vendor error code
#define BT_ECODE_DISCONNECT_ITSELF      0xba
#define BT_ECODE_IBRT_SLAVE_CLEANUP     0xbb
#define BT_ECODE_SDP_OPEN_TIMEOUT       0xbc
#define BT_ECODE_SDP_ClIENT_TX_TIMEOUT  0xbd

/**
 * bt address
 */

#define BTIF_BD_ADDR_SIZE   6
#define BTIF_LINK_KEY_SIZE  16

struct bdaddr_t {
    uint8_t address[6];
}__attribute__ ((packed));

typedef struct bdaddr_t bt_bdaddr_t;

typedef enum {
    BT_ADDR_TYPE_PUBLIC = 0x00,
    BT_ADDR_TYPE_RANDOM = 0x01,
    BT_ADDR_TYPE_PUBLIC_IDENTITY = 0x02,
    BT_ADDR_TYPE_RANDOM_IDENTITY = 0x03,
} bt_addr_type_t;

typedef struct ble_dev_addr_t {
    uint8_t address[6];
    uint8_t addr_type;
} __attribute__ ((packed)) ble_dev_addr_t;

typedef struct bt_remver_t
{
    uint8_t     vers;
    uint16_t    compid;
    uint16_t    subvers;
} __attribute__((packed)) bt_remver_t;

/**
 * bt device
 */

#ifndef BT_DEVICE_NUM
#if defined(IBRT_V2_MULTIPOINT)
#define BT_DEVICE_NUM 2
#elif defined(__BT_ONE_BRING_TWO__)
#define BT_DEVICE_NUM 2
#else
#define BT_DEVICE_NUM 1
#endif
#endif

enum BT_DEVICE_ID_T {
    BT_DEVICE_ID_1 = 0,
    BT_DEVICE_ID_2 = 1,
    BT_DEVICE_ID_3 = 2,
    BT_DEVICE_ID_N = BT_DEVICE_NUM,
    BT_DEVICE_TWS_ID = 0x0f,
    BT_DEVICE_AUTO_CHOICE_ID = 0xee,
    BT_DEVICE_SEND_AVRCP_PLAY = 0xf0,
    BT_DEVICE_INVALID_ID = 0xff,
};

#ifdef BT_SOURCE
#if defined(BT_MULTI_SOURCE)
#define BT_SOURCE_DEVICE_NUM 2
#else
#define BT_SOURCE_DEVICE_NUM 1
#endif
#endif

#ifndef BT_SOURCE_DEVICE_NUM
#define BT_SOURCE_DEVICE_NUM 0
#endif

#define BT_SOURCE_DEVICE_ID_BASE 0x10

#define BT_INVALID_CONN_HANDLE 0xFFFF

enum BT_SOURCE_DEVICE_ID_T {
    BT_SOURCE_DEVICE_ID_1 = BT_SOURCE_DEVICE_ID_BASE,
    BT_SOURCE_DEVICE_ID_2 = BT_SOURCE_DEVICE_ID_BASE + 1,
    BT_SOURCE_DEVICE_ID_N = BT_SOURCE_DEVICE_ID_BASE + BT_SOURCE_DEVICE_NUM,
    BT_SOURCE_DEVICE_INVALID_ID = 0xff,
};

#ifndef BT_ACL_MAX_LINK_NUMS
#if defined(IBRT_V2_MULTIPOINT) || defined(BT_MULTI_SOURCE)
#define BT_ACL_MAX_LINK_NUMS   0x03
#else
#define BT_ACL_MAX_LINK_NUMS   0x02
#endif
#endif

#define ble_buffer_malloc(size) ble_buffer_malloc_with_ca((size), (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__)
#define ble_buffer_free(buffer) ble_buffer_free_with_ca((buffer), (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__)
void *ble_buffer_malloc_with_ca(uint16_t size, uint32_t ca, uint32_t line);
void ble_buffer_free_with_ca(void *buffer, uint32_t ca, uint32_t line);

#ifdef BT_APP_USE_COHEAP
#define bt_app_buffer_malloc(size) bt_app_buffer_malloc_with_ca((size), (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__)
#define bt_app_buffer_free(buffer) bt_app_buffer_free_with_ca((buffer), (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__)
void *bt_app_buffer_malloc_with_ca(uint16_t size, uint32_t ca, uint32_t line);
void bt_app_buffer_free_with_ca(void *buffer, uint32_t ca, uint32_t line);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __BT_COMMON_DEFINE_H__ */

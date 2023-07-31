/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "app_utils.h"
#include "app_thread.h"
#include <string.h>

#ifndef APP_THREAD_STACK_SIZE
#define APP_THREAD_STACK_SIZE 3072
#endif

static APP_MOD_HANDLER_T mod_handler[APP_MODUAL_NUM];
#if !defined(USE_BASIC_THREADS)

static void app_thread(void const *argument);
osThreadDef(app_thread, osPriorityHigh, 1, APP_THREAD_STACK_SIZE, "app_thread");

osMailQDef (app_mailbox, APP_MAILBOX_MAX, APP_MESSAGE_BLOCK);
static osMailQId app_mailbox = NULL;
osThreadId app_thread_tid;

static int app_mailbox_init(void)
{
    app_mailbox = osMailCreate(osMailQ(app_mailbox), NULL);
    if (app_mailbox == NULL)  {
        TRACE(0,"Failed to Create app_mailbox\n");
        return -1;
    }
    return 0;
}

int app_mailbox_put(APP_MESSAGE_BLOCK* msg_src)
{
    osStatus status;

    APP_MESSAGE_BLOCK *msg_p = NULL;

    msg_p = (APP_MESSAGE_BLOCK*)osMailAlloc(app_mailbox, 0);

    if (!msg_p){
        osEvent evt;
        TRACE_IMM(0,"osMailAlloc error dump");
        for (uint8_t i=0; i<APP_MAILBOX_MAX; i++){
            evt = osMailGet(app_mailbox, 0);
            if (evt.status == osEventMail) {
                TRACE_IMM(9,"cnt:%d mod:%d src:%08x tim:%d id:%8x ptr:%08x para:%08x/%08x/%08x",
                            i,
                            ((APP_MESSAGE_BLOCK *)(evt.value.p))->mod_id,
                            ((APP_MESSAGE_BLOCK *)(evt.value.p))->src_thread,
                            ((APP_MESSAGE_BLOCK *)(evt.value.p))->system_time,
                            ((APP_MESSAGE_BLOCK *)(evt.value.p))->msg_body.message_id,
                            ((APP_MESSAGE_BLOCK *)(evt.value.p))->msg_body.message_ptr,
                            ((APP_MESSAGE_BLOCK *)(evt.value.p))->msg_body.message_Param0,
                            ((APP_MESSAGE_BLOCK *)(evt.value.p))->msg_body.message_Param1,
                            ((APP_MESSAGE_BLOCK *)(evt.value.p))->msg_body.message_Param2);
            }else{
                TRACE_IMM(2,"cnt:%d %d", i, evt.status);
                break;
            }
        }
        TRACE_IMM(0,"osMailAlloc error dump end");
    }

    ASSERT(msg_p, "osMailAlloc error");
    msg_p->src_thread = (uint32_t)osThreadGetId();
    msg_p->dest_thread = (uint32_t)NULL;
    msg_p->system_time = hal_sys_timer_get();
    msg_p->mod_id = msg_src->mod_id;
    msg_p->msg_body.message_id = msg_src->msg_body.message_id;
    msg_p->msg_body.message_ptr = msg_src->msg_body.message_ptr;
    msg_p->msg_body.message_Param0 = msg_src->msg_body.message_Param0;
    msg_p->msg_body.message_Param1 = msg_src->msg_body.message_Param1;
    msg_p->msg_body.message_Param2 = msg_src->msg_body.message_Param2;

    status = osMailPut(app_mailbox, msg_p);
    return (int)status;
}

int app_mailbox_free(APP_MESSAGE_BLOCK* msg_p)
{
    osStatus status;

    status = osMailFree(app_mailbox, msg_p);

    return (int)status;
}

int app_mailbox_get(APP_MESSAGE_BLOCK** msg_p)
{
    osEvent evt;
    evt = osMailGet(app_mailbox, osWaitForever);
    if (evt.status == osEventMail) {
        *msg_p = (APP_MESSAGE_BLOCK *)evt.value.p;
        return 0;
    }
    return -1;
}

static void app_thread(void const *argument)
{
    while(1){
        APP_MESSAGE_BLOCK *msg_p = NULL;

        if (!app_mailbox_get(&msg_p)) {
            if (msg_p->mod_id < APP_MODUAL_NUM) {
                if (mod_handler[msg_p->mod_id]) {
                    int ret = mod_handler[msg_p->mod_id](&(msg_p->msg_body));
                    if (ret)
                        TRACE(2,"%s, mod_handler[%d] ret=%d", __func__, msg_p->mod_id, ret);
                }
            }
            app_mailbox_free(msg_p);
        }
    }
}

int app_os_init(void)
{
    if (app_mailbox_init())
        return -1;

    app_thread_tid = osThreadCreate(osThread(app_thread), NULL);
    if (app_thread_tid == NULL)  {
        TRACE(0,"Failed to Create app_thread\n");
        return 0;
    }
    return 0;
}
#else
static void app_thread(void const *argument);
osThreadDef(app_thread, osPriorityAboveNormal, 1, APP_THREAD_STACK_SIZE, "app_thread");

static osMailQId app_mailbox = NULL;
osMailQDef (app_mailbox, APP_MAILBOX_MAX, APP_MESSAGE_BLOCK);
static osMailQId app_mailbox_level0 = NULL;
osMailQDef (app_mailbox_level0, APP_MAILBOX_MAX/4, APP_MESSAGE_BLOCK);
static osMailQId app_mailbox_level1 = NULL;
osMailQDef (app_mailbox_level1, APP_MAILBOX_MAX, APP_MESSAGE_BLOCK);
static osMailQId app_mailbox_level2 = NULL;
osMailQDef (app_mailbox_level2, APP_MAILBOX_MAX, APP_MESSAGE_BLOCK);
osThreadId app_thread_tid;
uint32_t millisec = osWaitForever;

//static osMutexId app_mutex_id = NULL;
//osMutexDef(app_mutex);

static int app_mailbox_init(void)
{
    app_mailbox = osMailCreate(osMailQ(app_mailbox), NULL);
    if (app_mailbox == NULL)  {
        TRACE(0,"Failed to Create app_mailbox\n");
        return -1;
    }

    app_mailbox_level0 = osMailCreate(osMailQ(app_mailbox_level0), NULL);
    if (app_mailbox_level0 == NULL)  {
        TRACE(0,"Failed to Create app_mailbox_level0\n");
        return -1;
    }

    app_mailbox_level1 = osMailCreate(osMailQ(app_mailbox_level1), NULL);
    if (app_mailbox_level1 == NULL)  {
        TRACE(0,"Failed to Create app_mailbox_level1\n");
        return -1;
    }

    app_mailbox_level2 = osMailCreate(osMailQ(app_mailbox_level2), NULL);
    if (app_mailbox_level2 == NULL)  {
        TRACE(0,"Failed to Create app_mailbox_level2\n");
        return -1;
    }
    return 0;
}

int app_mailbox_put(APP_MESSAGE_BLOCK* msg_src)
{
    osStatus status;
//    osMutexWait(app_mutex_id, osWaitForever);

    APP_MESSAGE_BLOCK *msg_p = NULL;

    msg_p = (APP_MESSAGE_BLOCK*)osMailAlloc(app_mailbox, 0);

    if (!msg_p){
        osEvent evt;
        TRACE_IMM(0,"osMailAlloc error dump");
        for (uint8_t i=0; i<APP_MAILBOX_MAX; i++){
            evt = osMailGet(app_mailbox, 0);
            if (evt.status == osEventMail) {
                TRACE_IMM(9,"cnt:%d mod:%d level:%d src:%08x tim:%d id:%8x ptr:%08x para:%08x/%08x/%08x/%08x",
                            i,
                            ((APP_MESSAGE_BLOCK *)(evt.value.p))->mod_id,
                            ((APP_MESSAGE_BLOCK *)(evt.value.p))->mod_level,
                            ((APP_MESSAGE_BLOCK *)(evt.value.p))->src_thread,
                            ((APP_MESSAGE_BLOCK *)(evt.value.p))->system_time,
                            ((APP_MESSAGE_BLOCK *)(evt.value.p))->msg_body.message_id,
                            ((APP_MESSAGE_BLOCK *)(evt.value.p))->msg_body.message_ptr,
                            ((APP_MESSAGE_BLOCK *)(evt.value.p))->msg_body.message_Param0,
                            ((APP_MESSAGE_BLOCK *)(evt.value.p))->msg_body.message_Param1,
                            ((APP_MESSAGE_BLOCK *)(evt.value.p))->msg_body.message_Param2,
                            (uint32_t)((APP_MESSAGE_BLOCK *)(evt.value.p))->msg_body.message_Param3);
            }else{
                TRACE_IMM(2,"cnt:%d %d", i, evt.status);
                break;
            }
        }
        TRACE_IMM(0,"osMailAlloc error dump end");
    }

    ASSERT(msg_p, "osMailAlloc error");
    msg_p->src_thread = (uint32_t)osThreadGetId();
    msg_p->dest_thread = (uint32_t)NULL;
    msg_p->system_time = hal_sys_timer_get();
    msg_p->mod_id = msg_src->mod_id;
    msg_p->mod_level = msg_src->mod_level;
    msg_p->msg_body.message_id = msg_src->msg_body.message_id;
    msg_p->msg_body.message_ptr = msg_src->msg_body.message_ptr;
    msg_p->msg_body.message_Param0 = msg_src->msg_body.message_Param0;
    msg_p->msg_body.message_Param1 = msg_src->msg_body.message_Param1;
    msg_p->msg_body.message_Param2 = msg_src->msg_body.message_Param2;
    msg_p->msg_body.message_Param3 = msg_src->msg_body.message_Param3;
    msg_p->msg_body.p = msg_src->msg_body.p;

    status = osMailPut(app_mailbox, msg_p);
//    osMutexRelease(app_mutex_id);
    return (int)status;
}

int app_mailbox_process(APP_MESSAGE_BLOCK* msg_p)
{
    if (msg_p->mod_id < APP_MODUAL_NUM){
        if (mod_handler[msg_p->mod_id]){
            int ret = 0 ;
            if(APP_MODUAL_AUDIO_MANAGE == msg_p->mod_id){
                int Priority = osThreadGetPriority(app_thread_tid);
                osThreadSetPriority(app_thread_tid, osPriorityRealtime);
                ret = mod_handler[msg_p->mod_id](&(msg_p->msg_body));
                osThreadSetPriority(app_thread_tid, Priority);
            }else{
                ret = mod_handler[msg_p->mod_id](&(msg_p->msg_body));
            }
            if (ret)
                TRACE(2,"%s, mod_handler[%d] ret=%d", __func__, msg_p->mod_id, ret);
        }
    }
    return 0;
}

int app_mailbox_get(void)
{
    osEvent evt;
    while(1){
        evt = osMailGet(app_mailbox, millisec);
        if (evt.status == osEventMail) {
            millisec = 0;
            APP_MESSAGE_BLOCK* msg_p = NULL;
            if(((APP_MESSAGE_BLOCK *)evt.value.p)->mod_level == APP_MOD_LEVEL_0) {
                msg_p = (APP_MESSAGE_BLOCK*)osMailAlloc(app_mailbox_level0, 0);
                ASSERT(msg_p, "osMailAlloc error0 %p",msg_p);
                memcpy((uint8_t*)msg_p,(uint8_t*)evt.value.p,sizeof(APP_MESSAGE_BLOCK));
                osMailPut(app_mailbox_level0, msg_p);
            }else if(((APP_MESSAGE_BLOCK *)evt.value.p)->mod_level == APP_MOD_LEVEL_1) {
                msg_p = (APP_MESSAGE_BLOCK*)osMailAlloc(app_mailbox_level1, 0);
                ASSERT(msg_p, "osMailAlloc error1 %p",msg_p);
                memcpy((uint8_t*)msg_p,(uint8_t*)evt.value.p,sizeof(APP_MESSAGE_BLOCK));
                osMailPut(app_mailbox_level1, msg_p);
            }else {
                msg_p = (APP_MESSAGE_BLOCK*)osMailAlloc(app_mailbox_level2, 0);
                ASSERT(msg_p, "osMailAlloc error2 %p",msg_p);
                memcpy((uint8_t*)msg_p,(uint8_t*)evt.value.p,sizeof(APP_MESSAGE_BLOCK));
                osMailPut(app_mailbox_level2, msg_p);
            }
            osMailFree(app_mailbox, (APP_MESSAGE_BLOCK *)evt.value.p);
        }else{
            millisec = osWaitForever;
            break;
        }
    }

    while(osMailGetCount(app_mailbox_level0)){
        evt = osMailGet(app_mailbox_level0, 0);
        app_mailbox_process((APP_MESSAGE_BLOCK *)evt.value.p);
        osMailFree(app_mailbox_level0, (APP_MESSAGE_BLOCK *)evt.value.p);
    }

    while(osMailGetCount(app_mailbox_level1)){
        evt = osMailGet(app_mailbox_level1, 0);
        app_mailbox_process((APP_MESSAGE_BLOCK *)evt.value.p);
        osMailFree(app_mailbox_level1, (APP_MESSAGE_BLOCK *)evt.value.p);
    }

    while(osMailGetCount(app_mailbox_level2)){
        evt = osMailGet(app_mailbox_level2, 0);
        app_mailbox_process((APP_MESSAGE_BLOCK *)evt.value.p);
        osMailFree(app_mailbox_level2, (APP_MESSAGE_BLOCK *)evt.value.p);
    }
    return -1;
}

static void app_thread(void const *argument)
{
    while(1){
        app_mailbox_get();
    }
}

int app_os_init(void)
{
    if (app_mailbox_init())
        return -1;

    millisec = osWaitForever;
    app_thread_tid = osThreadCreate(osThread(app_thread), NULL);
    if (app_thread_tid == NULL)  {
        TRACE(0,"Failed to Create app_thread\n");
        return 0;
    }
    return 0;
}
#endif
int app_set_threadhandle(enum APP_MODUAL_ID_T mod_id, APP_MOD_HANDLER_T handler)
{
    if (mod_id>=APP_MODUAL_NUM)
        return -1;

    mod_handler[mod_id] = handler;
    return 0;
}

void * app_os_tid_get(void)
{
    return (void *)app_thread_tid;
}

bool app_is_module_registered(enum APP_MODUAL_ID_T mod_id)
{
    return mod_handler[mod_id];
}


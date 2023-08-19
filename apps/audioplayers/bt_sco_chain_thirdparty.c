#include "bt_sco_chain.h"
#include "speech_memory.h"
#include "speech_utils.h"
#include "hal_trace.h"
#include "audio_dump.h"
#include "bt_sco_chain_cfg.h"
#include "bt_sco_chain_tuning.h"
#include "speech_cfg.h"
#include "app_bt_stream.h"

#if defined(SCO_CP_ACCEL)
#include "hal_location.h"
#include "bt_sco_chain_cp.h"
#endif

#if defined(SPEECH_ALGO_DSP)
#include "bt_sco_chain_dsp.h"
#endif

// #define BT_SCO_CHAIN_AUDIO_DUMP
// #define SPEEHC_THIRDPARTY_REFRAME

#define SPEECH_TX_CH_NUM            (SPEECH_CODEC_CAPTURE_CHANNEL_NUM)

#if defined(SPEECH_TX_24BIT)
typedef int     SPEECH_PCM_T;
#else
typedef short   SPEECH_PCM_T;
#endif

SPEECH_PCM_T *aec_echo_buf = NULL;
// Use to free buffer
static SPEECH_PCM_T *aec_echo_buf_ptr;

#if defined(SPEEHC_THIRDPARTY_REFRAME)
#include "bt_sco_chain_reframe.h"
#define SPEECH_HEAP_RESERVE_SIZE (1024 * 70)
SPEECH_PCM_T *g_pcm_tmp_buf = NULL;
#else
// TODO: #include "thirdparty_algo.h"
#define SPEECH_HEAP_RESERVE_SIZE (1024 * 12)
#endif

#if defined(SPEECH_TX_DC_FILTER)
SpeechDcFilterState *speech_tx_dc_filter_st = NULL;
#endif

#if defined(SPEECH_RX_COMPEXP)
static MultiCompexpState *speech_rx_compexp_st = NULL;
#endif

#if defined(SPEECH_RX_EQ)
EqState *speech_rx_eq_st = NULL;
#endif

extern const SpeechConfig speech_cfg_default;
static SpeechConfig *speech_cfg = NULL;

FrameResizeState *speech_frame_resize_st = NULL;

static int speech_tx_frame_len = 256;
static int speech_rx_frame_len = 256;
static bool speech_tx_frame_resizer_enable = false;
static bool speech_rx_frame_resizer_enable = false;

int _speech_tx_process_(void *_pcm_buf, void *_ref_buf, int *_pcm_len);
int _speech_rx_process_(void *pcm_buf, int *pcm_len);

#if defined(SPEECH_BONE_SENSOR)
#include "speech_bone_sensor.h"
static SPEECH_PCM_T *vpu_buf = NULL;
#endif

#ifdef BT_SCO_CHAIN_AUDIO_DUMP
typedef SPEECH_PCM_T    _DUMP_PCM_T;
static _DUMP_PCM_T audio_dump_buf[256];
#endif

extern bool bt_sco_codec_is_cvsd(void);

#if defined(SPEECH_TUNING_CALL_VIA_CMD)
static call_algo_cfg_t g_tuning_call_cfg;
static bool g_tuning_call_cfg_flag = false;

int speech_tuning_call_algo_set_cfg(const uint8_t *buf, uint32_t len)
{
    ASSERT(buf != NULL, "[%s] buf is NULL", __func__);

    if (len != sizeof(call_algo_cfg_t)) {
        TRACE(3, "[%s] Invalid len: %d != %d", __func__, len, sizeof(call_algo_cfg_t));
        return 1;
    }

    uint32_t lock = int_lock();
    memcpy((uint8_t *)&g_tuning_call_cfg, buf, len);
    g_tuning_call_cfg_flag = true;
    int_unlock(lock);

    return 0;
}
#endif

int speech_store_config(const SpeechConfig *cfg)
{
    if (speech_cfg) {
        memcpy(speech_cfg, cfg, sizeof(SpeechConfig));
    } else {
        TRACE(1,"[%s] WARNING: Please phone call...", __func__);
    }

    return 0;
}

int speech_set_config(const SpeechConfig *cfg)
{
#if defined(SPEECH_TX_DC_FILTER)
    speech_dc_filter_set_config(speech_tx_dc_filter_st, &cfg->tx_dc_filter);
#endif
#if defined(SPEECH_RX_COMPEXP)
    multi_compexp_set_config(speech_rx_compexp_st, &cfg->rx_compexp);
#endif
#if defined(SPEECH_RX_EQ)
    eq_set_config(speech_rx_eq_st, &cfg->rx_eq);
#endif

    return 0;
}

int speech_init(int tx_sample_rate, int rx_sample_rate,
                     int tx_frame_ms, int rx_frame_ms,
                     int sco_frame_ms,
                     uint8_t *buf, int len)
{
    speech_tx_frame_len = SPEECH_FRAME_MS_TO_LEN(tx_sample_rate, tx_frame_ms);
    speech_rx_frame_len = SPEECH_FRAME_MS_TO_LEN(rx_sample_rate, rx_frame_ms);

    // we shoule keep a minmum buffer for speech heap
    // sample rate 8k : 4k bytes
    // sample rate 16k: 10k bytes
    speech_heap_init(buf, len);

    int frame_len = speech_tx_frame_len;

    speech_cfg = (SpeechConfig *)speech_calloc(1, sizeof(SpeechConfig));
    speech_store_config(&speech_cfg_default);

#if defined(SPEECH_TUNING_CALL_VIA_CMD)
    g_tuning_call_cfg_flag = false;
#endif

#ifdef AUDIO_DEBUG
    speech_tuning_open();
#endif

    int aec_enable = 0;
#if defined(SPEECH_TX_AEC) || defined(SPEECH_TX_AEC2) || defined(SPEECH_TX_AEC3) || defined(SPEECH_TX_AEC2FLOAT)
    aec_enable = 1;
#endif

    int capture_sample_size = sizeof(int16_t), playback_sample_size = sizeof(int16_t);
#if defined(SPEECH_TX_24BIT)
    capture_sample_size = sizeof(int32_t);
#endif
#if defined(SPEECH_RX_24BIT)
    playback_sample_size = sizeof(int32_t);
#endif

    CAPTURE_HANDLER_T tx_handler = (tx_frame_ms == sco_frame_ms) ? NULL : _speech_tx_process_;
    PLAYBACK_HANDLER_T rx_handler = (rx_frame_ms == sco_frame_ms) ? NULL : _speech_rx_process_;

    speech_tx_frame_resizer_enable = (tx_handler != NULL);
    speech_rx_frame_resizer_enable = (rx_handler != NULL);

    if (speech_tx_frame_resizer_enable || speech_rx_frame_resizer_enable) {
        speech_frame_resize_st = frame_resize_create(SPEECH_FRAME_MS_TO_LEN(tx_sample_rate, sco_frame_ms),
                                                     SPEECH_CODEC_CAPTURE_CHANNEL_NUM,
                                                     speech_tx_frame_len,
                                                     capture_sample_size,
                                                     playback_sample_size,
                                                     aec_enable,
                                                     tx_handler,
                                                     rx_handler
                                                    );
    }

#if defined(SCO_CP_ACCEL)
    // NOTE: change channel number for different case.
    sco_cp_init(speech_tx_frame_len, 1);
#endif

#if defined(SPEECH_ALGO_DSP)
    if (amgr_is_bluetooth_sco_on()) {
        SCO_DSP_CFG_T dsp_cfg;
        memset(&dsp_cfg, 0, sizeof(SCO_DSP_CFG_T));
        dsp_cfg.sample_rate = tx_sample_rate;
#if defined(SPEECH_TX_24BIT)
        dsp_cfg.sample_bytes= sizeof(int32_t);
#else
        dsp_cfg.sample_bytes= sizeof(int16_t);
#endif
        dsp_cfg.frame_len   = speech_tx_frame_len;
        dsp_cfg.mic_num     = SPEECH_TX_CH_NUM;
        dsp_cfg.mode        = (bt_sco_codec_is_cvsd() == false);
        dsp_cfg.capture_enable = true;
        dsp_cfg.playback_enable = false;
        dsp_cfg.user        = SPEECH_ALGO_DSP_USER_CALL;
        speech_algo_dsp_open(&dsp_cfg);
    }
#endif

    aec_echo_buf = (SPEECH_PCM_T *)speech_calloc(frame_len, sizeof(SPEECH_PCM_T));
    aec_echo_buf_ptr = aec_echo_buf;

#if defined(SPEECH_TX_DC_FILTER)
    int channel_num = SPEECH_TX_CH_NUM;
    int data_separation = 0;

    speech_tx_dc_filter_st = speech_dc_filter_create(tx_sample_rate, frame_len, &speech_cfg->tx_dc_filter);
    speech_dc_filter_ctl(speech_tx_dc_filter_st, SPEECH_DC_FILTER_SET_CHANNEL_NUM, &channel_num);
    speech_dc_filter_ctl(speech_tx_dc_filter_st, SPEECH_DC_FILTER_SET_DATA_SEPARATION, &data_separation);
#endif

#if defined(SPEECH_RX_COMPEXP)
    speech_rx_compexp_st = multi_compexp_create(rx_sample_rate, frame_len, &speech_cfg->rx_compexp);
#endif

#if defined(SPEECH_RX_EQ)
    speech_rx_eq_st = eq_init(rx_sample_rate, frame_len, &speech_cfg->rx_eq);
#endif

#if defined(SPEEHC_THIRDPARTY_REFRAME)
    g_pcm_tmp_buf = (SPEECH_PCM_T *)speech_calloc(frame_len, sizeof(SPEECH_PCM_T));
    bt_sco_chain_reframe_open(tx_sample_rate, frame_len);
#else
    // TODO: Call thridpart algo init function
#endif

#if defined(SPEECH_BONE_SENSOR)
    vpu_buf = (SPEECH_PCM_T *)speech_calloc(frame_len, sizeof(SPEECH_PCM_T));
#endif

#if defined(BT_SCO_CHAIN_AUDIO_DUMP)
    audio_dump_init(frame_len, sizeof(_DUMP_PCM_T), 3);
#endif

    return 0;
}

int speech_deinit(void)
{
#if defined(SPEECH_BONE_SENSOR)
    speech_free(vpu_buf);
#endif

#if defined(SPEEHC_THIRDPARTY_REFRAME)
    bt_sco_chain_reframe_close();
    speech_free(g_pcm_tmp_buf);
#else
    // TODO: Call thridpart algo deinit function
#endif

#if defined(SPEECH_RX_EQ)
    eq_destroy(speech_rx_eq_st);
#endif

#if defined(SPEECH_RX_COMPEXP)
    multi_compexp_destroy(speech_rx_compexp_st);
#endif

#if defined(SPEECH_TX_DC_FILTER)
    speech_dc_filter_destroy(speech_tx_dc_filter_st);
#endif

    speech_free(aec_echo_buf_ptr);

#if defined(SCO_CP_ACCEL)
    sco_cp_deinit();
#endif

#if defined(SPEECH_ALGO_DSP)
    if (amgr_is_bluetooth_sco_on()) {
        speech_algo_dsp_close();
    }
#endif

    if (speech_frame_resize_st != NULL) {
        frame_resize_destroy(speech_frame_resize_st);
        speech_tx_frame_resizer_enable = false;
        speech_rx_frame_resizer_enable = false;
    }

#ifdef AUDIO_DEBUG
    speech_tuning_close();
#endif

    speech_free(speech_cfg);
    speech_cfg = NULL;

    size_t total = 0, used = 0, max_used = 0;
    speech_memory_info(&total, &used, &max_used);
    TRACE(3,"SPEECH MALLOC MEM: total - %d, used - %d, max_used - %d.", total, used, max_used);
    ASSERT(used == 0, "[%s] used != 0", __func__);

    return 0;
}

#if defined(SCO_CP_ACCEL)
CP_TEXT_SRAM_LOC
int sco_cp_algo(short *pcm_buf, short *ref_buf, int *_pcm_len, sco_status_t *status)
{
#if defined(SCO_TRACE_CP_ACCEL)
    TRACE(1,"[%s] ...", __func__);
#endif

    //TRACE("[%s] cp vad = %d", __FUNCTION__, status->vad);

    return 0;
}
#endif

int _speech_tx_process_(void *_pcm_buf, void *_ref_buf, int *_pcm_len)
{
    sco_status_t POSSIBLY_UNUSED  status = {
        .vad = true,
    };

    SPEECH_PCM_T POSSIBLY_UNUSED *pcm_buf = (SPEECH_PCM_T *)_pcm_buf;
    SPEECH_PCM_T POSSIBLY_UNUSED *ref_buf = (SPEECH_PCM_T *)_ref_buf;

    int POSSIBLY_UNUSED pcm_len = *_pcm_len;
    int POSSIBLY_UNUSED frame_len = pcm_len / SPEECH_TX_CH_NUM;

#if defined(SPEECH_BONE_SENSOR)
#if defined(SPEECH_TX_24BIT)
    speech_bone_sensor_get_data(vpu_buf, frame_len, SPEECH_BS_CHANNEL_X, 24);
#else
    speech_bone_sensor_get_data(vpu_buf, frame_len, SPEECH_BS_CHANNEL_X, 16);
#endif
#endif

#ifdef AUDIO_DEBUG
    if (speech_tuning_get_status())
    {
        speech_set_config(speech_cfg);

        speech_tuning_set_status(false);

        // If has MIPS problem, can move this block code into speech_rx_process or return directly
        // return 0
    }
#endif

#if defined(BT_SCO_CHAIN_AUDIO_DUMP)
    uint32_t dump_ch = 0;
    audio_dump_clear_up();
    audio_dump_add_channel_data(dump_ch++, ref_buf, frame_len);
    audio_dump_add_channel_data_from_multi_channels(dump_ch++, pcm_buf, frame_len, SPEECH_TX_CH_NUM, 0);
    // audio_dump_add_channel_data_from_multi_channels(dump_ch++, pcm_buf, frame_len, SPEECH_TX_CH_NUM, 1);
    // audio_dump_add_channel_data_from_multi_channels(dump_ch++, pcm_buf, frame_len, SPEECH_TX_CH_NUM, 2);
#if defined(SPEECH_BONE_SENSOR)
    audio_dump_add_channel_data(dump_ch++, vpu_buf, frame_len);
#endif
#endif

#if defined(SPEECH_TX_DC_FILTER)
#if defined(SPEECH_TX_24BIT)
    speech_dc_filter_process_int24(speech_tx_dc_filter_st, pcm_buf, pcm_len);
#else
    speech_dc_filter_process(speech_tx_dc_filter_st, pcm_buf, pcm_len);
#endif
#endif

    // Add your algrithm here and disable #if macro
#if defined(SPEEHC_THIRDPARTY_REFRAME)
    for (int i = 0; i < frame_len; i++) {
        g_pcm_tmp_buf[i] = pcm_buf[i * SPEECH_TX_CH_NUM + REFRAME_CHANNEL_VP];
    }
    bt_sco_chain_reframe_cache_tx_pcm(g_pcm_tmp_buf, frame_len, REFRAME_CHANNEL_VP);

    for (int i = 0; i < frame_len; i++) {
        g_pcm_tmp_buf[i] = pcm_buf[i * SPEECH_TX_CH_NUM + REFRAME_CHANNEL_FF];
    }
    bt_sco_chain_reframe_cache_tx_pcm(g_pcm_tmp_buf, frame_len, REFRAME_CHANNEL_FF);

    for (int i = 0; i < frame_len; i++) {
        g_pcm_tmp_buf[i] = pcm_buf[i * SPEECH_TX_CH_NUM + REFRAME_CHANNEL_FB];
    }
    bt_sco_chain_reframe_cache_tx_pcm(g_pcm_tmp_buf, frame_len, REFRAME_CHANNEL_FB);

    bt_sco_chain_reframe_cache_tx_pcm(ref_buf, frame_len, REFRAME_CHANNEL_REF);

#if defined(SPEECH_BONE_SENSOR)
    bt_sco_chain_reframe_cache_tx_pcm(vpu_buf, frame_len, REFRAME_CHANNEL_BS);
#endif

    bt_sco_chain_reframe_tx_process(NULL, frame_len, pcm_buf);
#elif defined(SPEECH_ALGO_DSP_TEST)
    speech_algo_dsp_pcm_t pcm_cfg = {0};
    memset(&pcm_cfg, 0, sizeof(pcm_cfg));
    pcm_cfg.mic = pcm_buf;
    pcm_cfg.ref = ref_buf;
#if defined(SPEECH_BONE_SENSOR)
    pcm_cfg.vpu = vpu_buf;
#else
    pcm_cfg.vpu = NULL;
#endif
    pcm_cfg.out = pcm_buf;
    pcm_cfg.len = frame_len;
    speech_algo_dsp_test_process(&pcm_cfg);
#elif defined(SPEECH_ALGO_DSP)
    SCO_DSP_TX_PCM_T pcm_cfg;
    memset(&pcm_cfg, 0, sizeof(pcm_cfg));
    pcm_cfg.mic = pcm_buf;
    pcm_cfg.ref = ref_buf;
#if defined(SPEECH_BONE_SENSOR)
    pcm_cfg.vpu = vpu_buf;
#else
    pcm_cfg.vpu = NULL;
#endif
    pcm_cfg.out = pcm_buf;
    pcm_cfg.frame_len = frame_len;

#if defined(SPEECH_TUNING_CALL_VIA_CMD)
    uint32_t lock = int_lock();
    if (g_tuning_call_cfg_flag) {
        pcm_cfg.cfg = &g_tuning_call_cfg;
        pcm_cfg.cfg_len = sizeof(call_algo_cfg_t);
        pcm_cfg.ret = -1;
    }
    int_unlock(lock);
#endif

    speech_algo_dsp_capture_process(&pcm_cfg);

#if defined(SPEECH_TUNING_CALL_VIA_CMD)
    if (g_tuning_call_cfg_flag) {
        g_tuning_call_cfg_flag = false;
        if (pcm_cfg.ret != 0) {
            TRACE(1, "[%s] SV tx tuning OK", __func__);
        } else {
            TRACE(2, "[%s] WARNING: SV tx tuning error: %d", __func__, pcm_cfg.ret);
        }
    }
#endif
#else
    // TODO: Call thirdparty algo process function

    for (int i = 0; i < frame_len; i++) {
        // choose main microphone data
        pcm_buf[i] = pcm_buf[i * SPEECH_TX_CH_NUM + 0];
        // choose reference data, i.e. loopback
        // pcm_buf[i] = ref_buf[j];
    }

    // Choose bone sensor data
// #if defined(SPEECH_BONE_SENSOR)
//     memcpy(pcm_buf, vpu, frame_len);
// #endif
#endif

#if defined(SCO_CP_ACCEL)
    sco_cp_process(pcm_buf, ref_buf, &frame_len, &status);
#endif

#if defined(BT_SCO_CHAIN_AUDIO_DUMP)
    audio_dump_add_channel_data(dump_ch++, pcm_buf, frame_len);

    audio_dump_run();
#endif

    *_pcm_len = frame_len;

    return 0;
}

int _speech_rx_process_(void *pcm_buf, int *pcm_len)
{
#if 0 // defined(SCO_DSP_ACCEL)
    SCO_DSP_RX_PCM_T pcm_cfg;
    memset(&pcm_cfg, 0, sizeof(pcm_cfg));
    pcm_cfg.in = pcm_buf;
    pcm_cfg.out = pcm_buf;
    pcm_cfg.frame_len = *pcm_len;
    sco_dsp_playback_process(&pcm_cfg);
#endif

#if defined(SPEECH_RX_24BIT)
#if defined(SPEECH_RX_COMPEXP)
    multi_compexp_process_int24(speech_rx_compexp_st, pcm_buf, *pcm_len);
#endif

#if defined(SPEECH_RX_EQ)
    eq_process_int24(speech_rx_eq_st, pcm_buf, *pcm_len);
#endif

#else
#if defined(SPEECH_RX_COMPEXP)
    multi_compexp_process(speech_rx_compexp_st, pcm_buf, pcm_len);
#endif

#if defined(SPEECH_RX_EQ)
    eq_process(speech_rx_eq_st, pcm_buf, pcm_len);
#endif
#endif

    return 0;
}

int speech_tx_process(void *pcm_buf, void *ref_buf, int *pcm_len)
{
    if (speech_tx_frame_resizer_enable == false) {
        _speech_tx_process_(pcm_buf, ref_buf, (int32_t *)pcm_len);
    } else {
        // MUST use (int32_t *)??????
        frame_resize_process_capture(speech_frame_resize_st, pcm_buf, ref_buf, (int32_t *)pcm_len);
    }

    return 0;
}

int speech_rx_process(void *pcm_buf, int *pcm_len)
{
    if (speech_rx_frame_resizer_enable == false) {
        _speech_rx_process_(pcm_buf, (int32_t *)pcm_len);
    } else {
        frame_resize_process_playback(speech_frame_resize_st, pcm_buf, (int32_t *)pcm_len);
    }

    return 0;
}

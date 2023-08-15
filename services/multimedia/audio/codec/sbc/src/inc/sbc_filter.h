#ifndef SBC_FILTER_H
#define SBC_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

void plc_sbc_analysis_filter8(sbc_encoder_t *Encoder, sbc_pcm_data_t *PcmData, U8 startBlock, U8 endBlock);

void plc_sbc_synthesis_filter8(sbc_decoder_t *Decoder, sbc_pcm_data_t *PcmData, U8 startBlock, U8 endBlock, float* gain);

void plc_sbc_synthesis_filter8_without_output(sbc_decoder_t *Decoder, U8 startBlock, U8 endBlock, float* gain);

void plc_sbc_copy_state(sbc_decoder_t *Decoder, sbc_encoder_t *Encoder);

void plc_sbc_copy_state_blocks(sbc_decoder_t *Decoder, U8 DecStartBlock, sbc_encoder_t *Encoder, U8 EncStartBlock, U8 EncEndBlock);

void plc_sbc_flush_state(sbc_codec_information_t *streamInfo, U8 numBlocks, U8 *remainBlocks);

#ifdef __cplusplus
}
#endif

#endif
/*
 * H.26L/H.264/AVC/JVT/14496-10/... parameter set decoding
 * Copyright (c) 2003 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * H.264 / AVC / MPEG4 part10 parameter set decoding.
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#include "libavcore/imgutils.h"
#include "internal.h"
#include "dsputil.h"
#include "avcodec.h"
#include "h264.h"
#include "h264data.h" //FIXME FIXME FIXME (just for zigzag_scan)
#include "golomb.h"

#ifdef JZC_DCORE_OPT
#include "jzsoc/h264_dcore.h"
#include "jzsoc/h264_tcsm1.h"
#include "jzsoc/h264_sram.h"
//#include "jzsoc/h264_cavlc_sram.h"
#include "jzsoc/h264_tcsm0.h"
#include "../libjzcommon/jz4760e_2ddma_hw.h"
#include "../libjzcommon/jzasm.h"

#undef printf
int init_every_movie;
extern int *tmp_hm_buf;
#endif


#if 1 //#ifdef JZ_LINUX_OS
extern unsigned int h264_auxcodes_len, h264_aux_task_codes[];
extern unsigned int h264_cavlc_auxcodes_len, h264_cavlc_aux_task_codes[];
#endif

//#undef NDEBUG
#include <assert.h>

static const AVRational pixel_aspect[17]={
 {0, 1},
 {1, 1},
 {12, 11},
 {10, 11},
 {16, 11},
 {40, 33},
 {24, 11},
 {20, 11},
 {32, 11},
 {80, 33},
 {18, 11},
 {15, 11},
 {64, 33},
 {160,99},
 {4, 3},
 {3, 2},
 {2, 1},
};

const uint8_t ff_h264_chroma_qp[52]={
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,
   12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
   28,29,29,30,31,32,32,33,34,34,35,35,36,36,37,37,
   37,38,38,38,39,39,39,39
};

static const uint8_t default_scaling4[2][16]={
{   6,13,20,28,
   13,20,28,32,
   20,28,32,37,
   28,32,37,42
},{
   10,14,20,24,
   14,20,24,27,
   20,24,27,30,
   24,27,30,34
}};

static const uint8_t default_scaling8[2][64]={
{   6,10,13,16,18,23,25,27,
   10,11,16,18,23,25,27,29,
   13,16,18,23,25,27,29,31,
   16,18,23,25,27,29,31,33,
   18,23,25,27,29,31,33,36,
   23,25,27,29,31,33,36,38,
   25,27,29,31,33,36,38,40,
   27,29,31,33,36,38,40,42
},{
    9,13,15,17,19,21,22,24,
   13,13,17,19,21,22,24,25,
   15,17,19,21,22,24,25,27,
   17,19,21,22,24,25,27,28,
   19,21,22,24,25,27,28,30,
   21,22,24,25,27,28,30,32,
   22,24,25,27,28,30,32,33,
   24,25,27,28,30,32,33,35
}};

static inline int decode_hrd_parameters(H264Context *h, SPS *sps){
    MpegEncContext * const s = &h->s;
    int cpb_count, i;
    cpb_count = get_ue_golomb_31(&s->gb) + 1;

    if(cpb_count > 32U){
        av_log(h->s.avctx, AV_LOG_ERROR, "cpb_count %d invalid\n", cpb_count);
        return -1;
    }

    get_bits(&s->gb, 4); /* bit_rate_scale */
    get_bits(&s->gb, 4); /* cpb_size_scale */
    for(i=0; i<cpb_count; i++){
        get_ue_golomb(&s->gb); /* bit_rate_value_minus1 */
        get_ue_golomb(&s->gb); /* cpb_size_value_minus1 */
        get_bits1(&s->gb);     /* cbr_flag */
    }
    sps->initial_cpb_removal_delay_length = get_bits(&s->gb, 5) + 1;
    sps->cpb_removal_delay_length = get_bits(&s->gb, 5) + 1;
    sps->dpb_output_delay_length = get_bits(&s->gb, 5) + 1;
    sps->time_offset_length = get_bits(&s->gb, 5);
    sps->cpb_cnt = cpb_count;
    return 0;
}

static inline int decode_vui_parameters(H264Context *h, SPS *sps){
    MpegEncContext * const s = &h->s;
    int aspect_ratio_info_present_flag;
    unsigned int aspect_ratio_idc;

    aspect_ratio_info_present_flag= get_bits1(&s->gb);

    if( aspect_ratio_info_present_flag ) {
        aspect_ratio_idc= get_bits(&s->gb, 8);
        if( aspect_ratio_idc == EXTENDED_SAR ) {
            sps->sar.num= get_bits(&s->gb, 16);
            sps->sar.den= get_bits(&s->gb, 16);
        }else if(aspect_ratio_idc < FF_ARRAY_ELEMS(pixel_aspect)){
            sps->sar=  pixel_aspect[aspect_ratio_idc];
        }else{
            av_log(h->s.avctx, AV_LOG_ERROR, "illegal aspect ratio\n");
            return -1;
        }
    }else{
        sps->sar.num=
        sps->sar.den= 0;
    }
//            s->avctx->aspect_ratio= sar_width*s->width / (float)(s->height*sar_height);

    if(get_bits1(&s->gb)){      /* overscan_info_present_flag */
        get_bits1(&s->gb);      /* overscan_appropriate_flag */
    }

    sps->video_signal_type_present_flag = get_bits1(&s->gb);
    if(sps->video_signal_type_present_flag){
        get_bits(&s->gb, 3);    /* video_format */
        sps->full_range = get_bits1(&s->gb); /* video_full_range_flag */

        sps->colour_description_present_flag = get_bits1(&s->gb);
        if(sps->colour_description_present_flag){
            sps->color_primaries = get_bits(&s->gb, 8); /* colour_primaries */
            sps->color_trc       = get_bits(&s->gb, 8); /* transfer_characteristics */
            sps->colorspace      = get_bits(&s->gb, 8); /* matrix_coefficients */
            if (sps->color_primaries >= AVCOL_PRI_NB)
                sps->color_primaries  = AVCOL_PRI_UNSPECIFIED;
            if (sps->color_trc >= AVCOL_TRC_NB)
                sps->color_trc  = AVCOL_TRC_UNSPECIFIED;
            if (sps->colorspace >= AVCOL_SPC_NB)
                sps->colorspace  = AVCOL_SPC_UNSPECIFIED;
        }
    }

    if(get_bits1(&s->gb)){      /* chroma_location_info_present_flag */
        s->avctx->chroma_sample_location = get_ue_golomb(&s->gb)+1;  /* chroma_sample_location_type_top_field */
        get_ue_golomb(&s->gb);  /* chroma_sample_location_type_bottom_field */
    }

    sps->timing_info_present_flag = get_bits1(&s->gb);
    if(sps->timing_info_present_flag){
        sps->num_units_in_tick = get_bits_long(&s->gb, 32);
        sps->time_scale = get_bits_long(&s->gb, 32);
        if(!sps->num_units_in_tick || !sps->time_scale){
            av_log(h->s.avctx, AV_LOG_ERROR, "time_scale/num_units_in_tick invalid or unsupported (%d/%d)\n", sps->time_scale, sps->num_units_in_tick);
            return -1;
        }
        sps->fixed_frame_rate_flag = get_bits1(&s->gb);
    }

    sps->nal_hrd_parameters_present_flag = get_bits1(&s->gb);
    if(sps->nal_hrd_parameters_present_flag)
        if(decode_hrd_parameters(h, sps) < 0)
            return -1;
    sps->vcl_hrd_parameters_present_flag = get_bits1(&s->gb);
    if(sps->vcl_hrd_parameters_present_flag)
        if(decode_hrd_parameters(h, sps) < 0)
            return -1;
    if(sps->nal_hrd_parameters_present_flag || sps->vcl_hrd_parameters_present_flag)
        get_bits1(&s->gb);     /* low_delay_hrd_flag */
    sps->pic_struct_present_flag = get_bits1(&s->gb);

    sps->bitstream_restriction_flag = get_bits1(&s->gb);
    if(sps->bitstream_restriction_flag){
        get_bits1(&s->gb);     /* motion_vectors_over_pic_boundaries_flag */
        get_ue_golomb(&s->gb); /* max_bytes_per_pic_denom */
        get_ue_golomb(&s->gb); /* max_bits_per_mb_denom */
        get_ue_golomb(&s->gb); /* log2_max_mv_length_horizontal */
        get_ue_golomb(&s->gb); /* log2_max_mv_length_vertical */
        sps->num_reorder_frames= get_ue_golomb(&s->gb);
        get_ue_golomb(&s->gb); /*max_dec_frame_buffering*/

        if(s->gb.size_in_bits < get_bits_count(&s->gb)){
            av_log(h->s.avctx, AV_LOG_ERROR, "Overread VUI by %d bits\n", get_bits_count(&s->gb) - s->gb.size_in_bits);
            sps->num_reorder_frames=0;
            sps->bitstream_restriction_flag= 0;
        }

        if(sps->num_reorder_frames > 16U /*max_dec_frame_buffering || max_dec_frame_buffering > 16*/){
            av_log(h->s.avctx, AV_LOG_ERROR, "illegal num_reorder_frames %d\n", sps->num_reorder_frames);
            return -1;
        }
    }

    return 0;
}

static void decode_scaling_list(H264Context *h, uint8_t *factors, int size,
                                const uint8_t *jvt_list, const uint8_t *fallback_list){
    MpegEncContext * const s = &h->s;
    int i, last = 8, next = 8;
    const uint8_t *scan = size == 16 ? zigzag_scan : ff_zigzag_direct;
    if(!get_bits1(&s->gb)) /* matrix not written, we use the predicted one */
        memcpy(factors, fallback_list, size*sizeof(uint8_t));
    else
    for(i=0;i<size;i++){
        if(next)
            next = (last + get_se_golomb(&s->gb)) & 0xff;
        if(!i && !next){ /* matrix not written, we use the preset one */
            memcpy(factors, jvt_list, size*sizeof(uint8_t));
            break;
        }
        last = factors[scan[i]] = next ? next : last;
    }
}

static void decode_scaling_matrices(H264Context *h, SPS *sps, PPS *pps, int is_sps,
                                   uint8_t (*scaling_matrix4)[16], uint8_t (*scaling_matrix8)[64]){
    MpegEncContext * const s = &h->s;
    int fallback_sps = !is_sps && sps->scaling_matrix_present;
    const uint8_t *fallback[4] = {
        fallback_sps ? sps->scaling_matrix4[0] : default_scaling4[0],
        fallback_sps ? sps->scaling_matrix4[3] : default_scaling4[1],
        fallback_sps ? sps->scaling_matrix8[0] : default_scaling8[0],
        fallback_sps ? sps->scaling_matrix8[1] : default_scaling8[1]
    };
    if(get_bits1(&s->gb)){
        sps->scaling_matrix_present |= is_sps;
        decode_scaling_list(h,scaling_matrix4[0],16,default_scaling4[0],fallback[0]); // Intra, Y
        decode_scaling_list(h,scaling_matrix4[1],16,default_scaling4[0],scaling_matrix4[0]); // Intra, Cr
        decode_scaling_list(h,scaling_matrix4[2],16,default_scaling4[0],scaling_matrix4[1]); // Intra, Cb
        decode_scaling_list(h,scaling_matrix4[3],16,default_scaling4[1],fallback[1]); // Inter, Y
        decode_scaling_list(h,scaling_matrix4[4],16,default_scaling4[1],scaling_matrix4[3]); // Inter, Cr
        decode_scaling_list(h,scaling_matrix4[5],16,default_scaling4[1],scaling_matrix4[4]); // Inter, Cb
        if(is_sps || pps->transform_8x8_mode){
            decode_scaling_list(h,scaling_matrix8[0],64,default_scaling8[0],fallback[2]);  // Intra, Y
            decode_scaling_list(h,scaling_matrix8[1],64,default_scaling8[1],fallback[3]);  // Inter, Y
        }
    }
}

int ff_h264_decode_seq_parameter_set(H264Context *h){
    MpegEncContext * const s = &h->s;
    int profile_idc, level_idc;
    unsigned int sps_id;
    int i;
    SPS *sps;

    profile_idc= get_bits(&s->gb, 8);
    get_bits1(&s->gb);   //constraint_set0_flag
    get_bits1(&s->gb);   //constraint_set1_flag
    get_bits1(&s->gb);   //constraint_set2_flag
    get_bits1(&s->gb);   //constraint_set3_flag
    get_bits(&s->gb, 4); // reserved
    level_idc= get_bits(&s->gb, 8);
    sps_id= get_ue_golomb_31(&s->gb);

    if(sps_id >= MAX_SPS_COUNT) {
        av_log(h->s.avctx, AV_LOG_ERROR, "sps_id (%d) out of range\n", sps_id);
        return -1;
    }
    sps= av_mallocz(sizeof(SPS));
    if(sps == NULL)
        return -1;

    sps->profile_idc= profile_idc;
    sps->level_idc= level_idc;

    memset(sps->scaling_matrix4, 16, sizeof(sps->scaling_matrix4));
    memset(sps->scaling_matrix8, 16, sizeof(sps->scaling_matrix8));
    sps->scaling_matrix_present = 0;

    if(sps->profile_idc >= 100){ //high profile
        sps->chroma_format_idc= get_ue_golomb_31(&s->gb);
        if(sps->chroma_format_idc == 3)
            sps->residual_color_transform_flag = get_bits1(&s->gb);
        sps->bit_depth_luma   = get_ue_golomb(&s->gb) + 8;
        sps->bit_depth_chroma = get_ue_golomb(&s->gb) + 8;
        sps->transform_bypass = get_bits1(&s->gb);
        decode_scaling_matrices(h, sps, NULL, 1, sps->scaling_matrix4, sps->scaling_matrix8);
    }else{
        sps->chroma_format_idc= 1;
        sps->bit_depth_luma   = 8;
        sps->bit_depth_chroma = 8;
    }

    sps->log2_max_frame_num= get_ue_golomb(&s->gb) + 4;
    sps->poc_type= get_ue_golomb_31(&s->gb);

    if(sps->poc_type == 0){ //FIXME #define
        sps->log2_max_poc_lsb= get_ue_golomb(&s->gb) + 4;
    } else if(sps->poc_type == 1){//FIXME #define
        sps->delta_pic_order_always_zero_flag= get_bits1(&s->gb);
        sps->offset_for_non_ref_pic= get_se_golomb(&s->gb);
        sps->offset_for_top_to_bottom_field= get_se_golomb(&s->gb);
        sps->poc_cycle_length                = get_ue_golomb(&s->gb);

        if((unsigned)sps->poc_cycle_length >= FF_ARRAY_ELEMS(sps->offset_for_ref_frame)){
            av_log(h->s.avctx, AV_LOG_ERROR, "poc_cycle_length overflow %u\n", sps->poc_cycle_length);
            goto fail;
        }

        for(i=0; i<sps->poc_cycle_length; i++)
            sps->offset_for_ref_frame[i]= get_se_golomb(&s->gb);
    }else if(sps->poc_type != 2){
        av_log(h->s.avctx, AV_LOG_ERROR, "illegal POC type %d\n", sps->poc_type);
        goto fail;
    }

    sps->ref_frame_count= get_ue_golomb_31(&s->gb);
    if(sps->ref_frame_count > MAX_PICTURE_COUNT-2 || sps->ref_frame_count >= 32U){
        av_log(h->s.avctx, AV_LOG_ERROR, "too many reference frames\n");
        goto fail;
    }
    sps->gaps_in_frame_num_allowed_flag= get_bits1(&s->gb);
    sps->mb_width = get_ue_golomb(&s->gb) + 1;
    sps->mb_height= get_ue_golomb(&s->gb) + 1;
    if((unsigned)sps->mb_width >= INT_MAX/16 || (unsigned)sps->mb_height >= INT_MAX/16 ||
       av_image_check_size(16*sps->mb_width, 16*sps->mb_height, 0, h->s.avctx)){
        av_log(h->s.avctx, AV_LOG_ERROR, "mb_width/height overflow\n");
        goto fail;
    }

    sps->frame_mbs_only_flag= get_bits1(&s->gb);
    if(!sps->frame_mbs_only_flag)
        sps->mb_aff= get_bits1(&s->gb);
    else
        sps->mb_aff= 0;

    sps->direct_8x8_inference_flag= get_bits1(&s->gb);

#ifdef PROFILE_3DOT0_ABOVE_OPT
    s->level_3dot0_above=sps->direct_8x8_inference_flag;
#endif

    if(!sps->frame_mbs_only_flag && !sps->direct_8x8_inference_flag){
        av_log(h->s.avctx, AV_LOG_ERROR, "This stream was generated by a broken encoder, invalid 8x8 inference\n");
        goto fail;
    }

#ifndef ALLOW_INTERLACE
    if(sps->mb_aff)
        av_log(h->s.avctx, AV_LOG_ERROR, "MBAFF support not included; enable it at compile-time.\n");
#endif
    sps->crop= get_bits1(&s->gb);
    if(sps->crop){
        sps->crop_left  = get_ue_golomb(&s->gb);
        sps->crop_right = get_ue_golomb(&s->gb);
        sps->crop_top   = get_ue_golomb(&s->gb);
        sps->crop_bottom= get_ue_golomb(&s->gb);
        if(sps->crop_left || sps->crop_top){
            av_log(h->s.avctx, AV_LOG_ERROR, "insane cropping not completely supported, this could look slightly wrong ...\n");
        }
        if(sps->crop_right >= 8 || sps->crop_bottom >= (8>> !sps->frame_mbs_only_flag)){
            av_log(h->s.avctx, AV_LOG_ERROR, "brainfart cropping not supported, this could look slightly wrong ...\n");
        }
    }else{
        sps->crop_left  =
        sps->crop_right =
        sps->crop_top   =
        sps->crop_bottom= 0;
    }

    sps->vui_parameters_present_flag= get_bits1(&s->gb);
    if( sps->vui_parameters_present_flag )
        if (decode_vui_parameters(h, sps) < 0)
            goto fail;

    if(s->avctx->debug&FF_DEBUG_PICT_INFO){
        av_log(h->s.avctx, AV_LOG_DEBUG, "sps:%u profile:%d/%d poc:%d ref:%d %dx%d %s %s crop:%d/%d/%d/%d %s %s %d/%d\n",
               sps_id, sps->profile_idc, sps->level_idc,
               sps->poc_type,
               sps->ref_frame_count,
               sps->mb_width, sps->mb_height,
               sps->frame_mbs_only_flag ? "FRM" : (sps->mb_aff ? "MB-AFF" : "PIC-AFF"),
               sps->direct_8x8_inference_flag ? "8B8" : "",
               sps->crop_left, sps->crop_right,
               sps->crop_top, sps->crop_bottom,
               sps->vui_parameters_present_flag ? "VUI" : "",
               ((const char*[]){"Gray","420","422","444"})[sps->chroma_format_idc],
               sps->timing_info_present_flag ? sps->num_units_in_tick : 0,
               sps->timing_info_present_flag ? sps->time_scale : 0
               );
    }

    av_free(h->sps_buffers[sps_id]);
    h->sps_buffers[sps_id]= sps;
    h->sps = *sps;
    return 0;
fail:
    av_free(sps);
    return -1;
}

static void
build_qp_table(PPS *pps, int t, int index)
{
    int i;
    for(i = 0; i < 52; i++)
        pps->chroma_qp_table[t][i] = ff_h264_chroma_qp[av_clip(i + index, 0, 51)];
}

int ff_h264_decode_picture_parameter_set(H264Context *h, int bit_length){
    MpegEncContext * const s = &h->s;
    unsigned int pps_id= get_ue_golomb(&s->gb);
    PPS *pps;

    if(pps_id >= MAX_PPS_COUNT) {
        av_log(h->s.avctx, AV_LOG_ERROR, "pps_id (%d) out of range\n", pps_id);
        return -1;
    }

    pps= av_mallocz(sizeof(PPS));
    if(pps == NULL)
        return -1;
    pps->sps_id= get_ue_golomb_31(&s->gb);
    if((unsigned)pps->sps_id>=MAX_SPS_COUNT || h->sps_buffers[pps->sps_id] == NULL){
        av_log(h->s.avctx, AV_LOG_ERROR, "sps_id out of range\n");
        goto fail;
    }

    pps->cabac= get_bits1(&s->gb);

    if(init_every_movie){
      init_every_movie=0;
      H264_XCH2_T *XCH2_T0, *XCH2_T1;

      if (pps->cabac){
	p1_dblk_decode_cabac_init();
	h264_idct_cabac_init();
	
	XCH2_T0 = (H264_XCH2_T *)TCSM1_VUCADDR(TCSM1_XCH2_T_BUF0);
	XCH2_T1 = (H264_XCH2_T *)TCSM1_VUCADDR(TCSM1_XCH2_T_BUF1);
	
	XCH2_T0->dblk_des_ptr = (uint8_t *)DBLK_DES_CHAIN0;
	XCH2_T0->dblk_mv_ptr = (uint8_t *)DBLK_MV_CHAIN0;
	XCH2_T0->dblk_out_addr = (uint8_t *)TCSM1_DBLK_MBOUT_Y0;
	XCH2_T0->dblk_out_addr_c = (uint8_t *)TCSM1_DBLK_MBOUT_U0;
	XCH2_T0->add_error_buf = (short *)TCSM1_ADD_ERROR_BUF0;
#ifdef JZC_DBLKLI_OPT
        XCH2_T0->gp1_chain_ptr = (int *)TCSM1_DDMA_GP1_DES_CHAIN1; 
#ifdef JZC_ROTA90_OPT
	XCH2_T0->rota_upmb_ybuf = (uint8_t *)TCSM1_ROTA_UPMB_YBUF1;
	XCH2_T0->rota_upmb_ubuf = (uint8_t *)TCSM1_ROTA_UPMB_UBUF1;
	XCH2_T0->rota_upmb_vbuf = (uint8_t *)TCSM1_ROTA_UPMB_VBUF1;
#endif
#else
	XCH2_T0->gp2_chain_ptr = DDMA_GP2_DES_CHAIN0;
#endif
	
	int *gp1_chain_ptr = TCSM1_VUCADDR(XCH2_T0->gp1_chain_ptr);
	gp1_chain_ptr[2]=GP_STRD(64,GP_FRM_NML,64);
	gp1_chain_ptr[3]=GP_UNIT(GP_TAG_LK,64,64);

	gp1_chain_ptr[6]=GP_STRD(64,GP_FRM_NML,64);
	gp1_chain_ptr[7]=GP_UNIT(GP_TAG_LK,64,64);
	
        gp1_chain_ptr[18]=GP_STRD(256,GP_FRM_NML,256);
	gp1_chain_ptr[19]=GP_UNIT(GP_TAG_LK,256,256);

	gp1_chain_ptr[22]=GP_STRD(128,GP_FRM_NML,128);
	gp1_chain_ptr[23]=GP_UNIT(GP_TAG_LK,128,128);

	gp1_chain_ptr[26]=GP_STRD(4,GP_FRM_NML,16);
	gp1_chain_ptr[27]=GP_UNIT(GP_TAG_LK,4,4*16);
	
        gp1_chain_ptr[30]=GP_STRD(4,GP_FRM_NML,16);
	gp1_chain_ptr[31]=GP_UNIT(GP_TAG_LK,4,4*8);
	
	gp1_chain_ptr[34]=GP_STRD(4,GP_FRM_NML,16);

	gp1_chain_ptr[38]=GP_STRD(256,GP_FRM_NML,256);
	gp1_chain_ptr[39]=GP_UNIT(GP_TAG_LK,256,256);

        gp1_chain_ptr[42]=GP_STRD(256,GP_FRM_NML,256);
	gp1_chain_ptr[43]=GP_UNIT(GP_TAG_LK,256,256);

        gp1_chain_ptr[46]=GP_STRD(128,GP_FRM_NML,128);
	gp1_chain_ptr[47]=GP_UNIT(GP_TAG_LK,128,128);
	  
	gp1_chain_ptr[50]=GP_STRD(128,GP_FRM_NML,128);
	gp1_chain_ptr[51]=GP_UNIT(GP_TAG_UL,128,128);


	XCH2_T1->dblk_des_ptr = (uint8_t *)DBLK_DES_CHAIN1;
	XCH2_T1->dblk_mv_ptr = (uint8_t *)DBLK_MV_CHAIN1;
	XCH2_T1->dblk_out_addr = (uint8_t *)TCSM1_DBLK_MBOUT_Y1;
	XCH2_T1->dblk_out_addr_c = (uint8_t *)TCSM1_DBLK_MBOUT_U1;
	XCH2_T1->add_error_buf = TCSM1_ADD_ERROR_BUF1;
	//XCH2_T1->add_error_buf = (short *)TCSM1_ADD_ERROR_BUF0;
#ifdef JZC_DBLKLI_OPT
        XCH2_T1->gp1_chain_ptr = (int *)TCSM1_DDMA_GP1_DES_CHAIN2; 
#ifdef JZC_ROTA90_OPT
	XCH2_T1->rota_upmb_ybuf = (uint8_t *)TCSM1_ROTA_UPMB_YBUF2;
	XCH2_T1->rota_upmb_ubuf = (uint8_t *)TCSM1_ROTA_UPMB_UBUF2;
	XCH2_T1->rota_upmb_vbuf = (uint8_t *)TCSM1_ROTA_UPMB_VBUF2;
#endif
#else
	XCH2_T1->gp2_chain_ptr = DDMA_GP2_DES_CHAIN1;
#endif
	gp1_chain_ptr = TCSM1_VUCADDR(XCH2_T1->gp1_chain_ptr);
	gp1_chain_ptr[2]=GP_STRD(64,GP_FRM_NML,64);
	gp1_chain_ptr[3]=GP_UNIT(GP_TAG_LK,64,64);

	gp1_chain_ptr[6]=GP_STRD(64,GP_FRM_NML,64);
	gp1_chain_ptr[7]=GP_UNIT(GP_TAG_LK,64,64);
	
        gp1_chain_ptr[18]=GP_STRD(256,GP_FRM_NML,256);
	gp1_chain_ptr[19]=GP_UNIT(GP_TAG_LK,256,256);

	gp1_chain_ptr[22]=GP_STRD(128,GP_FRM_NML,128);
	gp1_chain_ptr[23]=GP_UNIT(GP_TAG_LK,128,128);

	gp1_chain_ptr[26]=GP_STRD(4,GP_FRM_NML,16);
	gp1_chain_ptr[27]=GP_UNIT(GP_TAG_LK,4,4*16);
	
        gp1_chain_ptr[30]=GP_STRD(4,GP_FRM_NML,16);
	gp1_chain_ptr[31]=GP_UNIT(GP_TAG_LK,4,4*8);
	
	gp1_chain_ptr[34]=GP_STRD(4,GP_FRM_NML,16);

	gp1_chain_ptr[38]=GP_STRD(256,GP_FRM_NML,256);
	gp1_chain_ptr[39]=GP_UNIT(GP_TAG_LK,256,256);

        gp1_chain_ptr[42]=GP_STRD(256,GP_FRM_NML,256);
	gp1_chain_ptr[43]=GP_UNIT(GP_TAG_LK,256,256);

        gp1_chain_ptr[46]=GP_STRD(128,GP_FRM_NML,128);
	gp1_chain_ptr[47]=GP_UNIT(GP_TAG_LK,128,128);
	  
	gp1_chain_ptr[50]=GP_STRD(128,GP_FRM_NML,128);
	gp1_chain_ptr[51]=GP_UNIT(GP_TAG_UL,128,128);

#ifdef JZC_DBLKLI_OPT
#else	
	int * gp2_chain_ptr = TCSM1_VUCADDR(XCH2_T0->gp2_chain_ptr);
	//UP_Y out
	gp2_chain_ptr[0]=XCH2_T0->dblk_upout_addr;
	gp2_chain_ptr[3]=GP_UNIT(GP_TAG_LK,16,16*4);
	//MB_Y out
	gp2_chain_ptr[4]=TCSM1_PADDR(XCH2_T0->dblk_out_addr);
	gp2_chain_ptr[7]=GP_UNIT(GP_TAG_LK,20,20*12);
	//UP_U out
	gp2_chain_ptr[8]=XCH2_T0->dblk_upout_addr_c;
	gp2_chain_ptr[11]=GP_UNIT(GP_TAG_LK,8,8*4);
	//MB_U out
	gp2_chain_ptr[12]=TCSM1_PADDR(XCH2_T0->dblk_out_addr_c);
	gp2_chain_ptr[15]=GP_UNIT(GP_TAG_LK,12,12*4);
	//UP_V out
	gp2_chain_ptr[16]=XCH2_T0->dblk_upout_addr_c+TCSM1_DBLK_UPOUT_UV_OFFSET;
	gp2_chain_ptr[19]=GP_UNIT(GP_TAG_LK,8,8*4);
	//MB_V out
	gp2_chain_ptr[20]=TCSM1_PADDR(XCH2_T0->dblk_out_addr_c+TCSM1_DBLK_MBOUT_UV_OFFSET);
	gp2_chain_ptr[23]=GP_UNIT(GP_TAG_UL,12,12*4);
	
	gp2_chain_ptr = TCSM1_VUCADDR(XCH2_T1->gp2_chain_ptr);
	//UP_Y out
	gp2_chain_ptr[0]=XCH2_T1->dblk_upout_addr;
	gp2_chain_ptr[3]=GP_UNIT(GP_TAG_LK,16,16*4);
	//MB_Y out
	gp2_chain_ptr[4]=TCSM1_PADDR(XCH2_T1->dblk_out_addr);
	gp2_chain_ptr[7]=GP_UNIT(GP_TAG_LK,20,20*12);
	//UP_U out
	gp2_chain_ptr[8]=XCH2_T1->dblk_upout_addr_c;
	gp2_chain_ptr[11]=GP_UNIT(GP_TAG_LK,8,8*4);
	//MB_U out
	gp2_chain_ptr[12]=TCSM1_PADDR(XCH2_T1->dblk_out_addr_c);
	gp2_chain_ptr[15]=GP_UNIT(GP_TAG_LK,12,12*4);
	//UP_V out
	gp2_chain_ptr[16]=XCH2_T1->dblk_upout_addr_c+TCSM1_DBLK_UPOUT_UV_OFFSET;
	gp2_chain_ptr[19]=GP_UNIT(GP_TAG_LK,8,8*4);
	//MB_V out
	gp2_chain_ptr[20]=TCSM1_PADDR(XCH2_T1->dblk_out_addr_c+TCSM1_DBLK_MBOUT_UV_OFFSET);
	gp2_chain_ptr[23]=GP_UNIT(GP_TAG_UL,12,12*4);
#endif	

	int *gp0_chain_ptr=(int *)TCSM1_VUCADDR(DDMA_GP0_DES_CHAIN);
	gp0_chain_ptr[2] = GP_STRD(64,GP_FRM_NML,64);

	gp0_chain_ptr[4] = TCSM0_PADDR(TCSM0_GP0_END_FLAG);
	gp0_chain_ptr[5] = TCSM1_PADDR(TCSM1_GP0_END_FLAG);
	gp0_chain_ptr[6] = GP_STRD(4,GP_FRM_NML,4);       
	gp0_chain_ptr[7] = GP_UNIT(GP_TAG_UL,4,4);

	set_gp0_dha(TCSM1_PADDR(DDMA_GP0_DES_CHAIN));
      }else{
	p1_dblk_decode_cavlc_init();
	h264_idct_cavlc_init();

	XCH2_T0 = (H264_XCH2_T *)TCSM1_VUCADDR(TCSM1_XCH2_T_BUF0);
	XCH2_T1 = (H264_XCH2_T *)TCSM1_VUCADDR(TCSM1_XCH2_T_BUF1);
	
	XCH2_T0->dblk_des_ptr = (uint8_t *)DBLK_DES_CHAIN0;
	XCH2_T0->dblk_mv_ptr = (uint8_t *)DBLK_MV_CHAIN0;
	XCH2_T0->dblk_out_addr = (uint8_t *)TCSM1_DBLK_MBOUT_Y0;
	XCH2_T0->dblk_out_addr_c = (uint8_t *)TCSM1_DBLK_MBOUT_U0;
	XCH2_T0->add_error_buf = (short *)TCSM1_ADD_ERROR_BUF0;

        XCH2_T0->gp1_chain_ptr = (int *)TCSM1_DDMA_GP1_DES_CHAIN1; 

	int *gp1_chain_ptr = TCSM1_VUCADDR(XCH2_T0->gp1_chain_ptr);
	gp1_chain_ptr[2]=GP_STRD(64,GP_FRM_NML,64);
	gp1_chain_ptr[3]=GP_UNIT(GP_TAG_LK,64,64);

	gp1_chain_ptr[6]=GP_STRD(64,GP_FRM_NML,64);
	gp1_chain_ptr[7]=GP_UNIT(GP_TAG_LK,64,64);
	
        gp1_chain_ptr[10]=GP_STRD(192,GP_FRM_NML,192);
	gp1_chain_ptr[11]=GP_UNIT(GP_TAG_LK,192,192);
	
        gp1_chain_ptr[14]=GP_STRD(64,GP_FRM_NML,64);
	gp1_chain_ptr[15]=GP_UNIT(GP_TAG_LK,64,64);
	
        gp1_chain_ptr[18]=GP_STRD(256,GP_FRM_NML,256);
	gp1_chain_ptr[19]=GP_UNIT(GP_TAG_LK,256,256);

	gp1_chain_ptr[22]=GP_STRD(128,GP_FRM_NML,128);
	gp1_chain_ptr[23]=GP_UNIT(GP_TAG_LK,128,128);

	gp1_chain_ptr[26]=GP_STRD(4,GP_FRM_NML,16);
	gp1_chain_ptr[27]=GP_UNIT(GP_TAG_LK,4,4*16);
	
        gp1_chain_ptr[30]=GP_STRD(4,GP_FRM_NML,16);
	gp1_chain_ptr[31]=GP_UNIT(GP_TAG_LK,4,4*8);
	
	gp1_chain_ptr[34]=GP_STRD(4,GP_FRM_NML,16);
	gp1_chain_ptr[35]=GP_UNIT(GP_TAG_UL,4,4*8);


#ifdef JZC_ROTA90_OPT
	XCH2_T0->rota_upmb_ybuf = (uint8_t *)TCSM1_ROTA_UPMB_YBUF1;
	XCH2_T0->rota_upmb_ubuf = (uint8_t *)TCSM1_ROTA_UPMB_UBUF1;
	XCH2_T0->rota_upmb_vbuf = (uint8_t *)TCSM1_ROTA_UPMB_VBUF1;
#endif
	
	XCH2_T1->dblk_des_ptr = (uint8_t *)DBLK_DES_CHAIN1;
	XCH2_T1->dblk_mv_ptr = (uint8_t *)DBLK_MV_CHAIN1;
	XCH2_T1->dblk_out_addr = (uint8_t *)TCSM1_DBLK_MBOUT_Y1;
	XCH2_T1->dblk_out_addr_c = (uint8_t *)TCSM1_DBLK_MBOUT_U1;
	//XCH2_T1->add_error_buf = TCSM1_ADD_ERROR_BUF1;
	XCH2_T1->add_error_buf = (short *)TCSM1_ADD_ERROR_BUF0;

        XCH2_T1->gp1_chain_ptr = (int *)TCSM1_DDMA_GP1_DES_CHAIN2; 
	
	gp1_chain_ptr = TCSM1_VUCADDR(XCH2_T1->gp1_chain_ptr);
	gp1_chain_ptr[2]=GP_STRD(64,GP_FRM_NML,64);
	gp1_chain_ptr[3]=GP_UNIT(GP_TAG_LK,64,64);

	gp1_chain_ptr[6]=GP_STRD(64,GP_FRM_NML,64);
	gp1_chain_ptr[7]=GP_UNIT(GP_TAG_LK,64,64);
	
        gp1_chain_ptr[10]=GP_STRD(192,GP_FRM_NML,192);
	gp1_chain_ptr[11]=GP_UNIT(GP_TAG_LK,192,192);
	
        gp1_chain_ptr[14]=GP_STRD(64,GP_FRM_NML,64);
	gp1_chain_ptr[15]=GP_UNIT(GP_TAG_LK,64,64);
	
        gp1_chain_ptr[18]=GP_STRD(256,GP_FRM_NML,256);
	gp1_chain_ptr[19]=GP_UNIT(GP_TAG_LK,256,256);

	gp1_chain_ptr[22]=GP_STRD(128,GP_FRM_NML,128);
	gp1_chain_ptr[23]=GP_UNIT(GP_TAG_LK,128,128);

	gp1_chain_ptr[26]=GP_STRD(4,GP_FRM_NML,16);
	gp1_chain_ptr[27]=GP_UNIT(GP_TAG_LK,4,4*16);
	
        gp1_chain_ptr[30]=GP_STRD(4,GP_FRM_NML,16);
	gp1_chain_ptr[31]=GP_UNIT(GP_TAG_LK,4,4*8);
	
	gp1_chain_ptr[34]=GP_STRD(4,GP_FRM_NML,16);
	gp1_chain_ptr[35]=GP_UNIT(GP_TAG_UL,4,4*8);

	XCH2_T1->rota_upmb_ybuf = (uint8_t *)TCSM1_ROTA_UPMB_YBUF2;
	XCH2_T1->rota_upmb_ubuf = (uint8_t *)TCSM1_ROTA_UPMB_UBUF2;
	XCH2_T1->rota_upmb_vbuf = (uint8_t *)TCSM1_ROTA_UPMB_VBUF2;

	int *gp0_chain_ptr=(int *)TCSM1_VUCADDR(DDMA_GP0_DES_CHAIN);
	gp0_chain_ptr[2] = GP_STRD(64,GP_FRM_NML,64);

	gp0_chain_ptr[4] = TCSM1_PADDR(CV_FOR_SET_FIFO_ZERO);
	gp0_chain_ptr[6] = GP_STRD(64,GP_FRM_NML,64);

	set_gp0_dha(TCSM1_PADDR(DDMA_GP0_DES_CHAIN));
      }

#ifdef JZC_DCORE_OPT
      {
	//several setup instructions direct p1 to execute p1_boot
	int i;
	//volatile int *src, *dst;
	// load p1 insn and data to reserved mem
	FILE *fp_text;
	int len, *reserved_mem;
	int *load_buf;
	if (pps->cabac){
#ifdef ANDROID
        printf("h264_p1.bin load addr: %p",TCSM1_VUCADDR(P1_MAIN_ADDR));
	  
	  if(loadfile("h264_p1.bin",(int *)TCSM1_VUCADDR(P1_MAIN_ADDR),SPACE_HALF_MILLION_BYTE,1) == -1){
		  printf("LOAD H264_P1_BIN ERROR.....................\n");
		  return -1;
	  }
#else
#if 1   //#ifdef JZ_LINUX_OS
	printf("h264 len of aux task = %d\n", h264_auxcodes_len);
	reserved_mem = (int *)TCSM1_VCADDR(P1_MAIN_ADDR);
        for (i=0; i< h264_auxcodes_len/4; i++)
          reserved_mem[i] = h264_aux_task_codes[i];

#else
	  fp_text = fopen("./h264_p1.bin", "r+b");
	  if (!fp_text){
	    printf(" error while open h264_p1.bin \n");
	    exit_player_with_rc();
	  }
	  load_buf = tmp_hm_buf;
	  len = fread(load_buf, 4, SPACE_HALF_MILLION_BYTE, fp_text);
	  reserved_mem = (int *)TCSM1_VCADDR(P1_MAIN_ADDR);
#endif
#endif
	}else{
#ifdef ANDROID
        if(loadfile("h264_cavlc_p1.bin",(int *)TCSM1_VUCADDR(P1_MAIN_ADDR),SPACE_HALF_MILLION_BYTE,1) == -1){
		  printf("LOAD H264_CAVLC_P1_BIN ERROR.....................\n");
		  return -1;
	  }
#else
#if 1  //#ifdef JZ_LINUX_OS
	reserved_mem = (int *)TCSM1_VCADDR(P1_MAIN_ADDR);
	printf("h264 len of cavlc aux task = %d\n", h264_cavlc_auxcodes_len);
        for (i=0; i< h264_cavlc_auxcodes_len/4; i++)
          reserved_mem[i] = h264_cavlc_aux_task_codes[i];
#else
	  fp_text = fopen("./h264_cavlc_p1.bin", "r+b");
	  if (!fp_text){
	    printf(" error while open h264_cavlc_p1.bin \n");
	    exit_player_with_rc();
	  }
	  load_buf = tmp_hm_buf;
	  len = fread(load_buf, 4, SPACE_HALF_MILLION_BYTE, fp_text);
	  reserved_mem = (int *)TCSM1_VCADDR(P1_MAIN_ADDR);
#endif
#endif
	}
	//#undef printf
	//printf("----------------------h264_p1.bin len = %d\n", len);
#ifndef ANDROID
#if 0 //#ifndef JZ_LINUX_OS
	for(i=0; i<len; i++)
	  reserved_mem[i] = load_buf[i];
	fclose(fp_text);
#endif
#endif
	jz_dcache_wb(); /*flush cache into reserved mem*/
	i_sync();
      }
#endif
    }
    pps->pic_order_present= get_bits1(&s->gb);
    pps->slice_group_count= get_ue_golomb(&s->gb) + 1;
    if(pps->slice_group_count > 1 ){
        pps->mb_slice_group_map_type= get_ue_golomb(&s->gb);
        av_log(h->s.avctx, AV_LOG_ERROR, "FMO not supported\n");
        switch(pps->mb_slice_group_map_type){
        case 0:
#if 0
|   for( i = 0; i <= num_slice_groups_minus1; i++ ) |   |        |
|    run_length[ i ]                                |1  |ue(v)   |
#endif
            break;
        case 2:
#if 0
|   for( i = 0; i < num_slice_groups_minus1; i++ )  |   |        |
|{                                                  |   |        |
|    top_left_mb[ i ]                               |1  |ue(v)   |
|    bottom_right_mb[ i ]                           |1  |ue(v)   |
|   }                                               |   |        |
#endif
            break;
        case 3:
        case 4:
        case 5:
#if 0
|   slice_group_change_direction_flag               |1  |u(1)    |
|   slice_group_change_rate_minus1                  |1  |ue(v)   |
#endif
            break;
        case 6:
#if 0
|   slice_group_id_cnt_minus1                       |1  |ue(v)   |
|   for( i = 0; i <= slice_group_id_cnt_minus1; i++ |   |        |
|)                                                  |   |        |
|    slice_group_id[ i ]                            |1  |u(v)    |
#endif
            break;
        }
    }
    pps->ref_count[0]= get_ue_golomb(&s->gb) + 1;
    pps->ref_count[1]= get_ue_golomb(&s->gb) + 1;
    if(pps->ref_count[0]-1 > 32-1 || pps->ref_count[1]-1 > 32-1){
        av_log(h->s.avctx, AV_LOG_ERROR, "reference overflow (pps)\n");
        goto fail;
    }

    pps->weighted_pred= get_bits1(&s->gb);
    pps->weighted_bipred_idc= get_bits(&s->gb, 2);
    pps->init_qp= get_se_golomb(&s->gb) + 26;
    pps->init_qs= get_se_golomb(&s->gb) + 26;
    pps->chroma_qp_index_offset[0]= get_se_golomb(&s->gb);
    pps->deblocking_filter_parameters_present= get_bits1(&s->gb);
    pps->constrained_intra_pred= get_bits1(&s->gb);
    pps->redundant_pic_cnt_present = get_bits1(&s->gb);

    pps->transform_8x8_mode= 0;
    h->dequant_coeff_pps= -1; //contents of sps/pps can change even if id doesn't, so reinit
    memcpy(pps->scaling_matrix4, h->sps_buffers[pps->sps_id]->scaling_matrix4, sizeof(pps->scaling_matrix4));
    memcpy(pps->scaling_matrix8, h->sps_buffers[pps->sps_id]->scaling_matrix8, sizeof(pps->scaling_matrix8));

    if(get_bits_count(&s->gb) < bit_length){
        pps->transform_8x8_mode= get_bits1(&s->gb);
        decode_scaling_matrices(h, h->sps_buffers[pps->sps_id], pps, 0, pps->scaling_matrix4, pps->scaling_matrix8);
        pps->chroma_qp_index_offset[1]= get_se_golomb(&s->gb); //second_chroma_qp_index_offset
    } else {
        pps->chroma_qp_index_offset[1]= pps->chroma_qp_index_offset[0];
    }

    build_qp_table(pps, 0, pps->chroma_qp_index_offset[0]);
    build_qp_table(pps, 1, pps->chroma_qp_index_offset[1]);
    if(pps->chroma_qp_index_offset[0] != pps->chroma_qp_index_offset[1])
        pps->chroma_qp_diff= 1;

    if(s->avctx->debug&FF_DEBUG_PICT_INFO){
        av_log(h->s.avctx, AV_LOG_DEBUG, "pps:%u sps:%u %s slice_groups:%d ref:%d/%d %s qp:%d/%d/%d/%d %s %s %s %s\n",
               pps_id, pps->sps_id,
               pps->cabac ? "CABAC" : "CAVLC",
               pps->slice_group_count,
               pps->ref_count[0], pps->ref_count[1],
               pps->weighted_pred ? "weighted" : "",
               pps->init_qp, pps->init_qs, pps->chroma_qp_index_offset[0], pps->chroma_qp_index_offset[1],
               pps->deblocking_filter_parameters_present ? "LPAR" : "",
               pps->constrained_intra_pred ? "CONSTR" : "",
               pps->redundant_pic_cnt_present ? "REDU" : "",
               pps->transform_8x8_mode ? "8x8DCT" : ""
               );
    }

    av_free(h->pps_buffers[pps_id]);
    h->pps_buffers[pps_id]= pps;
    return 0;
fail:
    av_free(pps);
    return -1;
}

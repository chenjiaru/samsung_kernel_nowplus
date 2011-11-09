/*
 * ==========================================================================
 *               Texas Instruments OMAP(TM) Platform Firmware
 * (c) Copyright 2009, Texas Instruments Incorporated.  All Rights Reserved.
 *
 *  Use of this firmware is controlled by the terms and conditions found
 *  in the license agreement under which this firmware has been supplied.
 * ==========================================================================
 */

#include "abe_main.h"

/*
 *  ABE_DEFAULT_IRQ_PINGPONG_PLAYER
 *
 *
 *  Operations :
 *      generates data for the cache-flush buffer
 *
 *  Return value :
 *      None.
 */
void abe_default_irq_pingpong_player(void)
{
	/* ping-pong access to MM_DL at 48kHz Mono with 20ms packet sizes */
	#define N_SAMPLES_MAX ((int)(1024))

	static abe_int32 idx;
	abe_uint32 i, dst, n_samples;
	abe_int32 temp [N_SAMPLES_MAX], audio_sample;
	const abe_int32 audio_pattern[8] = {16383,16383,16383,16383,-16384,-16384,-16384,-16384};

	/* read the address of the Pong buffer */
	abe_read_next_ping_pong_buffer (MM_DL_PORT, &dst, &n_samples);

	/* generate a test pattern */
	for (i = 0; i < n_samples; i++) {
		idx = (idx +1) & 7;	/* circular addressing */
		audio_sample = audio_pattern [idx];
		temp [i] = ((audio_sample << 16) + audio_sample);
	}

	/* copy the pattern (flush it) to DMEM pointer update
	 * not necessary here because the buffer size do not
	 * change from one ping to the other pong
	*/
	abe_block_copy(COPY_FROM_HOST_TO_ABE, ABE_DMEM, dst,
				(abe_uint32 *)&(temp[0]), n_samples * 4);
	abe_set_ping_pong_buffer(MM_DL_PORT, n_samples * 4);
}

/*
 *  initialize the default values for call-backs to subroutines
 *      - FIFO IRQ call-backs for sequenced tasks
 *      - FIFO IRQ call-backs for audio player/recorders (ping-pong protocols)
 *      - Remote debugger interface
 *      - Error monitoring
 *      - Activity Tracing
 */

/*
 *  ABE_READ_SYS_CLOCK
 *
 *  Parameter  :
 *      pointer to the system clock
 *
 *  Operations :
 *      returns the current time indication for the LOG
 *
 *  Return value :
 *      None.
 */
void abe_read_sys_clock(abe_micros_t *time)
{
	static abe_micros_t clock;

	*time = clock;
	clock ++;
}

/*
 *  ABE_APS_TUNING
 *
 *  Parameter  :
 *
 *
 *  Operations :
 *
 *
 *  Return value :
 *
 */
void abe_aps_tuning(void)
{
}

/**
* @fn abe_lock_executione()
*
*  Operations : set a spin-lock and wait in case of collision
*
*
* @see	ABE_API.h
*/
void abe_lock_execution(void)
{
}

/**
* @fn abe_unlock_executione()
*
*  Operations : reset a spin-lock (end of subroutine)
*
*
* @see	ABE_API.h
*/
void abe_unlock_execution(void)
{
}

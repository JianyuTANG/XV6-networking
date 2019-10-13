#include "types.h"
#include "user.h"
#include "sound.h"
#include "common.h"
#include "decodemp3.h"
int main()
{
    //struct coreBuf* corebuf;
    III_scalefac_t III_scalefac;
    typedef short PCM[2][SSLIMIT][SBLIMIT];
    PCM *pcm_sample;
    pcm_sample = (PCM *) mem_alloc((long) sizeof(PCM), "PCM Samp");
    unsigned long sample_frames = 0;
    //corebuf = (struct coreBuf*)getCoreBuf();
    struct frame_params fr_ps;
    struct III_side_info_t III_side_info; 
    while (1) {
    printf(0, "decode\n");
    waitForMP3Decode(&fr_ps, &III_side_info);
    int clip = 0, gr, ch, ss, sb;
    for (gr=0;gr<2;gr++) {
	double lr[2][SBLIMIT][SSLIMIT],ro[2][SBLIMIT][SSLIMIT];
	for (ch=0; ch<fr_ps.stereo; ch++) {
	    long int is[SBLIMIT][SSLIMIT];
	    int part2_start;
	    part2_start = hsstell();
	    III_get_scale_factors(&III_scalefac,&(III_side_info), gr, ch, &(fr_ps));
	    III_hufman_decode(is, &(III_side_info), ch, gr, part2_start, &(fr_ps));
	    III_dequantize_sample(is, ro[ch], &III_scalefac, &(III_side_info.ch[ch].gr[gr]), ch, &(fr_ps));
	}
	III_stereo(ro, lr, &III_scalefac, &(III_side_info.ch[0].gr[gr]), &(fr_ps));
	printf(0, "%d\n", fr_ps.stereo);
	for (ch=0; ch<fr_ps.stereo; ch++) {
	    double re[SBLIMIT][SSLIMIT];
	    double hybridIn[SBLIMIT][SSLIMIT];/* Hybrid filter input */
	    double hybridOut[SBLIMIT][SSLIMIT];/* Hybrid filter out */
	    double polyPhaseIn[SBLIMIT];     /* PolyPhase Input. */
	    III_reorder(lr[ch], re, &(III_side_info.ch[ch].gr[gr]), &(fr_ps));
	    III_antialias(re, hybridIn, /* Antialias butterflies. */
		&(III_side_info.ch[ch].gr[gr]), &(fr_ps));
	    //IMDCT
	    for (sb=0; sb<SBLIMIT; sb++) { /* Hybrid synthesis. */
		III_hybrid(hybridIn[sb], hybridOut[sb], sb, ch,	&(III_side_info.ch[ch].gr[gr]), &(fr_ps));
	    }
	    for (ss=0;ss<18;ss++)	
		for (sb=0; sb<SBLIMIT; sb++)
		    if ((ss%2) && (sb%2))
			hybridOut[sb][ss] = -hybridOut[sb][ss];
	    for (ss=0;ss<18;ss++) { 
		for (sb=0; sb<SBLIMIT; sb++)
		    polyPhaseIn[sb] = hybridOut[sb][ss];
		    
		    clip += SubBandSynthesis(polyPhaseIn, ch, &((*pcm_sample)[ch][ss][0]));
	    }
	}
	/* Output PCM sample points for one granule. */
 	out_fifo(*pcm_sample, 18, &(fr_ps), &sample_frames);
	endMP3Decode();	
    }
    }
    exit();
}


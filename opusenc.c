/*
 * faraday-rf/opusenc.c
 * 
 * This module reads 16-bit signed-linear samples at 16KHz from stdin
 * and writes a sequence number and fixed length Opus encoded payload
 * on stdout
 * 
 * We sample at 16KHz rather than Opus' desired 48KHz to make sure we
 * don't run afoul of soundcards that only go up to 44.1KHz.
 */

#include <stdio.h>      // fprintf and stderr stuff
#include <unistd.h>     // read,write
#include <string.h>     // memset
#include <opus/opus.h>
#include <netinet/in.h>         // htons, we send in "network" order

// Refer to stdin/out by numeric id rather than FILE handle
const uint8_t infd  = 0;
const uint8_t outfd = 1;

const uint16_t frame_size_in = 640; // 16000Hz * 16-bits (2 byte) * 20 ms (0.02)

opus_int16 sample_buf[1024];    // Input audio sample buffer
OpusEncoder *oe;                // Opus encoder state
int oerr;                       // Error code from the encoder
opus_int32 opus_len;            // Return length from the encoder
uint8_t out_buf[256];           // Radio frame buffer (overly generous)
uint16_t i;                     // generic iterator
uint16_t frame_seq;             // Frame sequence number
uint16_t *p_frame_seq;          // Pointer to frame sequence in outgoing frame

int main(void) {

    // Set up the Opus encoder
    oe = opus_encoder_create(16000,1,OPUS_APPLICATION_VOIP,&oerr);
    opus_encoder_ctl(oe,OPUS_SET_BITRATE(12000));
    opus_encoder_ctl(oe,OPUS_SET_VBR(0)); // Hard CBR

    for(frame_seq=1;;frame_seq++) {
        // For sanity, blank out the output buffer
        memset(out_buf,0x00,sizeof(out_buf));
    
        if (read(infd,&sample_buf,frame_size_in) == 0) {
            fprintf(stderr,"Short read on input sample\n");
            break;
        };
   
        // Load output payload at offset to allow for sequence number
        // Max encode size set to 240, len(out_buf) minus some breathing room
        opus_len = opus_encode(oe,sample_buf,frame_size_in/2,out_buf+2,240);
        if (opus_len < 0) {
            fprintf(stderr,"Opus encoder threw a wobbly\n");
            break;
        };

        // Load frame sequence to the beginning of our buffer
        p_frame_seq = out_buf + 0;  // This causes a type warning
        *p_frame_seq = htons(frame_seq);

        // opus_len should be a constant 30 bytes
        fprintf(stderr,"DEBUG: seq=%5d, opus_len=%d\n",frame_seq,opus_len);
        
        write(outfd,&out_buf,32);
    
    } /* end for */
    
    return(0); 
} /* end main */

/*
 * faraday-rf/opusenc.c
 * 
 * This module reads fixed length packets from the radio and decodes
 * them. At present we simply pass the payload through the Opus
 * decoder. We will later pay closer attention to the sequence number
 * to invoke packet loss concealment or reset the decoder as needed.
 * 
 * Opus prefers to operate at 48KHz, but we run the pipeline at 16KHz
 * to avoid any issues with directly accessing a soundcard that can
 * only handle 44.1KHz.
 */

#include <stdio.h>      // fprintf and stderr stuff
#include <unistd.h>     // read,write
#include <string.h>     // memset
#include <opus/opus.h>
#include <netinet/in.h>         // htons, we send in "network" order

// Refer to stdin/out by numeric id rather than FILE handle
const uint8_t infd  = 0;
const uint8_t outfd = 1;

opus_int16 sample_buf[1024];    // PulseAudio sample buffer
OpusDecoder *od;                // Opus decoder state
int oerr;                       // Error code from the decoder
int num_samples;                // Number of samples decoded by Opus
uint8_t in_buf[256];            // Radio frame buffer

uint16_t frame_seq;             // Frame sequence number, local
uint16_t frame_seq_in;          // Frame sequence recieved
uint16_t *p_frame_seq_in;       // Pointer to frame sequence in incoming frame

int main(void) {
    // Set up the Opus encoder
    od = opus_decoder_create(16000,1,&oerr);

    // Set initial frame sequence counter
    frame_seq = 1;

    while (1) {
        // For sanity, blank out the input buffer
        memset(in_buf,0x00,sizeof(in_buf));
        
        // Read expected number of bytes
        if (read(infd,&in_buf,32) != 32) {
            fprintf(stderr,"Error reading from input\n");
            break;
        }
        
        // Extract frame sequence number
        p_frame_seq_in = in_buf+0;  // This throws a type warning
        frame_seq_in = ntohs(*p_frame_seq_in);

        // Someday we'll actually DO something with frame sequence
        // For now it's a good visual indicator of a dropped packet
        fprintf(stderr,"DEBUG: seq_mine=%5d, seq_theirs=%5d\n", frame_seq, frame_seq_in);
                
        num_samples = opus_decode(od,in_buf+2,30,sample_buf,sizeof(sample_buf)/2,0);

        write(outfd,&sample_buf,num_samples*2);
        frame_seq++;
    } /* end while (infinite loop) */
    
    return(0);
} /* end main*/


#include "_cgo_export.h"
#include <stdio.h>
#include <stdlib.h>
#include "FLAC/stream_decoder.h"

// ----------------------------------------------------------------------------
typedef struct {
  int failed;
  int length;
  int16_t* L;
  int16_t* R;
} UserData; 

// ----------------------------------------------------------------------------
static FLAC__StreamDecoderWriteStatus writeCB(const FLAC__StreamDecoder *dec, 
                                              const FLAC__Frame *frame, 
                                              const FLAC__int32 * const buf[],
                                              void *data) {
  UserData *userData = data;
  // This is the first sample in the frame. 
  int idx = frame->header.number.sample_number;

  int i;
  for(i = 0; i < frame->header.blocksize; i++) {
    userData->L[idx + i] = (FLAC__int16)(buf[0][i]);
    userData->R[idx + i] = (FLAC__int16)(buf[1][i]);
  }

  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

// ----------------------------------------------------------------------------
static void metaCB(const FLAC__StreamDecoder *dec, 
                   const FLAC__StreamMetadata *metadata, 
                   void *data){
  if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
    UserData *userData = data;
    
    // Only allow 16-bit stereo at 48 kHz. 
    if((metadata->data.stream_info.channels != 2) ||
       (metadata->data.stream_info.sample_rate != 48000) ||
       (metadata->data.stream_info.bits_per_sample != 16)) {
        userData->failed = 1;
    }

    // Set the total length and current index (0).
    userData->length = metadata->data.stream_info.total_samples;
    
    // Create new arrays for L and R arrays. 
    userData->L = malloc(userData->length * sizeof(int16_t));
    userData->R = malloc(userData->length * sizeof(int16_t));
  }
}

// ----------------------------------------------------------------------------
static void errorCB(const FLAC__StreamDecoder *dec, 
                    FLAC__StreamDecoderErrorStatus status, 
                    void *data) {
  UserData *userData = data;
  userData->failed = 1;
}

// ----------------------------------------------------------------------------
void LoadFlacFromFile(char* path, UserData* userData) {
  FLAC__bool ok = true;
  FLAC__StreamDecoder *decoder = 0;
  FLAC__StreamDecoderInitStatus initStatus;
  
  // Create the decoder. 
  decoder = FLAC__stream_decoder_new();
  if(decoder == NULL) {
    userData->failed = 1;
    return;
  }
  
  // Initialize the decoder. 
  initStatus = FLAC__stream_decoder_init_file(decoder, path, writeCB, metaCB, 
                                              errorCB, userData);
  if(initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
    userData->failed = 1;
    FLAC__stream_decoder_delete(decoder);
    return;
  }
  
  // Decode the stream. 
  ok = FLAC__stream_decoder_process_until_end_of_stream(decoder);
  if(!ok) {
    userData->failed = 1;
  }
  
  FLAC__stream_decoder_finish(decoder);
  FLAC__stream_decoder_delete(decoder);
}


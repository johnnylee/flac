
#include "_cgo_export.h"
#include <stdio.h>
#include <stdlib.h>
#include "FLAC/stream_decoder.h"

extern void createArrays(void* ptr, int length);
extern void* getL(void* ptr);
extern void* getR(void* ptr);
extern void setFailed(void* ptr);

typedef struct {
  void *ptr;
  int16_t *L;
  int16_t *R;
} data_t;

// ----------------------------------------------------------------------------
static FLAC__StreamDecoderWriteStatus writeCB(const FLAC__StreamDecoder *dec, 
                                              const FLAC__Frame *frame, 
                                              const FLAC__int32 * const buf[],
                                              void *_data) {
  data_t* data = _data; 
  int16_t* L = data->L;
  int16_t* R = data->R;

  // This is the first sample in the frame. 
  int idx = frame->header.number.sample_number;

  int i;
  for(i = 0; i < frame->header.blocksize; i++) {
    L[idx + i] = (FLAC__int16)(buf[0][i]);
    R[idx + i] = (FLAC__int16)(buf[1][i]);
  }

  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

// ----------------------------------------------------------------------------
static void metaCB(const FLAC__StreamDecoder *dec, 
                   const FLAC__StreamMetadata *metadata, 
                   void *_data) {
  data_t* data = _data;

  if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
    // Only allow 16-bit stereo at 48 kHz. 
    if((metadata->data.stream_info.channels != 2) ||
       (metadata->data.stream_info.sample_rate != 48000) ||
       (metadata->data.stream_info.bits_per_sample != 16)) {
      setFailed(data->ptr);
    }

    // Create new arrays for L and R arrays. 
    int length = metadata->data.stream_info.total_samples;
    createArrays(data->ptr, length);
    data->L = getL(data->ptr);
    data->R = getR(data->ptr);
  }
}

// ----------------------------------------------------------------------------
static void errorCB(const FLAC__StreamDecoder *dec, 
                    FLAC__StreamDecoderErrorStatus status, 
                    void *_data) {
  data_t* data = _data;
  setFailed(data->ptr);
}

// ----------------------------------------------------------------------------
void LoadFlacFromFile(char* path, void* ptr) {
  FLAC__bool ok = true;
  FLAC__StreamDecoder *decoder = 0;
  FLAC__StreamDecoderInitStatus initStatus;
  
  data_t data; 
  data.ptr = ptr;
  
  // Create the decoder. 
  decoder = FLAC__stream_decoder_new();
  if(decoder == NULL) {
    setFailed(ptr);
    return;
  }
  
  // Initialize the decoder. 
  initStatus = FLAC__stream_decoder_init_file(decoder, path, writeCB, metaCB, 
                                              errorCB, &data);
  if(initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
    FLAC__stream_decoder_delete(decoder);
    setFailed(ptr);
    return;
  }
  
  // Decode the stream. 
  ok = FLAC__stream_decoder_process_until_end_of_stream(decoder);
  if(!ok) {
    setFailed(ptr);
  }
  
  FLAC__stream_decoder_finish(decoder);
  FLAC__stream_decoder_delete(decoder);
}


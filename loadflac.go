package flac

import (
	"errors"
	"reflect"
	"unsafe"
)

/*
#cgo pkg-config: flac
#include <FLAC/stream_decoder.h>

typedef struct {
  int failed;
  int length;
  int16_t* L;
  int16_t* R;
} UserData;

extern void LoadFlacFromFile(char*, UserData*);
*/
import "C"

// ----------------------------------------------------------------------------
func cArrayToSlice16(cArray *C.int16_t, length int) []int16 {
	var goSlice []int16
	sliceHeader := (*reflect.SliceHeader)((unsafe.Pointer(&goSlice)))
	sliceHeader.Cap = length
	sliceHeader.Len = length
	sliceHeader.Data = uintptr(unsafe.Pointer(cArray))
	return goSlice
}

// ----------------------------------------------------------------------------
// Load the stero flac file.
// Returns the left and right channels as int16 slices.
// Currently only loads stereo, 16-bit, 48 kHz flac files.
func LoadInt16(path string) ([]int16, []int16, error) {
	var data C.UserData
	C.LoadFlacFromFile(C.CString(path), &data)

	if data.failed != 0 {
		return nil, nil, errors.New("Failed to load file: " + path)
	}

	l := int(data.length)
	return cArrayToSlice16(data.L, l), cArrayToSlice16(data.R, l), nil
}

// ----------------------------------------------------------------------------
// Like LoadInt16, but returns float32 slices.
func LoadFloat32(path string) ([]float32, []float32, error) {
	L1, R1, err := LoadInt16(path)
	if err != nil {
		return nil, nil, err
	}

	L := make([]float32, len(L1))
	R := make([]float32, len(R1))

	for i, _ := range L1 {
		L[i] = float32(L1[i]) / 32767.0
		R[i] = float32(R1[i]) / 32767.0
	}

	return L, R, nil
}

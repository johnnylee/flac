package flac

import (
	"errors"
	"unsafe"
)

/*
#cgo pkg-config: flac
#include <FLAC/stream_decoder.h>

extern void LoadFlacFromFile(char*, void*);
*/
import "C"

type flacLoader struct {
	ok bool 
	L []int16
	R []int16
}

//export createArrays
func createArrays(ptr unsafe.Pointer, cLength C.int) {
	length := int(cLength)
	loader := (*flacLoader)(ptr)
	loader.L = make([]int16, length)
	loader.R = make([]int16, length)
}

//export getL
func getL(ptr unsafe.Pointer) unsafe.Pointer {
	loader := (*flacLoader)(ptr)
	return unsafe.Pointer(&loader.L[0])
}

//export getR
func getR(ptr unsafe.Pointer) unsafe.Pointer {
	loader := (*flacLoader)(ptr)
	return unsafe.Pointer(&loader.R[0])
}

//export setFailed
func setFailed(ptr unsafe.Pointer) {
	(*flacLoader)(ptr).ok = false
}

func LoadInt16(path string) ([]int16, []int16, error) {
	loader := new(flacLoader)
	loader.ok = true
	
	C.LoadFlacFromFile(C.CString(path), unsafe.Pointer(loader))

	if !loader.ok {
		return nil, nil, errors.New("Failed to load flac file.")
	}
	return loader.L, loader.R, nil
}

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

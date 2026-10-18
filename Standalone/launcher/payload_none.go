//go:build !payload

package main

// Without the payload tag the runtime is found on disk instead: in an .app
// bundle's Resources dir, next to the executable, or on PATH.
var payloadZip []byte

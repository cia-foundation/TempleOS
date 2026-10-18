//go:build payload

package main

import _ "embed"

// payload.zip holds runtime/ (QEMU and its firmware) and TempleOS.qcow2.
// The Windows build embeds it so the app is a single .exe.
//
//go:embed payload.zip
var payloadZip []byte

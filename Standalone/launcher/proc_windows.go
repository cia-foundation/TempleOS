//go:build windows

package main

import (
	"bytes"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"syscall"
	"time"
	"unsafe"
)

const createNoWindow = 0x08000000

// runQEMU starts QEMU and waits for it. Windows has no exec(), and the
// launcher is a GUI-subsystem program, so errors go to a message box.
func runQEMU(qemu string, args []string, env []string) error {
	err := runOnce(qemu, args, env)
	var quick *quickExit
	if errors.As(err, &quick) && hasAudio(args) {
		// No usable sound device: try again without the PC speaker.
		return runOnce(qemu, withoutAudio(args), env)
	}
	return err
}

type quickExit struct{ msg string }

func (e *quickExit) Error() string { return e.msg }

func runOnce(qemu string, args []string, env []string) error {
	var stderr bytes.Buffer
	cmd := exec.Command(qemu, args...)
	cmd.Env = env
	cmd.Stderr = &stderr
	cmd.SysProcAttr = &syscall.SysProcAttr{CreationFlags: createNoWindow}
	start := time.Now()
	if err := cmd.Run(); err != nil {
		msg := fmt.Sprintf("QEMU failed: %v\n\n%s", err, stderr.String())
		if time.Since(start) < 5*time.Second {
			return &quickExit{msg}
		}
		return errors.New(msg)
	}
	return nil
}

func startQEMU(qemu string, args []string, env []string) (*exec.Cmd, error) {
	cmd := exec.Command(qemu, args...)
	cmd.Env = env
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	cmd.SysProcAttr = &syscall.SysProcAttr{CreationFlags: createNoWindow}
	return cmd, cmd.Start()
}

func fatal(format string, a ...any) {
	msg := fmt.Sprintf(format, a...)
	fmt.Fprintln(os.Stderr, "TempleOS: "+msg)
	user32 := syscall.NewLazyDLL("user32.dll")
	box := user32.NewProc("MessageBoxW")
	text, _ := syscall.UTF16PtrFromString(msg)
	title, _ := syscall.UTF16PtrFromString("TempleOS")
	const mbIconError = 0x10
	box.Call(0, uintptr(unsafe.Pointer(text)), uintptr(unsafe.Pointer(title)),
		mbIconError)
	os.Exit(1)
}

//go:build !windows

package main

import (
	"fmt"
	"os"
	"os/exec"
	"syscall"
)

// runQEMU replaces the launcher with QEMU so the window belongs to the app
// (on macOS the Dock shows TempleOS, not a child process).
func runQEMU(qemu string, args []string, env []string) error {
	return syscall.Exec(qemu, append([]string{qemu}, args...), env)
}

func startQEMU(qemu string, args []string, env []string) (*exec.Cmd, error) {
	cmd := exec.Command(qemu, args...)
	cmd.Env = env
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd, cmd.Start()
}

func fatal(format string, a ...any) {
	fmt.Fprintf(os.Stderr, "TempleOS: "+format+"\n", a...)
	os.Exit(1)
}

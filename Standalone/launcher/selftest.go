package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"io"
	"net"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"time"
)

// selfTest boots the disk headless (as a throwaway snapshot), then grabs
// the screen over QMP and checks it shows the TempleOS desktop: 640x480
// and mostly TempleOS white and blue. The BIOS, the boot loader and a
// crash all look different. CI runs this on every platform build.
func selfTest(qemu string, args, env []string, wait time.Duration) int {
	if err := checkBackends(qemu, env); err != nil {
		return failTest("%v", err)
	}
	ln, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		return failTest("%v", err)
	}
	addr := ln.Addr().String()
	ln.Close()
	args = append(args, "-snapshot", "-display", "none",
		"-qmp", "tcp:"+addr+",server=on,wait=off")
	fmt.Println("self-test: qemu", args)
	cmd, err := startQEMU(qemu, args, env)
	if err != nil {
		return failTest("starting QEMU: %v", err)
	}
	exited := make(chan error, 1)
	go func() { exited <- cmd.Wait() }()

	select {
	case err := <-exited:
		return failTest("QEMU exited early: %v", err)
	case <-time.After(wait):
	}
	defer cmd.Process.Kill()

	dir, err := os.MkdirTemp("", "templeos-selftest")
	if err != nil {
		return failTest("%v", err)
	}
	defer os.RemoveAll(dir)
	shot := filepath.Join(dir, "screen.ppm")
	if err := qmpScreendump(addr, shot); err != nil {
		return failTest("screendump: %v", err)
	}
	w, h, frac, err := desktopFraction(shot)
	if err != nil {
		return failTest("reading screendump: %v", err)
	}
	fmt.Printf("self-test: screen %dx%d, %.0f%% TempleOS white/blue\n",
		w, h, frac*100)
	if w != 640 || h != 480 || frac < 0.5 {
		return failTest("TempleOS did not reach its desktop")
	}
	fmt.Println("self-test: PASS")
	return 0
}

// checkBackends makes sure this QEMU build has the window and sound
// backends the app uses (they can be separate module files on Windows),
// since the headless boot below doesn't exercise them.
func checkBackends(qemu string, env []string) error {
	ui := uiArgs(options{})
	for i := 0; i+1 < len(ui); i++ {
		var opt, want string
		switch ui[i] {
		case "-display":
			opt, want = "-display", ui[i+1]
		case "-audiodev":
			opt, want = "-audiodev", strings.Split(ui[i+1], ",")[0]
		default:
			continue
		}
		cmd := exec.Command(qemu, opt, "help")
		cmd.Env = env
		out, err := cmd.Output()
		if err != nil {
			return fmt.Errorf("%s help: %v", opt, err)
		}
		if !hasWord(string(out), want) {
			return fmt.Errorf("this QEMU has no %s %q backend:\n%s",
				opt, want, out)
		}
		fmt.Printf("self-test: %s %s available\n", opt, want)
	}
	return nil
}

func hasWord(s, w string) bool {
	for _, f := range strings.Fields(s) {
		if f == w {
			return true
		}
	}
	return false
}

func failTest(format string, a ...any) int {
	fmt.Printf("self-test: FAIL: "+format+"\n", a...)
	return 1
}

func qmpScreendump(addr, path string) error {
	conn, err := net.DialTimeout("tcp", addr, 10*time.Second)
	if err != nil {
		return err
	}
	defer conn.Close()
	conn.SetDeadline(time.Now().Add(30 * time.Second))
	r := bufio.NewReader(conn)
	if _, err := r.ReadBytes('\n'); err != nil { // greeting
		return err
	}
	call := func(cmd string, args any) error {
		req := map[string]any{"execute": cmd}
		if args != nil {
			req["arguments"] = args
		}
		b, _ := json.Marshal(req)
		if _, err := conn.Write(append(b, '\n')); err != nil {
			return err
		}
		for { // skip async events until the reply
			line, err := r.ReadBytes('\n')
			if err != nil {
				return err
			}
			var resp map[string]any
			if json.Unmarshal(line, &resp) != nil {
				continue
			}
			if e, ok := resp["error"]; ok {
				return fmt.Errorf("%s: %v", cmd, e)
			}
			if _, ok := resp["return"]; ok {
				return nil
			}
		}
	}
	if err := call("qmp_capabilities", nil); err != nil {
		return err
	}
	if err := call("screendump", map[string]string{"filename": path}); err != nil {
		return err
	}
	call("quit", nil)
	return nil
}

// desktopFraction reads a binary PPM and returns its size and the share of
// pixels that are TempleOS WHITE or BLUE.
func desktopFraction(path string) (w, h int, frac float64, err error) {
	f, err := os.Open(path)
	if err != nil {
		return
	}
	defer f.Close()
	r := bufio.NewReader(f)
	var magic string
	var maxval int
	if _, err = fmt.Fscan(r, &magic, &w, &h, &maxval); err != nil {
		return
	}
	if magic != "P6" || maxval != 255 {
		err = fmt.Errorf("unexpected PPM header %s %d", magic, maxval)
		return
	}
	r.ReadByte() // single whitespace before pixel data
	px := make([]byte, 3)
	match := 0
	for i := 0; i < w*h; i++ {
		if _, err = io.ReadFull(r, px); err != nil {
			return
		}
		white := px[0] == 255 && px[1] == 255 && px[2] == 255
		blue := px[0] == 0 && px[1] == 0 && px[2] >= 160 && px[2] <= 176
		if white || blue {
			match++
		}
	}
	frac = float64(match) / float64(w*h)
	return
}

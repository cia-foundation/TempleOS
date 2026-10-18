// Command TempleOS runs TempleOS as an ordinary desktop app on Windows,
// macOS and Linux.
//
// It ships a private copy of QEMU and a hard disk image with TempleOS
// already installed (see ../mkdisk.py). On first launch the disk is copied
// to the user's data dir, so changes made inside TempleOS persist across
// launches. Nothing needs to be installed.
package main

import (
	"archive/zip"
	"bytes"
	"crypto/sha256"
	"encoding/hex"
	"errors"
	"flag"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strconv"
	"strings"
	"time"
)

const diskName = "TempleOS.qcow2"

type options struct {
	reset      bool
	memoryMB   int
	cpus       int
	accel      string
	fullscreen bool
	dataDir    string
	selfTest   time.Duration
	qemuArgs   []string
}

func main() {
	var o options
	flag.BoolVar(&o.reset, "reset", false,
		"throw away changes and start over from a fresh TempleOS install")
	flag.IntVar(&o.memoryMB, "memory", 1024, "RAM in MiB (TempleOS needs 512+)")
	flag.IntVar(&o.cpus, "cpus", 2, "number of CPU cores")
	flag.StringVar(&o.accel, "accel", "",
		"QEMU accelerator: tcg, kvm, hvf or whpx (default: best available)")
	flag.BoolVar(&o.fullscreen, "fullscreen", false, "start full screen")
	flag.StringVar(&o.dataDir, "data", "",
		"where the TempleOS disk is kept (default: per-user app data dir)")
	flag.DurationVar(&o.selfTest, "self-test", 0,
		"boot headless for this long, then check TempleOS reached its desktop")
	flag.Usage = func() {
		fmt.Fprintf(flag.CommandLine.Output(),
			"Usage: %s [flags] [-- extra QEMU args]\n", filepath.Base(os.Args[0]))
		flag.PrintDefaults()
	}
	flag.Parse()
	o.qemuArgs = flag.Args()
	if o.dataDir == "" {
		o.dataDir = defaultDataDir()
	}

	res, err := resourcesDir(o.dataDir)
	if err != nil {
		fatal("%v", err)
	}
	qemu, share, err := findQEMU(res)
	if err != nil {
		fatal("%v", err)
	}
	disk, err := prepareDisk(res, o.dataDir, o.reset)
	if err != nil {
		fatal("%v", err)
	}
	env := qemuEnv(qemu)

	if o.selfTest > 0 {
		os.Exit(selfTest(qemu, baseArgs(o, share, disk), env, o.selfTest))
	}
	args := append(baseArgs(o, share, disk), uiArgs(o)...)
	args = append(args, o.qemuArgs...)
	if err := runQEMU(qemu, args, env); err != nil {
		fatal("%v", err)
	}
}

func defaultDataDir() string {
	switch runtime.GOOS {
	case "windows":
		if d := os.Getenv("LOCALAPPDATA"); d != "" {
			return filepath.Join(d, "TempleOS")
		}
	case "darwin":
		if home, err := os.UserHomeDir(); err == nil {
			return filepath.Join(home, "Library", "Application Support",
				"TempleOS")
		}
	default:
		if d := os.Getenv("XDG_DATA_HOME"); d != "" {
			return filepath.Join(d, "templeos")
		}
		if home, err := os.UserHomeDir(); err == nil {
			return filepath.Join(home, ".local", "share", "templeos")
		}
	}
	return "TempleOS-data"
}

// resourcesDir returns the dir holding runtime/ (bundled QEMU) and the
// pristine disk image: the embedded payload extracted into dataDir, an .app
// bundle's Contents/Resources, or the dir the executable is in.
func resourcesDir(dataDir string) (string, error) {
	if len(payloadZip) > 0 {
		return extractPayload(dataDir)
	}
	exe, err := os.Executable()
	if err != nil {
		return "", err
	}
	exe, _ = filepath.EvalSymlinks(exe)
	dir := filepath.Dir(exe)
	candidates := []string{
		filepath.Join(dir, "..", "Resources"), // TempleOS.app/Contents/MacOS
		dir,
	}
	for _, c := range candidates {
		if exists(filepath.Join(c, diskName)) {
			return filepath.Clean(c), nil
		}
	}
	return "", fmt.Errorf("can't find %s next to %s", diskName, exe)
}

func extractPayload(dataDir string) (string, error) {
	sum := sha256.Sum256(payloadZip)
	name := "runtime-" + hex.EncodeToString(sum[:6])
	dest := filepath.Join(dataDir, name)
	done := filepath.Join(dest, ".complete")
	if exists(done) {
		return dest, nil
	}
	zr, err := zip.NewReader(bytes.NewReader(payloadZip), int64(len(payloadZip)))
	if err != nil {
		return "", fmt.Errorf("damaged payload: %v", err)
	}
	tmp := dest + ".partial"
	os.RemoveAll(tmp)
	for _, f := range zr.File {
		path := filepath.Join(tmp, filepath.FromSlash(f.Name))
		if !strings.HasPrefix(path, tmp+string(os.PathSeparator)) {
			return "", fmt.Errorf("bad path in payload: %s", f.Name)
		}
		if f.FileInfo().IsDir() {
			if err := os.MkdirAll(path, 0o755); err != nil {
				return "", err
			}
			continue
		}
		if err := extractFile(f, path); err != nil {
			return "", err
		}
	}
	os.RemoveAll(dest)
	if err := os.Rename(tmp, dest); err != nil {
		return "", err
	}
	if err := os.WriteFile(done, nil, 0o644); err != nil {
		return "", err
	}
	// Drop runtimes left behind by older versions of the app.
	old, _ := filepath.Glob(filepath.Join(dataDir, "runtime-*"))
	for _, d := range old {
		if filepath.Base(d) != name {
			os.RemoveAll(d)
		}
	}
	return dest, nil
}

func extractFile(f *zip.File, path string) error {
	if err := os.MkdirAll(filepath.Dir(path), 0o755); err != nil {
		return err
	}
	r, err := f.Open()
	if err != nil {
		return err
	}
	defer r.Close()
	w, err := os.OpenFile(path, os.O_CREATE|os.O_WRONLY|os.O_TRUNC,
		f.Mode()|0o600)
	if err != nil {
		return err
	}
	if _, err := io.Copy(w, r); err != nil {
		w.Close()
		return err
	}
	return w.Close()
}

// findQEMU returns the bundled QEMU and its firmware dir, falling back to
// a qemu-system-x86_64 on PATH (share is then "", meaning QEMU's default).
func findQEMU(res string) (qemu, share string, err error) {
	bin := filepath.Join(res, "runtime", "bin")
	names := []string{"qemu-system-x86_64"}
	if runtime.GOOS == "windows" {
		// The "w" build is a GUI-subsystem exe: no console window.
		names = []string{"qemu-system-x86_64w.exe", "qemu-system-x86_64.exe"}
	}
	for _, n := range names {
		if p := filepath.Join(bin, n); exists(p) {
			return p, filepath.Join(res, "runtime", "share", "qemu"), nil
		}
	}
	for _, n := range names {
		if p, err := exec.LookPath(n); err == nil {
			return p, "", nil
		}
	}
	return "", "", errors.New("QEMU is missing from this build of TempleOS")
}

func qemuEnv(qemu string) []string {
	env := os.Environ()
	if runtime.GOOS == "windows" {
		// Bundled DLLs live next to qemu-system-x86_64w.exe.
		env = append(env, "PATH="+filepath.Dir(qemu)+";"+os.Getenv("PATH"))
	}
	return env
}

func prepareDisk(res, dataDir string, reset bool) (string, error) {
	if err := os.MkdirAll(dataDir, 0o755); err != nil {
		return "", err
	}
	disk := filepath.Join(dataDir, diskName)
	if exists(disk) && !reset {
		return disk, nil
	}
	src, err := os.Open(filepath.Join(res, diskName))
	if err != nil {
		return "", err
	}
	defer src.Close()
	tmp := disk + ".new"
	dst, err := os.Create(tmp)
	if err != nil {
		return "", err
	}
	if _, err := io.Copy(dst, src); err != nil {
		dst.Close()
		return "", err
	}
	if err := dst.Close(); err != nil {
		return "", err
	}
	os.Remove(disk)
	return disk, os.Rename(tmp, disk)
}

// baseArgs is the machine TempleOS expects: a PC with an IDE disk, standard
// VGA and a PS/2 keyboard and mouse. TempleOS has no networking.
func baseArgs(o options, share, disk string) []string {
	args := []string{
		"-name", "TempleOS",
		"-machine", "pc",
		"-m", strconv.Itoa(o.memoryMB),
		"-smp", strconv.Itoa(o.cpus),
		"-rtc", "base=localtime",
		"-drive", "file=" + disk + ",format=qcow2,if=ide,index=0",
		"-boot", "order=c",
		"-vga", "std",
		"-nic", "none",
	}
	if share != "" {
		args = append(args, "-L", share)
	}
	for _, a := range accelerators(o.accel) {
		args = append(args, "-accel", a)
	}
	return args
}

// accelerators lists what to try, best first; QEMU uses the first that
// works. Plain emulation (tcg) is always last and always works.
func accelerators(force string) []string {
	if force != "" {
		return []string{force}
	}
	var accel []string
	if runtime.GOOS == "linux" {
		if f, err := os.OpenFile("/dev/kvm", os.O_RDWR, 0); err == nil {
			f.Close()
			accel = append(accel, "kvm")
		}
	}
	// HVF (Intel Macs) and WHPX (Windows) are opt-in through -accel: QEMU's
	// HVF instruction decoder aborts on code TempleOS runs at boot, and WHPX
	// needs an optional Windows feature and is unreliable with TempleOS.
	return append(accel, "tcg")
}

func uiArgs(o options) []string {
	var args []string
	switch runtime.GOOS {
	case "windows":
		args = append(args, "-display", "sdl",
			"-audiodev", "dsound,id=snd0")
	case "darwin":
		args = append(args, "-display", "cocoa",
			"-audiodev", "coreaudio,id=snd0")
	default:
		return fullscreen(o, args)
	}
	// TempleOS plays hymns on the PC speaker.
	args = append(args, "-machine", "pcspk-audiodev=snd0")
	return fullscreen(o, args)
}

func fullscreen(o options, args []string) []string {
	if o.fullscreen {
		args = append(args, "-full-screen")
	}
	return args
}

func hasAudio(args []string) bool {
	for _, a := range args {
		if a == "-audiodev" {
			return true
		}
	}
	return false
}

func withoutAudio(args []string) []string {
	var out []string
	for i := 0; i < len(args); i++ {
		if args[i] == "-audiodev" ||
			(args[i] == "-machine" && i+1 < len(args) &&
				strings.HasPrefix(args[i+1], "pcspk-audiodev=")) {
			i++
			continue
		}
		out = append(out, args[i])
	}
	return out
}

func exists(path string) bool {
	_, err := os.Stat(path)
	return err == nil
}

# Profiling tsundoku with FlameGraph

CPU profiling using `perf` + [FlameGraph](https://github.com/brendangregg/FlameGraph).

---

## Prerequisites

### NixOS system config (`configuration.nix`)

```nix
boot.kernel.sysctl."kernel.perf_event_paranoid" = -1;
boot.kernel.sysctl."kernel.kptr_restrict"       = 0;
```

### System packages (`packages.nix`)

```nix
pkgs.perf
pkgs.flamegraph
```

### CMake build flags (`CMakeLists.txt`)

```cmake
target_compile_options(${PROJECT_NAME} PRIVATE
    -fno-omit-frame-pointer
    -g
)
```

> Build in **Debug** mode to get full symbol resolution and frame pointers.
> Build in **Release** mode (`-DCMAKE_BUILD_TYPE=Release`) to disable validation layers for a cleaner profile.

---

## Generating a Flame Graph

```bash
# 1. Run the app in the background, wait for init to finish
nvidia-offload ./tsundoku &
sleep 5

# 2. Record CPU samples for 120 seconds
perf record -F 99 -p $(pgrep tsundoku) -g -- sleep 120

# 3. Dump the samples
perf script > out.perf

# 4. Generate the flame graph
stackcollapse-perf.pl out.perf | flamegraph.pl > flame.svg

# 5. Open in browser
firefox flame.svg
```

---

## Reading the Graph

- **Width** = proportion of total CPU time spent in that function
- **Height** = call stack depth
- **Click** a frame to zoom in
- **Ctrl-F** to search for a function by name (e.g. `tsundoku`)

---

## Notes

- `[libnvidia-glcore]` + `ioctl` dominating the graph is normal — it's the CPU blocking on GPU work
- `[unknown]` frames are stripped third-party libraries (NVIDIA driver, GLFW, etc.) with no debug symbols — expected
- `[vkps]_Update` is the NVIDIA driver's internal present/submission thread — not your code
- Validation layer overhead (`threadsafety::Counter::FindObject`) will disappear in Release builds
- The profile is most useful once real geometry, instance buffers, and transform updates are in the render loop

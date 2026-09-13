# PeachWallpaper project guidance

## Project purpose

PeachWallpaper is a weekend project exploring how to build a Wallpaper Engine-like runtime in C++ for Linux Wayland.

The goal is learning and deliberate engineering. This is not a project to maximize output by generating large amounts of code.

## Weekend-project rule

The whole point of this project is to avoid vibe coding.

AI may be used for:

- high-level architecture discussions;
- technology and protocol comparisons;
- explaining unfamiliar APIs or error messages;
- reviewing code the user has written;
- debugging and identifying likely causes;
- creating implementation plans, test plans, and documentation;
- tedious, repetitive, or low-value code only when the user explicitly asks for it.

AI must not generate or modify code by default. Do not silently scaffold modules, add abstractions, refactor files, or implement a proposed architecture. Wait for an explicit request such as “write this,” “implement this,” or “generate the boilerplate for this.”

When code is explicitly requested:

- keep the change as small and local as possible;
- explain what is being changed before making broad changes;
- do not add unrelated dependencies or features;
- prefer a focused snippet or one small file over a complete subsystem;
- preserve the user’s existing design and decisions;
- verify the result, but do not continue implementing the next feature without being asked.

If the user asks for advice, answer with reasoning, interfaces, data flow, tradeoffs, or pseudocode rather than production code.

## Current technical direction

The intended runtime is a Wayland client, not a Wayland compositor.

Initial target stack:

- C++20;
- CMake and Ninja;
- raw `wayland-client`;
- `wlr-layer-shell` for desktop-background placement;
- EGL and OpenGL ES for the first renderer;
- GStreamer for video playback later;
- PipeWire for audio-reactive input later;
- Qt 6/QML for a manager/settings application later;
- D-Bus or a Unix socket for control-plane IPC later.

The first compositor target is one that supports `wlr-layer-shell`, such as a wlroots-based compositor. GNOME requires a separate compositor/desktop-shell integration path and should not be assumed to work with the generic layer-shell client.

## Main modules

The project should be understood as several independent areas:

1. **Wayland platform adapter**
   - Connect to the display.
   - Discover outputs and output scale.
   - Create and manage layer-shell surfaces.
   - Handle configure, resize, frame callbacks, hotplug, and shutdown.

2. **Renderer**
   - Own the EGL/OpenGL or future Vulkan context.
   - Manage frame pacing, textures, buffers, shaders, and render passes.
   - Render one surface per monitor when multi-monitor support is added.

3. **Wallpaper/content runtime**
   - Define the project’s own wallpaper format first.
   - Load scenes, images, shaders, transforms, particles, and parameters.
   - Keep content data separate from the renderer and platform adapter.
   - Supporting original Wallpaper Engine assets is a separate compatibility project, not an automatic consequence of supporting live wallpapers.

4. **Video backend**
   - Decode and loop video wallpapers.
   - Synchronize decoded frames with rendering.
   - Start with a simple upload path; investigate DMA-BUF zero-copy only after profiling.

5. **Audio analysis**
   - Capture either wallpaper audio or an explicitly selected PipeWire stream.
   - Produce time-domain, level, and spectrum data for the content runtime.
   - Keep audio capture optional and clearly permissioned.

6. **Input**
   - Handle pointer events delivered to the wallpaper surface when applicable.
   - Do not assume a normal Wayland client can observe global keyboard or pointer input.
   - Preserve desktop click-through behavior unless interactive mode is explicitly enabled.

7. **Playback and lifecycle policy**
   - Pause or reduce work when appropriate.
   - Handle monitor changes, compositor restarts, lock/suspend behavior, and renderer failures.
   - Do not rely on a generic Wayland API for compositor-specific visibility or lock-state information.

8. **Control plane**
   - Start, stop, pause, resume, select, and configure wallpapers.
   - Expose renderer state and errors.
   - Keep this independent from the rendering loop.

9. **Manager UI and library**
   - Browse wallpaper assets.
   - Edit wallpaper properties.
   - Store per-wallpaper and per-output settings.
   - This can be added after the renderer works from a command line.

10. **Compositor integrations**
    - Generic layer-shell backend.
    - KDE/Plasma-specific integration if needed.
    - GNOME Shell extension or Mutter-specific integration if needed.
    - Each integration should be isolated from the core renderer.

11. **Packaging and safety**
    - Provide a user service and desktop entry.
    - Consider Flatpak or distro packages later.
    - Treat downloaded wallpaper content as untrusted; do not execute arbitrary native code from an asset.

## Current scaffold

The repository currently contains a minimal layer-shell/OpenGL starter:

- `CMakeLists.txt` discovers Wayland, EGL, OpenGL ES, and Wayland protocol tools.
- `protocols/wlr-layer-shell-unstable-v1.xml` supplies the layer-shell definition.
- `src/main.cpp` creates a background surface on the first output and clears it with OpenGL ES.
- `.clangd` and `.vscode/settings.json` configure clangd and CMake Tools.

This scaffold is only a connectivity/rendering experiment. It is not the architecture of the finished application.

## Next step

The Wayland/EGL/GLES pipeline now runs end to end: it connects, loads a
config-driven shader wallpaper, and renders it live. The next step is a
small tray application to control the running program (start/stop/switch
wallpaper), rather than expanding the renderer further. This is the
beginning of the control plane / manager UI modules described above, scoped
down to the smallest useful piece first.

## Naming conventions

Member variable prefixes:

- Private member variables: `m_<name>` (e.g. `m_display`).
- Public member variables: `M_<name>`.
- Static variables: `s_<name>`.

All variables (locals, parameters, and member names after their prefix) use
`snake_case`: `native_display`, `log_len`, `m_shader_id`. This applies to
variables only - type names and function/method names keep their existing
PascalCase convention (e.g. `WaylandBackend`, `Initialize`).

## Working style

Before implementation, establish:

1. the exact user-visible behavior;
2. the module being changed;
3. the smallest useful experiment;
4. how success will be verified.

Prefer small experiments that answer one technical question. Document decisions and rejected alternatives briefly so the project remains understandable after the weekend.

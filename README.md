# Custom Skia Seekbar (Android, C++/CMake)

This is an Android C++ demo application that renders UI using **Skia** and implements a **custom seek bar inspired by YouTube**:

- **Chapter segments** (with visible gaps between segments)
- **Drag-and-drop seeking**
- **Double-tap seeking** on left/right half of the screen (**−10s / +10s**)
- **Buffering animation** (YouTube-like spinner with **8 dots**) shown for **2 seconds** after each seek action

The project uses **CMake** to generate build files and renders the final Skia output via an OpenGL ES fullscreen textured quad:

> Skia raster surface → upload to GL texture → draw fullscreen quad

---

## Requirements Checklist (Task Compliance)

### ✅ Skia + custom seek bar
- Seek bar and all UI are drawn using Skia (`SkCanvas`).

### ✅ Segments (chapters)
- The seek bar supports multiple chapters.
- Chapters are rendered as separate segments with a configurable **gap** between them.
- While dragging, the **chapter under the thumb** is rendered thicker (YouTube-like hover/active emphasis).

### ✅ Loading animation (indefinite)
- A YouTube-like buffering spinner is rendered (8 dots, intensity rotates around the circle).
- After a seek action, buffering is simulated for **2 seconds** (spinner visible) and then stops automatically.

### ✅ Seeking interactions
- **Drag-n-drop** on the seek bar:
  - Drag preview updates the played position visually.
  - On release, the player position commits.
- **Double-tap**:
  - Two taps within a time window and within a distance threshold trigger seeking.
  - Left half of the screen: **−10 seconds**
  - Right half of the screen: **+10 seconds**
  - Each double-tap triggers buffering for 2 seconds.

### ✅ Build system
- Uses **CMake**.

### ✅ Platform / windowing
- Android app based on **GameActivity / NativeAppGlue**, rendering via **OpenGL ES (EGL)** and Skia raster surface.

---

## Repository / Dependencies (Git submodules)

This repository includes a **Git submodule** that contains third-party dependencies, including the **Skia library** (and related build files/scripts).

### Clone with submodules

To clone the repository **and fetch all submodules** in one step:

```bash
git clone --recurse-submodules https://github.com/thesayat/MobileSeekbar.git
```

If you already cloned the repo without submodules, initialize and update them with:

```bash
git submodule update --init --recursive
```
## High-Level Architecture

The project is split into small, single-responsibility modules:

### Rendering pipeline
- **`SkiaRasterTexture`** (`Render/SkiaRasterTexture.*`)
  - Owns a raster `SkSurface` sized to the screen.
  - Uploads pixels to an OpenGL texture using `glTexSubImage2D`.
  - Provides `SkCanvas*` to draw UI each frame.
- **`FullscreenTexturedQuad`** (`Render/FullscreenTexturedQuad.*`)
  - Builds a fullscreen quad in the app’s orthographic coordinate space.
  - Draws the quad using the app’s shader.

### UI
- **`SeekbarStyle`** (`UI/SeekbarStyle.h`)
  - All tweakable UI parameters (bar height, margins, gaps, thumb size/color, hitbox slop).
- **`SeekbarLayout`** (`UI/SeekbarLayout.*`)
  - Computes `barRect` and `hitboxRect` based on screen size and style.
- **`SeekbarView`** (`UI/SeekbarView.*`)
  - Pure rendering: draws segments, played portion, thumb, and buffering spinner.

### Input / interaction
- **`SeekbarController`** (`Input/SeekbarController.*`)
  - Pure interaction logic:
    - Drag start/move/end (including pointer id tracking)
    - Double-tap detection (time window + slop distance)
    - Starts 2-second buffering on seek actions
  - Writes back to `AppPlaybackModel`.

### State model
- **`AppPlaybackModel`** (`Model/AppPlaybackModel.h`)
  - Holds player state (duration, position, buffering flag), chapters, and seek (drag state).

### Animation
- **`LoadingAnimator`** (`Common/LoadingAnimator.*`)
  - Updates a normalized phase (0..1) during buffering for spinner rotation.
- **`ProjectionConfig`** (`Render/ProjectionConfig.h`)
  - Single source of truth for orthographic projection constants.

---

## How It Works (Frame Flow)

Per frame (`Renderer::render()`):

1. Update viewport/projection if the surface size changed.
2. Update buffering timer (via `SeekbarController.update()`).
3. Update loading animation phase (`LoadingAnimator.tick(isBuffering)`).
4. Ensure Skia raster surface and GL texture size (`SkiaRasterTexture.ensure(w,h)`).
5. Draw UI on Skia canvas (`SeekbarView.draw(...)`).
6. Upload pixels to GL texture (`SkiaRasterTexture.upload()`).
7. Draw fullscreen quad with that texture (`FullscreenTexturedQuad.draw(...)`).
8. Swap buffers (`eglSwapBuffers`).

---

## Controls / Interaction

### Drag seek
- Touch down on the seek bar hitbox → drag horizontally → release to commit.
- While dragging, the active chapter (under the thumb) is rendered thicker.

### Double-tap seek
- Double-tap anywhere on the screen:
  - Left half: seek **−10s**
  - Right half: seek **+10s**
- Each double-tap triggers buffering spinner for 2 seconds.

### Buffering simulation
- After any seek commit:
  - `player.isBuffering = true` for 2 seconds
  - then automatically set to `false`

---

## Configuration / Tuning

Most visual parameters are in **`SeekbarStyle`**:

- Bar height, margins, gaps
- Thumb radius multiplier and color
- Hitbox padding
- Chapter hover thickness

Spinner size and placement are in `SeekbarView` (easy to tweak).

---

## Notes / Known Limitations

- This demo renders Skia into a CPU raster surface and uploads to a GL texture each frame.
  - This is simple and portable, but not as efficient as a fully GPU-backed Skia surface.
- The buffering indicator is simulated (2 seconds after seek); there is no real media pipeline here.
- DP scaling is not implemented yet; values are currently treated as pixels for simplicity.

---

## Build

Open the project in the latest Android Studio and build normally.  
The native code is built via **CMake** (Android NDK).

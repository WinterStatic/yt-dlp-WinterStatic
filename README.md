# WinterStatic yt-dlp Downloader

**Version 0.1.37**

![WinterStatic yt-dlp Downloader](yt-dlp_winterstatic.png)


### 0.1.37 - STABLE-BACKEND POLICY + HOVER HELP

- The primary `yt-dlp.exe` now deliberately tracks the **latest official stable** release whenever the portable package is built or the GUI **Update** action is used. Nightly builds are not selected for the primary slot.
- The authenticated YouTube helper is now **stable-only rather than permanently pinned**. It follows the latest stable release unless that exact release is on WinterStatic's authenticated-quality blacklist. The initial blacklist contains **2026.08.19**; while that remains the latest stable, authenticated YouTube falls back to the known-good **2026.07.04** build. Later stable releases are allowed automatically unless testing proves they also need blacklisting.
- The compact Tools status is simplified to `primary+authenticated`, for example `2026.08.19+2026.07.04`. If the authenticated helper is missing or unreadable, the second side displays `--`.
- The status probe also normalizes its old `Detected`/`Missing` sentinel strings before composing the dual-version label, so an unreadable primary correctly shows **Detected** and an unreadable auth helper correctly shows `+--` instead of treating those words as version numbers.
- Hovering the dual yt-dlp version status now explains which side is primary/authenticated and summarizes the stable-only/blacklist policy.
- Hovering **Browser sweep** now explains that it is an optional, off-by-default, last-resort visible Chromium network observer used only after normal yt-dlp/recovery/static discovery is exhausted.
- The build kit now supports `build-native.bat release`, which performs the normal validated build and then creates canonical GitHub-ready `0.1.37-portable.zip` and `0.1.37-source.zip` assets. The source asset is allow-listed so local `Tools` binaries and browser/profile data cannot be swept into it accidentally. The GitHub README screenshot is included in that allow-list as `yt-dlp_winterstatic.png`.
- Download selection, authentication triggering, progress handling, CDP candidate discovery, and the 0.1.36 restart-aware progress clamp are otherwise unchanged.


### 0.1.36 - MID-ATTEMPT RESTART-AWARE PROGRESS

- Refines the 0.1.35 monotonic GUI progress clamp so a real yt-dlp restart *inside the same attempt* cannot leave the progress bar pinned at an old high percentage while the transfer recounts from near zero.
- Tracks the last `downloaded_bytes` value alongside the displayed-percent high-water mark. Normal DASH/HLS estimate wobble leaves downloaded bytes increasing, so the anti-jitter clamp still holds; a substantial byte-count drop releases the clamp and lets the GUI reflect the genuine restart.
- Uses a 64 KiB restart tolerance to avoid treating small fragment/concurrency counter wobble as a restart. The byte tracker resets at the same existing boundaries as the percentage tracker: a new attempt banner and a new download stream/pass.
- This remains display-layer only: yt-dlp output, download/retry behavior, authentication, format selection, split backends, Browser sweep/CDP, updater logic, and PowerShell generation are unchanged from 0.1.35.


### 0.1.35 - MONOTONIC PROGRESS DISPLAY + SMALL CLEANUP

- Smooths the GUI percentage for fragmented DASH/HLS downloads by keeping a per-pass high-water mark. If yt-dlp revises its estimated total upward and reports a lower percentage, WinterStatic holds the last displayed value instead of making the progress bar/detail move backwards.
- The high-water mark resets for every new download stream/pass, so a separate audio pass can correctly begin near 0% after a video pass reaches 100%.
- This is display-only: yt-dlp's raw PowerShell output, size estimates, download behavior, recovery logic, and any future ETA work remain untouched. During an estimate correction the GUI may briefly pause at the previous percentage rather than oscillating backward.
- Removes the redundant fixed-128-KiB `buffer.size()` sanity condition from `ProcessCommandLine()`. The buffer is always 128 KiB there, so the check could never fail; this is cleanup only and does not change dedicated Edge profile detection.
- No changes to authentication, split yt-dlp backend routing, Browser sweep/CDP discovery, format selection, or updater behavior from 0.1.34.


### 0.1.34 - DEDICATED EDGE PROFILE SAFETY FIX

- Fixes a porting regression in `AccountBrowserProfileInUse()`: the Edge account-profile gate no longer looks for a Firefox/LibreWolf-style `lockfile` that Chromium does not create on Windows.
- WinterStatic now checks running Edge process command lines for the exact dedicated `--user-data-dir=<Tools\BrowserProfile>` argument. This means **only the WinterStatic account browser** blocks cookie/session checks, Reset Login, or an authenticated download; the user's normal Edge windows can remain open.
- Keeps the existing 4-second shutdown grace period so a just-closed dedicated Edge instance can finish exiting before WinterStatic reads or renames its profile.
- No download, yt-dlp routing, browser-sweep/CDP, format-selection, or updater behavior changed from 0.1.33.
- The 0.1.33 `Missing`/`Detected` status fix and `+A--` diagnostic remain unchanged.

### 0.1.33 - MAINSTREAM RELEASE

- Promotes the tested 0.2.x development line into the mainstream 0.1.x release series.
- Keeps the split yt-dlp backend policy: current stable for normal/public downloads and pinned **2026.07.04** for authenticated YouTube compatibility.
- Keeps **Automatic** authentication as anonymous-first, only adding the dedicated Edge session when YouTube explicitly requires authentication.
- Keeps the **Browser sweep** recovery option off by default and persisted in `settings.ini`; static/non-browser recovery remains available when it is disabled.
- Keeps the compact dual-backend Tools status such as `2026.08.19+A2026.07.04` and the tightened Tools layout.
- Restores the long-standing yt-dlp status distinction: an executable that exists but whose version cannot be read is shown as **Detected**, while **Missing** is reserved for an absent executable.
- `+A--` remains intentional when the primary yt-dlp version is known but the authenticated helper is unavailable or unreadable.

The 0.2.x entries below are retained as development-history notes for the work that led to this mainstream build.

### 0.2.69experimental - COMPACT DUAL VERSION LABEL + TOOLS ROW TIGHTENING

- Keeps the 0.2.68 browser-sweep opt-in, split yt-dlp routing, updater behavior, and recovery logic unchanged.
- The Tools yt-dlp status now uses `2026.08.19+A2026.07.04`: the `A` explicitly marks the authenticated backend while removing spaces and punctuation that were consuming the last few pixels.
- The Tools app-name column is narrowed by 12 px. The path/button/status block keeps its previous widths and shifts left by the saved amount, creating a little extra breathing room at the right edge.
- This is a UI-only refinement; no download, authentication, browser-sweep, or yt-dlp selection behavior changed.


### 0.2.68experimental - FULL DUAL YT-DLP VERSION DISPLAY

- Keeps the 0.2.67 browser-sweep opt-in and split yt-dlp backend behavior unchanged.
- The compact Tools status now shows both complete versions, for example `2026.08.19 / A:2026.07.04`, instead of dropping the common year.
- No downloader, authentication, recovery, update, or browser-observer logic changed in this build.


### 0.2.68experimental - BROWSER SWEEP OPT-IN + COMPACT DUAL VERSION STATUS

- Adds a compact **Browser sweep** checkbox in the Run panel. It is **off by default** and persisted as `BrowserAssistedRecovery=0/1` in `settings.ini`.
- With Browser sweep off, WinterStatic still performs the normal yt-dlp attempts, resilience retries, and static/captured-page discovery. It simply skips the final visible temporary-browser CDP/network sweep.
- When the browser sweep would otherwise have run, the live PowerShell window says that browser-assisted recovery is disabled instead of opening a browser unexpectedly.
- The cramped yt-dlp status field now reports both split backends compactly. When both are same-year date releases it displays, for example, `08.19 / A:07.04` (primary / authenticated). The live PowerShell window continues to print the complete executable paths and full versions.
- The 0.2.66 stable-primary / pinned-2026.07.04 authenticated backend policy is otherwise unchanged.

### 0.2.66experimental - PRIMARY STABLE RESTORE / SPLIT BACKENDS

0.2.65 proved that the public/anonymous and authenticated YouTube paths currently need different yt-dlp behavior. This follow-up removes the now-unnecessary global blacklist on stable **2026.08.19** for the public side. The 2026.08.19 problem we reproduced was the authenticated/age-gated **360p-only** regression; the same release also contains the post-`android_vr` fix needed by ordinary public videos.

The split is therefore now deliberately **stable + stable**, with no nightly required:

- `Tools\yt-dlp\yt-dlp.exe` - current official stable **2026.08.19 or newer**, used for normal anonymous/public downloads and the ordinary recovery pipeline.
- `Tools\yt-dlp\yt-dlp-auth.exe` - pinned **2026.07.04**, used only for authenticated YouTube.

`build-native.bat` and the GUI **Update** action now use yt-dlp's normal stable release channel for the primary slot and accept 2026.08.19. A copied nightly in the primary slot is replaced with the current stable build so the portable package is predictable. The authenticated helper remains pinned to 2026.07.04.

This keeps the ordinary-video `android_vr` fix while isolating the 2026.08.19/newer authenticated-quality problem to a path where those builds are not used.

### 0.2.65experimental - SPLIT YOUTUBE BACKENDS / AUTH NIGHTLY BLACKLIST

Testing exposed a genuine backend conflict rather than one universally good yt-dlp release. The current nightly fixes the ordinary/public YouTube mid-download 403 caused by the retired `android_vr` client, but the same nightly reproduces the authenticated/age-gated **360p-only** regression. Conversely, **2026.07.04** still gives the desired authenticated quality on the known age-gated test but is too old to be trusted as the general public-video backend.

0.2.65 therefore deliberately keeps **two yt-dlp executables**:

- `Tools\yt-dlp\yt-dlp.exe` - current acceptable official nightly; used for normal anonymous/public downloads and the ordinary recovery pipeline.
- `Tools\yt-dlp\yt-dlp-auth.exe` - pinned **2026.07.04**; used only when a YouTube task actually enters WinterStatic's authenticated Edge-cookie path.

In **Automatic** mode, the public/anonymous first attempt still uses the current nightly. If YouTube explicitly requires authentication, WinterStatic switches the entire authenticated YouTube attempt (including its preflight and recovery calls) to the pinned 2026.07.04 backend, then restores the primary backend afterward. **Use account browser** does the same immediately for YouTube. Authenticated non-YouTube sites continue using the normal primary backend.

The GUI Update action now refreshes the primary nightly **and re-pins the authenticated helper to exactly 2026.07.04**, preventing a nightly update from silently replacing the known-good authenticated backend. Both selected backend versions are printed in the visible PowerShell task window.

This is intentionally an experimental compatibility split, not a claim that 2026.07.04 is globally preferable. The split should be removed once a newer yt-dlp build is verified to handle both ordinary YouTube and the known authenticated/age-gated quality case correctly.

### 0.2.64experimental - YOUTUBE BACKEND REFRESH / ANDROID_VR RETIREMENT

This experiment keeps the **0.2.63 Automatic-authentication-on-demand** behavior intact and changes the bundled yt-dlp policy instead. YouTube retired the `android_vr` media path in August 2026; yt-dlp 2026.07.04 can still select that client and may begin downloading normally before the media host starts returning HTTP 403. WinterStatic therefore no longer falls back to 2026.07.04.

A copied yt-dlp is retained only when it is new enough to contain the post-`android_vr` client maintenance and is not the specifically problematic stable 2026.08.19 build. Otherwise `build-native.bat` downloads the current official **yt-dlp nightly** Windows executable. The GUI **Update** button likewise installs the latest official nightly build. This keeps the newer YouTube client set (including `visionos`) without returning to the old 2026.07.04 default-client behavior.

The existing 2026.08.19 authenticated/age-restricted 360p concern is still treated cautiously: that exact stable build is not selected by WinterStatic's bootstrap. The current authenticated mweb quality probe remains available, and no authentication or runtime-browser discovery logic was otherwise changed in this version.

Primary regression target for this build: `https://www.youtube.com/watch?v=D9290H1-ZRM`, which reproducibly reached roughly 2% with yt-dlp 2026.07.04 before failing with HTTP 403.

### 0.2.63experimental - AUTOMATIC AUTHENTICATION ON DEMAND

This experiment changes only the **Automatic** authentication policy. Automatic mode now tries the normal anonymous yt-dlp path first, even when a saved dedicated Edge login is available. If that first YouTube attempt fails and yt-dlp emits strong authentication-required wording (for example an explicit sign-in/cookies/private/members-only requirement), WinterStatic retries through the existing authenticated Edge path. Ordinary network, format, 403/429, 404, unsupported-URL, and transport failures do **not** by themselves cause account cookies to be added.

**Use account browser** is unchanged and still forces the saved Edge profile from the first attempt. **Anonymous** is unchanged and never supplies account cookies. The 0.2.62 post-success runtime/CDP hardening is retained unchanged. The copied one-line PowerShell command for Automatic represents the anonymous first attempt; the managed download task contains the conditional authentication fallback.

Automatic mode also no longer refuses to start a public anonymous-first download merely because the dedicated account-browser window is open; in that state the saved session is simply unavailable for fallback until the browser is closed.

The purpose is to test whether keeping account cookies away from ordinary public YouTube downloads restores the reliability seen in older builds while preserving automatic access to genuinely gated videos.

### 0.2.62experimental - POST-SUCCESS BUG SWEEP

After 0.2.61 successfully proved Edge runtime discovery works with scalar CDP target handling, this pass tightens nearby failure paths without changing the successful normal path. Runtime target selection now keeps a current HTTP(S) fallback for cross-host redirects, only falls back after a short grace period (or when there is only one usable page target), verifies that `Network.enable` was actually sent, and redacts any URL-like text from attachment exceptions. Recovery signals are reset at the start of each yt-dlp attempt so a 404/progress/format signal from an earlier retry cannot incorrectly steer a later retry. Hidden helper captures now enforce their timeout instead of falling through into a blocking pipe read if a helper process wedges.

The build documentation also now reports the current portable output-folder version correctly.

### 0.2.61experimental - CDP TARGET NORMALIZATION / EDGE RETEST

The Chrome diagnostic exposed a concrete PowerShell-side type bug before the WebSocket handshake: the selected `webSocketDebuggerUrl` reached `ClientWebSocket.ConnectAsync()` as `System.Object[]` instead of one URI. That means the failure happened in our target/URI handling before Chrome or Edge had a chance to accept or reject the WebSocket connection.

This build explicitly flattens the CDP target list, selects one scalar page target, converts one scalar WebSocket URL into a `System.Uri`, and then attaches. Runtime discovery is Edge-first again, with the 60-second retry window and safe first-exception diagnostic retained. No media URLs are printed.

### 0.2.60experimental - CHROME CDP COMPARISON TEST

This experiment keeps the 60-second runtime-observer startup/attachment window but deliberately uses **Google Chrome only** for difficult-download runtime network discovery. It does not fall back to Edge or Brave during this test, so the result cleanly compares another Chromium browser with Edge.

The normal account/login browser remains Edge; only the isolated temporary runtime detector changes to Chrome.

The first safe WebSocket attachment exception is now printed and logged. Media URLs and detected candidate URLs remain hidden. If Chrome attaches successfully while Edge repeatedly returned `ATTACH_FAILED`, that points to an Edge-specific CDP/WebSocket/policy difference rather than a general timing problem.

### 0.2.59experimental - ONE-MINUTE EDGE OBSERVER STARTUP WINDOW

- Built directly on **0.2.58experimental** after Edge still failed while its detector page was visibly loading.
- Extends only the Edge runtime-observer startup/attachment window from **20 seconds to 60 seconds**.
- During that minute WinterStatic keeps re-checking the local debugging endpoint/page target and retrying the read-only WebSocket + `Network.enable` attachment.
- The normal **30-second media-capture window still begins only after attachment succeeds**, so the extra minute is not taken away from actual network listening time.
- If Edge still cannot attach after the full minute, the attempt count remains in the failure message; that would strongly suggest the problem is attachment compatibility rather than simple browser startup speed.

### 0.2.58experimental - EDGE OBSERVER STARTUP/ATTACH RETRY

- Keeps Edge first for difficult runtime-network discovery.
- Fixes a startup race seen in 0.2.57experimental: the detector previously stopped waiting as soon as an Edge page target appeared, then made only one WebSocket/`Network.enable` attach attempt.
- The detector now waits up to 20 seconds for the requested page target and retries the read-only CDP attachment instead of failing on the first early target.
- The normal 30-second media-capture window starts only after the observer has attached successfully.

### 0.2.57experimental - EDGE-FIRST RUNTIME NETWORK DISCOVERY

- Built on **0.2.56experimental**; the yt-dlp 2026.08.19 blacklist, portable Tools bootstrap, and conditional authenticated-YouTube mweb fallback are unchanged.
- Difficult-download runtime network discovery now tries **Microsoft Edge first**, then Chrome, then Brave.
- The detector still uses its own temporary isolated browser profile and a random local debugging port; it does **not** reuse or lock the persistent `Tools\BrowserProfile` account profile.
- This deliberately gives Edge's read-only CDP/network-observer path another clean test without changing static discovery, candidate scoring, preview rejection, 404 recovery, or the normal authenticated Edge cookie path.
- If Edge is installed but its observer endpoint/attachment fails, this experiment reports that failure; it does not silently switch browsers mid-attempt. That makes the Edge test unambiguous.

### 0.2.56experimental - YT-DLP STABLE BLACKLIST + PORTABLE UPDATE SAFETY

> Historical note: the 2026.08.19 blacklist itself remains relevant, but the old fallback to 2026.07.04 was superseded by 0.2.64 after YouTube retired the `android_vr` path used by that older backend.

- Built on **0.2.55experimental**, retaining the conditional authenticated-YouTube mweb quality fallback.
- The exact yt-dlp release **2026.08.19 is blacklisted** because it is the release under test that collapses authenticated/age-restricted YouTube to 360p in our known reproduction.
- `build-native.bat` selects the newest official stable GitHub release **except 2026.08.19** when it needs to populate `Tools\yt-dlp\yt-dlp.exe`.
- If GitHub's release API is unavailable, the BAT validates the normal latest-stable download; if that resolves to 2026.08.19 it falls back to the known-good **2026.07.04** executable.
- A copied portable yt-dlp is still retained when valid, but an existing 2026.08.19 portable executable is rejected and replaced.
- The GUI **Update** button no longer delegates to `yt-dlp -U` (which can preserve a nightly update channel). It now installs the newest official stable release except 2026.08.19 and uses the same validated fallback behavior.
- Portable Tools remain first in runtime lookup; external/configured/PATH copies are only relevant when the portable executable is absent.

### 0.2.55experimental - AUTHENTICATED YOUTUBE MWEB QUALITY FALLBACK

- Built on **0.2.54experimental**, keeping the portable runtime bootstrap and Tools-first/fallback-second lookup.
- Keeps the normal authenticated Edge command unchanged as the first choice: `--cookies-from-browser edge:<Tools\BrowserProfile\Default>`.
- Adds a lightweight **authenticated YouTube format preflight** for video modes. WinterStatic asks the normal yt-dlp client selection what height it would choose without downloading media.
- If the normal authenticated selection is already above 360p, nothing changes and the normal client is used.
- If the normal authenticated selection is only 360p or lower, WinterStatic probes one compatibility fallback using `--extractor-args youtube:player_client=mweb`.
- The mweb route is used only when that probe exposes a strictly better selected height. If the real mweb download then fails, WinterStatic returns to yt-dlp's normal authenticated client before the existing recovery chain. If the probe itself fails, needs something unavailable, or is not better, the normal client is kept from the start.
- The workaround is conditional rather than globally forcing mweb, so older yt-dlp builds such as 2026.07.04 should continue using their normal working client selection.
- Audio-only mode and non-YouTube downloads do not run the mweb quality fallback.
- PowerShell reports only the safe selected heights and whether the fallback was used; it does not print cookies or signed media URLs.

### 0.2.54experimental - PORTABLE RUNTIME BOOTSTRAP + TOOLS-FIRST LOOKUP

- Built directly from the working **0.2.50experimental** branch; the 0.2.51/0.2.52 browser detours and the 0.2.53 hard-pin wording are not carried forward.
- `build-native.bat` produces a self-contained runtime package: if the copied local Tools tree does not already contain them, it downloads an acceptable **official yt-dlp Windows executable** (nightly when the current stable is the excluded 2026.08.19 build) and the **Gyan FFmpeg release essentials ZIP**, placing them under `Tools\yt-dlp` and `Tools\FFmpeg\bin`.
- Existing local portable copies are retained only when they meet the current YouTube-backend floor and are not the excluded 2026.08.19 stable; older copies are replaced.
- Runtime lookup is **portable first, fallback second** for both yt-dlp and FFmpeg.
- The live PowerShell window prints the exact yt-dlp executable, yt-dlp version, and FFmpeg executable selected for each task.
- Keeps the 0.2.50 Edge authentication path, quality selectors, Brave-first runtime observer, 404 recovery, preview/image filtering, and recovery single-output guard.

### 0.2.49experimental - FORCED TV_DOWNGRADED TEST (NOT RETAINED)

- Tested forcing `youtube:player_client=tv_downgraded` with the dedicated Edge cookies; this was not retained after the current target returned `The page needs to be reloaded`.
- Kept the dedicated WinterStatic Edge account profile while testing that forced client route.
- Normal anonymous YouTube and non-YouTube extractors were unchanged by the test.
- The existing GUI quality selector remained unchanged.
- Keeps the 0.2.48 Brave-first runtime detector and recovery single-output guard unchanged.

- **Dedicated Microsoft Edge account profile experiment:** LibreWolf is no longer used by this branch. The account browser now launches installed Microsoft Edge with a persistent WinterStatic-only user-data directory under `Tools\BrowserProfile`.
- The dedicated Edge profile is separate from the user's normal Edge profile, so signing into YouTube there does not sign the normal Edge profile in.
- yt-dlp authentication now reads cookies from the dedicated Edge `Default` profile. Close the WinterStatic Edge window before an authenticated download; normal Edge windows may remain open.
- Runtime network discovery uses a separate temporary detector profile and now prefers Brave, then Chrome, then Edge. The persistent Edge account profile remains reserved for cookie extraction and cannot be locked by the detector.
- Keeps the 0.2.45 pre-download HTTP 404 recovery and all static/captured-page/preview filtering experiments.

### 0.2.48experimental - RECOVERY OUTPUT + RUNTIME BROWSER CLEANUP

- Keeps the dedicated Microsoft Edge account profile introduced in 0.2.46/0.2.47.
- Runtime network discovery now prefers Brave, then Chrome, then Edge because Brave was the proven observer path during testing.
- Discovered-media retries add a recovery-only single-output guard (`--playlist-items 1`, `--no-video-multistreams`, `--no-audio-multistreams`) so recovered player/manifest resources do not fan out into every available representation.


- Replaces the portable LibreWolf account-browser path with installed Microsoft Edge plus an isolated persistent user-data directory.
- Removes the LibreWolf bootstrap/update dependency from the build kit.
- Adds **Open Profile** controls for inspecting the dedicated browser data directory.
- Keeps browser visibility and user control: no Selenium, WebDriver, scripted clicks, or hidden browser automation.
- Runtime detection uses a temporary Chromium profile; Brave is preferred because its CDP observer path was proven in testing, while the persistent Edge login profile is reserved for cookie extraction.
- This is deliberately still experimental because Chromium cookie decryption/profile behavior must be proven on the target Windows machine before replacing LibreWolf in stable.


WinterStatic yt-dlp Downloader is a lightweight native Win32 frontend for
[`yt-dlp`](https://github.com/yt-dlp/yt-dlp). It provides a simple Windows GUI,
optional FFmpeg integration, and a dedicated Microsoft Edge profile for
authenticated downloads when yt-dlp can use browser cookies.

Project: https://github.com/WinterStatic/yt-dlp-WinterStatic

The frontend is written in C++17 and uses the Windows API directly. It does not
require Python, Tkinter, Qt, .NET, or a non-system GUI runtime.

## FEATURES

- Native C++17 / Win32 interface with a compact dark theme.
- Paste, clear, browse, and open controls for the normal download workflow.
- Quality presets:
  - Best quality
  - Best MP4-compatible
  - Maximum 1080p
  - Maximum 720p
  - Audio only (source format)
- FFmpeg-aware format selection and merging.
- 1080p / 720p modes prioritize the requested resolution rather than container;
  use **Best MP4-compatible** when MP4/M4A preference is specifically required.
- Automatic download-resilience layer: modest retry hardening plus targeted
  recovery retries for HTTP rejection / anti-bot symptoms, unstable or throttled
  transfers, and conservative static-page media discovery after `Unsupported URL`.
- Dedicated Microsoft Edge account profile for authenticated YouTube use.
- Authentication modes:
  - Automatic
  - Anonymous
  - Use account browser
- Saved-session check, login reset, and dedicated profile-folder controls.
- yt-dlp version display, browse control, and updater.
- FFmpeg detection from the portable tree, application folder, or PATH.
- Visible PowerShell task window for the real yt-dlp process and full output.
- Optional **Close PowerShell on success** setting; failed tasks remain open for inspection.
- Automatic / manual PowerShell snapping to the right side of the downloader.
- Nuke task button for terminating the currently tracked PowerShell / yt-dlp
  process tree.
- Dynamic GUI pass display that reports the work yt-dlp is actually doing without
  guessing a fixed total number of passes, for example:
  - Pass 1 - Downloading video
  - Pass 2 - Downloading audio
  - Pass 3 - Assembling
- GUI log and persistent `settings.ini`.
- Embedded multi-size application icon and Windows version metadata.

## WHAT IS IN THIS BUILD KIT

- `main.cpp`  
  Native Win32 frontend source.

- `resource.rc`  
  Application icon and Windows version metadata.

- `app.manifest`  
  Windows application identity, DPI-awareness, Common Controls, and long-path
  declarations.

- `app.ico` / `app-arrow-transparent.png`  
  Multi-size Windows application icon and its transparent source artwork.

- `build-native.bat`  
  Finds the Visual Studio C++ toolchain, builds the frontend, creates a portable
  folder and copies an existing local `Tools` tree when present. `release` mode
  additionally creates GitHub-ready portable and frontend/source-kit ZIP assets.
  Microsoft Edge is detected from the normal Windows installation and is not bundled.

- `yt-dlp_winterstatic.png`  
  Main application screenshot used by this README on GitHub.

- `settings.ini`  
  Clean source-tree settings template.

- `LICENSE`  
  MIT license for the WinterStatic frontend source.

- `THIRD-PARTY-NOTICE.txt`  
  Notes for yt-dlp, FFmpeg, Microsoft Edge use, and binary redistribution.

- `PUBLIC-RELEASE-CHECKLIST.md`  
  Source / binary publication checklist for GitHub releases.

- `.gitignore`  
  Keeps build outputs, generated portable folders, local third-party tools, and
  IDE clutter out of the repository.

## BUILD REQUIREMENTS

- Windows 10 or Windows 11.
- Visual Studio 2022 Build Tools (or Visual Studio 2022) with the **Desktop
  development with C++** workload.
- Windows SDK / `rc.exe`.

The frontend itself links only against Windows system libraries.

## BUILDING

Run:

```text
build-native.bat
```

When successful, the build creates:

```text
WinterStatic-yt-dlp-Downloader-0.1.37-portable
```

If that folder already exists and cannot be replaced, the builder may add a
numeric suffix.

For a public GitHub release, run:

```text
build-native.bat release
```

Release mode performs the same validated portable build and additionally creates
two canonical release assets beside the source tree:

```text
WinterStatic-yt-dlp-Downloader-0.1.37-portable.zip
WinterStatic-yt-dlp-Downloader-0.1.37-source.zip
```

The portable ZIP contains the completed self-contained package under a canonical
`WinterStatic-yt-dlp-Downloader-0.1.37-portable` root, even if the temporary
working output folder needed a numeric suffix. The source ZIP is assembled from
an explicit allow-list of the public frontend source/resources/build files, including
`yt-dlp_winterstatic.png` for the GitHub README; it contains no `Tools` binaries,
BrowserProfile data, generated portable package, downloaded media, or other local state. Existing release ZIPs with those names
are replaced only after the portable build has passed its normal validation.

The builder first copies an existing local `Tools` folder beside the source tree
(or one directory above it) when available. It then refreshes the primary yt-dlp
slot from yt-dlp's **latest official stable** Windows release; nightlies are never
selected for that slot. The separate `Tools\yt-dlp\yt-dlp-auth.exe` slot normally
receives that same latest stable. If the latest stable is on WinterStatic's
authenticated-quality blacklist (currently **2026.08.19**), the auth slot instead
uses the known-good **2026.07.04** fallback. Later stable releases are therefore
accepted automatically unless they are explicitly added to the auth blacklist.
If `Tools\FFmpeg\bin\ffmpeg.exe` is missing, the builder downloads and extracts
the Gyan FFmpeg **release essentials** ZIP.

The generated package also creates `Tools\BrowserProfile` for WinterStatic's
dedicated Edge data; Edge itself is used from the Windows installation and is not
copied into the portable folder. Because the builder deliberately refreshes
yt-dlp to the latest stable release, internet access is required for that yt-dlp
refresh; FFmpeg is downloaded only when it is not already available locally.

At runtime, the normal primary yt-dlp path remains portable-first with the existing
configured/application/PATH fallback behavior. Authenticated **YouTube** still
uses the separate `yt-dlp-auth.exe`, but that helper is now maintained by a
stable-only blacklist policy rather than a permanent pin. This keeps the known
2026.08.19 authenticated-quality regression isolated while allowing a later
stable yt-dlp release to become the authenticated backend automatically.

## PORTABLE LAYOUT

Recommended self-contained layout:

```text
WinterStatic-yt-dlp-Downloader.exe
settings.ini
README.txt
LICENSE
BUILD-INFO.txt
THIRD-PARTY-NOTICE.txt
Tools\
  yt-dlp\
    yt-dlp.exe
    yt-dlp-auth.exe
  BrowserProfile\
    Default\
      Network\
        Cookies
  FFmpeg\
    bin\
      ffmpeg.exe
```

A clean `build-native.bat` run now supplies FFmpeg automatically, so the generated
portable folder normally has full merging support out of the box. Runtime fallback
behavior still exists for manually assembled/source-only layouts where FFmpeg is
absent.

## AUTHENTICATION / MICROSOFT EDGE

The account browser uses the Microsoft Edge installation already present on
Windows, but launches it with a dedicated WinterStatic user-data directory:

```text
Tools\BrowserProfile\
```

That directory is separate from the user's normal Edge data. Logging into
YouTube in the WinterStatic window therefore does not log the ordinary Edge
profile into the same account.

Typical setup:

1. Click **Open Account Browser**.
2. Sign into YouTube / Google in the dedicated Edge window.
3. Confirm the site works, then close that WinterStatic Edge window.
4. Click **Check Login**.
5. Leave Authentication on **Automatic**, or select **Use account browser** to
   force the saved browser session.

Authentication modes:

- **Automatic** - starts with the primary anonymous/public backend. If YouTube
  explicitly requires authentication and a saved Edge session is available, the
  task switches to the separate stable-only `yt-dlp-auth.exe` backend plus Edge cookies.
- **Anonymous** - never supplies account-browser cookies and stays on the primary backend.
- **Use account browser** - always asks yt-dlp to load cookies from the dedicated
  Edge profile; on YouTube this also selects the separate authenticated backend.

The dedicated WinterStatic Edge profile must be closed before yt-dlp reads its
cookie database. WinterStatic identifies Edge processes launched with its exact
`--user-data-dir=Tools\BrowserProfile` command-line argument, so the user's
ordinary Edge profile can remain open. A short shutdown grace period is allowed
for Edge to finish exiting after its last WinterStatic window closes.

The green login indicator remains intentionally lightweight: the frontend
checks the Chromium cookie database / WAL for strong Google authentication-cookie
names without adding a SQLite runtime dependency. Cookie values remain encrypted;
yt-dlp performs the actual cookie loading/decryption and remains the final judge
of whether the saved session is usable.

## QUALITY MODES

**Best quality**  
Lets yt-dlp choose its preferred best video/audio combination. With FFmpeg,
separate streams can be merged.

**Best MP4-compatible**  
Prefers MP4 video and M4A audio and requests MP4 output when FFmpeg is present.

**Maximum 1080p / Maximum 720p**  
Prefer MP4 + M4A within the selected height limit, then fall back to other
formats if needed rather than failing a download solely because MP4 is absent.
Formats whose height is not reported by the extractor are also allowed through
these capped selectors, so usable SD/HD streams are not rejected solely because
resolution metadata is missing.

**Audio only (source format)**  
Selects the best audio stream. With FFmpeg available, yt-dlp performs its normal
extract-audio processing.

## DOWNLOAD RESILIENCE

The normal yt-dlp attempt remains the preferred path. WinterStatic adds modest
retry hardening (`12` normal retries, `15` fragment retries, and `5` extractor /
file-access retries) with a one-second pause between retry attempts.

If the normal workflow still fails, the PowerShell task watches yt-dlp's error
output and selectively escalates:

- HTTP 403/429, Forbidden, Cloudflare/challenge/CAPTCHA, or similar anti-bot
  signals can trigger one browser-impersonation recovery attempt. WinterStatic
  first asks the installed yt-dlp build for its available `curl_cffi` Chrome
  impersonation targets and only retries when a compatible target is actually
  exposed.
- Repeated throttling, incomplete reads, connection resets/timeouts, or HTTP
  502/503/504 errors can trigger one conservative `5M` chunked-HTTP recovery
  attempt. If browser impersonation was already selected for the same failed
  job, the chunked retry keeps it.

These recovery attempts retain the selected quality, output path, FFmpeg setup,
and account-browser authentication arguments. They are intentionally conditional: the
frontend does not globally force browser impersonation or chunked downloading on
healthy sites, because either option can make some servers less reliable.

If yt-dlp reports that the requested format is unavailable or that no video
formats were found, WinterStatic makes one relaxed best-available format attempt.
This is useful on sites that expose a valid stream but incomplete format metadata.
An authenticated format-selection failure also skips the otherwise redundant
second identical authenticated attempt and proceeds directly to this recovery.

This is a network / extractor resilience feature, not a DRM bypass. It does not
remove account, subscription, geographic, or encryption requirements imposed by
a service.

## UNSUPPORTED URL RECOVERY

When yt-dlp reports `ERROR: Unsupported URL`, or an HTTP 404 occurs before actual media download progress begins, WinterStatic performs one
conservative static-page discovery pass before giving up. The normal extractor
path is always tried first.

The recovery fetches the original webpage and looks for high-confidence media
references that are commonly exposed even on sites without a dedicated yt-dlp
extractor:

- Open Graph / Twitter video metadata, including `twitter:player`
- HTML5 `<video>` and `<source>` URLs, including lazy `data-src` forms
- JSON-LD / structured-data `contentUrl` and `embedUrl` entries
- serialized `playerUrl` / embedded-player references when present in page state
- structured hosted-player component metadata when the final player URL is assembled by page script
- direct MP4/WebM/audio URLs
- HLS `.m3u8` playlists and DASH `.mpd` manifests
- up to one level of embedded player frames
- one declared `amphtml` alternate representation when the main page exposes one

If a candidate is found, WinterStatic feeds that URL back through yt-dlp and
preserves the original page as the Referer while reusing the same browser-like
User-Agent used for discovery. Existing account-browser authentication arguments are
also retained when the original attempt was authenticated. Signed or tokenised
media URLs are deliberately not copied into the GUI log.

The discovery pass is intentionally conservative and bounded: it does not run on
normal successful downloads, scans at most the first 8 MiB of page HTML, follows
only a few iframe candidates and only one iframe level, follows at most one declared
AMP alternate, and does not attempt to bypass DRM or decrypt protected media. It
also decodes common HTML / JavaScript escape forms before checking serialized player
metadata. Pages that expose media only through runtime network activity may still
require a future browser-assisted fallback.

## POWERSHELL TASK WINDOW

Downloads run in a visible PowerShell window rather than being hidden inside the
GUI. This is deliberate:

- the user can see yt-dlp's complete output
- `Ctrl+C` remains available
- closing the GUI does not kill an active download
- the finished PowerShell window remains open for inspection

The frontend attempts to place its tracked PowerShell / Windows Terminal window
to the right of the downloader. **Snap PowerShell right** retries this manually.
**Nuke task** terminates only the currently tracked task tree and closes its
tracked terminal window where possible.

**Close PowerShell on success** is optional and disabled by default. When enabled,
the choice is baked into the launched task so a successful PowerShell window closes
even if the GUI itself has already been closed. Failed tasks deliberately stay open
so yt-dlp's full diagnostics remain available. The preference is saved in
`settings.ini`.

A cancelled yt-dlp job can leave a `.part` file, which is normal yt-dlp partial
-download behaviour.

## TEMPORARY TASK HOUSEKEEPING

Each download uses a small `%TEMP%\WinterStaticDL_*` working folder for its
PowerShell script, progress log, and completion marker. These are not media
downloads and do not contain browser cookies.

- Finished task folders more than 24 hours old are removed on frontend startup.
- Active tasks are not selected by the startup cleanup because they have not yet
  produced the normal `done.txt` completion marker.
- A task folder is removed immediately when PowerShell fails to launch.
- **Nuke task** also makes a best-effort cleanup after the tracked PowerShell
  process has actually stopped.

This keeps normal use from accumulating tiny task-control files while avoiding
cleanup of a live download.

## PROGRESS DISPLAY

The GUI does not assume that every site or quality mode has the same number of
passes. Each distinct media stream reported by yt-dlp starts the next pass and is
labelled according to the codecs yt-dlp reports. A common separate-stream download
therefore appears as **Pass 1 - Downloading video**, **Pass 2 - Downloading audio**,
then **Pass 3 - Assembling** when FFmpeg merges them. A site that supplies one
combined video/audio file may only show **Pass 1 - Downloading**, while audio-only
or other workflows can use fewer or different passes. The pass number is deliberately
shown without an "of N" total because the required work is not always known in
advance. The visible PowerShell window remains the authoritative detailed yt-dlp
output display.

## SETTINGS

`settings.ini` stores:

- output directory
- yt-dlp path
- FFmpeg path
- quality preset
- authentication mode
- close PowerShell on success preference

In a normal portable build the file lives beside the EXE.

## UPDATES

- **yt-dlp Update** launches yt-dlp's own `-U` updater in PowerShell.
- **Open Profile** opens the dedicated `Tools\BrowserProfile` directory used by the account browser.
- **FFmpeg Detect** refreshes the FFmpeg path; the frontend does not choose or
  download a third-party FFmpeg build automatically.

## ABOUT / BRANDING

Product name:

```text
WinterStatic yt-dlp Downloader
```

Executable:

```text
WinterStatic-yt-dlp-Downloader.exe
```

Project:

```text
https://github.com/WinterStatic/yt-dlp-WinterStatic
```

Created by WinterStatic. Developed with ChatGPT (OpenAI). Additional code review by Claude (Anthropic).

This project is a frontend for yt-dlp. It is not the yt-dlp project itself and
is not presented as an official yt-dlp build.

## LICENSING

The WinterStatic frontend source is licensed under the MIT License. See
`LICENSE`.

`yt-dlp`, FFmpeg, and Microsoft Edge are separate programs/projects and retain
their own licenses, copyright notices, build options, and distribution terms.
They are not linked into the frontend executable.

The source repository intentionally ignores `Tools/` so local third-party
binaries are not accidentally committed. `build-native.bat` can obtain missing
runtime binaries from the official yt-dlp release and Gyan's FFmpeg release
essentials package when preparing a portable build. Before publishing a **Full
Portable** release containing those programs, check `THIRD-PARTY-NOTICE.txt` and
`PUBLIC-RELEASE-CHECKLIST.md` and document the exact binaries being distributed.
FFmpeg licensing in particular depends on the configuration of the exact build.

## VERSION HISTORY

### 0.2.42experimental - CANDIDATE RANKING

- Experimental unsupported-page recovery now collects and ranks multiple media/player candidates instead of immediately accepting the first valid-looking video URL.
- Embedded/structured player URLs and HLS/DASH manifests are preferred over generic direct-file references.
- Obvious preview/hover/teaser/poster-style URLs are strongly deprioritised rather than permanently blocked.
- Bounded Range probes record the total resource size when available; very small direct video files are deprioritised while larger full-video candidates receive a modest preference.
- Candidate validation remains bounded to at most ten direct-resource probes per scanned page and still rejects image and HTML false positives.
- No site-specific rules, browser automation, or runtime network interception were added.

### 0.2.40experimental - GENERIC MEDIA DISCOVERY IMPROVEMENTS

- Built directly from the 0.1.31 stable baseline, carrying forward only the generic discovery changes that proved useful during experimental testing.
- Expanded unsupported-page HTML5 discovery to recognise lazy `data-src`, `data-video-src`, and lazy iframe sources.
- Added direct media discovery from relative or absolute links/references for common video/audio files, HLS (`.m3u8`), and DASH (`.mpd`).
- JSON-LD `contentUrl` discovery now resolves relative URLs as well as absolute URLs.
- Discovered media remembers the page or iframe that exposed it and uses that page as the retry Referer.
- Added a bounded Range probe for discovered candidates to reject obvious HTML/error-page false positives while allowing protected or inconclusive candidates to continue to yt-dlp.
- If the normal recovery page fetch is blocked or exposes no usable media, WinterStatic can ask yt-dlp to capture the webpage source and scan that captured copy once. This recovered media on pages that blocked the frontend's separate HTTP fetch during testing.
- Added generic embedded-player bootstrap/reference detection for common `/embed/` and `/players/` style URLs found in static page markup.
- No site-specific adapters, route rewriting, WebDriver/GeckoDriver/Selenium, remote debugging, automatic browser control, AMP-specific handling, or unproven structured-player metadata experiments are included.

### 0.1.31 - RELIABILITY AND FORMAT COMPATIBILITY

- Added a short LibreWolf shutdown grace period so normal background profile/cookie flushing after the final browser window closes is not mistaken for an open browser. Visible and background-only LibreWolf states are reported separately, and WinterStatic never kills LibreWolf automatically.
- Updated the Maximum 1080p / Maximum 720p selectors to accept formats with unknown height metadata, preserving capped-format preference while allowing usable streams from sites such as Facebook.
- Added one best-available format recovery attempt when yt-dlp reports `Requested format is not available` or `No video formats found`.
- Authenticated downloads now skip a redundant identical second cookie attempt when the failure is specifically format selection and proceed directly to format recovery.
- Fixed portable-build settings generation so `ClosePowerShellOnSuccess=0` is explicitly written to the generated `settings.ini`.
- Retained the 0.1.30 conservative Unsupported URL static discovery, 0.1.29 network resilience, stable PowerShell placement, Nuke, dynamic pass reporting, and existing GUI layout without adding automatic browser-based media discovery.

### 0.1.30 - UNSUPPORTED URL RECOVERY

- Added an automatic recovery path specifically for yt-dlp `Unsupported URL`
  failures; normal supported URLs still use the normal extractor path first.
- The fallback performs a bounded static-page scan for Open Graph/Twitter video
  metadata, HTML5 video/source URLs, JSON-LD `contentUrl`, direct media links, HLS
  playlists, DASH manifests, and a small number of embedded-player frames.
- Discovered media is handed back to yt-dlp rather than downloaded by a second
  custom downloader, preserving the existing format, FFmpeg, progress, Nuke, and
  terminal workflow.
- The original webpage is supplied as Referer on the discovered-media retry, the
  discovery User-Agent is preserved, and existing LibreWolf cookie arguments are
  retained for authenticated attempts.
- Candidate URLs themselves are not written to the GUI log, avoiding accidental
  exposure of signed/tokenised media URLs.
- Discovery is intentionally limited to static non-DRM media references: 8 MiB
  maximum HTML scan, four iframe candidates, one iframe level, and no JavaScript
  execution or DRM circumvention.
- No GUI layout, terminal placement, dynamic pass reporting, format selection,
  Nuke, or Close PowerShell behaviour was intentionally changed.

### 0.1.29 - DOWNLOAD RESILIENCE

- Added a conservative automatic recovery layer around the normal yt-dlp workflow.
- Raised retry limits modestly to 12 normal, 15 fragment, and 5 extractor /
  file-access retries, with a one-second retry pause to avoid hammering unstable
  endpoints.
- Failed attempts now classify common HTTP rejection / anti-bot symptoms such as
  403, 429, Forbidden, Cloudflare/challenge, and CAPTCHA responses.
- When those signals are seen, WinterStatic checks the installed yt-dlp build for
  an actually available `curl_cffi` Chrome impersonation target and retries once
  using that target. No impersonation flag is forced when the local yt-dlp build
  does not support it.
- Throttling, incomplete-read, timeout/reset, and HTTP 502/503/504 failures can
  trigger one additional retry using a conservative 5 MiB HTTP chunk size.
- Recovery retries preserve the current authentication arguments, so a saved
  LibreWolf session remains in use when the failed job was authenticated.
- Recovery markers are written to the GUI log so an automatic escalation is
  visible when it happens, while the normal successful path is unchanged.
- Browser impersonation and chunked HTTP remain conditional rather than global,
  avoiding unnecessary regressions on sites that already work normally.
- No GUI layout, terminal placement, Nuke, dynamic pass reporting, format
  selection, or Close PowerShell behaviour was intentionally changed.

### 0.1.28 - CLOSE-ON-SUCCESS FIX

- Fixed **Close PowerShell on success** when PowerShell is launched with `-NoExit`.
- The previous script used a normal PowerShell `exit`, which could return to the
  interactive `-NoExit` host instead of terminating the PowerShell process.
- Successful auto-close now terminates the actual PowerShell host only after the
  download log is flushed and `done.txt` has been written.
- Failed tasks still remain open for inspection, and unchecked behaviour is unchanged.
- No changes to download formats, authentication, Nuke, dynamic pass reporting,
  terminal placement, or GUI layout.

### 0.1.27 - CLOSE POWERSHELL ON SUCCESS

- Added an optional **Close PowerShell on success** checkbox in the Run panel.
- The preference is disabled by default and persisted in `settings.ini`.
- The setting is captured when a download starts, so the PowerShell task can close
  itself after success even if the frontend GUI has already been closed.
- Failed downloads always keep PowerShell open for inspection, regardless of the
  checkbox state.
- The completion marker is written before automatic PowerShell exit so the GUI can
  still report successful completion reliably.
- No changes to yt-dlp format selection, authentication, Nuke, or terminal placement.

### 0.1.26 - DYNAMIC PASS REPORTING

- Replaced the fixed **Pass 1 of 2 / Pass 2 of 2** presentation with dynamic pass
  numbering that does not claim a total number of passes in advance.
- Each distinct yt-dlp media stream now starts the next pass, using the reported
  format ID as the primary stream identity.
- Download passes are labelled from yt-dlp codec metadata as **Downloading video**,
  **Downloading audio**, or simply **Downloading** for a combined/unknown stream.
- The GUI progress percentage now describes the current stream rather than combining
  separate video/audio byte totals into an artificial single download sweep.
- FFmpeg merge or extraction work advances to the next pass and reports
  **Assembling**, **Processing audio**, or **Processing** as appropriate.
- Once post-processing begins, late buffered download lines cannot move the GUI
  backwards into another download pass.
- Removed the 0.1.25 pre-download size-planning markers because dynamic pass reporting
  no longer needs to predict the selected streams' combined size.
- No format-selection, authentication, Nuke, terminal-placement, logging, or GUI
  layout behaviour was intentionally changed.

### 0.1.25 - TWO-PASS PROGRESS RESTORATION

- Restored the original two-sweep progress model from the Python frontend instead
  of treating each selected yt-dlp stream as a separate GUI progress sweep.
- Pass 1 now plans the selected video/audio stream sizes before download and
  tracks their combined downloaded bytes as one continuous overall percentage.
- The second media stream therefore continues Pass 1 instead of resetting the GUI
  progress bar and appearing to be another mislabeled Pass 1.
- Pass 2 remains reserved for FFmpeg merge / audio post-processing and becomes
  sticky once assembly begins; native postprocessor progress markers provide a
  second detection path in addition to the normal yt-dlp `[Merger]` /
  `[ExtractAudio]` messages.
- Structured planning/progress lines remain hidden from the live PowerShell view;
  the terminal continues to show a compact human-readable per-stream percentage
  and speed.
- No format-selection, authentication, Nuke, terminal-placement, or GUI layout
  behaviour was intentionally changed.

### 0.1.24 - PASS-STATE FIX

- Fixed a GUI progress-state race where a late yt-dlp progress line could change
  the stage label back to **Pass 1 of 2** after merge / audio post-processing had
  already begun.
- Pass 2 is now sticky for the active yt-dlp attempt until a genuinely new retry
  starts.
- No download, format-selection, authentication, Nuke, or terminal-placement
  behaviour was intentionally changed.

### 0.1.23 - WINDOWS TERMINAL PLACEMENT SAFETY

- Reworked automatic PowerShell / Windows Terminal placement to avoid moving a
  newly created Terminal host while its non-client/title-bar layout is still
  changing.
- Automatic snap now waits for the selected visible terminal HWND to remain at a
  stable rectangle before performing one placement operation.
- Removed the early `STARTF_USEPOSITION` launch hint so Windows Terminal can
  finish creating its own window chrome before the frontend intervenes.
- Placement now uses one `SetWindowPos` operation after stabilization rather than
  moving/resizing a just-created host immediately.
- If no confidently identified terminal window becomes stable, automatic snap is
  skipped instead of forcing an unsafe move; **Snap PowerShell right** uses the
  same stability check for manual retries.
- No download, logging, authentication, Nuke, format-selection, or GUI layout
  behaviour is intentionally changed from 0.1.22.

### 0.1.22 - POWERSHELL LOGGING RELIABILITY

- Replaced repeated PowerShell `Add-Content` calls with one explicitly shared
  `FileStream` / `StreamWriter` for the per-task progress log.
- The log writer now uses `FileShare.ReadWrite`, allowing the native GUI monitor
  to read progress while PowerShell continues writing without fighting over the
  log file.
- Logging failures are isolated from the yt-dlp task instead of flooding the live
  PowerShell window with `Stream was not readable` errors.
- The log stream is flushed automatically and disposed before the normal task
  completion marker is written.
- Download, authentication, format-selection, PowerShell placement, Nuke, and GUI
  layout behaviour are otherwise unchanged from 0.1.21.

### 0.1.21 - RELEASE HOUSEKEEPING

- Added conservative cleanup for finished `%TEMP%\WinterStaticDL_*` task folders
  older than 24 hours.
- Added immediate cleanup for temporary task folders when PowerShell cannot be
  started.
- Added best-effort temporary-task cleanup after **Nuke task** confirms the
  tracked PowerShell process is no longer running.
- Updated the Windows build script to remove its temporary compiler workspace
  after failed builds and discard incomplete portable folders after packaging /
  LibreWolf setup failures.
- Replaced the remaining development-only pre-release wording in the public
  changelog.
- No download, authentication, format-selection, PowerShell placement, Nuke, or
  GUI workflow behaviour is intentionally changed from 0.1.20.

### 0.1.20 - GITHUB / DISTRIBUTION PREP

- Rebuilt the README as public project documentation and added the official
  repository URL.
- Consolidated the internal 0.1 development history into one readable entry
  rather than exposing every cosmetic internal revision as a public changelog.
- Expanded the public-release checklist and third-party runtime notice.
- Removed remaining pre-release wording from the frontend source comments.
- Added the project URL and frontend license to generated `BUILD-INFO.txt`.
- Hardened portable packaging so required README, license, notice, settings,
  build-info, executable, and LibreWolf files are checked before the builder
  reports success.
- Performed a final source / branding / version-metadata audit for the first
  GitHub publication.
- No downloader, authentication, format-selection, PowerShell, Nuke, or GUI
  workflow behaviour is intentionally changed from 0.1.19.

### 0.1 - INITIAL NATIVE DEVELOPMENT

The 0.1.x line was the private development cycle that established the native
frontend. Its changes are intentionally consolidated here.

- Reimplemented the earlier downloader frontend in native C++17 / Win32 with no
  Python, Tkinter, Qt, or .NET runtime requirement.
- Established the WinterStatic product branding, application icon, manifest,
  version resources, build script, portable settings, and release-kit layout.
- Built the dark charcoal / teal interface and progressively polished control
  spacing, typography, button ordering, ComboBox rendering, and native dark-mode
  presentation.
- Added URL paste / clear, output browse / open, quality selection, authentication
  selection, GUI log, progress display, and persistent settings.
- Added yt-dlp detection, version reporting, browse selection, update launching,
  and PATH fallback.
- Retained FFmpeg as an active runtime component for separate-stream merging,
  audio extraction, and higher-quality format selection; added detection and
  clear full-quality / fallback status reporting.
- Added MP4-preferred 1080p / 720p selection while preserving non-MP4 fallbacks
  when necessary.
- Added the dedicated portable LibreWolf account browser, profile preparation,
  login check, reset, and updater integration.
- Added clean-build LibreWolf bootstrap from the official LibreWolf portable
  download, including SHA-256 verification when the published sidecar is
  available.
- Added Automatic, Anonymous, and Use account browser authentication modes with
  saved-session handling and authenticated retry behaviour.
- Added visible PowerShell ownership of yt-dlp tasks so downloads survive GUI
  closure and full yt-dlp output remains accessible.
- Reworked terminal-window identification and placement until automatic right-side
  snapping was reliable with PowerShell / Windows Terminal; retained a manual
  Snap PowerShell right control.
- Added the Nuke task action for force-stopping the currently tracked task tree.
- Added GUI pass/status reporting for downloading, assembly, and audio processing.
- Added extensive final UI alignment and dark-mode polish without changing the
  underlying download workflow.

### 0.2.40experimental

- Experimental false-positive hardening for unsupported-page media discovery.
- Rejects common image extensions before probing a discovered candidate.
- Rejects `image/*` responses and common JPEG/PNG/GIF/WebP/BMP file signatures during the bounded discovery probe.
- Keeps fail-open handling for unusual video CDNs that return ambiguous non-image content types such as `application/octet-stream`.
- No site-specific rules were added.


## Changelog

### 0.2.42experimental - PREVIEW REFUSAL + DIAGNOSTICS

- Marks autoplay/muted/loop-style HTML5 video elements as likely previews.
- Refuses to select preview-like candidates when no normal candidate exists; a clean failure is preferred to downloading the wrong hover clip.
- Prints a privacy-safe candidate summary (kind, score, MIME type, size, preview flag; URLs hidden).


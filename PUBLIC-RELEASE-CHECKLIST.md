## 0.1.37 release-packaging mode check

- Run `build-native.bat` with no argument and confirm the normal portable folder behavior is unchanged.
- Run `build-native.bat release` and confirm it performs the normal validated build first, then creates `WinterStatic-yt-dlp-Downloader-0.1.37-portable.zip` and `WinterStatic-yt-dlp-Downloader-0.1.37-source.zip` beside the source tree.
- Also test release mode from a clean source tree with no local `Tools\FFmpeg` folder, so the Gyan FFmpeg download/extract path is exercised before the release ZIPs are created.
- Open the portable ZIP and confirm it contains one canonical `WinterStatic-yt-dlp-Downloader-0.1.37-portable` root with the EXE, clean settings, README/license/notices, BUILD-INFO, both yt-dlp backends, FFmpeg, and an empty/clean BrowserProfile area only.
- Confirm `yt-dlp_winterstatic.png` is present in the source tree and the README renders it directly below the version heading.
- Open the source ZIP and confirm it contains the public source/resources/build files only: `.gitignore`, `LICENSE`, `PUBLIC-RELEASE-CHECKLIST.md`, `README.md`, `THIRD-PARTY-NOTICE.txt`, `yt-dlp_winterstatic.png`, artwork/icon, manifest, `build-native.bat`, `main.cpp`, `resource.rc`, and the clean `settings.ini` template.
- Confirm the source ZIP contains no `Tools`, browser cookies/profile backups, downloaded media, generated EXE/OBJ/RES files, local IDE files, or portable output folder.
- Confirm the copy of `build-native.bat` inside the source ZIP itself still supports the `release` argument.
- If canonical release ZIPs already exist, confirm a new successful release build replaces them; if they cannot be removed/written, release packaging must fail rather than silently leaving a stale asset.
- Keep the existing third-party redistribution checks below before publishing the Full Portable asset; the generated frontend/source-kit ZIP is not a substitute for any source/notice obligations of the exact yt-dlp/FFmpeg binaries shipped.

## 0.1.37 stable-backend policy / hover-help check

- Confirm the application, EXE metadata, manifest, build banner, BUILD-INFO, and portable output folder all report **0.1.37**.
- Build/update while **2026.08.19** is the latest official stable: primary must be `2026.08.19`, authenticated must fall back to `2026.07.04`, and no nightly may be selected.
- When a later official stable is available and is not on the auth blacklist, confirm both primary and authenticated slots advance to that later stable.
- Confirm the auth blacklist initially contains only **2026.08.19**; adding a future bad stable should require only another explicit blacklist entry (and, if it is the current latest, choosing the desired known-good fallback).
- Confirm the Tools yt-dlp status is `primary+authenticated` with no `A`, e.g. `2026.08.19+2026.07.04`; missing/unreadable auth helper should display `+--`.
- Hover the yt-dlp version status and confirm the tooltip explains left=primary, right=authenticated, latest-stable/no-nightly policy, the current auth blacklist, and `--`.
- Hover **Browser sweep** and confirm the tooltip explains the optional last-resort visible Edge/Chrome/Brave network-observer behavior and that it is off by default.
- Re-test one public YouTube download, the known authenticated/age-gated case, one progress-jitter case, and one Browser sweep recovery target. No unrelated runtime behavior should differ from 0.1.36.

## 0.1.36 restart-aware progress clamp check

- Confirm the application, EXE metadata, manifest, build banner, BUILD-INFO, and portable output folder all report **0.1.36**.
- Re-test the fragmented download that previously produced percentage jitter. When `downloaded_bytes` continues increasing, the GUI percentage must remain monotonic exactly as in 0.1.35.
- Simulate or capture a same-attempt restart where `downloaded_bytes` drops by more than 64 KiB without a new `=== ... ===` banner. Confirm the GUI releases the old high-water mark and recounts from the new lower percentage instead of appearing stuck.
- Confirm a byte-count decrease of 64 KiB or less does not release the percentage clamp.
- Confirm existing reset boundaries still work: each new attempt banner and each genuine new stream/pass must reset both the displayed-percent and downloaded-byte trackers.
- Confirm visible PowerShell/yt-dlp output remains raw and unchanged; this patch must affect only the native GUI progress display.
- Re-run the 0.1.35 progress tests plus one public YouTube, one authenticated/age-gated YouTube, and one Browser sweep case; no download/recovery/backend behavior should differ from 0.1.35.

## 0.1.35 monotonic progress / cleanup check

- Confirm the application, EXE metadata, manifest, build banner, BUILD-INFO, and portable output folder all report **0.1.35**.
- Test a fragmented YouTube/DASH-HLS download that previously showed percentage jitter. Within one pass the GUI percentage and progress bar must never move backward; a revised yt-dlp estimate may make the displayed value pause temporarily.
- Confirm a genuine new stream/pass resets the high-water mark: for a video+audio download, the audio pass must be able to start near 0% after the video pass reaches 100%, rather than remaining pinned at 100%.
- Confirm the visible PowerShell/yt-dlp output remains raw and unchanged; only the native GUI display is clamped.
- Test one direct/progressive HTTP download with a known total and confirm ordinary progress remains smooth and reaches 100%.
- Re-run the 0.1.34 account-browser test: normal Edge must not block WinterStatic, while the dedicated `Tools\BrowserProfile` Edge instance must block profile reads/reset/authenticated startup until closed.
- Confirm public YouTube, authenticated/age-gated YouTube, Browser sweep, split yt-dlp routing, and Update behavior are unchanged from 0.1.34.

## 0.1.34 dedicated Edge profile-use detection check

- Confirm the application, EXE metadata, manifest, build banner, BUILD-INFO, and portable output folder all report **0.1.34**.
- Keep a normal Edge window/profile open, with no WinterStatic account browser running. **Check Login**, **Reset Login**, and authenticated-download validation must not claim the WinterStatic profile is open.
- Open WinterStatic's dedicated account browser. While it remains open, **Check Login** must report that the WinterStatic Edge profile is open, **Reset Login** must stop with the friendly close-browser warning, and **Use account browser** must refuse to start a download until that dedicated window is closed.
- Close the dedicated WinterStatic Edge window and confirm the existing shutdown grace period releases the gate within about four seconds.
- Repeat the normal-Edge-open test after closing the dedicated window to confirm ordinary `msedge.exe` processes are ignored.
- Confirm the yt-dlp status regression fix from 0.1.33 remains intact: present/unreadable primary -> **Detected**, absent primary -> **Missing**, missing auth helper with known primary -> `+A--`.
- Re-test one public YouTube download and one authenticated/age-gated YouTube download; no download/backend behavior should differ from 0.1.33.

## 0.1.33 mainstream release / status regression check

- Confirm the application, EXE metadata, manifest, build banner, BUILD-INFO, and portable output folder all report **0.1.33** with no `experimental` suffix.
- Confirm the normal Tools status shows both backends compactly, e.g. `2026.08.19+A2026.07.04`.
- Temporarily make `YtDlpVersion()` unable to return a version while leaving the primary executable present; confirm the GUI shows **Detected**, not **Missing**.
- Remove or point away from the primary executable and confirm the GUI shows **Missing**.
- Remove the authenticated helper while leaving the primary healthy and confirm the status uses `+A--`.
- Confirm `yt-dlp`, `Edge`, and `FFmpeg` labels are not clipped in the 68 px Tools label column.
- Confirm **Browser sweep** is off by default on a fresh settings file, persists when changed, and does not launch the runtime browser when disabled.
- Re-test ordinary public YouTube, the known authenticated/age-gated case, and one stubborn non-YouTube recovery target before publishing.
- Confirm the GUI **Update** action refreshes the primary stable backend while restoring/revalidating authenticated `yt-dlp-auth.exe` as exactly **2026.07.04**.

## 0.2.69experimental compact dual-version / Tools layout check

- Confirm the yt-dlp status reads `2026.08.19+A2026.07.04` for the current split and is not clipped.
- Confirm `yt-dlp`, `Edge`, and `FFmpeg` labels still render comfortably in the narrowed first column.
- Confirm the path fields, buttons, and status fields have shifted left slightly without overlapping or changing widths.
- Confirm Browser sweep remains off by default on a fresh config and its persistence/behavior is unchanged from 0.2.68.
- Confirm ordinary public YouTube and the authenticated age-gated case behave identically to 0.2.68.


## 0.2.68experimental full dual-version display check

- Confirm the Tools yt-dlp status shows both complete versions, e.g. `2026.08.19 / A:2026.07.04`, without clipping.
- Confirm the 0.2.67 **Browser sweep** checkbox remains off by default and persists its saved state.
- Confirm no functional download/recovery behavior differs from 0.2.67.


## 0.2.68experimental browser-sweep opt-in / compact version status check

- Fresh/default `settings.ini`: confirm **Browser sweep** starts unchecked.
- Enable Browser sweep, restart the app, and confirm the setting persists; disable it and confirm that state also persists.
- With Browser sweep **off**, test a URL that reaches discovery: static/captured-page recovery should still run, no temporary browser should open, and PowerShell should explicitly report that browser-assisted recovery is disabled.
- With Browser sweep **on**, re-test the known runtime-discovery case and confirm the visible isolated browser/CDP observer behaves exactly as before.
- Confirm the yt-dlp Tools status shows both versions compactly (for the current split, `08.19 / A:07.04`) without widening the existing status column.
- Re-test ordinary public YouTube and an authenticated/age-gated YouTube case to ensure the 0.2.66 split-backend routing is unchanged.

## 0.2.66experimental primary-stable / split-backend check

- Confirm `Tools\yt-dlp\yt-dlp.exe` reports an official stable version **2026.08.19 or newer**, not a nightly version, and `Tools\yt-dlp\yt-dlp-auth.exe` reports exactly **2026.07.04**.
- Build once with a 2026.08.19 primary present and confirm it is retained rather than blacklisted.
- Build once with a nightly primary present and confirm the generated portable package replaces it with the current official stable build.
- Re-test ordinary public `D9290H1-ZRM`; Automatic mode must use the primary stable backend and pass the old ~2% / HTTP 403 failure point.
- Re-test the known age-gated/authenticated video; Automatic should switch to the pinned 2026.07.04 authenticated backend and avoid the newer-backend 360p regression.
- Test **Use account browser** on YouTube and confirm it selects `yt-dlp-auth.exe` immediately.
- Click the GUI yt-dlp **Update** button and confirm it refreshes the primary from the normal stable channel while restoring/revalidating `yt-dlp-auth.exe` as exactly 2026.07.04.
- Confirm the live PowerShell header prints both primary and authenticated backend paths/versions.


## 0.2.65experimental split-backend / authenticated-nightly blacklist check

- Confirm `Tools\yt-dlp\yt-dlp.exe` is a current post-`android_vr` nightly and `Tools\yt-dlp\yt-dlp-auth.exe` reports exactly **2026.07.04**.
- Re-test ordinary public `D9290H1-ZRM`: Automatic mode must stay on the primary nightly and pass the old ~2% / HTTP 403 failure point.
- Re-test the known age-gated/authenticated video that dropped to 360p on the nightly. Automatic should begin anonymously, detect the explicit authentication requirement, then print that it is switching to the pinned 2026.07.04 authenticated backend.
- Confirm the authenticated preflight and real download expose the expected quality above 360p when the source provides it.
- Test **Use account browser** on the same YouTube case and confirm it uses `yt-dlp-auth.exe` immediately.
- Confirm an authenticated non-YouTube download still uses the primary backend rather than the pinned YouTube-only backend.
- Click the GUI yt-dlp **Update** button and confirm it refreshes the primary nightly while restoring/revalidating `yt-dlp-auth.exe` as exactly 2026.07.04.
- Confirm the live PowerShell header prints both primary and authenticated backend paths/versions.


# Public release checklist


## 0.2.64experimental YouTube backend-refresh check

- Build from a source tree containing yt-dlp 2026.07.04 and confirm the generated portable package replaces it with a current official nightly build.
- Confirm the live PowerShell window prints a yt-dlp version newer than the 2026.08.18 `android_vr` removal floor.
- Re-test `https://www.youtube.com/watch?v=D9290H1-ZRM`; it must not stall around the first ~1 MiB / ~2% and fail with the old `android_vr` HTTP 403 behavior.
- If verbose output is inspected manually, confirm the ordinary logged-out YouTube path no longer selects the retired `android_vr` client by default.
- Re-test the 0.2.63 public YouTube case and confirm Automatic mode still begins anonymously.
- Re-test the known authenticated/age-restricted case and record selected height; the existing conditional mweb fallback must remain available.
- Click **yt-dlp Update** and confirm it installs the latest official nightly build rather than returning to 2026.07.04.

## 0.2.63experimental automatic-auth-on-demand check

- With a confirmed saved Edge session present and Authentication set to **Automatic**, download a normal public YouTube video and confirm the first attempt is anonymous (no `--cookies-from-browser` on that first attempt).
- Re-test `https://www.youtube.com/watch?v=9cpBOfcf2w8` several times in Automatic mode; it should normally stay on the anonymous path rather than pre-emptively using Edge cookies.
- Test an age/login/private/members-only YouTube case that yt-dlp explicitly reports as requiring authentication; Automatic should then retry using the saved dedicated Edge session.
- Confirm an unrelated transient failure does not trigger the account fallback merely because a saved login exists.
- Confirm **Use account browser** still supplies Edge cookies from the first attempt.
- Confirm **Anonymous** never supplies Edge cookies.
- Leave the dedicated Edge account-browser window open and confirm **Automatic** can still start a public anonymous-first download; the account fallback should be unavailable until the browser is closed.
- Confirm 0.2.62 runtime/CDP recovery behaviour remains unchanged, including the difficult page that succeeded in 0.2.61.


## 0.2.62experimental post-success bug-sweep check

- Re-test the difficult page that succeeded in 0.2.61; Edge runtime discovery must still attach and find the full media candidate.
- Test a page that redirects to a different host if available; the observer should be able to use the isolated HTTP(S) fallback target rather than timing out solely on the original host filter.
- Confirm a failed CDP `Network.enable` send is treated as an attachment failure/retry rather than a false successful attachment.
- Confirm WebSocket diagnostics do not print URL-like text.
- Exercise at least one multi-attempt failure/recovery path and confirm 404/progress/format signals from one attempt do not leak into the next attempt.
- Confirm hidden helper/version probes actually stop at their timeout instead of blocking afterward in the output-pipe read.


## 0.2.61experimental CDP target-normalization / Edge retest

- Use the same difficult page that produced `System.Object[]` -> `System.Uri` in 0.2.60experimental.
- Confirm runtime discovery opens Edge first.
- Confirm the previous `Cannot convert the "System.Object[]" ... to type "System.Uri"` error is gone.
- If attachment succeeds, confirm the 30-second network capture begins only after attachment.
- If attachment still fails, record the first safe WebSocket exception; that failure is now after scalar target/URI normalization and is meaningful for origin/policy investigation.
- Confirm media URLs remain hidden.

## 0.2.60experimental Chrome CDP comparison check

- Use a difficult page that exhausts static discovery and reaches runtime network discovery.
- Confirm the isolated detector is **Google Chrome**, not Edge or Brave.
- Confirm the console allows up to **60 seconds** for endpoint/target/WebSocket attachment.
- If attachment fails, record the first printed WebSocket attachment exception and final attempt count.
- If Chrome succeeds on the same machine/page where Edge failed, treat that as evidence of an Edge-specific CDP/WebSocket/policy issue.
- Confirm account/login behavior is unchanged and still uses the dedicated Edge account profile.

## 0.2.59experimental one-minute Edge startup/attach check

- Use the same difficult page that failed in 0.2.58 while Edge was still loading.
- Confirm the console states that the Edge observer startup/attachment window is **60 seconds**.
- Confirm WinterStatic continues retrying instead of failing at the old 20-second point.
- If attachment succeeds, confirm the separate 30-second media-capture window starts only after that success.
- If it still fails after the full minute, record the attach-attempt count; do not interpret that result as a startup-time problem.

## 0.2.58experimental Edge observer startup/attach retry check

- Use a page that requires runtime network discovery and confirm Edge is opened first.
- Confirm the PowerShell window says it is waiting for the Edge observer endpoint/page target instead of failing immediately while Edge is still loading.
- Confirm early CDP/WebSocket attachment failures are retried for up to 20 seconds.
- Confirm the normal 30-second media capture begins only after a successful observer attachment.
- If attachment still fails, confirm the log reports the number of attach attempts.

## 0.2.57experimental Edge-first runtime observer check

- Use a difficult/unsupported page that reaches runtime network discovery.
- Confirm the visible detector window is **Microsoft Edge** when Edge is installed.
- Confirm the detector uses a fresh temporary `runtime-browser-*` profile rather than the persistent `Tools\BrowserProfile` account profile.
- Start/play the main video if necessary and confirm the observer can attach and capture a plausible full-video or manifest response.
- Confirm detected media URLs remain hidden from the console/log and preview/segment rejection is unchanged.
- If Edge exposes no debugging endpoint or the WebSocket attach fails, confirm the failure is reported clearly rather than silently switching to Brave during the same attempt.
- Re-test one previously successful Brave-assisted recovery case so the Edge result can be compared directly.
- Confirm authenticated YouTube still uses the dedicated Edge account profile and the 0.2.55 mweb fallback / 0.2.56 yt-dlp blacklist behavior is unchanged.

## 0.2.56experimental yt-dlp stable blacklist check (historical; 2026.07.04 fallback superseded by 0.2.64)

- Build with no local `Tools\yt-dlp\yt-dlp.exe` and confirm the newest official stable release is selected unless its version is exactly `2026.08.19`.
- If `2026.08.19` is the newest stable release, confirm the bootstrap rejects it and installs `2026.07.04` instead.
- Seed the source Tools tree with a `2026.08.19` executable and confirm the BAT replaces it rather than retaining it.
- Seed the source Tools tree with another valid version (for example `2026.07.04`) and confirm it is retained.
- Click the GUI **Update** button while using a normal stable or nightly yt-dlp and confirm WinterStatic installs the newest official stable release excluding `2026.08.19`, rather than delegating to the executable's own `-U` channel.
- Re-run the authenticated age-restricted YouTube case and confirm the selected yt-dlp version is printed in the live PowerShell window.
- Re-test the 0.2.55 conditional mweb fallback; it must remain dormant when normal authenticated selection is already above 360p.

## 0.2.55experimental authenticated YouTube mweb fallback check

- Use current stable yt-dlp with the dedicated WinterStatic Edge profile logged in and closed.
- Test an authenticated/age-restricted YouTube video that otherwise selects only 360p with the current yt-dlp build.
- Confirm PowerShell first reports the normal authenticated preflight height.
- If the normal height is 360p or lower, confirm WinterStatic probes `youtube:player_client=mweb`.
- Confirm mweb is used only when its selected height is strictly better than the normal client's height.
- Confirm a working older yt-dlp such as 2026.07.04 does not force mweb when its normal authenticated client already exposes more than 360p.
- Confirm Anonymous mode, audio-only mode, and non-YouTube URLs do not run the mweb quality fallback.
- Confirm no cookies, Authorization values, or signed media URLs are printed by the preflight.
- Re-test the 0.2.54 Tools-first runtime bootstrap: portable yt-dlp and FFmpeg should still be preferred when present and downloaded by the BAT when missing.



## 0.2.54experimental portable runtime bootstrap check

- Build once with no local `Tools\yt-dlp\yt-dlp.exe` and confirm the official stable yt-dlp executable is downloaded into the generated portable `Tools\yt-dlp` folder.
- Build once with no local `Tools\FFmpeg\bin\ffmpeg.exe` and confirm the Gyan release essentials ZIP is downloaded/extracted into the generated portable `Tools\FFmpeg\bin` folder.
- Confirm existing local portable copies are retained rather than overwritten.
- Confirm runtime detection selects portable Tools copies first and falls back to configured/application/PATH copies only when the portable file is absent.
- Record the exact yt-dlp and FFmpeg versions/checksums before publishing a Full Portable binary asset.

This file is a release-engineering checklist, not legal advice.

Project repository: https://github.com/WinterStatic/yt-dlp-WinterStatic

## Repository / frontend

- Confirm `APP_VERSION`, `resource.rc`, `app.manifest`, `README.md`, package names,
  and `BUILD-INFO.txt` all match the intended release.
- Confirm the repository root contains:
  - `main.cpp`
  - `resource.rc`
  - `app.manifest`
  - `app.ico`
  - `app-arrow-transparent.png`
  - `build-native.bat`
  - `settings.ini`
  - `README.md`
  - `LICENSE`
  - `THIRD-PARTY-NOTICE.txt`
  - `PUBLIC-RELEASE-CHECKLIST.md`
  - `.gitignore`
- Confirm `Tools/`, generated portable folders, MSVC objects/resources, and local
  IDE files are not committed.
- Confirm there are no obsolete usernames / branding strings or development-only
  pre-release wording in public-facing files.
- Build from a clean source tree on Windows 10/11.
- Confirm the embedded icon, window title, and Windows file/product version
  metadata show the intended version.

## Functional smoke test

Before tagging a release:

- Paste and clear a URL.
- Browse and open the output directory.
- Test Best quality.
- Test Best MP4-compatible and verify the final container where appropriate.
- Test Maximum 1080p and Maximum 720p with FFmpeg available.
- Test a site whose usable video format has unknown height metadata (Facebook is a useful regression case).
- Confirm a `Requested format is not available` failure makes one best-available recovery attempt.
- Open and close the dedicated WinterStatic Edge profile and confirm its profile lock is released before authenticated use.
- Test Audio only with FFmpeg available.
- Test lazy-loaded `<video data-src>` / `<source data-src>` media.
- Test a relative direct-media link or reference and confirm it resolves against the page URL.
- Confirm a discovered iframe/nested-page candidate uses the page that exposed it as Referer.
- Confirm the bounded candidate probe rejects an obvious HTML error page without blocking inconclusive/auth-protected candidates from reaching yt-dlp.
- Test a page exposing both a hover/preview clip and the real player/full video; confirm candidate ranking prefers the real media path.
- Confirm a page whose only usable media is a small direct clip still succeeds rather than being permanently rejected by ranking heuristics.
- Test an unsupported page whose direct frontend fetch is blocked or barren and confirm the one-time yt-dlp page-capture fallback can still discover a static media/player reference.
- Test an intentionally unsupported public page that exposes a static HTML5/direct
  media reference and confirm WinterStatic performs one media-discovery retry.
- Confirm unsupported-page recovery preserves the original page as Referer and
  does not print the discovered signed/tokenised media URL into the GUI log.
- Confirm an unsupported page with no usable static media reference fails cleanly
  without entering a retry loop.
- Temporarily remove / hide FFmpeg and confirm the GUI reports fallback mode and
  a suitable single-file download still works where available.
- Open the dedicated Microsoft Edge account browser, sign in, close that dedicated window, and run
  **Check Login**.
- Test Automatic, Anonymous, and Use account browser modes.
- Confirm an authenticated download works with the dedicated Edge profile closed while an unrelated normal Edge window remains open.
- Confirm the visible PowerShell window opens and automatically snaps right.
- Confirm **Snap PowerShell right** works manually.
- Confirm closing the GUI does not terminate an active yt-dlp task.
- Test **Close PowerShell on success**: successful tasks close the terminal, failed tasks remain open, and the setting persists after restart.
- Confirm **Nuke task** terminates the currently tracked PowerShell / yt-dlp
  process tree without closing unrelated terminals.
- Confirm separate video/audio downloads report sequential dynamic passes (for
  example **Pass 1 — Downloading video**, **Pass 2 — Downloading audio**) and that
  merge / extraction advances to the next pass without displaying a fixed "of N"
  total.
- Confirm a single combined-file download does not invent extra passes.
- Confirm an ordinary working download still succeeds without entering a
  resilience-retry path.
- On a controlled failing/test URL, confirm HTTP rejection / anti-bot signals can
  request a browser-impersonation retry only when yt-dlp reports an available
  Chrome `curl_cffi` target; unsupported builds must skip that recovery cleanly.
- Confirm timeout/reset/throttling-style failures can trigger the one-time 5 MiB
  chunked-HTTP recovery without changing the normal download settings.
- Run the yt-dlp updater and both **Open Profile** controls.
- Restart the GUI and confirm `settings.ini` persistence.

## Source-only GitHub publication

The safest initial repository publication is the frontend source kit only.
`Tools/` is deliberately git-ignored, so yt-dlp / FFmpeg binaries and the
dedicated Edge profile are not part of the source commit.

Suggested first repository commit:

1. Source / resources / build scripts.
2. MIT `LICENSE`.
3. `README.md` with the consolidated 0.1 history.
4. `THIRD-PARTY-NOTICE.txt`.
5. This checklist.

## Full Portable binary release

A generated portable package can contain third-party programs. Do not assume a
local development `Tools/` directory is ready for redistribution.

For every binary included in a public Full Portable asset:

- Record its exact filename and version.
- Record where that binary was obtained.
- Keep the applicable copyright / license / notice material.
- Check whether the exact binary has source-offer / corresponding-source or other
  redistribution obligations.
- Keep the runtime notices with the same GitHub Release as the binary asset.

### yt-dlp

- Record the exact `yt-dlp --version` output.
- Record the upstream project / release source for the binary.
- Preserve the notices required by that exact distributed build.

### FFmpeg

- Record the exact FFmpeg binary source and `ffmpeg -version` output.
- Record the build configuration where available.
- Determine the license obligations of that exact build; FFmpeg's obligations can
  differ depending on the configure options and linked components.
- Do not publish an arbitrary local / Winget-linked FFmpeg binary in a portable
  archive without documenting where it came from and what it requires.

### Microsoft Edge

- Do not bundle Microsoft Edge in the portable package.
- Confirm the frontend detects the installed Edge executable on supported Windows 10/11 systems.
- Confirm source/release archives contain no populated `Tools\BrowserProfile` data. The builder may create an empty profile directory for layout purposes.
- Confirm no account cookies, `BrowserProfile_backup_*` directories, or other browser-profile files are accidentally included in a release asset.

## Suggested GitHub release assets

For the first public release, sensible assets are:

1. **Repository source** — GitHub's tag/source archive.
2. **Frontend/source kit ZIP** — optional convenience copy of the same public
   source files.
3. **Full Portable** — only after the bundled yt-dlp / FFmpeg binary provenance
   and redistribution requirements have been checked for that exact package. Edge
   remains an external Windows installation.

## Final release check

- Build once more from the exact tagged source.
- Run the smoke-test list above against that build.
- Check the generated `README.txt`, `LICENSE`, `THIRD-PARTY-NOTICE.txt`,
  `settings.ini`, and `BUILD-INFO.txt` are present beside the EXE.
- Confirm no personal browser profile, cookies, downloaded media, temp scripts,
  logs, or user-specific paths are present in the release archive.
- Confirm a completed test download does not leave unbounded `WinterStaticDL_*`
  task folders accumulating under `%TEMP%` across repeated launches.
- Confirm the GitHub Release notes and README version history agree with the tag.


## 0.2.50experimental simple authenticated-cookie check

- With the dedicated WinterStatic Edge profile logged into YouTube and closed, test the authenticated video that failed in 0.2.49.
- Confirm the command path uses `--cookies-from-browser edge:<Tools\BrowserProfile\Default>` and does **not** include `youtube:player_client=tv_downgraded`.
- With **Maximum 1080p** selected, confirm the emitted selector is `bv*[height<=?1080]+ba/b[height<=?1080]` (when FFmpeg is available) and that one final 1080p output is produced when the source exposes 1080p.
- Confirm **Best MP4-compatible** still retains its MP4-specific selector and behavior.
- Re-test the ordinary 1080p YouTube URL in Anonymous mode.
- Re-test the preview/image site and the pre-download 404 site to ensure the Brave-first runtime recovery remains unchanged.

## 0.2.49experimental authenticated YouTube TV-downgraded check (not retained)

- With the dedicated WinterStatic Edge profile logged into YouTube and closed, test an age/login-gated video that previously fell back to 360p.
- Historical test only: the forced TV-downgraded path produced `The page needs to be reloaded` on the current authenticated target and was removed in 0.2.50.
- With **Maximum 1080p** selected, confirm only one final output is produced and the selected video stream is 1080p when the source provides 1080p.
- Re-test an ordinary non-login YouTube URL in Anonymous mode and confirm the authenticated TV client is not forced there.
- Re-test one authenticated non-YouTube URL/path if available; the YouTube-scoped extractor arg must not alter other extractors.

## 0.2.48experimental dedicated Edge profile + runtime observer check

- Confirm signing into YouTube in the WinterStatic Edge profile does not sign the user's normal Edge profile into that account.
- Confirm `--cookies-from-browser edge:<Tools\BrowserProfile\Default>` succeeds after the dedicated Edge window is closed.
- Confirm the runtime observer prefers Brave, then Chrome, then Edge, uses a temporary detector profile, and still selects the real video rather than poster/preview media.
- Confirm the runtime observer no longer fails with a null `$TaskDir` / `Join-Path` error before browser launch.
- Confirm browser discovery tolerates an unavailable Program Files environment variable.
- Runtime network discovery is experimental and only runs after static recovery fails for an unsupported URL or a pre-download HTTP 404.
- It prefers Brave with a temporary detector profile, then Chrome, then Edge, and observes network events for up to 30 seconds.
- Confirm no candidate URLs are printed to the console/log before any public release.
- Confirm images, common segment files, and preview-like/tiny video responses are not selected as the main video.
- Confirm normal yt-dlp, dedicated Edge account mode, Vimeo, TV5MONDE, Al Jazeera, and Global News regression tests still pass.

- Confirm a yt-dlp HTTP 404 before any WINTERSTATIC_PROGRESS line enters the same static/captured-page/runtime recovery chain as Unsupported URL.
- Confirm a 404 after real download progress does not trigger runtime browser recovery.

# WinterStatic yt-dlp Downloader public release checklist

## 0.1.39 functional check

- Confirm the application title, About/version text, EXE metadata, manifest, build banner, BUILD-INFO, portable folder, and release ZIP names all report `0.1.39`.
- Confirm the **Quality** and **Format** controls share one row cleanly at the normal window size.
- Confirm Format offers **Automatic**, **Prefer MP4**, and **Prefer WebM**.
- Confirm each tab keeps its own Format choice when switching between tabs.
- Confirm a new tab inherits the selected tab's Format choice.
- Confirm Nuke keeps the tab's Format choice while clearing the download state.
- Upgrade from a settings file containing `Quality=Best MP4-compatible` and no `PreferredFormat` key. Confirm it opens as **Best quality + Prefer MP4**.
- Start at least two downloads at the same time in separate tabs and confirm each tab keeps its own URL, output choices, progress, GUI log, PowerShell process, and completion state.
- Switch repeatedly between active tabs and confirm one tab never shows another tab's progress or log.
- Confirm the `+` tab creates a new reusable download tab and the 12-tab limit still works.
- Confirm progress updates do not resize the job tabs or move the `+` tab left and right.
- Fill the tab strip until the overflow arrows appear. Confirm the tab strip and arrows stay dark and both arrows scroll correctly.
- Confirm every job tab has a working `x` close control.
- Close an idle tab and confirm only that tab is removed.
- Close a completed tab with its PowerShell window still open and confirm the tracked PowerShell process/window is stopped before the tab disappears.
- Close a running tab and confirm only that tab's PowerShell/yt-dlp process tree is stopped. Other downloads must continue.
- Close the last remaining job tab and confirm a fresh blank download tab is created automatically.
- Confirm **Nuke task** stops the selected tab's task and resets that tab to a clean ready state.
- After Nuke, confirm the URL, progress, stage detail, and GUI log are cleared while output folder, quality, format preference, authentication, Browser sweep, and Close PowerShell on success choices are retained.
- Confirm **Snap PowerShell right** still acts on the selected tab only.
- Confirm automatic PowerShell placement still waits for a stable terminal window before moving it.
- Close the main GUI while downloads are active and confirm the PowerShell/yt-dlp tasks continue running.

## Progress check

- Re-test the download that could begin with a premature `100.0%` report followed by a low live percentage. The GUI must ignore the premature 100% and start from the next real live value.
- Confirm live GUI progress is allowed to move backwards when yt-dlp revises its estimate. It must not freeze at an earlier high-water mark.
- Confirm a genuine finished progress event reaches 100%.
- Confirm a new media stream/pass can begin near 0% after the previous pass completes.
- Confirm the visible PowerShell output remains yt-dlp's raw progress output.

## Download and authentication regression check

- Test one ordinary public YouTube download.
- Test one authenticated or age-restricted YouTube download through the dedicated Edge profile.
- Confirm **Automatic** starts anonymously and only switches to the saved account session when YouTube explicitly requires authentication.
- Confirm **Anonymous** never supplies account-browser cookies.
- Confirm **Use account browser** uses the dedicated Edge session immediately.
- Keep a normal Edge window open and confirm it does not block WinterStatic account checks or authenticated downloads.
- Open the dedicated WinterStatic Edge profile and confirm cookie reads/reset/authenticated startup wait until that dedicated window is closed.
- Test one FFmpeg merge and one audio-only download.
- Test **Prefer MP4** with Best quality and a capped quality mode. Confirm MP4/M4A is preferred when available and normal fallback still works when it is not.
- Test **Prefer WebM** with Best quality and a capped quality mode. Confirm WebM is preferred when available and normal fallback still works when it is not.
- Test audio-only with Prefer MP4 and Prefer WebM. Confirm M4A/WebM are preferred respectively without turning the preference into a hard failure.
- Test one normal unsupported URL/static recovery case if a known target is available.
- Test Browser sweep with a known target and confirm it remains off by default on a clean settings file.
- Confirm Nuke, tab close, and another running download do not interfere with Browser sweep or authentication state belonging to other tabs.

## Package check

- Run `build-native.bat` and confirm the normal portable folder is created successfully.
- Run `build-native.bat release` and confirm it creates:

```text
WinterStatic-yt-dlp-Downloader-0.1.39-portable.zip
WinterStatic-yt-dlp-Downloader-0.1.39-source.zip
```

- Open the portable ZIP and confirm it contains the EXE, clean settings, README, license, notices, BUILD-INFO, both yt-dlp backends, FFmpeg, and only a clean BrowserProfile area.
- Open the source ZIP and confirm it contains only the public source/resources/build files listed by the release builder.
- Confirm the source ZIP contains no `Tools`, browser cookies/profile data, downloaded media, generated EXE/OBJ/RES files, IDE files, or portable output folder.
- Confirm `build-native.bat release` inside the source ZIP still supports release mode.
- Replace `yt-dlp_winterstatic.png` with a final 0.1.39 screenshot before publication and check it for personal paths, account names, notifications, or other private information.

## Runtime and licensing check

- Confirm the primary yt-dlp backend is the latest official stable release and is not a nightly build.
- Confirm the authenticated yt-dlp backend follows the stable-only blacklist policy documented in `THIRD-PARTY-NOTICE.txt`.
- Record the exact yt-dlp versions included in the portable release.
- Record the exact FFmpeg build/version included in the portable release and check the license/source obligations for that build.
- Confirm Microsoft Edge is not bundled.
- Review `LICENSE` and `THIRD-PARTY-NOTICE.txt` against the exact binaries being shipped.

## GitHub release check

- Confirm the README version history includes 0.1.39 and contains only the public 0.1.x version history.
- Confirm the Git tag, release title, README version, EXE version, and asset filenames all agree.
- Attach the portable ZIP and source ZIP generated by `build-native.bat release`.
- Test both uploaded ZIPs after download before marking the release complete.

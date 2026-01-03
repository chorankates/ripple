## basics

requires the [Pebble SDK](https://developer.rebble.io/sdk/).

which requires
  * [python3](https://you-dont-need-a-link)
  * [uv](https://docs.astral.sh/uv/getting-started/installation/)
  * [Node.js/npm](https://docs.npmjs.com/downloading-and-installing-node-js-and-npm)

have not been able to make this work on apple silicon (m2, arm64) or an older raspberry pi (3B v1.2, aarch64), only x86_64, but the problem doesn't appear to be related to the architecture:

```
$ uv tool install pebble-tool
Resolved 45 packages in 11.45s
  × Failed to build `stpyv8==13.1.201.22`
  ├─▶ The build backend returned an error
  ╰─▶ Call to `setuptools.build_meta:__legacy__.build_wheel` failed (exit status: 1)
...
          exec(code, locals())
        File "<string>", line 12, in <module>
      ModuleNotFoundError: No module named 'settings'
...
or `uv pip install settings` into the environment and re-run with `--no-build-isolation`.
  help: `stpyv8` (v13.1.201.22) was included because `pebble-tool` (v5.0.13) depends on `pypkjs` (v2.0.6)
        which depends on `stpyv8`
```

considered arch because of this [unanswered reddit post](https://www.reddit.com/r/pebble/comments/1ozqaec/sdk_installation_troubles_on_macos/), which describes the same problem

coming back to this, trying to build stpyv8 from source, notice [settings.py](https://github.com/cloudflare/stpyv8/blob/master/settings.py), which is almost certainly the 'settings' module that can't be found, so it's just another pathing problem. 
 


but if you can overcome that..

```bash
make build
```

```bash
make install-ip IP=<IP_ADDRESS>

```

```bash
make emulator # default platform is 'basalt', can override with PLATFORM=
```

## rebuilding screenshots

individually
```bash
make screenshot-mode # default mode is 'text', can override with MODE=
```

all of them on single platform
```bash
make screenshot-all-modes # default platform is 'basalt', can override with PLATFORM=
```

all of them all of them
```bash
make screenshot-matrix
```

### potential issues

  * `qemu` screenshotting integration doesn't have a lot of error checking - if running remotely, and stalling on the first shot "Installing app..", check your environment variables
  * `qemu` screenshotting steals focus, best to leave it alone during the ~5 minutes it takes to run `make screenshot-matrix`
  * don't forget to add your new mode to `DISPLAY_MODES` in [Makefile](Makefile)

## platforms

currently supporting all platforms, need to think more about the tradeoffs of only supporting newer physical devices



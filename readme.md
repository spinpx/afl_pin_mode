# afl_pin_mode

afl_pin_mode is a instrumentation tool for AFL. It is inspired by [aflpin](https://github.com/mothran/aflpin). However, aflpin exists some issues and is unsupported by the author now.

## Features
- Support forkserver
- Needn't modifying AFL code

## Install
- Just run `make`

## Usage
`afl-fuzz -m 500 -i .. -o .. -f .. -- /path/to/pin_run TARGETAPP @@` 

## Test
- Ubuntu 14.04
- Pin 2.14

## ISSUES
- Though I make the tool support forkserver feature, it is still slow.

## TODO
- Test Pin 3.4 and Ubuntu 16.04/17.04

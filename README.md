# Smart Card Raw Comms Lib
A very thin layer of abstraction on top of WinSCard on Windows and PCSC-lite on Linux and macOS that exposes a very simple interface for sending raw APDU messages to smart cards.

## Building
1. Make sure to clone recursively so all dependencies are checked out.
2. If on Linux, install `libpcsclite-dev` and `libpcsclite1`. Windows and macOS doesn't need anything extra.
3. Run `make` to build the scraw library.

# Smart Card Raw Comms Lib
A very thin layer of abstraction on top of WinSCard on Windows and PCSC-lite on Linux and macOS that exposes a very simple interface for sending raw TPDU messages to smart cards.

## Building
1. Make sure to clone recursively so all dependencies are checked out.
2. If on Linux, install `libpcsclite-dev` and `libpcsclite1`. Windows and macOS doesn't need anything extra.
3. Run `make` to build the scraw library.

Take a look at the example to see how to use the library in a project. It basically comes down to linking with the generated static library and a PC/SC library for the platform used (WinSCard on Windows, PCSC-lite on Linux/Mac).

The list of possible error codes that get saved to the context struct (as the 'error reason') can be found at:
- https://docs.microsoft.com/en-us/windows/win32/secauthn/authentication-return-values
- https://pcsclite.apdu.fr/api/group__ErrorCodes.html

## Running
1. On Linux, the PC/SC smart card daemon (`pcscd` package) needs to be present (and must be running). On Windows and macOS it will work out of the box.
2. Compile as described above (and link with the application).
3. Run the binary.

If the `scraw` library fails to initialize (on Linux), then `pcscd` is most likely not working correctly.

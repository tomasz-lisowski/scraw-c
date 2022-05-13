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

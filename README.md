# usock
Platform agnostic C/C++ socket API.

Currently supported platforms:

| Platform | Status |
|----------|--------|
| Linux    | Supported |
| Winows   | Supported |
| Mac OS   | Work in progress |

## How to build
use ```make``` with one of the following arguments:

| Argument | Description |
|----------|-------------|
| | Build the usock integration test tool. |
| usock.a | Build an archive of the full usock library, including all utilities. | 
| usock-lite.a | Build an archive of only the core usock library. | 
| usock-lite.so | Build a shared library of the core usock library. |
| clean | Cleanup build artefacts. |

## How to test
The unit tests can be launched using the included integration test tool, once it's been built using ``` make ```.
The tool will automatically build the unit tests it needs to run, so there's no need to manually build any of them.
However, ```make``` needs to be installed and added to the system PATH for this to work.

To use the tool, ```cd``` to the root directory of the repository and run "```./usock-test arg```" to launch the specified test, where ```arg``` can be one of the following:

| arg | Description |
|-----|-------------|
| help | Display the help text. |
| all | Run all the tests. |
| tcp-server-client | Test the TCP server/client unit test. |
| udp-server-client | Test the UDP server/client unit test. |

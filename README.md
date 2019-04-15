# usock
Platform agnostic networking API

## How to build
use ```make``` with one of the following arguments:

| Argument | Description |
|----------|-------------|
| | Build the usock integration test tool. |
| build/usock.a | Build an archive of the full usock library, including all utilities. | 
| build/usock-lite.a | Build an archive of only the base usock library. | 
| OPTIONAL |
| build/TestClient | Build the client program. |
| build/TestServer | Build the server program. |

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

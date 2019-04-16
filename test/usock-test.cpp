/******************************************************************************
Copyright (c) 2019 Sagnik Chowdhury

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/

#include <stdio.h>
#include <vector>
#include <string>
#include <string.h>
#include <unordered_map>
#include <thread>

#define BUILDDIR "build"

//Test names
#define TCP_SERVER_CLIENT "tcp-server-client"
#define UDP_SERVER_CLIENT "udp-server-client"

//Target names
#define TCPCLIENT "TCPClient"
#define TCPSERVER "TCPServer"
#define UDPCLIENT "UDPClient"
#define UDPSERVER "UDPServer"

struct Test
{
	std::vector<const char *> targets;
	const char *description;

	enum Result
	{
		PASS = 0,
		FAIL_BUILD,
		FAIL_TEST,
	};
};

Test::Result RunTest(const std::string &name, const Test &test);
void PrintDivider();

int main(int argc, const char *argv[])
{
	printf("USOCK TEST SUITE\n");
	printf("================\n");

	//Initialize target rules
	static const std::unordered_map<std::string, Test> tests = {
		{ TCP_SERVER_CLIENT, Test({
			{ BUILDDIR "/" TCPSERVER, BUILDDIR "/" TCPCLIENT },
			"Run the TCP server/client test."})
		},
		{ UDP_SERVER_CLIENT, Test({
			{ BUILDDIR "/" UDPSERVER, BUILDDIR "/" UDPCLIENT },
			"Run the UDP server/client test."}) 
		}
	};

	if(argc <= 1 || strcmp(argv[1], "help") == 0)
	{
		//Display usage
		printf("Usage: usock-test arg\n");
		printf("accepted args:\n");
		printf("%-20s | Description\n", "Arg");
		printf("%.20s-------------------------\n", "------------------------------------------------------");
		printf("%-20s | Display this help text\n", "help");
		printf("%-20s | Run all tests.\n", "all");
		for(const auto &test : tests)
		{
			printf("%-20s | %s\n", test.first.c_str(), test.second.description);
		}
		printf("\n");
		printf("Note: This tool will automatically build the targets required for the test.\n");

		return 0;
	}

	if(strcmp(argv[1], "all") == 0)
	{
		//Loop over all tests
		bool fail = false;
		int i = 0;
		for(const auto &test: tests)
		{
			++i;
			printf("%4d. %s: \n", i, test.first.c_str());
			PrintDivider();
			int res = RunTest(test.first, test.second);
			printf("\t%s\n", res == Test::PASS ? "PASSED" : "FAILED");
			PrintDivider();
			if(res != Test::PASS)
			{
				fail = true;
			}
		}
		return fail ? 1 : 0;
	}
	else
	{
		const auto &test = tests.find(argv[1]);
		if(test == tests.end())
		{
			printf("Invalid test name.\n");
			printf("The following are valid tests:\n");
			for(const auto &t : tests)
			{
				printf("\t%s\n", t.first.c_str());
			}
			return 2;
		}

		//Run the test
		printf("%4d. %s: \n", 1, argv[1]);
		PrintDivider();
		int res = RunTest(test->first, test->second);
		printf("\t%s\n", res == Test::PASS ? "PASSED" : "FAILED");
		PrintDivider();
		return res == Test::PASS ? 0 : 1;
	}

	return 0;
}

int BuildTestTargets(const Test &test);
void RunTestTargets(const Test &test, std::vector<int> &results);

Test::Result RunTest(const std::string &name, const Test &test)
{
	//Build the required targets
	if(BuildTestTargets(test) > 0)
	{
		printf("Build failed. Aborting test.\n");
		return Test::FAIL_BUILD;
	}

	//Launch built targets
	std::vector<int> results;
	RunTestTargets(test, results);

	//Stall for a bit to allow the sockets to reset.
	std::this_thread::sleep_for(std::chrono::seconds(1));

	//Check return code of each target
	//All must be 0 for the test to pass.
	for(int ret : results)
	{
		if(ret != 0)
		{
			return Test::FAIL_TEST;
		}
	}
	return Test::PASS;
}

constexpr size_t cmdlen = 256;

int BuildTestTargets(const Test &test)
{
	char cmd[cmdlen] = {0};
	//Run the required make commands
	for(const auto &target : test.targets)
	{
		snprintf(cmd, cmdlen, "make -s %s", target);
		int buildRes = system(cmd);
		if(buildRes > 0)
			return Test::FAIL_BUILD;
	}
	return Test::PASS;
}

void CmdTest(const char *cmd, int *result);

void RunTestTargets(const Test &test, std::vector<int> &results)
{
	//Launch each target in its own thread
	std::vector<std::thread> processThreads(test.targets.size());
	results.resize(test.targets.size());
	for(size_t i = 0; i < results.size(); ++i)
	{
		const char *target = test.targets[i];
		char cmd[cmdlen] = {0};
		snprintf(cmd, cmdlen, "./%s", target);
		processThreads[i] = std::thread(CmdTest, cmd, &results[i]);
		//Sleep for a short amount between launching tests, just in case.
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	//Wait for test to finish
	for(std::thread &t : processThreads)
	{
		if(t.joinable())
			t.join();
	}
}

void CmdTest(const char *cmd, int *result)
{
	*result = system(cmd);
}

void PrintDivider()
{
	printf("--------------------------------\n");
}
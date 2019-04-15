#include <stdio.h>
#include <vector>
#include <string>
#include <string.h>
#include <unordered_map>
#include <thread>

#define BUILDDIR "build"

//Test names
#define TCP_SERVER_CLIENT "tcp-server-client"

//Target names
#define TESTCLIENT "TestClient"
#define TESTSERVER "TestServer"

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

int main(int argc, const char *argv[])
{
	printf("USOCK TEST SUITE\n");
	printf("================\n");

	//Initialize target rules
	static const std::unordered_map<std::string, Test> tests = {
		{ TCP_SERVER_CLIENT, Test({
			{ BUILDDIR "/" TESTSERVER, BUILDDIR "/" TESTCLIENT },
			"Run the TCP server/client test."})
		}
	};

	if(argc <= 1 || strcmp(argv[1], "help") == 0)
	{
		//Display usage
		printf("Usage: usock-test arg\n");
		printf("accepted args:\n");
		printf("help - Display this help text\n");
		printf("all - Run all tests.\n");
		for(const auto &test : tests)
		{
			printf("%s - %s\n", test.first.c_str(), test.second.description);
		}
		printf("\n");
		printf("Note: This tool will automatically build the targets required for the test.\n");

		return 0;
	}

	if(strcmp(argv[1], "all") == 0)
	{
		//Loop over all tests
		bool fail = false;
		for(const auto &test: tests)
		{
			int res = RunTest(test.first, test.second);
			if(res != Test::PASS)
			{
				printf("Test %s FAILED.\n", test.first.c_str());
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
		int res = RunTest(test->first, test->second);
		return res == Test::PASS ? 0 : 1;
	}

	return 0;
}

int BuildTestTargets(const Test &test);
void RunTestTargets(const Test &test, std::vector<int> &results);

Test::Result RunTest(const std::string &name, const Test &test)
{
	printf("Running test %s\n", name.c_str());
	//Build the required targets
	if(BuildTestTargets(test) > 0)
	{
		printf("Build failed. Aborting test.\n");
		return Test::FAIL_BUILD;
	}

	//Launch built targets
	std::vector<int> results;
	RunTestTargets(test, results);

	//Check return code of each target
	//All must be 0 for the test to pass.
	for(int ret : results)
	{
		if(ret != 0)
		{
			printf("Test failed!\n");
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
		snprintf(cmd, cmdlen, "make %s", target);
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

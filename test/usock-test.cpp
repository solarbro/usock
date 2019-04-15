#include <stdio.h>
#include <vector>
#include <string>
#include <string.h>
#include <unordered_map>

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
		for(const auto &test: tests)
		{
			RunTest(test.first, test.second);
		}
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
			return 0;
		}

		//Run the test
		RunTest(test->first, test->second);
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
			return Test::FAIL_TEST;
		}
	}
	return Test::PASS;
}

int BuildTestTargets(const Test &test)
{
	//Run the required make commands
	return 0;
}

void RunTestTargets(const Test &test, std::vector<int> &results)
{

}
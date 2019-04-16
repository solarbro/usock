#No ldflags yet
INC = -Iinclude
CFLAGS = $(INC) -Wall -Werror
CXXFLAGS = $(INC) -Wall -Werror

TC = g++

csrc = $(wildcard src/*.c)
ccsrc = $(wildcard src/*.cc)
obj = $(csrc:.c=.o) $(ccsrc:.cc=.o)

builddir = build

#build the test launcher tool
usock-test: test/usock-test.o
	$(TC) -o $@ $^ $(CCFLAGS) -lpthread

#build the full usock library into an archive
$(builddir)/usock.a: $(obj)
	mkdir -p $(builddir)
	ar r $@ $^ 

#build only the base usock library with none of the utils
$(builddir)/usock-lite.a: $(obj)
	mkdir -p $(builddir)
	ar r $@ src/usock.o

#build the specified test
testobj = $(wildcard test/*.o)

$(builddir)/TCPClient: $(obj) test/TCPClient.o
	mkdir -p $(builddir)
	$(TC) -o $@ $^ $(CFLAGS)

$(builddir)/TCPServer: $(obj) test/TCPServer.o
	mkdir -p $(builddir)
	$(TC) -o $@ $^ $(CFLAGS)

#clean up build artefacts
.PHONY: clean
clean:
	rm -f $(obj) $(testobj)
	rm -f -r $(builddir)
	rm -f usock-test

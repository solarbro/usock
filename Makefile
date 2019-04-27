#No ldflags yet
INC = -Iinclude
CFLAGS = $(INC) -Wall -Werror
CXXFLAGS = $(INC) -Wall -Werror

TC = g++

csrc = $(wildcard src/*.c)
ccsrc = $(wildcard src/*.cc)
obj = $(csrc:.c=.o) $(ccsrc:.cc=.o)

builddir = build

#build the automated test tool
usock-test: test/usock-test.o
	$(TC) -o $@ $^ $(CXXFLAGS) -lpthread

#build the full usock library into an archive
usock.a: $(obj)
	ar r $@ $^ 

#build only the core usock library with none of the utils
usock-lite.a: $(obj)
	ar r $@ src/usock.o

#build the core library as a shared object
usock-lite.so: $(csrc)
	$(CC) -o $@ $^ $(CFLAGS) -shared -fPIC

#build the specified test
#These commands don't need to be used manually.
#They're only used by the automated test tool.
testobj = $(wildcard test/*.o)

$(builddir)/TCPClient: $(obj) test/TCPClient.o
	mkdir -p $(builddir)
	$(TC) -o $@ $^ $(CFLAGS)

$(builddir)/TCPServer: $(obj) test/TCPServer.o
	mkdir -p $(builddir)
	$(TC) -o $@ $^ $(CFLAGS)

$(builddir)/UDPServer: $(obj) test/UDPServer.o
	mkdir -p $(builddir)
	$(TC) -o $@ $^ $(CFLAGS)

$(builddir)/UDPClient: $(obj) test/UDPClient.o
	mkdir -p $(builddir)
	$(TC) -o $@ $^ $(CFLAGS)

#clean up build artefacts
.PHONY: clean
clean:
	rm -f $(obj) $(testobj)
	rm -f -r $(builddir)
	rm -f usock-test
	rm -f usock.a
	rm -f usock-lite.a
	rm -f usock-lite.so

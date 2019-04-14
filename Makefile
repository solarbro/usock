#No ldflags yet
INC = -Iinclude
CFLAGS = $(INC) -Wall -Werror
CXXFLAGS = $(INC) -Wall -Werror

csrc = $(wildcard src/*.c)
ccsrc = $(wildcard src/*.cc)
obj = $(csrc:.c=.o) $(ccsrc:.cc=.o)

builddir = build

#build the full usock library into an archive
$(builddir)/usock.a: $(obj)
	mkdir -p build
	ar r $@ $^ 

#build only the base usock library with none of the utils
$(builddir)/usock-lite.a: $(obj)
	mkdir -p build
	ar r $@ src/usock.o

#build the specified test
testobj = $(wildcard test/*.o)

$(builddir)/TestClient: $(obj) test/TestClient.o
	mkdir -p build
	$(CC) -o $@ $^ $(CFLAGS)

$(builddir)/TestServer: $(obj) test/TestServer.o
	mkdir -p build
	$(CC) -o $@ $^ $(CFLAGS)

#clean up build artefacts
.PHONY: clean
clean:
	rm -f $(obj) $(testobj)
	rm -f -r $(builddir)

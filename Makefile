PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

OBJS = wcwidth.o utf8util.o utf8string.o utf8test.o

ifeq ($(BUILD_MODE),debug)
	CFLAGS += -Wall -g -O0
else ifeq ($(BUILD_MODE),build)
	CFLAGS += -Wall -O2
else ifeq ($(BUILD_MODE),run)
	CFLAGS += -Wall -O2
else ifeq ($(BUILD_MODE),linuxtools)
	CFLAGS += -g -pg -fprofile-arcs -ftest-coverage
	LDFLAGS += -pg -fprofile-arcs -ftest-coverage
	EXTRA_CLEAN += utf8string.gcda utf8string.gcno $(PROJECT_ROOT)gmon.out
	EXTRA_CMDS = rm -rf utf8string.gcda
else
    $(error Build mode $(BUILD_MODE) not supported by this Makefile)
endif
CFLAGS += -I$(PROJECT_ROOT)src

all:	utf8test run

run:
	./utf8test

build:	utf8test

utf8test:	$(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^
	$(EXTRA_CMDS)

%.o:	$(PROJECT_ROOT)src/%.cpp
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

%.o:	$(PROJECT_ROOT)test/%.cpp
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

clean:
	rm -fr utf8test $(OBJS) $(EXTRA_CLEAN)

OBJS_DIR = .objs

EXE_CLIENT = client
EXE_SERVER = server
EXES_RELEASE = $(EXE_CLIENT) $(EXE_SERVER)

OBJS_CLIENT=$(EXE_CLIENT).o
OBJS_SERVER=$(EXE_SERVER).o

CC = clang++
INCLUDES = -I./includes
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter
CFLAGS_DEBUG   = -O0 $(WARNINGS) $(INCLUDES) -g -std=c++11 -c -MMD -MP -D_GNU_SOURCE -pthread
CFLAGS_RELEASE = -O2 $(WARNINGS) $(INCLUDES) -g -std=c++11 -c -MMD -MP -D_GNU_SOURCE -pthread

LD = clang++
LDFLAGS = -pthread

.PHONY: all
all: release

.PHONY: debug
.PHONY: release

release: $(EXES_RELEASE)
debug:   clean $(EXES_RELEASE:%=%-debug)

-include $(OBJS_DIR)/*.d

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

$(OBJS_DIR)/%-debug.o: %.cpp | $(OBJS_DIR)
	$(CC) $(CFLAGS_DEBUG) $< -o $@

$(OBJS_DIR)/%-release.o: %.cpp | $(OBJS_DIR)
	$(CC) $(CFLAGS_RELEASE) $< -o $@

$(EXE_CLIENT): $(OBJS_CLIENT:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_CLIENT)-debug: $(OBJS_CLIENT:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_SERVER): $(OBJS_SERVER:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_SERVER)-debug: $(OBJS_SERVER:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

.PHONY: clean
clean:
	-rm -rf .objs $(EXES_RELEASE) $(EXES_RELEASE:%=%-debug)

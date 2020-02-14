SHELL := /bin/bash

# ==================================================
# COMMANDS
# ARMCC = arm-linux-gnueabihf-gcc -O3
CC = gcc -O3
RM = rm -f
MKDIR = mkdir -p
RMDIR = $(RM) -r

# ==================================================
# SET DEVICE_ID
ifndef DEVICE_ID
	DEVICE_ID = 8448
endif

# ==================================================
# FLAGS
LDFLAGS = -lpthread
CFLAGS = -D MY_ID=$(DEVICE_ID)

# ==================================================
# DIRECTORIES
SRC = src
BIN = bin

# ==================================================
# TARGETS
EXEC = $(BIN)/main_$(DEVICE_ID)

# ==================================================
# COMPILATION
all: $(BIN) $(EXEC)

$(BIN):
	$(MKDIR) $@

$(EXEC): $(wildcard $(SRC)/*.c)
	@echo "[*] DEVICE_ID is set to $(DEVICE_ID)"
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	$(RM) $(EXEC) $(SRC)/*~ *~

purge: clean
	$(RMDIR) $(BIN)

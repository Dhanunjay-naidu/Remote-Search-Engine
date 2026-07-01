##############################################################################
# Makefile — Remote Search Engine (RSE)
# Targets: server, client, all, clean
#
# Usage:
#   make           → builds both server and client
#   make server    → builds only the server binary
#   make client    → builds only the client binary
#   make clean     → removes all compiled binaries and object files
##############################################################################

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -g
INCLUDES := -I include

# ─── Directories ──────────────────────────────────────────────────────────────
SRC_DIR := src
OBJ_DIR := obj

# ─── Source / Object groups ───────────────────────────────────────────────────
# Server: main server + all three feature modules
SERVER_SRCS := $(SRC_DIR)/server.cpp             \
               $(SRC_DIR)/sfsrchf.cpp    \
               $(SRC_DIR)/sfsrchs.cpp  \
               $(SRC_DIR)/sfdisp.cpp \
               $(SRC_DIR)/sffind.cpp

SERVER_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SERVER_SRCS))

# Client: standalone binary
CLIENT_SRCS := $(SRC_DIR)/client.cpp
CLIENT_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CLIENT_SRCS))

# ─── Targets ──────────────────────────────────────────────────────────────────
.PHONY: all clean

all: server client
	@echo "✅  Build complete. Run ./server then ./client"

server: $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "✅  server binary ready"

client: $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "✅  client binary ready"

# ─── Pattern rule: compile each .cpp → .o ────────────────────────────────────
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# ─── Create obj/ directory if it doesn't exist ───────────────────────────────
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# ─── Clean ────────────────────────────────────────────────────────────────────
clean:
	rm -rf $(OBJ_DIR) server client
	@echo "🧹  Cleaned build artifacts"

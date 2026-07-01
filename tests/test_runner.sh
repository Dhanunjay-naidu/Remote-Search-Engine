#!/bin/bash
##############################################################################
# tests/test_runner.sh — RSE Smoke Test Script
#
# Usage: bash tests/test_runner.sh
# Run this after Phase 7 when both server and client are fully implemented.
#
# Currently prints placeholders so the script structure is visible.
##############################################################################

set -e  # Exit immediately on any error

PASS=0
FAIL=0
SERVER_PID=""

# ─── Colours ──────────────────────────────────────────────────────────────────
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'  # No Colour

ok()   { echo -e "${GREEN}[PASS]${NC} $1"; ((PASS++)) || true; }
fail() { echo -e "${RED}[FAIL]${NC} $1"; ((FAIL++)) || true; }

# ─── Start server in background ───────────────────────────────────────────────
start_server() {
    pkill -f "./server" || true
    ./server &
    SERVER_PID=$!
    sleep 1  # Give server time to bind and listen
    echo "[TEST] Server started (PID $SERVER_PID)"
}

# ─── Stop server ──────────────────────────────────────────────────────────────
stop_server() {
    if [ -n "$SERVER_PID" ]; then
        kill "$SERVER_PID" 2>/dev/null || true
        echo "[TEST] Server stopped"
    fi
    pkill -f "./server" || true
}

trap stop_server EXIT

# ─── Run client with piped menu input, capture output ─────────────────────────
run_client() {
    # $1 = piped input string (menu choices + args)
    echo -e "$1" | ./client 2>&1
}

# ─── Tests ────────────────────────────────────────────────────────────────────
echo "========================================"
echo " RSE Smoke Tests"
echo "========================================"

start_server

# Test 1: Search for a file that exists (by absolute path)
echo -e "\n--- Test 1: Search for existing file ---"
OUT=$(run_client "1\n1\n/etc/passwd\n4")
if echo "$OUT" | grep -qi "found"; then ok "File found test"; else fail "File found test"; fi

# Test 2: Search for a string
echo -e "\n--- Test 2: Search for string 'root' in /etc ---"
OUT=$(run_client "2\n/etc\nroot\n0\n4")
if echo "$OUT" | grep -qi "root\|match\|found"; then ok "String search test"; else fail "String search test"; fi

# Test 3: Display file content
echo -e "\n--- Test 3: Display /etc/passwd ---"
OUT=$(run_client "3\n/etc/passwd\n4")
if [ -n "$OUT" ]; then ok "Display content test"; else fail "Display content test"; fi

# Test 4: File that does not exist — expect error message
echo -e "\n--- Test 4: File not found error ---"
OUT=$(run_client "1\n1\n/nonexistent/fake_file.txt\n4")
if echo "$OUT" | grep -qi "not found\|error"; then ok "Error message test"; else fail "Error message test"; fi

# Test 5: Invalid menu choice — client should re-prompt, not crash
echo -e "\n--- Test 5: Invalid menu choice ---"
OUT=$(run_client "9\n4")
if echo "$OUT" | grep -qi "invalid\|try again\|menu"; then ok "Invalid input test"; else fail "Invalid input test"; fi

# Test 6: Search for a string with no matches
echo -e "\n--- Test 6: String search (no matches) ---"
OUT=$(run_client "2\n/etc\nASDFQWERTY12345XYZ\n4")
if echo "$OUT" | grep -qi "No matches\|Error"; then ok "String search (no matches)"; else fail "String search (no matches)"; fi

# Test 7: String search with default base path (./)
echo -e "\n--- Test 7: Default base path string search ---"
OUT=$(run_client "2\n\nmain\n0\n4")
if echo "$OUT" | grep -qi "server.cpp\|client.cpp\|match\|found\|Search Results"; then ok "Default path string search test"; else fail "Default path string search test"; fi

# Test 8: Display non-existent file error
echo -e "\n--- Test 8: Display non-existent file ---"
OUT=$(run_client "3\n/invalid/path/file.txt\n4")
if echo "$OUT" | grep -qi "Cannot open file\|Error"; then ok "Display non-existent file error"; else fail "Display non-existent file error"; fi

# Test 9: Search string in invalid directory path
echo -e "\n--- Test 9: Invalid directory string search ---"
OUT=$(run_client "2\n/invalid/dir/path\nroot\n4")
if echo "$OUT" | grep -qi "Cannot open directory\|No matches"; then ok "Invalid directory string search"; else fail "Invalid directory string search"; fi

# Test 10: File search for a directory path
echo -e "\n--- Test 10: File search (directory target) ---"
OUT=$(run_client "1\n1\n/etc\n4")
if echo "$OUT" | grep -qi "not a regular file\|Error\|match\|Cannot open"; then ok "File search (directory target)"; else fail "File search (directory target)"; fi

# Test 11: File search for an empty path
echo -e "\n--- Test 11: File search (empty path) ---"
OUT=$(run_client "1\n1\n\n4")
if echo "$OUT" | grep -qi "File not found\|Error\|Cannot open"; then ok "File search (empty path)"; else fail "File search (empty path)"; fi

# Test 12: String search where base path is a regular file
echo -e "\n--- Test 12: String search (file base path) ---"
OUT=$(run_client "2\n/etc/passwd\nroot\n4")
if echo "$OUT" | grep -qi "Cannot open directory\|No matches\|Error"; then ok "String search (file base path)"; else fail "String search (file base path)"; fi

# Test 13: Display file without read permissions (requires a protected file)
echo -e "\n--- Test 13: Display restricted file ---"
OUT=$(run_client "3\n/etc/shadow\n4")
if echo "$OUT" | grep -qi "Cannot open file\|Error"; then ok "Display restricted file"; else fail "Display restricted file"; fi

# Test 14: Client menu choice is a non-numeric string
echo -e "\n--- Test 14: Non-numeric menu choice ---"
OUT=$(run_client "invalid_choice\n4")
if echo "$OUT" | grep -qi "Invalid choice\|try again\|Exit"; then ok "Non-numeric menu choice"; else fail "Non-numeric menu choice"; fi

# Test 15: Empty client menu choice boundary/eof
echo -e "\n--- Test 15: EOF / Graceful generic string menu fallback ---"
OUT=$(run_client "\n4")
if echo "$OUT" | grep -qi "Invalid choice\|try again\|Exit"; then ok "Graceful generic string fallback"; else fail "Graceful generic string fallback"; fi

stop_server

# ─── Summary ──────────────────────────────────────────────────────────────────
echo ""
echo "========================================"
echo " Results: ${PASS} passed | ${FAIL} failed"
echo "========================================"
[ "$FAIL" -eq 0 ] && exit 0 || exit 1

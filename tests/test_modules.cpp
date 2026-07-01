#include <cstdio>
#include <cstring>
#include "common.h"
#include "logger.h"
#include "srvfuncs.h"

int main() {
    char result[BUFFER_SIZE];

    printf("\n=== Test 1: searchForFile (exists) ===\n");
    rse_srv_searchForFile("/etc/hosts", result);
    printf("Result: %s\n", result);

    printf("\n=== Test 2: searchForFile (does not exist) ===\n");
    rse_srv_searchForFile("/nonexistent/fake.txt", result);
    printf("Result: %s\n", result);

    printf("\n=== Test 3: displayFileContent ===\n");
    rse_srv_displayFileContent("/etc/hosts", result);
    printf("Result:\n%s\n", result);

    printf("\n=== Test 4: displayFileContent (bad path) ===\n");
    rse_srv_displayFileContent("/fake/path.txt", result);
    printf("Result: %s\n", result);

    printf("\n=== Test 5: searchInDirectory for 'localhost' in /etc ===\n");
    result[0] = '\0';
    rse_srv_searchInDirectory("/etc", "localhost", result);
    printf("Result:\n%s\n", strlen(result) ? result : "(no matches)\n");

    return 0;
}

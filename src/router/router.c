#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "router.h"

#pragma comment(lib, "ws2_32.lib")

#define MAX_ROUTES 50
static route_t routes[MAX_ROUTES];
static int route_count = 0;

void init_router() {
    route_count = 0;
    printf("[INFO] Router initialized\n");
}

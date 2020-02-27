
#include <stdio.h>
#include <stdlib.h>

#include "../../../ESP/src/menu_head.h"

int main(int argc, char** argv) {
    for (int i = 0 ; i < sizeof(MENU_HEAD) ; i++) {
        if (i % 40 == 0) {
            fprintf(stdout, "\n");
        }
        fprintf(stdout, "%c", MENU_HEAD[i]);
    }
    return 0;
}

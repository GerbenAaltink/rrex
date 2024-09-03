#define RREX3_DEBUG 0
#include "rrex3.h"
#include "../rlib/rlib.h"

rrex3_t *rrex;
char *expr;

void file_found(char *path) {
    if (rfile_size(path) < 1024) {

        if (risdir(path))
            return;
        if (!rstrendswith(path, ".c") && !rstrendswith(path, ".h")) {
            return;
        }
        if (rstrendswith(path, "rlib.h")) {
            return;
        }
        size_t size = rfile_size(path);
        char file_data[size + 1 + 1024 * 1024];
        rfile_readb(path, file_data, size);
        file_data[size] = 0;
        rrex->str = file_data;
        char * ptr = file_data;
        while (rrex3(rrex,ptr, expr)) {
            ptr = rrex->str;
            printf("%s\n", rrex->str);
            printf(">%s<\n", rrex->matches[0]);
            printf("J,");
            break;
            // printf("%s\n",rrex->matches[0]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    rrex = rrex3_new();
    if (argc != 3) {
        printf("Usage: <path> <expr>\n");
        return 1;
    }

    char fixed[strlen(argv[2]) + 20];
    fixed[0] = 0;
    strcpy(fixed, "int (.*)[; ]?");
    //sprintf(fixed, "%s", argv[2]);
    expr = fixed;;
    rforfile(argv[1], file_found);

    return 0;
}
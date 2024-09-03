#define RREX3_DEBUG 1
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
        printf("!%ld\n", size);
        file_data[size] = 0;
        // printf("%s\n",file_data);
        /* strcpy(file_data,
           "ponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponypony"
                         "ponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponypony"
                         "ponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponypony"
                         "ponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponypony"
                         "ponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponypony"
                         "ponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponypony"
                         "ponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponypony"
                         "ponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponypony"
                         "ponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponypony"
                         "ponyponyponyponyponyponyponyponyponyponyponyponyponyintponyponyponyponypony"
                         "ponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponypony"
                         "ponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponyponypony"
             "\n\n\n\n\n\n\n\n\nint\n\\n\n\n\n");*/
        strcpy(file_data ,  "int "
                         "abc " 
                         "#ifndef RMALLOC_H\n"

                          /*"#define RMALLOC_H\n"

                          "#include <stdio.h>\n"
                          
                          "#include <stdlib.h>\n"
                          "#include <string.h>\n"
                          "\n"
                          "unsigned long long rmalloc_count = 0;\n"
                          "unsigned long long rmalloc_alloc_count = 0;\n"
                          "unsigned long long int rmalloc_free_count = 0;\n"
                          "\n"
                          "void *rmalloc(size_t size) {\n"
                          "    rmalloc_count++;\n"
                          "    rmalloc_alloc_count++;\n"
                          "    return malloc(size);\n"
                          "}\n"
                          "void *rrealloc(void *obj, size_t size) {\n"
                          "    if (obj == NULL) {\n"
                          "        rmalloc_count++;\n"
                          "        rmalloc_alloc_count++;\n"
                          "    }\n"
                          "    return realloc(obj, size);\n"
                          "}\n"
                          "void *rfree(void *obj) {\n"
                          "    rmalloc_count--;\n"
                          "    rmalloc_free_count++;\n"
                          "    free(obj);\n"
                          "    return NULL;\n"
                          "}\n"
                          "\n"
                          "#define malloc rmalloc\n"
                          "#define realloc rrealloc\n"
                          "#define free rfree\n"
                          "\n"
                          "char *rmalloc_stats() {\n"
                          "    static char res[100] = {0};\n"
                          "    sprintf(res, \"Memory usage: 110 allocated, 0 "
                          "freed, 973 in use.\",\n"
                          "            rmalloc_alloc_count, "
                          "rmalloc_free_count, rmalloc_count);\n"
                          "    return res;\n"
                          "}\n"
                          "\n"
                          "char *rstrdup(char *str) {\n"
                          "\n"
                          "    char *res = (char *)strdup(str);\n"
                          "    rmalloc_alloc_count++;\n"
                          "    rmalloc_count++;\n"
                          "    return res;\n"
                          "}\n"
                          "\n"
                          "#endif\n"*/);
        //if (strstr(file_data, "int")) {
        //    printf("JA HOOR: %s\n", path);
            // char file_string_data[strlen(file_data) * 2];
            // rstrtocstring(file_data, file_string_data);
            // printf(file_string_data);
        //}
        rrex->str = file_data;
        while (rrex3(rrex, rrex->str, expr)) {
            printf(">%s<\n", rrex->matches[0]);
            printf("J,");
            break;
            // printf("%s\n",rrex->matches[0]);
        }
        exit(0);
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
    sprintf(fixed, "%s", argv[2]);
    expr = fixed;
    printf("HIERRR %s\n", argv[1]);
    rforfile(argv[1], file_found);

    return 0;
}
#define RREX3_DEBUG 0

#include "../rlib/rlib.h"
#include "rrex3.h"
#include <regex.h>



void benchmark(int times, char * str, char * expr){
     printf("Matching \"%s\" with \"%s\".\n",str,expr);
     regex_t regex;
        if(regcomp(&regex,expr,REG_EXTENDED)){
            printf("Creg: error in regular expression.\n");
            exit(1);
        }
    printf("creg: ");
    RBENCH(times,{
      
        if(regexec(&regex,str,0,NULL,0)){
            printf("Creg: error executing regular expression.\n");
        }
        
    })
    regfree(&regex); ;
    rrex3_t * rrex = rrex3_compile(NULL,expr);
    printf("rrex3 (%s): ",rrex->compiled);
    RBENCH(times, {
        rrex3(rrex, str,expr);
        if(rrex){
        
        }else{
            printf("Rrex3: error\n");
        }
    });
        rrex3_free(rrex);
    printf("\n");
}

int main() {
    rrex3_test();
    //int times = 5000000;
    int times = 100;
    benchmark(times, "abcdefghijklmnopqrstuvwxyz","abcdefghijklmnopqrstuvwxyz$");
    benchmark(times, "aaaaaaaaaaaaaaaaaaaaaaaaaa","aaaaaaaaaaaaaaaaaaaaaaaaaa$");
    // [abcm] failed
    benchmark(times, "abcdefghijklmnopqrstuvwxyz",".*z");
    benchmark(times, "abcde",".*e");
    benchmark(times, "abcdef",".*f");
    
benchmark(times, "abcdefghijklmnopqrstuvwxyz","..........................$");
    benchmark(times, "abcdefghijklmnopqrstuvwxyz","[a]b*c+d\\w[f-g][g][h-i][i][^a][abcdefgk][l][m][n][o][p][a-z][r][s][t][u][v][w].*z$");
    benchmark(times, "zzz", "[abcdefghijklmnopqrstuvwxyz][abcdefghijklmnopqrstuvwxyz][abcdefghijklmnopqrstuvwxyz]$");

    benchmark(times, "7245 Sr","[0-9][0-9][0-9][0-9] \\w\\w$");

    benchmark(times, "abcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyzesting","[z-z][e-e]");
    benchmark(times, "abcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyzesting","[z-z]esting");

}
/*
 * Pinyin Engin Code 
 *  - define Pinyin IME 
 *
 * Author : Leonardo Physh 
 * Date   : 2014.9.10 Rev01 
 */

#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#include "config.h"
#include "common.h"
#include "py_dict.h"

#ifdef CONFIG_DEBUG 
char input[10];
#endif 

/*
 * py_ime : Pinyin IME 
 *  @str : pinyin spell 
 *  @return : cn string 
 */
char * py_ime(char *str)
{
    int i;
    int pos, num;
    int length;
 
    char *spell;
    struct index *cn;

    if(*str == '\0') 
        return NULL; 

    length = strlen(str); 
    for (i = 0; i < length; i++) {
        if (!((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z')))
            return NULL;
    }

    /*
     * up-case to low-case 
     */
    for (i = 0; i < length; i++)
        str[i] |= 0x20;  

    if (*str == 'i' || *str == 'u' || *str == 'v')
        return NULL;

    printf("str length: %d\n", length);

    cn = index_headletter[str[0] - 'a'];
    num = index_num[str[0] - 'a'];
    
    pos = 0;
    while(pos < num) {
        spell = cn->py;

        for(i = 0; i < length; i++){
            if(str[i + 1] != spell[i]) {
                break; 
            }
        }

        if(i == length || i + 1 == length) {
            return (*cn).mb;
        }

        pos++;
        cn++;
    }

    return NULL;    
}

#if 0
int main(void)
{
    int c;
    char ptr[4];
    char *result;

    while (1) {
        printf("\ninput: ");
        scanf("%s", input);

        result = py_ime(input);
        if (result != NULL) {
            int i;
            for (i = 0; i < strlen(result); i+=3) {
                memcpy(ptr, result + i, 3);
                ptr[3] = '\0';
                printf("%s ", ptr);
            }

            //printf("result : %s\n", result) ;
        }

        memset(input, 0, 10); 
        while ((c = getchar()) != '\n' && c != EOF);
    }

    return 0;
}
#endif 

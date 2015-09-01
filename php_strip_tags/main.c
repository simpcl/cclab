#include <stdio.h>
#include <string.h>


size_t php_strip_tags_ex(char *rbuf, int len, int *stateptr, char *allow, int allow_len, int allow_tag_spaces);


int main(int argc, char** argv)
{
    char buf[] = "adfa<p>eee</p>ccc";
    size_t size = 0;

    size = php_strip_tags_ex(buf, strlen(buf), NULL, NULL, 0, 0);
    

    printf("size: %ld\n", size);
    printf("content: %s\n", buf);

    return 0;
}

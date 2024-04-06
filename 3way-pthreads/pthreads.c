// CHECKPOINT 1: PTHREADS IMPLEMENTATION GOES HERE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int find_max(char* line, int nchars) 
{
    int i;
    int maxVal = 0;

    for (i = 0; i < nchars; i++) {
        if (line[i] > maxVal) {
            maxVal = (int) line[i];
        }
    }

    return maxVal;
}

int main()
{
   int nlines = 0, maxlines = 100;
   int i, err;
   int nchars = 0; 
   FILE *fd;
   char *line = (char*) malloc( 2001 ); // No lines larger than 2000 chars

   // Open the data file
   fd = fopen( "/homes/dan/625/wiki_dump.txt", "r" );
   for ( i = 0; i < maxlines; i++ ) {
      err = fscanf(fd, "%[^\n]\n", line);
      if (err == EOF) break;
      nchars = strlen(line);
      printf("%d: %d\n", nlines, find_max(line, nchars));
      nlines++;
   }

   fclose(fd);
   free(line);

   return 0;
}

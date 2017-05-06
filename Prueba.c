#include <stdio.h>
#include <stdlib.h>

#define TAMUSR 80
#define LINE 160

int main(int argc, char** argv)
{
  FILE* file;
  char buffer[LINE];
  int grafo[TAMUSR][TAMUSR], i, j;
  file = fopen("test.txt", "r");
  for(i = 0; (i < TAMUSR) && (!feof(file)); i++)
  {
    fgets(buffer, LINE, file);
    for(j = 0; j < TAMUSR; j++)
      grafo[i][j] = buffer[j * 2] == '1';
  }
  fclose ( file );
  return 0;
}

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
/*#include <string.h>*/

#define PATH_RM_LIST "trm.list"

int main(int argc, char **argv)
{

  int frmlist;
  time_t casSec;
  struct tm casStruct;
  char casString[81];


  char c;
  while((c=getopt(argc,argv,"het:"))!=-1){
    switch (c) {
      case 'h':
        printf("Tak tady tisknu help :)\n");
        break;
      case 'e':
        printf("Tady projdi trm.list a vymaz co je potreba\n");
        break;
      case 't':
        printf("Tady to vymaz za x dni %s\n",optarg);
        break;
    }

  }




  if((frmlist=open(PATH_RM_LIST,O_RDWR|O_CREAT,0644))==-1) {
    perror(strcat("Nemohu otevrit soubor ",PATH_RM_LIST));
    return 1;
  }

  casSec=time(NULL);

  printf("casSec = %d\n",casSec);

  casStruct=*(localtime(&casSec));


  /*printf("Je prave \"%s\" LC\n", ctime(&casSec));*/


  strftime(casString,sizeof(casString),"%d.%m.%Y",&casStruct);

  printf("%s\n",casString);

  /*size_t strftime(char  *s, size_t max, const char *format, const struct tm *tm);*/



  close(frmlist);

  return 0;

}


#include <stdio.h>     /* pro printf */
#include <stdlib.h>    /*pro getenv */
#include <getopt.h>    /*pro cteni argumentu*/
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

/*nazev souboru s udaji, kdy se jaky soubor ma vymazat.Je umisten v $HOME*/
#define NAZEV_TRM_LISTU ".trmlist"

/*pocet dnu do vymazani souboru,neurci-li uzivatel parametrem -t jinak*/
#define POCET_DNU_DO_VYMAZANI 30

/*maximalni pocet znaku parametru time*/
#define MAX_DELKA_PARAMETRU_TIME 5



char *PATH_RM_LIST; /* "/home/bugs/.trm.list"!!!MUSIM ZJISTIT DOM ADR!!!!!!!*/

int je_time_argument(char *dny){
/* vrati 0, kdyz je to chybne zadany argument time, kdyz je spravne vrati 1 */
  int i;
  if (strlen(dny)>MAX_DELKA_PARAMETRU_TIME) return 0;
  for(i=0;i<strlen(dny);i+=1)
    if((dny[i]<'0')||(dny[i]>'9')) return 0;

  return 1;
}

void prevedNaAbsolutniCestu(char **co) {
  char *pracAdr[500];

  if(*co[0]=='/') return;
  getcwd(pracAdr,sizeof(pracAdr));
  *co=strcat(strcat(pracAdr,"/"),*co);
  return;
}

int smazObsahAktualnihoAdresare(void){

  DIR *adresar;
  struct dirent dirent_str,*udirent_str;
  struct stat mujstat;
  char stringadr[500];

  udirent_str = &dirent_str;

  if((adresar=opendir(getcwd(NULL,0)))==NULL){
    fprintf(stderr,"Nelze otevrit adresar %s pro vypis",getcwd(NULL,0));
    return 1;
  }

  while((udirent_str=(readdir(adresar)))!=NULL) {
    dirent_str=*udirent_str;


    if((strcmp(dirent_str.d_name,".")==0)||
         (strcmp(dirent_str.d_name,"..")==0)) continue;

    lstat(dirent_str.d_name, &mujstat);
    if(S_ISDIR(mujstat.st_mode)){
      chdir(dirent_str.d_name);
      smazObsahAktualnihoAdresare();
      chdir("..");
      remove(dirent_str.d_name);
    }else{
      remove(dirent_str.d_name);
    }

  }
  closedir(adresar);

  return 0;
}

int myrm_force(char *cesta) {

  char *pracAdr[500];
  struct stat mujstat;

  lstat(cesta, &mujstat);


  if(S_ISDIR(mujstat.st_mode)){
    chdir(cesta);
    smazObsahAktualnihoAdresare();
    chdir("..");
  }else{
    fprintf(stderr,"\"%s\" neni adresar \n",cesta);
  }

  return (remove(cesta));
}



int provedExecute(void) {
  /*int frmlist;
  if((frmlist=open(PATH_RM_LIST,O_RDWR,0644))==-1) {
    perror(strcat("Nemohu otevrit soubor ",PATH_RM_LIST));
    return;
  }*/

  time_t aktualCasSec;
  struct tm aktualCasStruct,pomCasStruct;
  char PATH_POMFILE[]="/tmp/trm_pomfile.XXXXXX";

  if(mkstemp(&PATH_POMFILE)==(-1)){
    fprintf(stderr,"CHYBA: mkstemp\n");
    return 4;
  }

  unsigned int den,mesic,rok;
  char *cesta[512];/*jak tohle funguje a kolik to mam nastavit?*/
  FILE *frmlist,*pomtmpfile;

  aktualCasSec=time(NULL);
  aktualCasStruct=*(gmtime(&aktualCasSec));
  pomCasStruct=aktualCasStruct;



  if ((frmlist = fopen(PATH_RM_LIST,"rt"))==NULL){
    fprintf(stderr,"CHYBA: Soubor ""%s"" neexistuje, nebo jej nelze otevrit\n",
            PATH_RM_LIST);
    return 1;
  }
  if((pomtmpfile = fopen(PATH_POMFILE,"wt"))==NULL){
    fprintf(stderr,"Soubor ""%s"" nelze otebrit pro zapis\n",PATH_POMFILE);
    return 1;
  }
  /*unlink(PATH_POMFILE);*/


  while(fscanf(frmlist,"%2u.%2u.%4u %s",&den,&mesic,&rok,cesta)!=EOF){
    printf("%d %d %d %s\n",den,mesic,rok,cesta);

    pomCasStruct.tm_mday=den;
    pomCasStruct.tm_mon=mesic-1;
    pomCasStruct.tm_year=rok-1900;

    printf("%f\n",difftime(mktime(&pomCasStruct),mktime(&aktualCasStruct)));
    if(difftime(mktime(&pomCasStruct),mktime(&aktualCasStruct)) < 1){
      if(myrm_force(cesta)==-1){/*PROBLEM NEUMI MAZAT NEPRAZDNE ADRESARE*/
        printf("CHYBA: Nezlze smazat %s\n",cesta);
      }
    } else {
    fprintf(pomtmpfile,"%d.%d.%d %s\n",pomCasStruct.tm_mday,pomCasStruct.tm_mon,
            pomCasStruct.tm_year+1900,cesta);
    }
  }

  fclose(frmlist);
  fclose(pomtmpfile);

  rename(PATH_POMFILE,PATH_RM_LIST);


  return 0;
}

int pridejDoRmListu(const char *co,const int zaKolikDni) {

  time_t casSec;
  struct tm casStruct;
  FILE *frmlist,*ftestExistence;

  casSec = time(NULL);

  casSec += 3600*24*zaKolikDni;

  casStruct=*(localtime(&casSec));


  if ((frmlist = fopen(PATH_RM_LIST,"a+t"))==NULL){
    fprintf(stderr,"CHYBA: Soubor ""%s"" nelze otevrit\n",
            PATH_RM_LIST);
    return 1;
  }

  prevedNaAbsolutniCestu(&co);

  if ((ftestExistence=fopen(co,"rt")) == NULL) {
    printf("CHYBA: Soubor \"%s\" nelze otevrit a nebyl pridan do \"%s\"\n",
           co,PATH_RM_LIST);
    return 4;
  }
  fclose(ftestExistence);

  fprintf(frmlist,"%d.%d.%d %s\n",casStruct.tm_mday,casStruct.tm_mon+1,
            casStruct.tm_year+1900,co);
  fclose(frmlist);

  /*printf("DAVAM TAM ""%30s"" za %.3d dni.\n",co,zaKolikDni);*/

  return 0;

}

void vypisHelp(void){
  printf("trm je program pomoci ktereho smazete soubory s casovym"
        " spozdenim.\nSeznam souboru je v adresari"
        " \"$HOME/.trmlist\".\n"
        "  trm [teh] file1 [file2 ...]\n"
        "Prepinace:\n"
        "  -t(--time) nastaveni pocet dnu do smazani souboru (default 30dnu)\n"
        "  -e(--execute) projde soubor .trm.list a provede vymazani\n"
        "  -h(--help) vypis helpu\n");
  return;
}

int main (int argc, char **argv) {

  /*za kolik dni se vymaze nenastavi-li se prepinacem -t(--time) jinak) */
  int pocetdnu = POCET_DNU_DO_VYMAZANI;
  int c,retval=0;

  /* nastavi cestu k PATH_RM_LISTU=$HOME + / + NAZEV_TRM_LISTU */
  PATH_RM_LIST = strcat(strcat(getenv("HOME"),"/"),NAZEV_TRM_LISTU);


  while (1) {
   /* int this_option_optind = optind ? optind : 1; */
    int option_index = 0;
    static struct option long_options[] = {
        {"time"/*jmeno*/, 1/*zda potrebuje parametr*/, 0,'t'/*to vrati*/},
        {"execute", 0, 0,'e'},
        {"help", 0, 0,'h'},
        {0, 0, 0, 0}
    };

    c = getopt_long (argc, argv, "het:",
             long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 'h':
       vypisHelp();
      break;
    case 'e':
      if(provedExecute()!=0) retval=2;
      break;
    case 't':
      printf("Tady to vymaz za x dni %s\n",optarg);
      if(je_time_argument(optarg)){
        pocetdnu=atoi(optarg);
      } else {
        fprintf(stderr,"CHYBA: Argument ""%s"" neni prirozene cislo, nebo je"
               " prilis velke\n",optarg);
        retval=3;
      }
      break;

    /*case '?':
      break;*/

    default:
      printf ("?? getopt returned character code 0%o ??\n", c);
    }
  }

  if (optind < argc) {
    while(optind < argc) {
      pridejDoRmListu(argv[optind++],pocetdnu);
    }
  }

  return retval;
}


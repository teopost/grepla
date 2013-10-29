#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#define L_strtmp 2048

int main(int, char**);
int chkistext(void);
int cerca(void);
void sostituisci(void);
int apri(void);
int canreplace(char *,char *);
char *strnstr(char *,char *,int);
void cambia(char *);
void chiudi(void);
int opzioni(int, char**);
void heading(void);

int queries = 0, ignorecase = 0, showlines = 0, beginofline = 0, endofline = 0;
int noreplace = 0, redo = 0;
int tabstops = 8;
char _ftmpname[L_tmpnam+1];
char *fname,*ftmpname;
FILE *ftosearch,*ftemp;
struct stat finfo;
char *strnone = "";
char *tosearch, *toreplace;
int ltosearch,ltoreplace;
char *strtmp;
long linee;

void heading(void)
{
   printf("GREP & REPLACE for TEXT files\n");
}

int opzioni(argc,argv)
int argc;
char **argv;
{
   int i,l,c,help = 0;
   char *s;

   argc--;
   argv = &argv[1];
   c = 0;
   fname = strnone;
   tosearch = strnone;
   toreplace = strnone;
   for (i = 0; i < argc; i++) {
      s = argv[i];
      l = strlen(s);
      if ((s[0] == '-') && (s[1] != '-')) {
         while (l-- > 0) {
            switch (s[l]) {
               case 'b': beginofline = !0; break;
               case 'e': endofline = !0; break;
               case 'i': ignorecase = !0; break;
               case 'q': queries = !0; break;
               case 's': showlines = !0; break;
               case 'h':
                  printf("Usage: grepla -bihqs <filename> <tosearch> <toreplace>\n\n");
                  printf("Options:\n");
                  printf("   -b   Replace at Beginning of line (can't use with -e option)\n");
                  printf("   -e   Replace at End of line (can't use with -b option)\n");
                  printf("   -i   Ignore Case (Default = Case Sensitive)\n");
                  printf("   -q   Query user before replace (Default = Replace all)\n");
                  printf("   -s   Show lines while changing (Default = No Screen Output)\n");
                  help = !0;
                  break;
            }
         }
      } else if (l > 0) {
         if (s[0] == '-') {
            s = &s[1]; l--;
         }
         switch (++c) {
            case 1: fname = s; break;
            case 2: tosearch = s; ltosearch = l; break;
            case 3: toreplace = s; ltoreplace = l; break;
         }
      }
   }
   if (beginofline && endofline) {
      printf("Command-line error: You can't use -b and -e option togheter!\n");
      c = 0;
   }
   if ((c < 3) && !help) {
      printf("(Use -h for help)...\n");
   }
   return ((c >= 3) && !help);
}

int apri(void)
{
   int apriOK,e = 0;

   ftosearch = fopen(fname,"r");
   apriOK = (ftosearch != NULL);
   if (apriOK) {
      apriOK = (stat(fname,&finfo) == 0);
   } else if (!e) {
      printf("Error: can't open '%s'.\n",fname);
      e = !0;
   }
   if (apriOK) {
      ftmpname = tmpnam(NULL);
      apriOK = (ftmpname != NULL);
   } else if (!e) {
      printf("Error: '%s': can't get file informations.\n",fname);
      e = !0;
   }
   if (apriOK) {
      ftemp = fopen(ftmpname,"w");
      apriOK = (ftemp != NULL);
      if (apriOK) {
         strtmp = calloc(L_strtmp+1,sizeof(char));
         apriOK = (strtmp != NULL);
         if (!apriOK) {
            printf("Error: Memory allocation request fault.\n");
            remove(ftmpname);
         }
      } else if (!e) {
         printf("Error: can't create temporary file named '%s'.\n",ftmpname);
      }
   } else if (!e) {
      printf("Error: can't create temporary file name.\n");
   }
   return apriOK;
}

int canreplace(str,s)
char *str,*s;
{
   int c,i = strlen(str) - strlen(s),can;

   if (noreplace) {
      can = 0;
   } else if (queries) {
      printf("%07ld:%s",linee,str);
      for (c = 0; c < i; c++) {
         if (str[c] == '\t') i += tabstops-1;
      }
      i += 8;
      while (i-- > 0) putchar(' ');
      i = ltosearch; while (i-- > 0) putchar('^');
      printf("\nReplace (Yes/No/All/Stop replacing/Redo from start) ? Y\b");
      c = toupper(getchar()); fflush(stdin);

      can = 0;
      switch (c) {
         case 'A': queries = 0;
         case '\n':;
         case 'Y': can = !0; break;
         case 'S': queries = 0; noreplace = !0; break;
         case 'R': redo = !0; noreplace = !0; break;
      }
   } else {
      can = !0;
   }
   return can;
}

char *strnstr(s1,s2,ls2)
char *s1,*s2;
int ls2;
{
   int c,i,j,l = strlen(s1);
   l = (beginofline ? (l>0 ? 1 : 0) : (l - ls2 + 1));
   i = ((endofline && (l>1)) ? l-2 : 0);
   c = 0;
   while (i < l) {
      if (ignorecase) {
         for (j = 0, c = !0; (j < ls2) && c; j++) {
            c = (toupper(s1[i+j]) == toupper(s2[j]));
         }
      } else {
         c = !strncmp(&s1[i],s2,ls2);
      }
      if (c) break;
      i++;
   }
   return (c ? &s1[i] : NULL);
}

void cambia(str)
char *str;
{
   char *s,*spark;
   s = noreplace ? strnone : str;
   while ((s = strnstr(s,tosearch,ltosearch)) != NULL) {
      spark = calloc(strlen(s)-ltosearch+1,sizeof(char));
      if (spark == NULL) {
         printf("WARNING: Memory allocation error.\n");
         s = &s[ltosearch];
      } else if (canreplace(str,s)) {
         strcpy(spark,&s[ltosearch]);
         strcpy(s,toreplace);
         strcat(s,spark);
         free(spark);
         s = &s[ltoreplace];
      } else {
         s = &s[1];
      }
   }
}

int chkistext(void)
{
   int isOK = !0,c;
   struct stat finfo;
   FILE *f = fopen(fname,"rb");

   if (f != NULL) {
      stat(fname,&finfo);
      while (ftell(f) < finfo.st_size) {
         if (fgetc(f) == 0) {
            isOK = 0;
            break;
         }
      }
      fclose(f);
      if (!isOK) {
         printf("\b\bWARNING!: '%s'\n");
         printf("This file doesn't appear to be a text file.\n");
         printf("Do You want to continue anyway ? Y\b");
         c = toupper(getchar()); fflush(stdin);
         isOK = (c == 'Y') || (c == '\n');
         if (!isOK) {
            printf("(GREPLA interrupted by user)...\n");
         }
      }
   }
   return isOK;
}

void chiudi(void)
{
   size_t blocksize = 2048,r;
   FILE *ff,*ft;
   char *buf;

   free(strtmp);
   fclose(ftosearch);
   fclose(ftemp);
   remove(fname);

   buf = calloc(blocksize,sizeof(char));
   ff = fopen(ftmpname,"rb");
   ft = fopen(fname,"wb");

   if ((ff != NULL) && (ft != NULL) && (buf != NULL)) {
      do {
         r = fread(buf,sizeof(char),blocksize,ff);
         fwrite(buf,sizeof(char),r,ft);
      } while (!feof(ff));
      fclose(ff);
      fclose(ft);
      chmod(fname,finfo.st_mode);
   } else {
      printf("File open error! - See in:\n   %s\nfor changed file.\n",ftmpname);
   }
}

int main(argc,argv)
int argc;
char **argv;
{
   int mainOK;

   heading();
   mainOK = opzioni(argc,argv);
   if (mainOK) {
      mainOK = apri();
   }
   if (mainOK) {
      mainOK = chkistext();
   }
   if (mainOK) {
      linee = 0;
      while (ftell(ftosearch) < finfo.st_size) {
         fgets(strtmp,L_strtmp,ftosearch); ++linee;
         cambia(strtmp);
         if (showlines) printf("%s",strtmp);
         fputs(strtmp,ftemp);
         if (redo) {
            rewind(ftosearch);
            fclose(ftemp);
            ftemp = fopen(ftmpname,"w");
            redo = 0; noreplace = 0; linee = 0;
         }
      }
      printf("GREPLA: Ok.\n");
      chiudi();
   }
   return !mainOK;
}

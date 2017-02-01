#include <stdio.h>
#include <string>
#include <string.h>

namespace std {}
using namespace std;

char* getword(char *&word)
  {
    char *c=word;
    while (*word!='\n'&&*word!='\0'&&*word!=' ')
      word++;
    if (*word!='\0') *word++='\0';
    return c;
  } 

// Transforms a file filed with line and section markers into
// minumial representation
//
//  inname = file to read
//  outname = file to output
//
//  Tags
//    #L __auto__  line should be tagged with output file line
//    #L # SS      line should be tagged from a specific file
//    #S #         section start
//
void fixate(string inname, string outname, int desired_section, bool debug)
  {
    string csource;

    FILE *infile;
    FILE *outfile;
    int line=1;       // current output line
    int cline=-1;     // current input line
    int blanklines=0; // number of swallowed blank lines
    int length,i1;

    bool generated=true;
    bool printline=false;
    bool endline=true;
    bool section=false;

    char buf[1024];
    char *a2,*end;
  
    infile=fopen(inname.c_str(),"r");
    outfile=fopen(outname.c_str(),"w");
    if (!infile||!outfile) return;

    {
     string::iterator i=outname.begin();
     while (i!=outname.end())
       if (*i++=='/') 
         {outname.erase(outname.begin(),i); i=outname.begin();}
    }  

    while (!feof(infile))
      {
        if (!fgets(buf,1023,infile)) break;
        cline++;
       
        // $buf=~s/\s+$//; # kill trailing ws
        end=&buf[strlen(buf)-1]; 
        if (endline=(*end=='\n'))
          {
           while(*end==' '||*end=='\n'||*end=='\t')
             *end--='\0';
           end++;
           *end='\n';
          }
 
        // suck up blank lines
        if (*buf=='\n')
          {
            if (!printline)
              blanklines++;
          }

        // section 
        else if (strncmp(buf,"#S ",3)==0)
          {
            char *word=buf;
            getword(word);
            i1=atoi(getword(word));
            section=(i1==desired_section);
            blanklines=0;
            printline=true;
          }

        // line is generated
        else if (strncmp(buf,"#L __auto__",10)==0)
          {
            if (!generated)
              {
                generated=true;
                printline=true;
              }
          }
  
        // line is from source file
        else if (strncmp(buf,"#L ",3)==0)
          {
            char *word=buf;
            getword(word);
            i1=atoi(getword(word));
            a2=getword(word);
 
            if (generated||string(a2)!=csource)
              {
                generated=false;
                csource=string(a2);
                printline=true;
              }
            else if (section)
              {
                if (cline<=i1+blanklines)
                  blanklines-=cline-i1; 
                else if (i1==cline)
                  cline--;      
                else if (i1<cline)
                  printline=true;
                else if (i1<cline+4)
                  for (int i=cline;i<i1;i++,line++)
                    fprintf(outfile,"\n");
              }
            cline=i1-1;
          }

        else if (!section)
          {
            printline=false;
            blanklines=0;
          }
 
        // print with line number
        else if (printline||(blanklines>5))
          {
            printline=false;
            if (blanklines) {line++; fprintf(outfile,"\n");}
            blanklines=0;
            line+=1;
            if (debug)
            if (generated)
              fprintf(outfile,"#line %d \"%s\"\n",line,outname.c_str());
            else
              fprintf(outfile,"#line %d \"%s\"\n",cline,csource.c_str());
            fprintf(outfile,"%s",buf);
            line++;
          }
 
        // print line
        else 
          {
            // deal with blank lines
            for (int i=0;i<blanklines;i++,line++)
              fprintf(outfile,"\n");
            blanklines=0;

            fprintf(outfile,"%s",buf);
            line++;
          }
      }
    fclose(infile);
    fclose(outfile);
  }

#ifdef FIXATE_TEST
int main(int argc,char **argv)
  {
    string in,out;
    if (argc!=4) exit(-1);
    int sec=atoi(argv[3]);
    in=string(argv[1]); 
    out=string(argv[2]); 
    if (out=="-") out="/dev/tty";
    fixate(in,out,sec,true);
    return 0;
  }
#endif


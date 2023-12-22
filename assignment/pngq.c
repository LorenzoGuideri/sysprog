/*
Lorenzo Guideri

All the code below is written by me, but there are some things to point out:
- Every part that imply the calculation of the crc copied from various internet sources,
  it's the result of patching together several script parts and hours of debugging to
  get to an optimal solution for my case.
- All the theory about PNG file format is extracted by the sources given by Professor
  Carzaniga and the post at the following link: https://www.quora.com/How-is-a-png-file-built
- I think my code is pretty straightforward, I don't use any particular library except for
  the standard ones and the main part is closed inside a single function. I'm available for 
  answering any question about it.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *pdef="_f: _w x _h, _c, _d bits per sample, _N chunks\n_C";
char *cdef="\t_n: _t (_l)\n";
char *kdef="\t_k: _t\n";
unsigned char PNGSignature[]={137, 80, 78, 71, 13, 10, 26, 10};

struct chunk{
    unsigned int len;
    unsigned char type[4];
    unsigned char *data;
    unsigned int crc;
    struct chunk *next;
};

unsigned long crc_table[256];
int crc_table_computed=0;
void makeTable(void){
  unsigned long c;
  int n, k;
  for (n=0; n < 256; n++){
    c=(unsigned long) n;
    for (k=0; k < 8; k++){
      if (c & 1)
        c=0xedb88320L ^ (c >> 1);
      else
        c=c >> 1;
    }
    crc_table[n]=c;
  }
  crc_table_computed=1;
}

unsigned long int calculateCRC(unsigned long crc, unsigned char *buf, int len){
  unsigned long c=crc;
  int n;
  if (!crc_table_computed)
    makeTable();
  for (n=0; n < len; n++)
    c=crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
  return c;
}
unsigned long crc(unsigned char *buf, int len){
    return calculateCRC(0xffffffffL, buf, len) ^ 0xffffffffL;
}

int main(int argc, char *argv[]){
    int metEnd=0;
    for (int i=1; i < argc; i++){
        if (!metEnd){
            if (strcmp(argv[i], "--")==0){
                metEnd=1;
                continue;
            }
            else if (strncmp(argv[i], "c=", 2)==0){
                cdef=argv[i] + 2;
                continue;
            }
            else if (strncmp(argv[i], "p=", 2)==0){
                pdef=argv[i] + 2;
                continue;
            }
            else if (strncmp(argv[i], "k=", 2)==0){
                kdef=argv[i] + 2;
                continue;
            }
        }
        char * filename=argv[i];   
        unsigned int n_of_chunks=0;
        struct chunk *head=NULL;
        struct chunk *tail=NULL;
        unsigned int width;
        unsigned int height;
        unsigned int bdp;
        unsigned int clr;
        FILE * file=fopen(filename, "r");
        if (!file){
            fprintf(stderr,"Error while opening: %s\n", filename);
            continue;
        }
        unsigned char first[8];
        if (fread(first, 1, 8, file) != 8){
            fclose(file);
            fprintf(stderr,"File not valid: %s (length is less than 8)\n", filename);
            continue;
        }
        int valid=1;
        for (int i=0; i < 8; i++)
            valid&=(first[i]==PNGSignature[i]);
        if (!valid){
            fprintf(stderr,"Invalid siganture for file: %s\n", filename);
            fclose(file);
            continue;
        }
        for (;;){
            if(!valid)
                break;
            unsigned char chunkLen[4];
            if (fread(chunkLen, 1, 4, file)!=4){
                valid=0;
                fprintf(stderr,"Invalid chunk length in file: %s\n", filename);
                break;
            }
            struct chunk * c=malloc(sizeof(struct chunk));
            if (!c){
                valid=0;
                fprintf(stderr,"Memory allocation failed\n");
                break;
            }
            c->len=(chunkLen[0]<<24) | (chunkLen[1] << 16) | (chunkLen[2] << 8) | chunkLen[3];
            if (fread(c->type, 1, 4, file) != 4){
                valid=0;
                fprintf(stderr,"Cannot read data (type)\n");
                break;
            }
            c->data=malloc(c->len);
            if (!c->data){
                valid=0;
                fprintf(stderr,"Memory allocation failed\n");
                break;
            }
            if (fread(c->data, 1, c->len, file) != c->len){
                valid=0;
                fprintf(stderr,"Cannot read data (data)\n");
                break;
            }
            unsigned char tempcrc[4];
            if (fread(tempcrc, 1, 4, file) != 4){
                valid=0;
                fprintf(stderr,"Cannot read data (crc)\n");
                break;
            }
            else
                c->crc=(tempcrc[0]<<24) | (tempcrc[1] << 16) | (tempcrc[2] << 8) | tempcrc[3];
            unsigned long crc=0xffffffffUL;
            crc=calculateCRC(crc, c->type, 4);
            crc=calculateCRC(crc, c->data, c->len);
            crc= crc ^ 0xffffffffUL; 
            if(crc != c->crc){
                valid=0;
                fprintf(stderr,"One or more chunks have invalid crc\n");
                break;
            }           
            if (head==NULL)
                head=c;
            else
                tail->next=c;
            c->next=NULL;
            tail=c;
            n_of_chunks += 1;
            if (memcmp(c->type, "IEND", 4)==0)
                break;
            if (memcmp(c->type, "IHDR", 4)==0){
                width=(c->data[0]<<24) | (c->data[1] << 16) | (c->data[2] << 8) | c->data[3];
                height=(c->data[4]<<24) | (c->data[5] << 16) | (c->data[6] << 8) | c->data[7];
                bdp=c->data[8];
                clr=c->data[9];
            }
        }
        fclose(file);
        unsigned int ii=0;
        if(!valid)
           continue;
        for (int i=0; i < strlen(pdef); i++){
            if (pdef[i]=='_'){
                i++;
                switch (pdef[i]){
                case 'f':
                    printf("%s", filename);
                    break;
                case 'w':
                    printf("%u", width);
                    break;
                case 'h':
                    printf("%u", height);
                    break;
                case 'd':
                    printf("%d", bdp);
                    break;
                case 'c':
                    switch (clr){
                        case 0:
                            printf("Greyscale");
                            break;
                        case 2:
                            printf("Truecolor");
                            break;
                        case 3:
                            printf("Indexed");
                            break;
                        case 4:
                            printf("Greyscale+alpha");
                            break;
                        case 6:
                            printf("Truecolor+alpha");
                            break;
                        default:
                            printf("Unknown color type");
                            break;
                    }
                    break;
                case 'N':
                    printf("%u", n_of_chunks);
                    break;
                case 'C':
                    for (struct chunk *c=head; c != NULL; c=c->next){
                        ii++;
                        for (int i=0; i < strlen(cdef); i++){
                            if (cdef[i]=='_'){
                                i++;
                                switch (cdef[i]){
                                case 'n':
                                    printf("%u", ii);
                                    break;
                                case 't':
                                    putchar(c->type[0]);
                                    putchar(c->type[1]);
                                    putchar(c->type[2]);
                                    putchar(c->type[3]);
                                    break;
                                case 'l':
                                    printf("%u", c->len);
                                    break;
                                case 'c':
                                    printf("%u", c->crc);
                                    break;
                                case 'D':
                                    for (int i=0; i < c->len; i++){
                                        if(c->data[i]<16) putchar(' ');
                                        if ((i+1) % 16==0)
                                            printf("%x\n", c->data[i]);
                                        else
                                            printf("%x ", c->data[i]);
                                    }
                                    printf("\n");
                                    break;
                                default:
                                    putchar(cdef[i]);
                                }
                            }
                            else
                                putchar(cdef[i]);
                        }
                    }
                    break;
                case 'K':
                    for (struct chunk *c=head; c != NULL; c=c->next){
                        if (memcmp(c->type, "tEXt", 4)==0){
                            unsigned int doppioZero=0;
                            while (c->data[doppioZero] != 0)
                                doppioZero++;
                            for (int i=0; i < strlen(kdef); i++){
                                if (kdef[i]=='_'){
                                    i++;
                                    switch (kdef[i]){
                                    case 'k':
                                        printf("%s", c->data);
                                        break;
                                    case 't':
                                        doppioZero++;
                                        while (doppioZero < c->len){
                                            putchar(c->data[doppioZero]);
                                            doppioZero++;
                                        }
                                        break;
                                    default:
                                        putchar(kdef[i]);
                                        break;
                                    }
                                }
                                else
                                    putchar(kdef[i]);
                            }
                        }
                    }
                    break;
                default:
                    putchar(pdef[i]);
                    break;
                }
            }
            else
                putchar(pdef[i]);
        }
        for (struct chunk *tmp=head; head!=NULL; head=head->next, free(tmp->data), free(tmp), tmp=head) {}
    }
}
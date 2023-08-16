
#include <unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<malloc.h>
#include "crypto.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <libgen.h>

#define HASH_BLOCK 8
#define HASH_LEN 20
#define CRYPTO_BYTES  16
void debug_dump_databuf(char * buf,int bufsiz)
{
    int i;
    int k=0;
    int tmp;
    for(i=0;i<bufsiz;i++){
     k++;
     tmp=(*(buf+i))&(0xff);
     printf("0x%x ",tmp);
     if(k==8){
             printf("\n");
             k=0;
         }
 }
    printf("\n");
}
void do_hash(char * hash_code,char * filename)
{

    int size;
    FILE *fd ;
    hashalg_CTX ctx;
    char * block_buf=(char *)malloc(HASH_BLOCK+1);
    fd = fopen (filename, "rb");
    hashalgInit(&ctx);
    while(!feof (fd)){
        size=fread (block_buf, 1, HASH_BLOCK, fd);
        hashalgUpdate(&ctx, (const unsigned char*)block_buf, size);
    }
    hashalgFinal((unsigned char *)hash_code, &ctx);
    hash_code[20] = '\0';
    fclose(fd);
}
void genkey(void)
{
    FILE * privatekey_fd ;
    FILE * publickey_fd ;
    uint8_t * public_key=(uint8_t *)calloc(CRYPTO_BYTES+1,sizeof(uint8_t)); 
    uint8_t * private_key=(uint8_t *)calloc(CRYPTO_BYTES,sizeof(uint8_t)); 

    privatekey_fd = fopen ("private_key", "wb");
    publickey_fd = fopen ("public_key", "wb");
    printf("make key ret=%d\n",cryalg_make_key(public_key,private_key));

    debug_dump_databuf((char *)public_key,CRYPTO_BYTES+1);
    debug_dump_databuf((char *)private_key,CRYPTO_BYTES);

    fwrite(public_key,sizeof(uint8_t),CRYPTO_BYTES+1,publickey_fd);
    fwrite(private_key,sizeof(uint8_t),CRYPTO_BYTES,privatekey_fd);
    fclose(publickey_fd);
    fclose(privatekey_fd);

}
int load_key(uint8_t * public_key, uint8_t * private_key)
{

    FILE * privatekey_fd ;
    FILE * publickey_fd ;
    privatekey_fd = fopen ("private_key", "rb");
    publickey_fd = fopen ("public_key", "rb");
    memset(public_key,0,CRYPTO_BYTES+1);
    memset(private_key,0,CRYPTO_BYTES);
    fread(private_key, sizeof(uint8_t), CRYPTO_BYTES, privatekey_fd);
    fread(public_key, sizeof(uint8_t), CRYPTO_BYTES+1, publickey_fd);
    fclose(privatekey_fd);
    fclose(publickey_fd);
    return 0;
}

int verify(char * sig_filename,char * filename)
{

    uint8_t * public_key=(uint8_t *)calloc(CRYPTO_BYTES+1,sizeof(uint8_t)); 
    uint8_t * private_key=(uint8_t *)calloc(CRYPTO_BYTES,sizeof(uint8_t)); 
    uint8_t * signature=(uint8_t *)calloc(CRYPTO_BYTES*2,sizeof(uint8_t));
    FILE * signature_fd ;
    char * hash_code=(char *)malloc(HASH_LEN+1);

    load_key(public_key,private_key);
    do_hash(hash_code,filename);
    signature_fd = fopen (sig_filename, "rb");
    fread(signature, sizeof(char), CRYPTO_BYTES*2, signature_fd);
    fclose(signature_fd);
    return cryalgsa_verify(public_key,(uint8_t *)hash_code,signature);
}


int main(int argc, char **argv)
{

    uint8_t * public_key=(uint8_t *)calloc(CRYPTO_BYTES+1,sizeof(uint8_t)); 
    uint8_t * private_key=(uint8_t *)calloc(CRYPTO_BYTES,sizeof(uint8_t)); 
    uint8_t * signature=(uint8_t *)calloc(CRYPTO_BYTES*2,sizeof(uint8_t));
    char * hash_code=(char *)malloc(HASH_LEN+1);
    char filename[1024]={0};
    char sig_filename[1024]={0};
    FILE * signature_fd ;

    if(argc<2) return -1;
    if(*argv[1] == 'k')
    {
        genkey();
        return 0;
    }
    getcwd(filename,sizeof(filename));  
    getcwd(sig_filename,sizeof(sig_filename));  
    filename[strlen(filename)]='/';
    sig_filename[strlen(sig_filename)]='/';
    memcpy(filename+strlen(filename),argv[1],strlen(argv[1]));
    memcpy(sig_filename+strlen(sig_filename),basename(filename),strlen(basename(filename)));
    memcpy(sig_filename+strlen(sig_filename),".sig",4);
    printf("%s\n",sig_filename);
    do_hash(hash_code,filename);
    printf("hash code:\n");
    debug_dump_databuf(hash_code,20);
    load_key(public_key,private_key);
    printf("sign ret=%d\n",cryalgsa_sign(private_key,(uint8_t *)hash_code,signature));
    printf("signature:\n");   
    debug_dump_databuf((char * )signature,CRYPTO_BYTES*2);

    signature_fd = fopen (sig_filename, "wb");
    fwrite(signature,sizeof(char),CRYPTO_BYTES*2,signature_fd);
    fclose(signature_fd);
    printf("verify ret=%d\n",verify(sig_filename,filename));
    printf("signature file:%s\n",basename(sig_filename));
    return 0;
}

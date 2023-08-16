#include "base64.h"  
#include "string.h"
int b64_encode(unsigned char *str,unsigned char * res,long str_len)  
{  
    long len;  
    int i,j;  
    char *base64_table="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";  
  
    if(str_len % 3 == 0)  
        len=str_len/3*4;  
    else  
        len=(str_len/3+1)*4;  
  
    res[len]='\0';  
  
    for(i=0,j=0;i<len-2;j+=3,i+=4)  
    {  
        res[i]=base64_table[str[j]>>2]; 
        res[i+1]=base64_table[(str[j]&0x3)<<4 | (str[j+1]>>4)]; 
        res[i+2]=base64_table[(str[j+1]&0xf)<<2 | (str[j+2]>>6)];   
        res[i+3]=base64_table[str[j+2]&0x3f];   
    }  
  
    switch(str_len % 3)  
    {  
        case 1:  
            res[i-2]='=';  
            res[i-1]='=';  
            break;  
        case 2:  
            res[i-1]='=';  
            break;  
    }  
  
    return 0;  
}  
  
int b64_decode(unsigned char *code,unsigned char * res,long len)  
{  
    int table[]={0,0,0,0,0,0,0,0,0,0,0,0,
    		 0,0,0,0,0,0,0,0,0,0,0,0,
    		 0,0,0,0,0,0,0,0,0,0,0,0,
    		 0,0,0,0,0,0,0,62,0,0,0,
    		 63,52,53,54,55,56,57,58,
    		 59,60,61,0,0,0,0,0,0,0,0,
    		 1,2,3,4,5,6,7,8,9,10,11,12,
    		 13,14,15,16,17,18,19,20,21,
    		 22,23,24,25,0,0,0,0,0,0,26,
    		 27,28,29,30,31,32,33,34,35,
    		 36,37,38,39,40,41,42,43,44,
    		 45,46,47,48,49,50,51
    	       };
    long str_len;
    int i,j;
    int table_len = sizeof(table)/sizeof(table[0]);
    if(strstr((const char *)code,"=="))
        str_len=len/4*3-2;
    else if(strstr((const char *)code,"="))
        str_len=len/4*3-1;
    else
        str_len=len/4*3;

    res[str_len]='\0';  
    for(i=0;i<len;i++){
       if(code[i]=='_')code[i]='/';
       if(code[i]=='-')code[i]='+';
    }
    for(i=0,j=0;i < len-2;j+=3,i+=4)
    {
        if(code[i]<table_len &&  code[i+1]<table_len  &&  code[i+2]<table_len  &&  code[i+3]<table_len){
            res[j]=((unsigned char)table[code[i]])<<2 | (((unsigned char)table[code[i+1]])>>4);
            res[j+1]=(((unsigned char)table[code[i+1]])<<4) | (((unsigned char)table[code[i+2]])>>2);
            res[j+2]=(((unsigned char)table[code[i+2]])<<6) | ((unsigned char)table[code[i+3]]); 
        }
    }

    return 0;

}  

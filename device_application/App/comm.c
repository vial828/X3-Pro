#include "HWI_Hal.h"
#include "kernel.h"
#include <string.h>
#include "uart.h"
#include "comm.h"
#include "log.h"
#include "cit.h"
#include "update_cmd.h"
#include "app_heat.h"
#include "app_charge.h"
#include "usr_cmd.h"
#include "sha256.h"
#include "base64.h"
#include "uECC.h"
#include "self_flash.h"
#include "dev_adc.h"
#include "FreeRTOS.h"
#include "semphr.h"

static SemaphoreHandle_t uart_buffer_lock=0;

#define COMM_SEND_BUF_LEN  4096
#define COMM_REC_BUF_LEN   4096
#define MAX_DATA_LEN   2048

#define CMD_HEAD    0x7E
#define CMD_VER     0x11
#define CMD_SID     0x01
#define CMD_TAIL    0x0D

#define DES_ADDR_INDEX 3
#define CMD_INDEX     4
#define H_LEN_INDEX   5
#define L_LEN_INDEX   6
#define DATA_INDEX    7

#define PACK_SHELL_LEN  10

static uint8_t comm_rec_buf[COMM_REC_BUF_LEN];
static uint16_t rec_r_pos = 0;
static uint16_t rec_w_pos = 0;

static uint8_t comm_send_buf[COMM_SEND_BUF_LEN];
static uint8_t dma_buf[COMM_SEND_BUF_LEN];
static uint16_t cur_pos = 0;
static uint8_t comm_lock = 1;
unsigned char hashbuf[100] = {0};
unsigned char pubkey[]= {
    0xcc, 0x24, 0xc6, 0xa3, 0x98, 0x66, 0x2e, 0x20,
    0x28, 0xdd, 0x17, 0x90, 0x1a, 0xe1, 0x98, 0x6e,
    0x87, 0x2d, 0xef, 0x08, 0xa8, 0xf7, 0xc4, 0xf0,
    0xa3, 0x26, 0x0b, 0xdf, 0x62, 0xcd, 0x01, 0x2a,
    0x6d, 0x61, 0xdc, 0x7a, 0x7f, 0x10, 0xc8, 0x7c,
    0xae, 0x8a, 0x45, 0x77, 0x80, 0xfe, 0xe2, 0xdf,
    0xdf, 0xa4, 0xdf, 0xb4, 0xa2, 0x8f, 0x32, 0x4d,
    0xf3, 0xb1, 0x33, 0xa4, 0x44, 0xff, 0x6d, 0x04,
    };

uint8_t get_comm_lock(void)
{
    return comm_lock;
}

void set_comm_lock(uint8_t flag)
{
    comm_lock = flag;
}

void bin2hex( char *  bin,  char *  hex,int len)
{
    const   char *  cHex  =   "0123456789abcdef" ;
    int  i = 0 ;
    int j;
    for (   j  = 0 ; j  <  len; j ++ ){
        unsigned  int  a  =   (unsigned  int ) bin[j];
        hex[i ++ ]  =  cHex[(a  &   0xf0 )  >>   4 ];
        hex[i ++ ]  =  cHex[(a  &   0x0f )];
    }
    hex[i]  = 0 ;
}

void get_32byte_rand(unsigned char *buf)
{
    int randv;
    srand(dev_get_random_seed());
    randv= rand();
    sha256_hash(buf, (unsigned char *)&randv, sizeof(randv));
}

void hashbuf_random_init(void)
{
    int randv;
    srand(dev_get_random_seed());
    randv= rand();
    sha256_hash(hashbuf, (unsigned char *)&randv, sizeof(randv));
}

void get_challenge(void)
{
    unsigned char* data = (unsigned char *) calloc(100,1);
    unsigned char* data_hex = (unsigned char *) calloc(100,1);
    unsigned char* hash_b64 = (unsigned char *) calloc(100,1);
    if(data == NULL || data_hex == NULL || hash_b64 == NULL){
        return;
    }
    get_32byte_rand(data);/* 32 bytes random challenge */
    bin2hex((char*)data, (char*)data_hex, 32);/* 64 bytes hex-string challenge */
    //LOGD("32 byte Challenge value (Ch) is:%s",data_hex);
    sha256_hash(hashbuf, data_hex, 64);/* 32 bytes digest of challenge */
    //LOGD("(Ch) sha256 Hash digest is:%s",hashbuf);
    //LOGD("Base64 value to be copied and pasted for Signing:");
    b64_encode (hashbuf,hash_b64,32);
    //LOGD("%s",hash_b64);
    //LOGD("https://soteria-app.azurewebsites.net/api/signingRequest?code=Epf9aEMuu4tSAjXYQmWCZnSMKwqpbPjz7Fn2DyTcp5KJIqw4nftGJQ%3D%3D&sn=SOTERIA01&challenge=%s",
    //               hash_b64);
    respond_usr_cmd(CMD_GET_CHALLENGE, (uint8_t*)hash_b64, strlen((char*)hash_b64));
    free(data);
    free(data_hex);
    free(hash_b64);
}

int8_t sig_verify(uint8_t * signature, uint8_t sig_len)
{
    int8_t ret = 0;
    if(sig_len > 0 && sig_len < 130)
    {
        unsigned char* sig_b64 = (unsigned char *) calloc(130,1);
        unsigned char* sigbuf = (unsigned char *) calloc(130,1);

        if(sig_b64 == NULL ||sigbuf == NULL)
        {
            return 0;
        }
        memcpy(sig_b64, signature, sig_len);
        LOGD("sig_b64 is %s", sig_b64);
        b64_decode (sig_b64,sigbuf,88);
        ret= uECC_verify(pubkey, hashbuf, 32, sigbuf, uECC_secp256k1());
        free(sig_b64);
        free(sigbuf);
    }
    else
    {
        ret = 0;
    }
    return ret;
}

/*************************************************************************************************
  * @brief    : create uart buffer lock for LOG from different task
  * @param1   : void
  * @return   : void
*************************************************************************************************/
void uart_lock_create(void)
{
    uart_buffer_lock = xSemaphoreCreateMutex();
}


/*************************************************************************************************
  * @brief    : calculate crc value of buffer
  * @param1   : uint8_t *:  pointer of the buffer
  * @param2   : uint32_t: the length of buffer that needs to be calculated
  * @return   : uint16_t: crc value
*************************************************************************************************/
static uint16_t check_crc(const uint8_t* pbuff,const uint32_t len)
{
    uint16_t crc = 0;
    uint32_t i = 0;

    for(i = 0; i < len; i++)
    {
        crc += pbuff[i];
    }

    return (crc);
}

/*************************************************************************************************
  * @brief    : save command to buffer(comm_send_buf) that will be sent out by uart
  * @param1   : uint8_t: command ID
  * @param2   : uint8_t: command direction
  * @param3   : uint8_t *: pointer to data that will be included in command
  * @param4   : uint16_t: data length
  * @return   : int8_t: -1(data length oversize or comm_send_buf is not enough to save command)/
                        data length + 10(command length)
*************************************************************************************************/
uint8_t  tmp_buf[ONE_LOG_SIZE + 10];  //other package info is 10
int8_t comm_send(uint8_t cmd, uint8_t des, uint8_t *pdata, uint16_t len)
{
    uint16_t crc = 0;
    //uint8_t  tmp_buf[ONE_LOG_SIZE + 10];  //other package info is 10
    if(len > ONE_LOG_SIZE){
        return -1;
    }
    if((len+10) > (COMM_SEND_BUF_LEN - 1 - cur_pos)){
        return -1;
    }

    if (xSemaphoreTake(uart_buffer_lock, portMAX_DELAY))
    {
        memset(tmp_buf,0,ONE_LOG_SIZE + 10);
        tmp_buf[0] = CMD_HEAD;
        tmp_buf[1] = CMD_VER;
        tmp_buf[2] = 0x20;
        tmp_buf[3] = des;
        tmp_buf[4] = cmd;
        tmp_buf[5] = (len >> 8) & 0xff;
        tmp_buf[6] = len & 0xff;
        memcpy(&tmp_buf[7],pdata,len);
        crc = check_crc(tmp_buf,len + 7);
        tmp_buf[len + 7] = (crc >> 8) & 0xff;
        tmp_buf[len + 8] = crc & 0xff;
        tmp_buf[len + 9] = CMD_TAIL;

        memcpy(&comm_send_buf[cur_pos], tmp_buf, len+10);
        cur_pos = cur_pos + (len+10);
       	// Use Guarded Resource
		// Give Semaphore back:
		xSemaphoreGive(uart_buffer_lock);
        return (int8_t)(len+10);
    }
    else{
		return -1;
	}
}

/*************************************************************************************************
  * @brief    : send command by uart immediately
  * @param1   : uint8_t: command ID
  * @param2   : uint8_t: command direction
  * @param3   : uint8_t *: pointer to data that will be included in command
  * @param4   : uint16_t: data length
  * @return   : void
*************************************************************************************************/
void comm_send_now(uint8_t cmd, uint8_t des, uint8_t *pdata, uint16_t len)
{
    comm_send(cmd, des, pdata, len);

    /*check uart dma idle*/
    while(UartStatusCheck() != SET);
    comm_send_proc();
    while(UartStatusCheck() != SET);
}

/*************************************************************************************************
  * @brief    : transfer data recieved by uart to buffer(comm_rec_buf)
  * @param1   : uint8_t *: pointer to data buffer
  * @param2   : uint32_t: data length
  * @return   : void
*************************************************************************************************/
void UartRxFun(uint8_t * buffer,uint32_t len)
{
    uint16_t tail_left = 0;

    //comm_rec_buf{--commands that have been read--read pos--commands that have not been read--write pos--}
    if(rec_w_pos >= rec_r_pos){
        tail_left = COMM_REC_BUF_LEN-rec_w_pos;

        //comm_rec_buf{buffer head--read pos--write pos--new data--buffer tail}
        if(len < tail_left){
            memcpy(&comm_rec_buf[rec_w_pos], buffer, len);
            rec_w_pos = rec_w_pos + len;

        //comm_rec_buf{buffer head--read pos--write pos--new data}
        }else if(len == tail_left && rec_r_pos !=0){  //w_pos can not get r_pos
            memcpy(&comm_rec_buf[rec_w_pos], buffer, len);
            rec_w_pos = 0;

        //comm_rec_buf{last part of new data--read pos--write pos--first part of new data}
        }else if(len < (tail_left+rec_r_pos)){
            memcpy(&comm_rec_buf[rec_w_pos], buffer, tail_left);
            memcpy(&comm_rec_buf[0], &buffer[tail_left], len-tail_left);
            rec_w_pos = len-tail_left;
        }

    //comm_rec_buf{--write pos--commands that have been read--read pos--commands that have not been read--}
    }else{

        //comm_rec_buf{buffer head--write pos--new data--read pos--buffer tail}
        if(len < (rec_r_pos - rec_w_pos)){
            memcpy(&comm_rec_buf[rec_w_pos], buffer, len);
            rec_w_pos = rec_w_pos + len;
        }
    }
}

/*************************************************************************************************
  * @brief    : get one command from comm_rec_buf
  * @param1   : uint8_t *: a pointer to the buffer where command is saved
  * @return   : int8_t :0/-1(did not get a legal command); 1 + length of data which included in command
*************************************************************************************************/
int16_t get_one_cmd(uint8_t * pbuf)
{
    uint16_t r_pos;
    uint16_t left;
    uint16_t tail_left;
    uint16_t data_len;
    uint16_t crc;

    //no data needs to be handled in comm_rec_buf
    if(rec_r_pos == rec_w_pos){
        return 0;
    }
    r_pos = rec_r_pos;
    /* find the CMD_HEAD*/
    while(comm_rec_buf[r_pos] != CMD_HEAD){
        r_pos = (r_pos+1)%COMM_REC_BUF_LEN;
        if(r_pos == rec_w_pos){
            rec_r_pos = r_pos; //update rec_r_pos
            return 0;
        }
    }
    /* check data integrity */
    if(r_pos < rec_w_pos){//--r_pos(CMD_HEAD)--rec_w_pos--
        left = rec_w_pos - r_pos;
        if(left < PACK_SHELL_LEN){ // 10 means tha package size without data
            rec_r_pos = rec_w_pos;
            return 0;
        }else{

            //get data len
            data_len = (uint16_t)comm_rec_buf[r_pos+H_LEN_INDEX]<<8 | comm_rec_buf[r_pos+L_LEN_INDEX];

            //data len oversize
            if(data_len > MAX_DATA_LEN){
                rec_r_pos = r_pos + 1;
                return -1;
            }

            //data length plus packet shell length is oversize
            if(data_len+PACK_SHELL_LEN > left){
                rec_r_pos = r_pos + 1;
                return -1;
            }

            //the byte in the position where CMD_TAIL supposed to be is not CMD_TAIL
            if(comm_rec_buf[r_pos+data_len+PACK_SHELL_LEN-1] != CMD_TAIL){
                rec_r_pos = r_pos + 1;
                return -1;
            }

            //calculate the crc of one packet
            crc = (comm_rec_buf[r_pos+data_len+PACK_SHELL_LEN-3] << 8) | comm_rec_buf[r_pos+data_len+PACK_SHELL_LEN-2];
            if(crc != check_crc(&comm_rec_buf[r_pos], data_len+PACK_SHELL_LEN-3))
            {
                rec_r_pos = r_pos + 1;
                //remove cit cmd crc check
                if((comm_rec_buf[r_pos + CMD_INDEX])<CMD_CIT_START || (comm_rec_buf[r_pos + CMD_INDEX])>CMD_CIT_END)
                {
                    return -1;
                }
            }

            pbuf[0] = comm_rec_buf[r_pos+DES_ADDR_INDEX];  //get des address
            pbuf[1] = comm_rec_buf[r_pos+CMD_INDEX];
            memcpy(&pbuf[2], &comm_rec_buf[r_pos+DATA_INDEX], data_len);
            rec_r_pos = r_pos+data_len+PACK_SHELL_LEN;
            return (data_len+1);
        }
    }else{//--rec_w_pos--r_pos(CMD_HEAD)--
        uint16_t h_index, l_index;
        uint8_t tail_value;

        left = COMM_REC_BUF_LEN - r_pos + rec_w_pos;
        if(left < PACK_SHELL_LEN){
            rec_r_pos = rec_w_pos;
            return 0;
        }

        h_index = (r_pos+H_LEN_INDEX)%COMM_REC_BUF_LEN;
        l_index = (r_pos+L_LEN_INDEX)%COMM_REC_BUF_LEN;

        //get data length
        data_len = (uint16_t)comm_rec_buf[h_index]<<8 | comm_rec_buf[l_index];

        //data length oversize
        if(data_len > MAX_DATA_LEN){
            rec_r_pos = (r_pos + 1)%COMM_REC_BUF_LEN;
            return -1;
        }

        //data length plus packet shell length is oversize
        if(left < data_len+PACK_SHELL_LEN){
            rec_r_pos = (r_pos + 1)%COMM_REC_BUF_LEN;
            return -1;
        }

        tail_value = comm_rec_buf[(r_pos+data_len+PACK_SHELL_LEN-1)%COMM_REC_BUF_LEN];
        //the byte in the position where CMD_TAIL supposed to be is not CMD_TAIL
        if(tail_value != CMD_TAIL){
            rec_r_pos = (r_pos + 1)%COMM_REC_BUF_LEN;  //update  rec_r_pos for find head from next comm_rec_buf index
            return -1;
        }

        tail_left = COMM_REC_BUF_LEN - r_pos;

        //calculate the crc of one packet
        crc = ((comm_rec_buf[(r_pos+data_len+PACK_SHELL_LEN-3)%COMM_REC_BUF_LEN]) << 8) | (comm_rec_buf[(r_pos+data_len+PACK_SHELL_LEN-2)%COMM_REC_BUF_LEN]);
        if(r_pos+data_len+PACK_SHELL_LEN-3 > COMM_REC_BUF_LEN)//--data--crc--tail--w--r--head--len--data--
        {
            if(crc !=  (uint16_t)(check_crc(&comm_rec_buf[r_pos], tail_left) + check_crc(&comm_rec_buf[0], data_len+PACK_SHELL_LEN-3-tail_left)))
            {
                rec_r_pos = (r_pos + 1)%COMM_REC_BUF_LEN;
                //remove cit cmd crc check
                if((comm_rec_buf[r_pos + CMD_INDEX])<CMD_CIT_START || (comm_rec_buf[r_pos + CMD_INDEX])>CMD_CIT_END)
                {
                    return -1;
                }
            }
        }
        else//--w--r--head--len--data--
        {
            if(crc != check_crc(&comm_rec_buf[r_pos], data_len+PACK_SHELL_LEN-3))
            {
                rec_r_pos = (r_pos + 1)%COMM_REC_BUF_LEN;
                //remove cit cmd crc check
                if((comm_rec_buf[r_pos + CMD_INDEX])<CMD_CIT_START || (comm_rec_buf[r_pos + CMD_INDEX])>CMD_CIT_END)
                {
                    return -1;
                }
            }
        }

        /*
            --data--w--r--head--len
        or  --w--r--head--len--data--
        or  --data--w--r--head--len--data--
        */
        if(tail_left >= DATA_INDEX){
            if(tail_left >= data_len+PACK_SHELL_LEN){//--w--r--head--len--data--crc--tail--
                pbuf[0] = comm_rec_buf[r_pos+DES_ADDR_INDEX];  //get des address
                pbuf[1] = comm_rec_buf[r_pos+CMD_INDEX];
                memcpy(&pbuf[2], &comm_rec_buf[r_pos+DATA_INDEX], data_len);
                rec_r_pos = (r_pos+data_len+PACK_SHELL_LEN)%COMM_REC_BUF_LEN;
                return (data_len+1);
            }else{
                pbuf[0] = comm_rec_buf[r_pos+DES_ADDR_INDEX];  //get des address
                pbuf[1] = comm_rec_buf[r_pos+CMD_INDEX];
                if(r_pos+ L_LEN_INDEX + 1 == COMM_REC_BUF_LEN){//--data--w--r--head--len
                    memcpy(&pbuf[2], &comm_rec_buf[0], data_len);
                    rec_r_pos = (r_pos+data_len+PACK_SHELL_LEN)%COMM_REC_BUF_LEN;
                    return (data_len+1);
                }else{//--data--w--r--head--len--data--
                    if(data_len != 0){
                        uint16_t part_len;
                        part_len = tail_left-DATA_INDEX;

                        if(part_len >= data_len){//--w--r--head--len--data--
                            memcpy(&pbuf[2], &comm_rec_buf[r_pos+DATA_INDEX], data_len);
                        }else{
                            memcpy(&pbuf[2], &comm_rec_buf[r_pos+DATA_INDEX], part_len);
                            memcpy(&pbuf[2+part_len], &comm_rec_buf[0], data_len-part_len);
                        }
                    }
                    rec_r_pos = (r_pos+data_len+PACK_SHELL_LEN)%COMM_REC_BUF_LEN;
                    return (data_len+1);
                }
            }
        }else{
            uint16_t data_index = (r_pos+DATA_INDEX)%COMM_REC_BUF_LEN;

            pbuf[0] = comm_rec_buf[(r_pos+DES_ADDR_INDEX)%COMM_REC_BUF_LEN];  //get des address
            pbuf[1] = comm_rec_buf[(r_pos+CMD_INDEX)%COMM_REC_BUF_LEN];
            memcpy(&pbuf[2], &comm_rec_buf[data_index], data_len);
            rec_r_pos = (r_pos+data_len+PACK_SHELL_LEN-1)%COMM_REC_BUF_LEN+1;
            return data_len+1;
        }
    }
}

/*************************************************************************************************
  * @brief    : parse single command
  * @param1   : uint8_t: command ID
  * @param3   : uint8_t *: the pointer to data which included in command
  * @param4   : uint16_t: data length
  * @return   : void
*************************************************************************************************/
void parse_cmd(uint8_t cmd, uint8_t *pdata, uint16_t len)
{
    if(cmd>=CMD_COMM_START && cmd<=CMD_COMM_END){
        //parse common cmd
        switch(cmd)
        {
            case CMD_GET_CHALLENGE:{
                get_challenge();
                break;
            }
            case CMD_SIG_VERIFY:{
                if(comm_lock == 0)
                {
                    LOGD("already unlocked!");
                    break;
                }
                boot_record_t *brt = get_boot_record_from_ram();
                if(sig_verify(pdata, len)){
                    comm_lock = 0;
                    log_mask = LOG_ALL;
                    LOGD("unlocked");

                    brt->comm_lock_flag = COMM_UNLOCK_FLAG;
                    update_data_flash(BOOT_RECORD, INVALID);
                }
                respond_usr_cmd(CMD_SIG_VERIFY, &comm_lock, 1);
                break;
            }
            default:
                break;
        }
    }else if((cmd>=CMD_USR_START && cmd<=CMD_USR_END) ||
        (cmd>=CMD_USR_EX_START && cmd<=CMD_USR_EX_END)){
        //parse user cmd
        if(cmd == USR_CMD_VER)
        {
            get_sw_version();
        }
        if(!get_comm_lock())
        {
            if(cmd == USR_CMD_HEAT_PROFILE){//update heat profile
                respond_usr_cmd(USR_CMD_HEAT_PROFILE, NULL, 0);
                app_parse_heat_profile_cmd(pdata, len);
            //modify LOG mask
            }else if(cmd == USR_CMD_LOG_MASK){
                respond_usr_cmd(USR_CMD_LOG_MASK, NULL, 0);
                LOGD("set log mask pdata[0]=0x%02x\r\n", pdata[0]);
                log_mask = pdata[0];
                LOGD("set log mask log_mask=0x%02x\r\n", log_mask);
            }else{
                parse_usr_cmd(cmd, pdata, len);
            }
        }
    }else if(cmd>=CMD_CIT_START && cmd<=CMD_CIT_END){
        //parse cit cmd
        parse_cit_cmd(cmd, pdata, len);
    }else if(cmd>=CMD_UPDATE_START && cmd<=CMD_UPDATE_END){
        //parse update cmd
        if(cmd == UPDATE_FLAG_1P){
             parse_update_cmd(cmd);
        }
        else{
            parse_image_update_cmd(cmd, pdata, len);
        }
    }
}

/*************************************************************************************************
  * @brief    : send command stored in comm_send_buf by uart
  * @return   : void
*************************************************************************************************/
void comm_send_proc(void)
{
    if(cur_pos != 0 && UartStatusCheck()==SET){
        if(xSemaphoreTake(uart_buffer_lock, portMAX_DELAY)){
            //there is data in the buffer needs to be sent and uart is not busy
            memcpy(dma_buf, comm_send_buf, cur_pos);
            /*uart DMA send*/
            UartSendDma((char *)dma_buf, cur_pos);
            memset(comm_send_buf, 0, COMM_SEND_BUF_LEN);
            cur_pos = 0;
            xSemaphoreGive(uart_buffer_lock);
        }
    }
}

void comm_init(void)
{
    hashbuf_random_init();
}
uint8_t data[MAX_DATA_LEN + PACK_SHELL_LEN]; //include des and cmd byte
/*************************************************************************************************
  * @brief    : command task function
  * @return   : void
*************************************************************************************************/
void comm_task(void)
{
    int16_t len;
    //uint8_t data[MAX_DATA_LEN + PACK_SHELL_LEN]; //include des and cmd byte

    //send commands through uart
    comm_send_proc();

    //get one legal command from uart buffer
    len = get_one_cmd(data);
    if(len>0){
        //comm_send(data[0], PC_ADDR, &data[1], len-1); //for debug
        TxUartEnable();
        if((data[0]&0x0F) == 0x01){
            parse_cmd(data[1], &data[2], len-1);
        }
    }
}



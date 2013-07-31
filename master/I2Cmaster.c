#include <util/twi.h>
#define SET(x,y) (x|=(1<<y))
#define CLR(x,y) (x&=(~(1<<y)))
#define CHK(x,y) (x&(1<<y)) 
#define TOG(x,y) (x^=(1<<y))

//global variables
#define BUFLEN_RECV 3
uint8_t r_index =0;
uint8_t recv[BUFLEN_RECV]; //buffer to store received bytes

#define BUFLEN_TRAN 12
uint8_t t_index=0;
uint8_t tran[BUFLEN_TRAN]= {1,2,3,4,5,6,7,8,9,10,11,12};
//variable to indicate if something went horribly wrong
uint8_t reset=0;

//prototypes
void handleI2C_master();


//---------------MAIN---------------------------------------------
int main(){
  //set bitrate for I2C
 TWBR = 10;
 //enable I2C hardware
 TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWSTA);

 while(1){
  handleI2C_master();
 }
}
//-----------END MAIN---------------------------------------------

//setup the I2C hardware to ACK the next transmission
//and indicate that we've handled the last one.
#define TWACK (TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWEA))
//setup the I2C hardware to NACK the next transmission
#define TWNACK (TWCR=(1<<TWINT)|(1<<TWEN))
//reset the I2C hardware (used when the bus is in a illegal state)
#define TWRESET (TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO)|(1<<TWEA))
//Send a start signal on the I2C bus
#define TWSTART (TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTA))
//slave address and SLA signals
#define I2C_SLAVE_ADDRESS 0x01
#define SLA_W ((I2C_SLAVE_ADDRESS<<1) | TW_WRITE)
#define SLA_R ((I2C_SLAVE_ADDRESS<<1) | TW_READ)

void handleI2C_master(){
  //keep track of the modus (receiver or transmitter)
	static uint8_t mode; 
 
 if(CHK(TWCR,TWINT)){
    switch(TW_STATUS){
   //start or rep start send, determine mode and send SLA R or W
    case 0x10:
    case 0x08: 
   //reset buffer indices
   t_index =0;
   r_index =0;
   //send SLA W or R depending on what mode we want.
   if(mode == TW_WRITE) TWDR = SLA_W;
   else TWDR = SLA_R;
   TWACK;
   break;
//--------------- Master transmitter mode-------------------------
  case 0x18: // SLA_W acked
   //load first data
   TWDR = tran[0];
   t_index=1;
   TWACK;
   break;

	//SLA_W not acked for some reason (disconnected?), keep trying
	case 0x20: 
   TWCR =0;
      TWSTART;
   break;
  case 0x28: //data acked by addressed receiver
   //load next byte if we're not at the end of the buffer
   if(t_index < BUFLEN_TRAN){
    TWDR =  tran[t_index];
    t_index++;
    TWACK;
    break;
   }
   //otherwise, switch mode and send a start signal
   else {
    mode = TW_READ;
    TWSTART;
    break;
   }
  case 0x38: //arbitration lost, do not want
	//data nacked, could be faulty buffer, could be dc, start over
  case 0x30: 
      TWCR = 0;
      TWSTART;
      break;
//-------------------------Master receiver mode-------------------
 //SLA_R acked, nothing to do for master, just wait for data
  case 0x40:
   TWACK;
   break;
  //SLA_R not acked, something went wrong, start over
	case 0x48:
      TWSTART;
      break;
  //non-last data acked (the last data byte has to be nacked)
	case 0x50:
   //store it
   recv[r_index] = TWDR;
   r_index++;
   //if the next byte is not the last, ack the next received byte
   if(r_index < BUFLEN_RECV){
    TWACK;
   }
   //otherwise NACK the next byte
   else {
    TWNACK;
    r_index =BUFLEN_RECV;
   }
   break;
  case 0x58: //last data nacked, as it should be
   //switch to other mode, and send start signal
   mode = TW_WRITE;
   TWSTART;
   break;
   
//--------------------- bus error---------------------------------
    case 0x00:
      TWRESET;
      ;
      TWSTART;
      break;
    }
  }
}

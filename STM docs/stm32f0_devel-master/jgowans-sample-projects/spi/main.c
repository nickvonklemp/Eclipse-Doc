#define STM32F051

#include <stdint.h>
#include "stm32f0xx.h"

/*
Properties of the SPI EEPROM are:
fmax = 10 MHz
SPI modes (0,0) and (1,1) are supported
Mode 0,0: clock idles low, data clocked in rising edge
The device must be enabled for writing by setting bits in the status register
Read sequence: Instruction ; 16 bit address; data starts coming out
Write sequence: Instruction; 16 bit address; supply up tp 32 bytes of data
Status register bits:
  7:WPEN. Non-volatile. When set, WP pin is respected. 
  6,5,4: x
  3: BP1: block 1 protect. Non-volatile.
  2: BP0: block 0 protect. Non-volatile.
    00: no protect. 01: upper 1/4. 10: upper half. 11: all protected 
  1: WEL: write enable latch: Read only. Set when writes to SR prohibited.
  0: WIP: write in progress. Read only. Set when write in progress.
Write enable is default RESET!!
After a write, write enable is is again reset!! :-(
CS must be held high for 5 ms after a write instruction to trigger the write.
*/
#define MEM_SIZE 8192 // bytes
#define WREN 0b00000110 // enable writing
#define WRDI 0b00000100 // disable writing
#define RDSR 0b00000101 // read status register
#define WRSR 0b00000001 // write status register
#define READ 0b00000011
#define WRITE 0b00000010

void main(void);
static void init_spi(void);
static void init_leds(void);
static void write_to_address(uint16_t address, uint8_t data);
static uint8_t read_from_address(uint16_t address);
static void delay(uint32_t delay_in_us);

static uint8_t test_pattern[] = {0xAA, 0x42, 0xF0, 0x01, 0x10};

void main(void) {
  init_leds();
  init_spi();
  // write all bytes to EEPROM
  for (uint32_t pos = 0; pos < sizeof(test_pattern); pos++) {
    write_to_address(pos, test_pattern[pos]);
  }
  GPIOB->BSRR = 0xAA;
  // output 0xAA if correct, else 0x01
  for (uint32_t pos = 0; pos < sizeof(test_pattern); pos++) {
    uint8_t data_in = read_from_address(pos);
    if  (data_in != test_pattern[pos]) {
      GPIOB->BSRR = (0xF0 << 16); //reset the upper nibble
    }
  }
  for(;;);
}

static void write_to_address(uint16_t address, uint8_t data) {
  uint8_t dummy; // a variable which will be used to pull junk from the DR

  // first, set the Write Enable latch
  GPIOB->BSRR |= GPIO_BSRR_BR_12; // pull CS low
  delay(1);
  *((uint8_t*)(&SPI2->DR)) = WREN;
  while ((SPI2->SR & SPI_SR_RXNE) == 0); // hang while RX is empty
  dummy = SPI2->DR;
  GPIOB->BSRR |= GPIO_BSRR_BS_12; // pull CS high
  delay(5000);

  // send the write instruction
  GPIOB->BSRR |= GPIO_BSRR_BR_12; // pull CS low
  delay(1);
  *((uint8_t*)(&SPI2->DR)) = WRITE;
  while ((SPI2->SR & SPI_SR_RXNE) == 0); // hang while RX is empty
  dummy = SPI2->DR;
  // send a 16 bit address
  *((uint8_t*)(&SPI2->DR)) = (address >> 8); // address MSB
  while ((SPI2->SR & SPI_SR_RXNE) == 0); // hang while RX is empty
  dummy = SPI2->DR;
  *((uint8_t*)(&SPI2->DR)) = (address); // address LSB
  while ((SPI2->SR & SPI_SR_RXNE) == 0); // hang while RX is empty
  dummy = SPI2->DR;
  // send the data
  *((uint8_t*)(&SPI2->DR)) = data;
  while ((SPI2->SR & SPI_SR_RXNE) == 0); // hang while RX is empty
  dummy = SPI2->DR;
  GPIOB->BSRR |= GPIO_BSRR_BS_12; // pull CS high
  delay(5000);
}

static uint8_t read_from_address(uint16_t address) {
  uint8_t dummy; // a variable which will be used to pull junk from the DR
  // send the read instruction
  GPIOB->BSRR |= GPIO_BSRR_BR_12; // pull CS low
  delay(1);
  *((uint8_t*)(&SPI2->DR)) = READ;
  while ((SPI2->SR & SPI_SR_RXNE) == 0); // hang while RX is empty
  dummy = SPI2->DR;
  // send a 16 bit address
  *((uint8_t*)(&SPI2->DR)) = (address >> 8); // address MSB
  while ((SPI2->SR & SPI_SR_RXNE) == 0); // hang while RX is empty
  dummy = SPI2->DR;
  *((uint8_t*)(&SPI2->DR)) = (address); // address LSB
  while ((SPI2->SR & SPI_SR_RXNE) == 0); // hang while RX is empty
  dummy = SPI2->DR;
  // clock in the data
  *((uint8_t*)(&SPI2->DR)) = 0x42; // clock out some junk data
  while ((SPI2->SR & SPI_SR_RXNE) == 0); // hang while RX is empty
  dummy = SPI2->DR;
  GPIOB->BSRR |= GPIO_BSRR_BS_12; // pull CS high
  delay(5000);
  return dummy;
}

static void init_spi(void) {
  // clock to PB
  RCC->AHBENR |= RCC_AHBENR_GPIOBEN; //enable clock for SPI port
  // no need to map pins to the perpiheral - they are AF0
  GPIOB->MODER |= GPIO_MODER_MODER13_1; // set pin SCK (PB13) to Alternate Function
  GPIOB->MODER |= GPIO_MODER_MODER14_1; // set pin MISO (PB14) to Alternate Function
  GPIOB->MODER |= GPIO_MODER_MODER15_1; // set pin MOSI (PB15) to Alternate Function
  GPIOB->MODER |= GPIO_MODER_MODER12_0; // set pin CS (PB12) to output push-pull
  GPIOB->BSRR |= GPIO_BSRR_BS_12; // pull CS high
  
  // clock enable to SPI2
  RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
  SPI2->CR1 |= SPI_CR1_BIDIOE; // enable output
  SPI2->CR1 |= (SPI_CR1_BR_0 |  SPI_CR1_BR_1); // set baud to fpclk / 16
  SPI2->CR1 |= SPI_CR1_MSTR; // set to master mode
  SPI2->CR2 |= SPI_CR2_FRXTH; // set RX threshold to be 8 bits
  SPI2->CR2 |= SPI_CR2_SSOE; // enable slave output to work in master mode
  SPI2->CR2 |= (SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2); // set to 8bit mode
  SPI2->CR1 |= SPI_CR1_SPE; // enable the SPI peripheral
}

static void init_leds(void) {
  RCC->AHBENR |= RCC_AHBENR_GPIOBEN; //enable clock for LEDs
  GPIOB->MODER |= GPIO_MODER_MODER0_0; //set PB0 to output
  GPIOB->MODER |= GPIO_MODER_MODER1_0; //set PB1 to output
  GPIOB->MODER |= GPIO_MODER_MODER2_0; //set PB2 to output
  GPIOB->MODER |= GPIO_MODER_MODER3_0; //set PB3 to output
  GPIOB->MODER |= GPIO_MODER_MODER4_0; //set PB4 to output
  GPIOB->MODER |= GPIO_MODER_MODER5_0; //set PB5 to output
  GPIOB->MODER |= GPIO_MODER_MODER6_0; //set PB6 to output
  GPIOB->MODER |= GPIO_MODER_MODER7_0; //set PB7 to output
}

static void delay(uint32_t delay_in_us) {
  /* Hangs for specified number of microseconds. */
  volatile uint32_t counter = 0;
  delay_in_us *= 3;
  for(; counter < delay_in_us; counter++) {
    __asm("nop");
    __asm("nop");
  }
}


// Wokwi RDM6300 Custom Chip
// wokwi-rdm6300-custom-chip
//
// SPDX-License-Identifier: MIT
// Copyright (C) 2023 Edu Santos (github.com/DjEdu28)
// Copyright (C) 2022 Uri Shaked / wokwi.com
// 
// RFID 125Khz RDM6300 Card Reader Module
// DATASHEET https://cdn.awsli.com.br/945/945993/arquivos/RDM630-Spec.pdf
// 
// Wokwi Custom Chip - For docs and examples see:
// https://docs.wokwi.com/chips-api/getting-started
//
/*************************************************************************
 * Programa para simular Custom Chip do Módulo RFID RDM6300 no WOkWI
 * Autor: Eduardo Santos (DjEdu28)
 * Repositório: https://github.com/djedu28/wokwi-rdm6300-custom-chip
 * Docs: https://github.com/djedu28/wokwi-rdm6300-custom-chip/tree/main/docs
 * Ultima alteração em: 05/11/2024
 * Versão: 1.28.0
**************************************************************************/

#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>
//#include <string.h> // Para usar strlen
#include <stdint.h>

#define GETCHIPSTATEFROM(usr_data) chip_state_t* chip = (chip_state_t*)usr_data

typedef struct {
  uart_dev_t uart0;

  uint8_t value;
  
  uint8_t manual_value;
  uint8_t manual_value_attribute; 

  uint8_t card1; 
  uint8_t card2; 
} chip_state_t;

static void on_uart_rx_data(void *user_data, uint8_t byte);
static void on_uart_write_done(void *user_data);

static void on_uart_rx_data(void *user_data, uint8_t byte) {
  chip_state_t *chip = (chip_state_t*)user_data;
  printf("RDM6300: Dados recebidos: %d\n", byte);
}

static void on_uart_write_done(void *user_data) {
  chip_state_t *chip = (chip_state_t*)user_data;
  printf("RDM6300: concluído\n");
}

// Função para calcular o checksum usando XOR dos bytes do ID da tag
uint8_t calculate_checksum(const char *data) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < 10; i++) {
      const char ed = data[i];
      // printf("%02X ", ed);
      uint8_t value_ = strtol(&ed, NULL, 16);
      checksum ^= value_;
    }
    printf("RDM6300: checksum = %02X\n", checksum);
    return checksum;
}

void manual_value_changed(void* user_data) {
  GETCHIPSTATEFROM(user_data);
  
  uint8_t curVal = attr_read(chip->manual_value_attribute);
  uint8_t card1 = attr_read(chip->card1);
  uint8_t card2 = attr_read(chip->card2);
  
  if (curVal != chip->manual_value) {

    printf("RDM6300: atualizado para proximo=%i, card1=%i, card2=%i\n", curVal, card1, card2);

    chip->manual_value = curVal;
    chip->value = curVal;

    if (curVal>=1){
      chip_state_t *chip = (chip_state_t*)user_data;

      char *data = "62E3086CED"; // ID da tag1
      
      if (card2 && !card1){
        printf("RDM6300: card2 <<\n");
        data = "62EDEDEDED"; // ID da tag2
      }

      uint8_t checksum = calculate_checksum(data); // Calcula o checksum usando XOR dos bytes da versão + ID da tag
    
      // Definindo os elementos do pacote
      uint8_t head = 0x02;
      uint8_t end = 0x03;

      // O tamanho dos dados
      size_t data_size = 13;//strlen(data_out); // 13
      
      // Buffer para armazenar o data_out completo
      // Formato do pacote: Byte de início (head)(1 bytes), Versão (version)(2 bytes), ID da tag (8 bytes), Checksum (1 bytes), Byte de fim (end)
      char data_out[1+2+8+1+1];

      data_out[0] = head;
      // Convertendo o ID da tag de hexadecimal para bytes
      for (int i = 0; i <= 10; i++) {
          data_out[i+1] = data[i];//strtol(data + i, NULL, 16);
      }
      // Adicionando o checksum e o byte final
      //uint8_t *checksum_bytes = (uint8_t*)&checksum;
      data_out[11] = checksum; // Primeiro byte do checksum
      data_out[12] = end;
      
      printf("RDM6300: Data Out: '%13s'\n", data_out);
      // Enviando data_out através do UART
      bool success = uart_write(chip->uart0, (uint8_t*)data_out, data_size);
      if (success) {
        printf("RDM6300: Dados enviados com sucesso.\n");
      } else {
        printf("RDM6300: Falha ao enviados dados.\n");
      }
    }
  }

}

void chip_init(void) {
  chip_state_t *chip = malloc(sizeof(chip_state_t));

  const uart_config_t uart_config = {
    .tx = pin_init("TX", INPUT_PULLUP),
    .rx = pin_init("RX", INPUT),
    .baud_rate = 9600,
    .rx_data = on_uart_rx_data,
    .write_done = on_uart_write_done,
    .user_data = chip,
  };
  chip->uart0 = uart_init(&uart_config);

  chip->manual_value_attribute =  attr_init("haveCard", 0);

  chip->card1 =  attr_init("card1", 1);
  chip->card2 =  attr_init("card2", 0);

  const timer_config_t valuewatchtimerconf = {
    .callback = manual_value_changed,
    .user_data = chip,
  };
  timer_t valwatchtimer = timer_init(&valuewatchtimerconf);
  timer_start(valwatchtimer, 5000, true);

  printf("RDM6300: Chip inicializado!\n");

}

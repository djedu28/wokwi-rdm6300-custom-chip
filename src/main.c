
// Wokwi RDM6300 Custom Chip
// wokwi-rdm6300-custom-chip
//
// Wokwi Custom Chip - For docs and examples see:
// https://docs.wokwi.com/chips-api/getting-started
//
// SPDX-License-Identifier: MIT
// Copyright (C) 2023 Edu Santos (github.com/DjEdu28)
// Copyright (C) 2022 Uri Shaked / wokwi.com
// 
// For information and examples see:
// https://docs.wokwi.com/pt-BR/chips-api/uart

#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Para usar strlen


#define GETCHIPSTATEFROM(usr_data) chip_state_t* chip = (chip_state_t*)usr_data

typedef struct {
  uart_dev_t uart0;

  uint8_t value;
  
  uint8_t manual_value;
  string_t manual_value_attribute; 
} chip_state_t;

static void on_uart_rx_data(void *user_data, uint8_t byte);
static void on_uart_write_done(void *user_data);

static void on_uart_rx_data(void *user_data, uint8_t byte) {
  chip_state_t *chip = (chip_state_t*)user_data;
  printf("RDM6300: Dados recebidos: %d\n", byte);
  //uint8_t data_out = rot13(byte);
  //uart_write(chip->uart0, &data_out, sizeof(data_out));
}

static void on_uart_write_done(void *user_data) {
  chip_state_t *chip = (chip_state_t*)user_data;
  printf("RDM6300: concluído\n");
}

void manual_value_changed(void* user_data) {
  GETCHIPSTATEFROM(user_data);
  
  uint8_t curVal = attr_read(chip->manual_value_attribute);
  if (curVal != chip->manual_value) {

    printf("RDM6300: atualizado para %i\n", curVal);

    chip->manual_value = curVal;
    chip->value = curVal;

    if (curVal>=1){
      chip_state_t *chip = (chip_state_t*)user_data;

      // Definindo data_out como uma string_t "ABCDEF"
      char data_out[] = "ABCDEF";
      
      // Calculando o tamanho dos dados
      size_t data_size = strlen(data_out);

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

  const timer_config_t valuewatchtimerconf = {
    .callback = manual_value_changed,
    .user_data = chip,
  };
  timer_t valwatchtimer = timer_init(&valuewatchtimerconf);
  timer_start(valwatchtimer, 5000, true);

  printf("RDM6300: Chip inicializado!\n");

}



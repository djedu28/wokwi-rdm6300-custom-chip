// Wokwi RDM6300 Custom Chip
// SPDX-License-Identifier: MIT
// Copyright (C) 2023~2024 Edu Santos (https://github.com/DjEdu28)

/******************************************************************************************************
   Programa exemplo para uso Custom Chip Módulo RFID RDM630/6300 no WOkWI
   Autor: Eduardo Santos (DjEdu28)
   Repositório: https://github.com/djedu28/wokwi-rdm6300-custom-chip
   Ultima alteração em: 05/11/2024
*                                                                                                     *
   Objetivo: Usar um módulo RFID para comunicar com o Arduino
   Bibliotecas usadas:
    LiquidCrystal.h - http://arduino.cc/en/Reference/LiquidCrystal
*******************************************************************************************************/

/******************************************************************************************************
   Informação sobre o programa:
*                                                                                                     *
   Pinos usados para o LCD:
     12 -> RS do LCD
     11 -> EN do LCD
     10 -> D4 do LCD
     9 -> D5 do LCD
     8 -> D6 do LCD
     7 -> D7 do LCD
     Você deve alimentar o LCD
     Ligar um Potenciômetro/trimpot ao pino VO do LCD para controlar o contraste
     Ligar o R/W do LCD ao GND
     Ligar os pinos VSS, K do LCD ao GND
*                                                                                                     *
   Pinos usados para o Módulo RFID RDM630:
     0(RX) -> TX do RFID  (Desconectar o RFID do arduino quando for fazer o upload)
       Você deve alimentar o módulo RFID: Ligue VCC ao Pino 5V e o GND ao GND
     PS: O código pode ser alterado para usar SoftSerial, 
     com SoftSerial não será necessário desconectar o modulo para carregar novo código
*                                                                                                     *
   Pinos usados para os LED´s
     6 -> Conectar ao LED vermelho
     5 -> Conectar ao LED verde
*******************************************************************************************************/

/******************************************************************************************************
  Funcionamento do programa:
    Quando aproximado o cartão, a tag é exibida no display LCD
    se der sucesso na leitura o led verde é ligado.
    se der falha na leitura o led vermelho é ligado.

*******************************************************************************************************/
//Para melhores informações sobre o módulo RFID, consulte o datasheet

//Para informações sobre a Biblioteca LiquidCrystal.h, vá na guia Help/Reference/Libraries/LiquidCrystal
#include <LiquidCrystal.h> //Incluindo a biblioteca para acionamento do LCD 16x2
//Rotina para mostrar o ID no LCD, se você não tem um LCD, retire esta parte ou mude para a porta serial
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);//Configurando os pinos que estão conectados ao LCD
//2 -> RS do LCD, 4 -> EN do LCD, 7 -> D4 do LCD, 8 -> D5 do LCD, 12 -> D6 do LCD, 13 -> D7 do LCD

//Definição dos LEDs
int vermelho = 6;//Pino que será ligado o LED vermelho
int verde = 5;//Pino que será ligado o LED verde

byte id[1+1+10+1];//Armazena o ID lido pela serial
byte CHECKSUM = 0;
byte TAG[10];

//Rotina de configurações iniciais do programa
void setup() {
  pinMode(vermelho, OUTPUT);//Configurando o pino que será conectado ao LED vermelho como saída
  pinMode(verde, OUTPUT);//Configurando o pino que será conectado ao LED verde como saída

  lcd.begin(16, 2);//Inicializa a biblioteca para trabalhar com display de 16 caracteres por 2 linhas (16x2)
  Serial.begin(9600);//Inicializando a comunicação serial com velocidade de 9600 baud rate (Velocidade padrão do módulo)

  lcd.setCursor(0, 0);//Aponta para o primeiro carácter da primeira linha
  lcd.print("ID:");//Escreve o texto

  lcd.setCursor(0, 1);//Aponta para o primeiro carácter da segunda linha
  lcd.print("TG:");//Escreve o texto
}

// Rotina que será executada infinitamente
void loop() {
  Serial.print("LOOP");
  Serial.print(" id ");
  //   EXIBE O ID LIDO
  lcd.setCursor(3, 0);
  for (int index = 0; index < 13; index++) {
    Serial.print("0x");
    Serial.print(id[index], HEX);
    Serial.print(" ");
    lcd.write(id[index]);
  }
  //   EXIBE A TAG LIDA
  Serial.print("\n tag: ");
  lcd.setCursor(3, 1);
  for (int index = 0; index < 10; index++) {
    Serial.print(TAG[index], HEX);
    Serial.print(" ");
    lcd.write(TAG[index]);
  }
  //   EXIBE O CHECKSUM
  lcd.print("(");
  lcd.print(CHECKSUM);
  lcd.print(")");
  Serial.print("\n");
  // VERIFICA SE TEM CONTEÚDO PARA LER
  if (Serial.available() >= 12) {
    Serial.readBytes(id, 13);//Lê os dados recebidos
    //Serial.readBytesUntil(0x03, id, 13);//Lê os dados recebidos
    if (id[0]==0x02 && id[12]==0x03){
      digitalWrite(verde, HIGH);digitalWrite(vermelho, LOW);
      Serial.println("OK");
      for (int index = 0; index < 10; index++) {
        TAG[index] = id[index+1];
      }
      CHECKSUM = id[11];
      Serial.flush();
    } else {
      digitalWrite(vermelho, HIGH);digitalWrite(verde, LOW);
      Serial.print("FALHA");
      Serial.print("id->0");
      Serial.print(id[0]);
      Serial.print("id->12");
      Serial.println(id[12]);
    }
  }
}
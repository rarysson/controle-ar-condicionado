#include <LiquidCrystal.h>
#include <IRremote.h>



//LCD
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);
//Sensor temperatura
int pin_temp = A0;
int valor_temp;
float R1 = 10000; //valor do resistor da placa
float logR2, R2, temperatura_ambiente;
float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741; //steinhart-hart coeficients for thermistor
//Rotatory Encoder
int inputCLK = 8;
int inputDT = 9;
int estado_atual_clk;
int estado_anterior_clk;
//essas variáveis é para criar um contador
//onde vai evitar de mudar de temperatura com pouca rotação do encoder
int volta_positiva = 0;
int volta_negativa = 0;
int rotacoes = 8;
//Emissor IR
IRsend emissor_IR;
//Outras variaveis
int temperatura_escolhida = 20;
unsigned long codigo_temperatura = 4294967295; //esse código indica a temperatura máxima



void diminuir_temperatura() {
    temperatura_escolhida--;

    if (temperatura_escolhida < 18) {
        temperatura_escolhida = 18; //o valor é 18 devido ao AC escolhido
    }
}

void aumentar_temperatura() {
    temperatura_escolhida++;

    if (temperatura_escolhida > 30) {
        temperatura_escolhida = 30; //o valor é 30 devido ao AC escolhido
    }
}

void setup() {
    lcd.begin(16, 2);

    pinMode(inputCLK,INPUT);
    pinMode(inputDT,INPUT);
    estado_anterior_clk = digitalRead(inputCLK);
    
    Serial.begin(9600);
    Serial.println("AJUSTE A TEMPERATURA QUE VOCE QUER");
    Serial.println("Quando quiser começar a monitorar envie qualquer coisa pela Serial");
}

void loop() {
    lcd.setCursor(0, 0);
    lcd.print("Temperatura");
    lcd.setCursor(0, 1);
    lcd.print(temperatura_escolhida);
    lcd.print(" graus");

    estado_atual_clk = digitalRead(inputCLK);

    if (estado_atual_clk != estado_anterior_clk) {
        //giro no sentido antihorário
        if (digitalRead(inputDT) != estado_atual_clk) {
            volta_negativa++;

            if ((volta_negativa % rotacoes) == 0) {
                diminuir_temperatura();
            }
        } else { //giro no sentido horário
            volta_positiva++;

            if ((volta_positiva % rotacoes) == 0) {
                aumentar_temperatura();
            }
        }
    }

    estado_anterior_clk = estado_atual_clk;

    if (Serial.read() != -1) {
        Serial.println("MONITORANDO");
        lcd.setCursor(0, 0);
        lcd.print("Tmp. Ambnt.");

        Serial.println("DIGITE pare PARA PARAR DE MONITORAR");

        //o passo_quente/_frio é só para ter a certeza que foi requisitado duas mudanças de temperatura
        //pois ao ler os códigos IR, percebi que o mesmo código era enviado para duas temperaturas diferentes
        int passo_quente = 0;
        int passo_frio = 0;

        while (true) {
            String texto = Serial.readString();

            if (texto.compareTo(String("pare\n")) == 0) {
                Serial.println("PARANDO DE MONITORAR");
                break;
            }

            //cálculo da equação de Steinhart–Hart
            valor_temp = analogRead(pin_temp);
            R2 = R1 * (1023.0 / (float)valor_temp - 1.0); //calculate resistance on thermistor
            logR2 = log(R2);
            temperatura_ambiente = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));//temperature in Kelvin
            temperatura_ambiente = temperatura_ambiente - 273.15; //convert Kelvin to Celcius
            int tmp_amb = temperatura_ambiente;
            
            lcd.setCursor(0, 1);
            lcd.print(tmp_amb);
            lcd.print(" graus");

            if (tmp_amb > temperatura_escolhida) {
                passo_quente++;

                if ((passo_frio % 2) == 0) {
                    codigo_temperatura--;
                    emissor_IR.sendNEC(codigo_temperatura, 32);
                }

                delay(1000 * 20);
            } else if (tmp_amb < temperatura_escolhida) {
                passo_frio++;

                if ((passo_frio % 2) == 0) {
                    codigo_temperatura++;
                    emissor_IR.sendNEC(codigo_temperatura, 32);
                }

                delay(1000 * 20);
            } else {
                delay(1000 * 60);
            }
        }        
    }
}

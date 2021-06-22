#include <EEPROM.h>

//Variables para los pines del arduino
const int motor1pin1 = 2;
const int motor1pin2 = 3;
const int motor2pin1 = 4;
const int motor2pin2 = 5;
const int ultrasonico_trig = 11;
const int ultrasonico_eco = 10;

//Constantes para el programa que modificadas podrian dar mejores mediciones o peores
const int cantidad_de_lados = 4;
const int cantidad_de_datos_recopilados = 100;
const int distancia_para_el_giro = 30;
const bool condicion_test_impresion_datos = false;

int contador = 0;
int condicion = true;
float datos[cantidad_de_lados][cantidad_de_datos_recopilados];
float lados[cantidad_de_lados];

//Variable creada por problemas
int contador_correccion_de_la_llanta = 1;

/*Funciones para el movimiento de los motores*/
//Funcion para encender los motores durante un periodo
void motor_encendido(int contador_correccion_de_la_llanta){
  digitalWrite(motor2pin1,HIGH);
  digitalWrite(motor2pin2,LOW);
  digitalWrite(motor1pin1,HIGH);
  digitalWrite(motor1pin2,LOW);
  delay(900);
  if (contador_correccion_de_la_llanta % 2 == 0)
    correccion_de_la_llanta();
}
//Funcion para apagar el motor
void motor_apagado(){
  digitalWrite(motor2pin1,LOW);
  digitalWrite(motor2pin2,LOW);
  digitalWrite(motor1pin1,LOW);
  digitalWrite(motor1pin2,LOW);
  delay(5000);
}
//Funcion para girar a la izquierda en caso de acortarse la distancia
void giro_izquiera(){
  digitalWrite(motor1pin1,HIGH);
  digitalWrite(motor1pin2,LOW);
  digitalWrite(motor2pin1,LOW);
  digitalWrite(motor2pin2,LOW);
  delay(2000);
}
//Funcion correctiva para fallo en el dispositivo(fallo en una rueda)
void correccion_de_la_llanta(){
  digitalWrite(motor2pin1,HIGH);
  digitalWrite(motor2pin2,LOW);
  digitalWrite(motor1pin1,LOW);
  digitalWrite(motor1pin2,HIGH);
  delay(100);
}
//Funcion para usar el ultrasonico, ya que posee condiciones para emplear
float ultrasonico() {
  digitalWrite(ultrasonico_trig, HIGH);
  delay(1);
  digitalWrite(ultrasonico_trig, LOW);
  float duracion = pulseIn(ultrasonico_eco, HIGH);
  float distancia = duracion / 58.2;
  return distancia;
}

//Funcion opcional para trabajar con la memoria integrada
int eeprom(int numero_indice, int dato=0,bool condicion = true){
  if(condicion)
    EEPROM.write(numero_indice, dato);
  else
    return EEPROM.read(numero_indice);
}

/*
Funciones de logistica para el trabajo de los datos.
Se implementaron funciones para almacenar en el arduino los datos de los lados y las medidas tomadas por el sensor ultrasonico
*/

//Funcion que permite almacenar los datos de los lados en la memoria del arduino con la libreria EEPROM
void guardado_de_los_lados(int contador){
  Serial.println("Guardando los datos");
  float media=0; 
  for(int i=0; i<cantidad_de_datos_recopilados; i++){
      datos[contador][i] = ultrasonico() + distancia_para_el_giro;
      media += datos[contador][i];
    }
  lados[contador] = media/cantidad_de_datos_recopilados;

  eeprom(contador, lados[contador]);
  Serial.println("Datos guardos");
}

//Funcion que permite almacenar los datos obtenidos por el sensor ultrasonico en la memoria, unado comprueba si la cantidad de lados obtenidos 
bool guardado_de_datos(int contador){
  if (contador < cantidad_de_lados){
    guardado_de_los_lados(contador);
    return false;
  }
  else{
    Serial.println("Los lados son: ");
    for(int i=0; i<cantidad_de_lados; i++){
      Serial.print("\tLado ");
      Serial.print(i+1);
      Serial.print(" valor: ");
      Serial.print(lados[i]);
      Serial.print(" cm ");
        for(int k=0; k<cantidad_de_datos_recopilados; k++){
          eeprom(i+cantidad_de_lados, datos[i][k]);
        }
      }
     Serial.println();
      return true;
    }
}

void impresion_de_datos(bool condicion_eeprom = false){
  Serial.println();
  if(condicion_eeprom==false){
    for(int l=0; l<cantidad_de_lados;l++){
      for(int g=0; g<cantidad_de_datos_recopilados;g++){
        Serial.print("Lado ");
        Serial.print(l+1);
        Serial.print(" No. de toma ");
        Serial.print(g+1);
        Serial.print(".- El valor es: ");
        Serial.print(datos[l][g]);
        Serial.print(",");
        Serial.println(".");
      }
    }
  }
  else{
    for (int i=0; i<((cantidad_de_datos_recopilados*cantidad_de_lados)+cantidad_de_lados); i++){
      Serial.print(i+1);
      Serial.print(".- El valor es: ");
      Serial.print(eeprom(i,NULL,false));      
      Serial.print(",");
      Serial.println(".");
    }
  }
}

//Se inicializan los pines del arduino mediante la funcion setup que ejecuta una sola vez este codigo
void setup(){
  pinMode(motor1pin1, OUTPUT);
  pinMode(motor1pin2, OUTPUT);
  pinMode(motor2pin1, OUTPUT);
  pinMode(motor2pin2, OUTPUT);
  pinMode(ultrasonico_trig, OUTPUT);
  pinMode(ultrasonico_eco, INPUT);
  pinMode(13,OUTPUT);
  Serial.begin(9600);
}

//Comienza el programa.
void loop(){
  if (ultrasonico()>0){
    if (condicion){
      Serial.println("Motores apagagos");
      motor_apagado();  
      digitalWrite(13,LOW);
      delay(1000);
      condicion = guardado_de_datos(contador);
      contador++;
      }
    else{
      if (ultrasonico()<distancia_para_el_giro){
        motor_apagado();
        Serial.println("girando");
        giro_izquiera();
        condicion = true;
      }
      else{
        Serial.println("Avanzando");
        motor_encendido(contador_correccion_de_la_llanta);
        contador_correccion_de_la_llanta++;
      }
    }
  }
  else{
    impresion_de_datos(condicion_test_impresion_datos);
  }
}

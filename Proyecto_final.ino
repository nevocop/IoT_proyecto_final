/***********************************************************************************************************
*                                                                                                          *
*              Proyecto final del curso Diseño de Dispositivos IoT de la Maestría en Ingeniería            *
*                                 Monitoreo de calidad de la energía                                       *
*                                                                                                          *
*                                        Nelson Barrera Tovar                                              *
*                                         Valentina Castaño                                                *
*                                          Federico Pareja                                                 *
*                                                                                                          *
*                                 Universidad Autónoma de Manizales                                        *
*                                       Maestria en Ingeniería                                             *
*                                             Manizales                                                    *
*                                                2023                                                      *
*                                                                                                          * 
***********************************************************************************************************/
//Bibliotecas necesarias para el funcionamiento del firmware
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <PZEM004Tv30.h>

//Solicitud de credenciales de la red wifi a utilizar
#define ssid "FABABE"
#define password "T9xz362021"

//Definición de los pines de comunicacion del módulo PZEM-004T
#define PZEM_RX 16 //Pin 16
#define PZEM_TX 17 //Pin 17

//Se crea un objeto PZEM004Tv30 con los pines asignados
PZEM004Tv30 pzem(Serial2, PZEM_RX, PZEM_TX);
float valor = 0;

//Estructura del sensor
struct Sensor
{ float voltaje;    
  float corriente;  
  float potencia;   
  float energia;    
  float frecuencia; 
  float fp;     
  float valor;    
} ;

//Inicialización de las variables de la estructura Sensor
Sensor InfoSensor = {0,0,0,0,0,0,0};

/****************************************************************************************************
*                                Función Datos Sensor                                            *
****************************************************************************************************/

void SensorData() {
  InfoSensor.voltaje = pzem.voltage();
  if (isnan(InfoSensor.voltaje)) { // Se validan los datos que arroja el módulo PZEM
    InfoSensor.voltaje = -1; //se muestra -1 mientras no haya carga conectada
  }

  InfoSensor.corriente = pzem.current();
  if (isnan(InfoSensor.corriente)) { // Se validan los datos que arroja el módulo PZEM
    InfoSensor.corriente = -1; //se muestra -1 mientras no haya carga conectada
  }

  InfoSensor.potencia = pzem.power();
  if (isnan(InfoSensor.potencia)) { // Se validan los datos que arroja el módulo PZEM
    InfoSensor.potencia = -1; //se muestra -1 mientras no haya carga conectada
  }

  InfoSensor.energia = pzem.energy();
  if (isnan(InfoSensor.energia)) { // Se validan los datos que arroja el módulo PZEM
    InfoSensor.energia = -1; //se muestra -1 mientras no haya carga conectada
  }

  InfoSensor.frecuencia = pzem.frequency();
  if (isnan(InfoSensor.frecuencia)) { // Se validan los datos que arroja el módulo PZEM
    InfoSensor.frecuencia = -1; //se muestra -1 mientras no haya carga conectada
  }

  InfoSensor.fp = pzem.pf();
  if (isnan(InfoSensor.fp)) { // Se validan los datos que arroja el módulo PZEM
    InfoSensor.fp = -1; //se muestra -1 mientras no haya carga conectada
  }  
  InfoSensor.valor = (827.8074 * pzem.energy());
}


//Conexión con el Web Server
AsyncWebServer server(80);

char* formato_html =
  "<!DOCTYPE html>"
  "<html lang=\"en\">"
  "<head>"
  "    <meta charset=\"UTF-8\">"
  "    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">"
  "    <META HTTP-EQUIV='Refresh' CONTENT='1'>"
  "    <title>ESP32 WebServer</title>"
  "    <style> body { background-color: #fffff; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }</style>"
  "</head>"
  "<body>"
  "    <h2>MONITOREO CONSUMO ENERGÉTICO</h2>"
  "    <h3>El Voltaje Actual es: %0.3f [V]</h3>"
  "    <h3>La Corriente Actual es: %0.3f [A]</h3>"
  "    <h3>La Potencia Actual es: %0.3f [W]</h3>"
  "    <h3>La Energia Actual es: %0.3f [KwH]</h3>"
  "    <h3>La Frecuencia Actual es: %0.3f [Hz]</h3>"
  "    <h3>El Factor Potencia Actual es: %0.3f</h3>"
  "    <h3>Valor estimado a pagar: $%0.2f</h3>"
  "</body>"
  "</html>";

char buffer_html[1000];

/****************************************************************************************************
*                                Función No se encuentra la página                                  *
****************************************************************************************************/

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

/****************************************************************************************************
*                                Función Conexión a Wifi                                          *
****************************************************************************************************/

void connectToWifi() {
  WiFi.enableSTA(true);

  delay(2000);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

}

/****************************************************************************************************
*                                       Función setup                                               *
*****************************************************************************************************/

void setup() {
  delay(5000);
  
  //Se activa el puerto serial
  Serial.begin(115200);
  Serial.println(WiFi.macAddress());

  //Se activa conexión a Wifi
  connectToWifi();

  //Se configura el Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    SensorData();

    //Presentación de datos en pantalla
    sprintf(buffer_html, 
    formato_html, 
    InfoSensor.voltaje,
    InfoSensor.corriente,
    InfoSensor.potencia,
    InfoSensor.energia,
    InfoSensor.frecuencia,
    InfoSensor.fp,
    InfoSensor.valor);

    request->send_P(200, "text/html", buffer_html);
  });

  server.onNotFound(notFound);
  server.begin();
}

/****************************************************************************************************
*                                        Función cíclica                                            *
*****************************************************************************************************/

void loop() {
  Serial.println();
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
  delay(4000);

  //Se realiza la lectura de los datos del módulo PZEM
  float voltaje = pzem.voltage(); 
  float corriente = pzem.current();
  float potencia = pzem.power();
  float energia = pzem.energy();
  float frequencia = pzem.frequency();
  float factor_potencia = pzem.pf();
  float valor = (827.8074 * energia);
  

  //Se realiza la validación de los datos del módulo PZEM
  if (isnan(voltaje)) {
    Serial.println("Error en lectura de voltaje");
  } else if (isnan(corriente)) {
    Serial.println("Error en lectura de corriente");
  } else if (isnan(potencia)) {
    Serial.println("Error en lectura de potencia");
  } else if (isnan(energia)) {
    Serial.println("Error en lectura de energía");
  } else if (isnan(frequencia)) {
    Serial.println("Error en lectura de frecuencia");
  } else if (isnan(factor_potencia)) {
    Serial.println("Error en lectura de factor de potencia");
  } else {

 // Se visualizan en la consola los datos suministrados por el módulo PZEM
    Serial.print("Voltaje: ");Serial.print(voltaje);Serial.println("V");
    Serial.print("Corriente: ");Serial.print(corriente);Serial.println("A");
    Serial.print("Potencia: ");Serial.print(potencia);Serial.println("W");
    Serial.print("Energía: ");Serial.print(energia, 3);Serial.println("kWh");
    Serial.print("Frecuencia: ");Serial.print(frequencia, 1);Serial.println("Hz");
    Serial.print("Factor de Potencia: ");Serial.println(factor_potencia);
    Serial.print("Valor estimado a pagar: $");Serial.println(valor, 2);
  }
  //Tiempo de espera
  //Serial.println();
  delay(2000);
}

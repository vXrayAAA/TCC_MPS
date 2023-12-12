#include <stdio.h>
#include <stdio.h>																// Acesso as opercoes de entradas e/ou saidas.
#include <driver/adc.h>															// Modulo conversor ADC.
#include "freertos/FreeRTOS.h"													// Acesso aos termos.
#include "freertos/task.h"														// Acesso as prioridades da TASK.
#include "driver/gpio.h"														// Acesso ao uso das GPIOs.
// #include "esp_log.h"															// Acesso ao uso dos LOGs.
#include <esp_http_server.h>    												// => httpd_uri_t
#include "rom/ets_sys.h"														// Acesso a escala de tempo em micro segundos.
#include <esp_event.h>          												// => esp_event_base_t
#include <nvs_flash.h>          												// => nvs_flash_init
#include <sys/param.h>          												// => MIN()
#include "esp_tls_crypto.h"														// => esp_crypto_base64_encode
#include "esp_netif.h"                                                          // 
#include "esp_wifi.h"                                                           // 
#include "mqtt_client.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include <stddef.h>
#include <string.h>
#include <esp_system.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include <inttypes.h>
#include "DHT22.h"
#include "cJSON.h"

#define CONFIG_ESP_WIFI_SSID            "IoT_AP"              // SSID do AP.
#define CONFIG_ESP_WIFI_PASSWORD        "12345678"              // Senha do AP.
#define CONFIG_ESP_WIFI_CHANNEL         1                       // Canal a ser utilizado (BR).
#define CONFIG_ESP_MAX_STA_CONN         4                       // Numero maximo de conexoes.

#define BROKER_URI "mqtt://192.168.0.101"
#define MQTT_PORT 1883

#define MQTT_TOPIC "cfp127/TT3B/"
#define MQTT_TOPIC_SENSOR1 "cfp127/TT3B/sensor1"
#define MQTT_TOPIC_SENSOR2 "cfp127/TT3B/sensor2"
#define MQTT_TOPIC_SENSOR3 "cfp127/TT3B/sensor3"
#define MQTT_TOPIC_TEMP "cfp127/TT3B/temp"
#define MQTT_TOPIC_HUM "cfp127/TT3B/hum"
#define MQTT_TOPIC_TOTAL "cfp127/TT3B/total"



uint32_t MQTT_CONNEECTED = 0;


#define CONFIG_ESP_MAXIMUM_RETRY 5

/* == Sinalizar se a conexão foi estabelecida ou não == */
static EventGroupHandle_t s_wifi_event_group;

/* == Definindo WIFI CONNECTED como BIT0 == */
#define WIFI_CONNECTED_BIT BIT0 

/* == Definindo WIFI FAIL como BIT1 == */
#define WIFI_FAIL_BIT BIT1

static int s_retry_num = 0;

// Pelo SDKCONFIG
#define CONFIG_ESP_WIFI_STATIC_RX_BUFFER_NUM 10                              // Define quantas pilhas estaticas (min).  //  Usar para nao dar erro no WIFI_INIT_CONFIG_DEFAULT()
#define CONFIG_ESP_WIFI_DYNAMIC_RX_BUFFER_NUM 32                             // Define quantas pilhas dinamicas (max).  //  Usar para nao dar erro no WIFI_INIT_CONFIG_DEFAULT()    
#define CONFIG_ESP_WIFI_TX_BUFFER_TYPE 1                                    // Define o tipo de Buffer da Tx do WiFi.   //  Usar para nao dar erro no WIFI_INIT_CONFIG_DEFAULT()


//Essa configuração define o número máximo de dispositivos que podem ser criptografados ao usar o protocolo ESP-NOW no ESP32. Se o valor for definido como 0, nenhum dispositivo será criptografado. Se um valor diferente de 0 for definido, somente os dispositivos especificados serão criptografados. A criptografia é importante para garantir a segurança das informações transmitidas entre os dispositivos.
#define CONFIG_ESP_WIFI_ESPNOW_MAX_ENCRYPT_NUM 0                          //  Usar para nao dar erro no WIFI_INIT_CONFIG_DEFAULT()


#define CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK                                     // Define a seguranca da conexao.           
#define ESP_MAXIMUM_RETRY 5                                                  // Numero de tentativas de conexao.
#define MAX_HTTP_RECV_BUFFER 1024                                            // Tamanho (em bytes) do buffer de entrada.
#define MAX_HTTP_OUTPUT_BUFFER 2048                                        // Tamanho (em bytes) do buffer de saida.
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK            // Modo de autenticacao.


#define CONFIG_HTTPD_MAX_REQ_HDR_LEN 512                                 //Para o erro: Header fields are too long
#define CONFIG_HTTPD_MAX_URI_LEN 1024

/* RTOS */
#define CONFIG_FREERTOS_HZ 100

/* Entrada */
#define ___gpiCK	12                       			                        // Seleciona o pino de 'clock' para o registrador.
#define ___gpiDT	13                       			                        // Seleciona o pino de 'data in' para o registrador.
#define ___gpiLD	14                       			                        // Seleciona o pino de 'load' para o registrador.


/* GERAL */
#define bitX(valor,bit) (valor&(1<<bit))				                        // Testa e retorna o 'bit' de 'valor'.
#define bit1(valor,bit) valor |= (1<<bit)				                        // Faz o 'bit' de 'valor' =1.
#define bit0(valor,bit) valor &= ~(1<<bit)				                        // Faz o 'bit' de 'valor' =0.

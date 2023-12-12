#include <stdio.h>
#include "config.h"


 
/* WiFi */
/**
 * @file SensorV1.c
 * @brief Implementação de um servidor web para monitoramento de sensores.
 * 
 * Este arquivo contém a implementação de um servidor web para monitoramento de sensores.
 * O servidor é implementado em um ESP32 e utiliza a biblioteca ESP-IDF.
 * 
 * @see https://github.com/espressif/esp-idf
 * @see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/index.html
 */


static const char *TAG = "TCC_MPS";                                           // Identificacao da 'Task'.
int vlrQuery1, vlrQuery2, vlrQuery3;                                            // Var. para 'Query' do metodo 'GET'.
char vlrAscIn[]={"0000"};    
char contador = 0;
// char sensor1 = 0;
// char sensor2 = 0;
// char sensor3 = 0;
// char sensor4 = 0;
char sensor1_prev = 0, sensor2_prev = 0, sensor3_prev = 0, sensor4_prev = 0;        //era int
int contador_rosa = 0;
int contador_preto = 0;
int contador_metalico = 0;

char html_buffer[4096];



char sensor_indutivo = 0;
char sensor_Optico = 0;
char sensor_Optico2 = 0;
char sensor_Barreira = 0;


char sensor_indutivo_TMP1 = 0;
char sensor_indutivo_TMP2 = 0;

char sensor_Optico_TMP1 = 0;
char sensor_Optico_TMP2 = 0;

char sensor_Optico2_TMP1 = 0;   
char sensor_Optico2_TMP2 = 0;

char sensor_Barreira_TMP1 = 0;
char sensor_Barreira_TMP2 = 0;







static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(TAG, "retry to connect to the AP");
		} else {
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		ESP_LOGI(TAG,"connect to the AP fail");
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

void wifi_init_sta(void)
{
	s_wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_netif_init());

	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
		ESP_EVENT_ANY_ID,
		&event_handler,
		NULL,
		&instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
		IP_EVENT_STA_GOT_IP,
		&event_handler,
		NULL,
		&instance_got_ip));

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = CONFIG_ESP_WIFI_SSID,
			.password = CONFIG_ESP_WIFI_PASSWORD,
			/* Setting a password implies station will connect to all security modes including WEP/WPA.
			 * However these modes are deprecated and not advisable to be used. Incase your Access point
			 * doesn't support WPA2, these mode can be enabled by commenting below line */
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,

			.pmf_cfg = {
				.capable = true,
				.required = false
			},
		},
	};
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
	ESP_ERROR_CHECK(esp_wifi_start() );

	ESP_LOGI(TAG, "wifi_init_sta finished.");

	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	 * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
		WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
		pdFALSE,
		pdFALSE,
		portMAX_DELAY);

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	 * happened. */
	if (bits & WIFI_CONNECTED_BIT) {
		ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
	} else if (bits & WIFI_FAIL_BIT) {
		ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
	} else {
		ESP_LOGE(TAG, "UNEXPECTED EVENT");
	}

	/* The event will not be processed after unregister */
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
	vEventGroupDelete(s_wifi_event_group);
}




void gpiIniciar(void)															// Inicializa o hardware da entrada.
{
    gpio_reset_pin(___gpiCK);													// Limpa configuracoes anteriores.
	gpio_reset_pin(___gpiDT);													// Limpa configuracoes anteriores.
	gpio_reset_pin(___gpiLD);													// Limpa configuracoes anteriores.
    gpio_set_direction(___gpiCK, GPIO_MODE_OUTPUT);								// Configura o pino como saida.
    gpio_set_direction(___gpiDT, GPIO_MODE_INPUT);								// Configura o pino como entrada.
    gpio_set_direction(___gpiLD, GPIO_MODE_OUTPUT);								// Configura o pino como saida.
	gpio_set_level(___gpiCK,0);													// Limpa o pino.
	gpio_set_level(___gpiDT,0);													// Limpa o pino.
	gpio_set_level(___gpiLD,0);													// Limpa o pino.

	/* Limpa o registrador */
	unsigned char __tmp011;									                    // Var local temporaria.					
	for(__tmp011=0;__tmp011<8;__tmp011++)                                       // Laco para zerar o registrador.
	{
		gpio_set_level(___gpiCK,1);									            // Gera um pulso de clock no registrador.
		gpio_set_level(___gpiCK,0);                                             // 
	}
}


char gpiDado(void)																// Le um dado da GPI (entrada).
{
	unsigned char entrada=0x00;				                                    // Var local temporaria para a entrada.
	unsigned char tmp002=0x00;				                                    // Var local temporaria para o laco.

	gpio_set_level(___gpiLD,1);					                                // Ativa o pino da carga do dado.
	for(tmp002=0;tmp002<8;tmp002++)							                    // Laco para varrer os bits.
	{
		if(gpio_get_level(___gpiDT)==1) bit1(entrada,(7-tmp002));		        // Se o pino de entrada estiver ativado...
		else			                bit0(entrada,(7-tmp002));		        // ... senao...
        gpio_set_level(___gpiCK,1);					                            // Gera um pulso de clock no registrador.
        gpio_set_level(___gpiCK,0);					                            //
	}
	gpio_set_level(___gpiLD,0);						                            // Desativa o pino da carga do dado.
	return (entrada);								                            // Retorna com o valor
}

void int2Asc(unsigned int valor, char *buffer, char digi) 						// Converte INT em ASCII: Valor(Bin), Matriz, Numero de digitos (0 a 5). 
{
	if(digi>5) digi=5;															// Previne erros.
	switch(digi)																// Seleciona o numero de algarismos.
	{
			case 0: 															// Nao converte o Valor(Bin).
				break;															// Retorno.
			case 1:																// Um algarismo.
				buffer[0]=(valor%10)       +0x30; 								// Separa a unidade.
				break;															// Retorno.
			case 2:																// Dois algarismos.
				buffer[0]=(valor/10)       +0x30; 								// Separa a dezena.
				buffer[1]=(valor%10)       +0x30; 								// Separa a unidade.
				break;															// Retorno.
			case 3:																// Tres algarismos.
				buffer[0]=(valor/100)      +0x30; 								// Separa a centena.
				buffer[1]=((valor/10)%10)  +0x30; 								// Separa a dezena.
				buffer[2]=(valor%10)       +0x30; 								// Separa a unidade.
				break;															// Retorno.	
			case 4:																// Quatro algarismos.
				buffer[0]=(valor/1000)     +0x30; 								// Separa a unidade de milhar.
				buffer[1]=((valor/100)%10) +0x30; 								// Separa a centena.
				buffer[2]=((valor/10)%10)  +0x30; 								// Separa a dezena.
				buffer[3]=(valor%10)       +0x30; 								// Separa a unidade.
				break;															// Retorno.
			case 5:																// Cinco algarismos.
				buffer[0]=(valor/10000)    +0x30; 								// Separa a dezena de milhar.
				buffer[1]=((valor/1000)%10)+0x30; 								// Separa a unidade de milhar
				buffer[2]=((valor/100)%10) +0x30; 								// Separa a centena.
				buffer[3]=((valor/10)%10)  +0x30; 								// Separa a dezena.
				buffer[4]=(valor%10)       +0x30; 								// Separa a unidade.
				break;															// Retorno.
	}
}


void hex2Asc(char vlrHex, char *vlrAsc)											// Converte decimal em hexa(ASCII)
{
	char uni,dez;																// Variavel temporaria da unidade e dezena.
	uni = vlrHex & 0x0F;														// Separa a unidade.
	dez = (vlrHex &0xF0)>>4;													// Separa a dezena.

	if(uni>9) uni += 0x37;														// Se maior que 9, sera de 'A' ate 'F'.
	else 	  uni += 0x30;														// Senao  so converte em ASCII.
	if(dez>9) dez += 0x37;														// Se maior que 9, sera de 'A' ate 'F'.
	else 	  dez += 0x30;														// Senao  so converte em ASCII.
	vlrAsc[2] = dez;															// Salva valor para retorno.
	vlrAsc[3] = uni;															// Salva valor para retorno.
}





static void mqtt_app_start(void);


static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{

   
    
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        // ESP_LOGI(TAG, "MQTT CONECTADO COM SUCESSO");
        printf("MQTT CONECTADO COM SUCESSO\n");
        MQTT_CONNEECTED=1;
        msg_id = esp_mqtt_client_subscribe(client, MQTT_TOPIC, 0); 
        ESP_LOGI(TAG, "Mensagem enviada com sucesso, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT DESCONECTADO");
        MQTT_CONNEECTED=0;
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MENSAGEM ENVIADA, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "Evento cancelado, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "Mensagem publicada, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MENSAGEM RECEBIDA");
        ESP_LOGI(TAG, "MENSAGEM ENVIADA, DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT ERRO!");
        break;
    default:
        ESP_LOGI(TAG, "Outro evento:%d", event->event_id);
        break;
    }
}

esp_mqtt_client_handle_t client = NULL;
static void mqtt_app_start(void)
{
    ESP_LOGI(TAG, "Iniciando MQTT");
    esp_mqtt_client_config_t mqtt_Config = {
        .broker.address.uri = BROKER_URI,
        .broker.address.port = MQTT_PORT,
        .credentials.username = "tecT3B",
        .credentials.authentication.password = "0z9y8x7w"
        
    };
    
    client = esp_mqtt_client_init(&mqtt_Config);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void DHT_task(void *pvParameter)
{
    setDHTgpio( 23 );
    printf( "Starting DHT Task\n\n");

    while(1) {
        printf("=== Reading DHT ===\n" );
        int ret = readDHT();
        
        errorHandler(ret);

        float humidity = getHumidity();
        float temperature = getTemperature();

        char humidityStr[30];
        sprintf(humidityStr, "%.1f", humidity);
        esp_mqtt_client_publish(client, MQTT_TOPIC_HUM, humidityStr, 0, 0, 0);

        char temperatureStr[30];
        sprintf(temperatureStr, "%.1f", temperature);
        esp_mqtt_client_publish(client, MQTT_TOPIC_TEMP, temperatureStr, 0, 0, 0);


		printf( "Hum %.1f\n", getHumidity() );
		printf( "Tmp %.1f\n", getTemperature() );
        vTaskDelay( 3000 /  portTICK_PERIOD_MS );
    }
}

void Publisher_Task(void *pvParameter)
{
    while (true)
    {
        if(MQTT_CONNEECTED)
        {
            char sensorIndutivoStr[30];
            sprintf(sensorIndutivoStr, "%d", contador_metalico);
            esp_mqtt_client_publish(client, MQTT_TOPIC_SENSOR1, sensorIndutivoStr, 0, 0, 0); 
            // printf("Sensor indutivo: %d\n", sensor_indutivo);
           // printf("mensagem enviada no topico %s: %s\n", MQTT_TOPIC_SENSOR1, sensorIndutivoStr);
            

            char sensorOptico1Str[30];
            sprintf(sensorOptico1Str, "%d", contador_rosa);
            esp_mqtt_client_publish(client, MQTT_TOPIC_SENSOR2, sensorOptico1Str, 0, 0, 0);
            // printf("Sensor optico: %d\n", sensor_Optico);
          // printf("mensagem enviada no topico %s: %s\n", MQTT_TOPIC_SENSOR2, sensorOptico1Str);

            char sensorOptico2Str[30];
			sprintf(sensorOptico2Str, "%d", contador_preto);
			esp_mqtt_client_publish(client, MQTT_TOPIC_SENSOR3, sensorOptico2Str, 0, 0, 0);
            // printf("Sensor optico2: %d\n", sensor_Optico2);
            //printf("mensagem enviada no topico %s: %s\n", MQTT_TOPIC_SENSOR3, sensorOptico2Str);
			


			char sensorBarreiraStr[30];
			sprintf(sensorBarreiraStr, "%d", contador);
			esp_mqtt_client_publish(client, MQTT_TOPIC_TOTAL, sensorBarreiraStr, 0, 0, 0);
            // printf("Sensor barreira: %d\n", sensor_Barreira);
            //printf("mensagem enviada no topico %s: %s\n", MQTT_TOPIC_TOTAL, sensorBarreiraStr);
			
		}
		
    }

}

const char *html_content = "<!DOCTYPE HTML><html><head><title>ESP-IDF MPS Web Server</title>"\
                           "<meta http-equiv=\"refresh\" content=\"1\">"\
                           "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"\
                           "<link rel=\"stylesheet\" href=\"https://use.fontawesome.com/releases/v5.7.2/css/all.css\" integrity=\"sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr\" crossorigin=\"anonymous\">"\
                           "<link rel=\"icon\" href=\"data\">"\
                           "<style>"\
                           "html {font-family: Arial; display: inline-block; text-align: center;}"\
                           "p {font-size: 1.2rem;}"\
                           "body {margin: 0;}"\
                           ".topnav {overflow: hidden; background-color: #2e0a36; color: white; font-size: 1.7rem;}"\
                           ".topnav img {height: 50px; margin-left: 20px; margin-top: 10px;}"\
                           ".content {padding: 20px;}"\
                           ".card {background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);}"\
                           ".cards {max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));}"\
                           ".reading {font-size: 2.8rem;}"\
                           ".card.temperature {color: #0e7c7b;}"\
                           ".card.humidity {color: #17bebb;}"\
                           "@media only screen and (max-width: 600px) {"\
                           "  .cards {grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));}"\
                           "}"\
                           "@media only screen and (max-width: 400px) {"\
                           "  .cards {grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));}"\
                           "}"\
                           "</style></head><body><div class=\"topnav\"><h3>MPS WEB SERVER</h3></div>"\
                           "<div class=\"content\"><div class=\"cards\"><div class=\"card pecas\"><h4><i class=\"fas fa-hockey-puck fa-fw\" style=\"color: #000000;\"></i>TOTAL</h4>"\
                           "<p><span class=\"contador\">%d</span></p></div></div><div style=\"height: 20px;\"></div><div class=\"cards\"><div class=\"card peca_rosa\"><h4><i class=\"fas fa-hockey-puck fa-fw\" style=\"color: #f500b4;\"></i>ROSA</h4>"\
                           "<p><span class=\"contador-peca_rosa\">%d</span></p></div></div><div style=\"height: 20px;\"></div><div class=\"cards\"><div class=\"card peca_preta\"><h4><i class=\"fas fa-hockey-puck fa-fw\" style=\"color: #000000;\"></i>PRETA</h4>"\
                           "<p><span class=\"contador-peca_preta\">%d</span></p></div></div><div style=\"height: 20px;\"></div><div class=\"cards\"><div class=\"card peca_metalica\"><h4><i class=\"fas fa-hockey-puck fa-fw\" style=\"color: #504f4f;\"></i>METALICA</h4>"\
                           "<p><span class=\"contador-peca_metalica\">%d</span></p></div></div></div></body></html>";

/* Manipulador de solicitação HTTP para servir o arquivo HTML */
esp_err_t html_get_handler(httpd_req_t *req)
{
    // for (int i = 0; i < strlen(html_buffer); i++) 
    // {
    // printf("%c", html_buffer[i]);
    // }
    httpd_resp_set_type(req, "text/html");
    sprintf(html_buffer, html_content,contador, contador_rosa, contador_preto, contador_metalico);
    httpd_resp_send(req, html_buffer, strlen(html_buffer));
    return (ESP_OK);
}


esp_err_t sensorGet(httpd_req_t *req) 
{
    cJSON *root = cJSON_CreateObject();

    // Adicionar os valores dos sensores ao objeto JSON
    cJSON_AddNumberToObject(root, "sensor1", contador);
    cJSON_AddNumberToObject(root, "sensor2", contador_preto);
    cJSON_AddNumberToObject(root, "sensor3", contador_metalico);
    cJSON_AddNumberToObject(root, "sensor4", contador_rosa);

    char *jsonStr = cJSON_Print(root);
	printf("Objeto JSON: %s\n", jsonStr);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, jsonStr, strlen(jsonStr));

    cJSON_free(jsonStr);
    cJSON_Delete(root);
    return ESP_OK;
}

httpd_uri_t sensorUri = {
    .uri = "/sensor",
    .method = HTTP_GET,
    .handler = sensorGet,
    .user_ctx = NULL
};


httpd_handle_t start_webserver(void)
{

    // printf(TAG, "Memória livre antes de iniciar o servidor: %d bytes", esp_get_free_heap_size());

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
   
    /* Inicializa o servidor HTTP */
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) 
    {
        /* Registra o manipulador de solicitação HTTP para o arquivo HTML */
        httpd_uri_t html_uri = {
            .uri       = "/mps",
            .method    = HTTP_GET,
            .handler   = html_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &html_uri);
		httpd_register_uri_handler(server, &sensorUri);
         /* Verifica a memória livre após iniciar o servidor */
        // printf(TAG, "Memória livre após iniciar o servidor: %d bytes", esp_get_free_heap_size());
        return (server);
    }

    return (NULL);
}





void stop_webserver(httpd_handle_t server)
{
    if (server)
    {
        httpd_stop(server);
    }
}



void wifi_init( void )
{
       // Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);


   // Initialize WiFi
	wifi_init_sta();
    // Start the HTTP Server
    start_webserver();
}

void gpiTask(void)
{

    sensor_indutivo = gpiDado(); // Read input from sensor 1
    sensor_indutivo &= 0x0F; // Limpa os 4 bits mais significativos
    sensor_indutivo = bitX(sensor_indutivo, 0);
    printf("Sensor indutivo: %d\n", sensor_indutivo);

    sensor_Optico = gpiDado(); // Read input from sensor 2
    sensor_Optico &= 0x0F; // Limpa os 4 bits mais significativos
    sensor_Optico = bitX(sensor_Optico, 1);
     printf("Sensor optico: %d\n", sensor_Optico);


    sensor_Optico2 = gpiDado(); // Read input from sensor 3 
    sensor_Optico2 &= 0x0F; // Limpa os 4 bits mais significativos
    sensor_Optico2 = bitX(sensor_Optico2, 2);
    printf("Sensor optico2: %d\n", sensor_Optico2);

    sensor_Barreira = gpiDado(); // Read input from sensor 4    
    sensor_Barreira &= 0x0F; // Limpa os 4 bits mais significativos 
    sensor_Barreira = bitX(sensor_Barreira, 3);
    printf("Sensor barreira: %d\n", sensor_Barreira);


      vTaskDelay(100 / portTICK_PERIOD_MS);  



}


unsigned char detectaBordaSubida(unsigned char pino) 
{
// b0=pinoEntrada; // Ler valor do pino de entrada.
// if(b0==pino && f0==0) // Borda de subida?
// {
// f0=1; // Sim. Lembrar que aconteceu.
// return(1); // Retornar que Ok.
// }
// else if(b0==0 && f0==1) // Borda de descida?
// {
// f0=0; // Sim. Lembrar que aconteceu.
// return(0); // Retornar que Nao Ok.
// }
// else if(b0==pino && f0==1) return(0);// Preso no nivel 1. Nao Ok.
// else return(0); // Preso no nivel 0. Nao Ok.


    sensor_indutivo = gpiDado(); // Read input from sensor 1

    sensor_indutivo_TMP1 = sensor_indutivo;

    if(sensor_indutivo_TMP1 == pino && sensor_indutivo_TMP2 == 0)
    {
        sensor_indutivo_TMP2 = 1;
        return(1);
    }
    else if(sensor_indutivo_TMP1 == 0 && sensor_indutivo_TMP2 == 1)
    {
        sensor_indutivo_TMP2 = 0;
        return(0);
    }
    else if(sensor_indutivo_TMP1 == pino && sensor_indutivo_TMP2 == 1)
    {
        return(0);
    }
    else
    {
        return(0);
    }
    {

    }

    sensor_Optico = gpiDado(); // Read input from sensor 2

    sensor_Optico_TMP1 = sensor_Optico;
    if(sensor_Optico_TMP1 == pino && sensor_Optico_TMP2 == 0)
    {
        sensor_Optico_TMP2 = 1;
        return(1);
    }
    else if(sensor_Optico_TMP1 == 0 && sensor_Optico_TMP2 == 1)
    {
        sensor_Optico_TMP2 = 0;
        return(0);
    }
    else if(sensor_Optico_TMP1 == pino && sensor_Optico_TMP2 == 1)
    {
        return(0);
    }
    else
    {
        return(0);
    }
    {

    }


    sensor_Optico2 = gpiDado(); // Read input from sensor 3
    sensor_Optico2_TMP1 = sensor_Optico2;
    
    if(sensor_Optico2_TMP1 == pino && sensor_Optico2_TMP2 == 0)
    {
        sensor_Optico2_TMP2 = 1;
        return(1);
    }
    else if(sensor_Optico2_TMP1 == 0 && sensor_Optico2_TMP2 == 1)
    {
        sensor_Optico2_TMP2 = 0;
        return(0);
    }
    else if(sensor_Optico2_TMP1 == pino && sensor_Optico2_TMP2 == 1)
    {
        return(0);
    }
    else
    {
        return(0);
    }
    {

    }



    sensor_Barreira = gpiDado(); // Read input from sensor 4

    sensor_Barreira_TMP1 = sensor_Barreira;


    if(sensor_Barreira_TMP1 == pino && sensor_Barreira_TMP2 == 0)
    {
        sensor_Barreira_TMP2 = 1;
        return(1);
    }
    else if(sensor_Barreira_TMP1 == 0 && sensor_Barreira_TMP2 == 1)
    {
        sensor_Barreira_TMP2 = 0;
        return(0);
    }
    else if(sensor_Barreira_TMP1 == pino && sensor_Barreira_TMP2 == 1)
    {
        return(0);
    }
    else
    {
        return(0);
    }
    {

    }





}


void app_main(void)
{
    								
	printf(TAG,"App Iniciado.");										// Imprime Info.

   

    wifi_init();


    gpiIniciar();
	mqtt_app_start();

   // xTaskCreate(gpiTask, "gpiTask", 2048, NULL, 6, NULL);
    xTaskCreate(Publisher_Task, "Publisher_Task", 1024 * 5, NULL, 5, NULL);
	xTaskCreate( &DHT_task, "DHT_task", 4096, NULL, 5, NULL );
    //xTaskCreate(sensor_task, "sensor_task", 2048, NULL, 5, NULL);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client); 

while (1)
{


    // Publisher_Task();
    gpiTask();
    // detectaBordaSubida(1);
    if (detectaBordaSubida(1))
    {
        printf("Borda de subida no sensor 1!\n");
        contador_metalico++;
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);

    if (detectaBordaSubida(2))
    {
        printf("Borda de subida no sensor 2!\n");
        contador_rosa++;
    }
    vTaskDelay(2 / portTICK_PERIOD_MS);

    if (detectaBordaSubida(4))
    {
   
        printf("Borda de subida no sensor 3!\n");
        contador_preto++;
    }
    vTaskDelay(3 / portTICK_PERIOD_MS);
     
    if (detectaBordaSubida(8))
    {
        printf("Borda de subida no sensor 4!\n");
        contador++;
    }


    // sensor1_prev = sensor1;
    // sensor2_prev = sensor2;
    // sensor3_prev = sensor3;
    // sensor4_prev = sensor4;

    // sensor1 = gpiDado(); // Read input from sensor 1
    // sensor2 = gpiDado(); // Read input from sensor 2
    // sensor3 = gpiDado(); // Read input from sensor 3
    // sensor4 = gpiDado(); // Read input from sensor 4





    // if (sensor1 == 1  && sensor2 == 0 && sensor3 == 0 && sensor4 == 8)
    // {
    //     printf("passou pelo sensor 1!\n");
    //     contador_metalico++;
    //     contador++;
    // }
    //    if (sensor1 == 0 && sensor2 == 2 && sensor3 == 4 && sensor4 == 8) // 0 2 4 8
    // {
    //     printf("passou pelo sensor rosa!\n");
    //     contador_rosa++;
    //     contador++;
    // }
    // if (sensor1 == 0 && sensor2 == 0 && sensor3 == 0 && sensor4 == 8) // 0 0 4 8
    // {
    //     printf("passou pelo sensor preta!\n");
    //     contador_preto++;
    //     contador++;
    // }
   
    // if (sensor1_prev && sensor1 == 1)
    // {
    //     printf("Borda de subida no sensor 1!\n");
    //     contador_metalico++;
    // }
    // if (sensor2_prev && sensor2 == 2)
    // {
    //     printf("Borda de subida no sensor 2!\n");
    //     contador_rosa++;
    // }
    // if (sensor3_prev && sensor3 == 4)
    // {
    //     printf("Borda de subida no sensor 3!\n");
    //     contador_preto++;   
    // }
    // if (sensor4_prev && sensor4 == 8)
    // {
    //     printf("Borda de subida no sensor 4!\n");
    //     contador++;
    // }
   

    // vTaskDelay(100 / portTICK_PERIOD_MS);
    printf("Contador: %d\n", contador);
    printf("Contador Rosa: %d\n", contador_rosa);
    printf("Contador Preto: %d\n", contador_preto);
    printf("Contador Metalico: %d\n", contador_metalico);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}
    

}



//     bitX(valor,bit) (valor&(1<<bit))
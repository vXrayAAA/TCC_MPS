# TCC_MPS


#Configuração e Inicialização:

*Incluições de bibliotecas e definição de constantes.
*Inicialização de variáveis e configuração de tags e contadores.
*Manipuladores de Eventos para WiFi e MQTT:

Configuração de manipuladores de eventos para lidar com eventos relacionados à conexão WiFi e MQTT.
Inicialização do WiFi:

Função wifi_init_sta para configurar e inicializar a conexão WiFi do ESP32.
Utilização da biblioteca ESP-IDF para configurações de WiFi.
Inicialização da GPIO:

Função gpiIniciar para inicializar a GPIO (General Purpose Input/Output).
Configuração de pinos como entrada ou saída.
Conversões e Manipulações de Dados:

Funções int2Asc e hex2Asc para converter inteiros em ASCII e hexadecimais em ASCII, respectivamente.
Utilização de máscaras de bits (bit1 e bit0) para manipulação de bits.
Manuseio de Eventos WiFi:

Função event_handler para lidar com eventos WiFi, como conexão, desconexão, e obtenção de IP.
Utilização de Event Groups para sincronização de eventos.
MQTT:

Funções para iniciar o cliente MQTT e manipular eventos MQTT.
Publicação e subscrição de tópicos MQTT.
Tarefas:

Tarefas para leitura de sensores (DHT_task e gpiTask).
Tarefa Publisher_Task para publicar informações dos sensores via MQTT.
Servidor Web:

Inicialização do servidor web por meio da função start_webserver.
Manipulador de solicitação HTTP para servir conteúdo HTML.
Detecção de Borda de Subida:

Função detectaBordaSubida para detectar bordas de subida nos sinais dos sensores.
Aplicação Principal:

Função app_main que inicializa a conexão WiFi, o servidor web, o cliente MQTT e inicia as tarefas.

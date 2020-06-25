/* WiFi Connection Example using WPA2 Enterprise
 *
 * Original Copyright (C) 2006-2016, ARM Limited, All Rights Reserved, Apache 2.0 License.
 * Additions Copyright (C) Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD, Apache 2.0 License.
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "config.h"
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "esp_tls.h"

#include "cJSON.h"
#include "user_http_s.h"
#include "lcd.h"
#include "tjpgd.h"


#define WEB_SERVER "api.seniverse.com"
#define WEB_PORT "443"
#define WEB_URL "https://api.seniverse.com/v3/weather/now.json?key=SK38bfeMbSo8aqcvn&location=yantai&language=zh-Hans&unit=c"
/*
#define WEB_SERVER "www.howsmyssl.com"
#define WEB_PORT "443"
#define WEB_URL "https://www.howsmyssl.com/a/check"
*/

// static const char *REQUEST = "GET " WEB_URL " HTTP/1.0\r\n"
//     "Host: "WEB_SERVER"\r\n"
//     "User-Agent: esp-idf/1.0 esp32\r\n"
//     "\r\n";


#define EXAMPLE_WIFI_SSID CONFIG_EXAMPLE_WIFI_SSID
#define EXAMPLE_EAP_METHOD CONFIG_EXAMPLE_EAP_METHOD

#define EXAMPLE_EAP_ID CONFIG_EXAMPLE_EAP_ID
#define EXAMPLE_EAP_USERNAME CONFIG_EXAMPLE_EAP_USERNAME
#define EXAMPLE_EAP_PASSWORD CONFIG_EXAMPLE_EAP_PASSWORD

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;

/* Constants that aren't configurable in menuconfig */
#define EAP_PEAP 1
#define EAP_TTLS 2
typedef struct 
{
    char cit[30];
    char weather_text[30];
    char weather_code[4];
    char temperatur[4];
    char temperatur2[4];
    char wind_dir[30];
    char wind_degree[4];
    char date[30];

}weather_info;
 
//weather_info weathe;

weather_info* weather;
int weather_count;
TaskHandle_t display_task_handle;
/* CA cert, taken from wpa2_ca.pem
   Client cert, taken from wpa2_client.crt
   Client key, taken from wpa2_client.key

   The PEM, CRT and KEY file were provided by the person or organization
   who configured the AP with wpa2 enterprise.

   To embed it in the app binary, the PEM, CRT and KEY file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/
extern uint8_t ca_pem_start[] asm("_binary_wpa2_ca_pem_start");
extern uint8_t ca_pem_end[]   asm("_binary_wpa2_ca_pem_end");
extern uint8_t client_crt_start[] asm("_binary_wpa2_client_crt_start");
extern uint8_t client_crt_end[]   asm("_binary_wpa2_client_crt_end");
extern uint8_t client_key_start[] asm("_binary_wpa2_client_key_start");
extern uint8_t client_key_end[]   asm("_binary_wpa2_client_key_end");

extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
extern const uint8_t server_root_cert_pem_end[]   asm("_binary_server_root_cert_pem_end");

// extern const uint8_t hzk16s_fnt_start[] asm("_binary_hzk16s_fnt_start");
// extern const uint8_t hzk16s_fnt_end[] asm("_binary_hzk16s_fnt_end");
static void display_task();
static void smartconfig_example_task(void * parm);
int wifi_flag;
int disconnected_count=0;
int line_count=0;
//????????????



/*int UTF8toGBK(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
    iconv_t cd;
    char **pin = &inbuf;
    char **pout = &outbuf;
     cd = iconv_open("gbk", "utf-8"); //gb2312
    if (0 == cd)
        return -1;
    if (-1 == iconv(cd, pin, &inlen, pout, &outlen))
    {
        iconv_close(cd);
        return -1;
    }
    iconv_close(cd);
    return 0;
}*/
void LCD_ShowWeather(weather_info* wi)
{
    LCD_Clear(RED);BACK_COLOR = RED;POINT_COLOR = BLUE;
    LCD_ShowHanzi(88,16,wi->cit);
    LCD_ShowImage(95,55,50,50,atoi(wi->weather_code));
    char tmp[40];
    LCD_ShowHanzi(104,120,wi->weather_text);
 //   printf("show temp\n");
    sprintf(tmp,"%s-%s℃",wi->temperatur2,wi->temperatur);
    LCD_ShowHanzi(64,160,tmp);
    sprintf(tmp,"%s风%s级",wi->wind_dir,wi->wind_degree);
//    printf("show wind\n");
    LCD_ShowHanzi(32,200,tmp);
    sprintf(tmp,"%s",wi->date);
//    printf("show date\n");
    LCD_ShowHanzi(8,240,tmp);
}

void cjson2weather(weather_info *wi,cJSON* daily)
{
    cJSON* psub;
    psub=cJSON_GetObjectItem(daily,"code_day");
    strcpy(wi->weather_code,psub->valuestring);
//    printf("CODE =%s,atoi code =%d\n",wi->weather_code,atoi(wi->weather_code));
    psub=cJSON_GetObjectItem(daily,"text_day");
    strcpy(wi->weather_text,psub->valuestring);
    psub=cJSON_GetObjectItem(daily,"high");
    strcpy(wi->temperatur,psub->valuestring);
    psub=cJSON_GetObjectItem(daily,"low");
    strcpy(wi->temperatur2,psub->valuestring);
    psub=cJSON_GetObjectItem(daily,"wind_direction");
    strcpy(wi->wind_dir,psub->valuestring);
    psub=cJSON_GetObjectItem(daily,"wind_scale");
    strcpy(wi->wind_degree,psub->valuestring);
    psub=cJSON_GetObjectItem(daily,"date");
    strcpy(wi->date,psub->valuestring);
}

void cjson_to_struct_info(char *text)
{
    cJSON *root,*psub;
    cJSON *arrayItem;
    //?????��json
    char *index=strchr(text,'{');
    strcpy(text,index);
    // ESP_LOGI(TAG,"json to struct");
    root = cJSON_Parse(text);
    
    if(root!=NULL)
    {
        // ESP_LOGI(TAG,"root");
        psub = cJSON_GetObjectItem(root, "results");
        arrayItem = cJSON_GetArrayItem(psub,0);
 
        cJSON *locat = cJSON_GetObjectItem(arrayItem, "location");
        cJSON *daily = cJSON_GetObjectItem(arrayItem,"daily");
//        int count=0;
        if(daily!=NULL)
        {
            weather_count=cJSON_GetArraySize(daily);
        }
        if((locat!=NULL)&&(daily!=NULL))
        {
            psub=cJSON_GetObjectItem(locat,"name");
            weather=malloc(sizeof(weather_info)*weather_count);
            cJSON** day=malloc(sizeof(cJSON*)*weather_count);
            for(int i=0;i<weather_count;i++)
            {
                strcpy(weather[i].cit,psub->valuestring);
                day[i]=cJSON_GetArrayItem(daily,i);
                cjson2weather(weather+i,day[i]);
            }
            xTaskCreate(display_task, "display_task", 4096, NULL, 3, &display_task_handle);
        }
    }
    //ESP_LOGI("cjson_to_struct_info","%s 222",__func__);
    cJSON_Delete(root);
}



// static void https_get_task(void *pvParameters)
// {
//     char buf[512];
//     int ret, flags, len;

//     mbedtls_entropy_context entropy;
//     mbedtls_ctr_drbg_context ctr_drbg;
//     mbedtls_ssl_context ssl;
//     mbedtls_x509_crt cacert;
//     mbedtls_ssl_config conf;
//     mbedtls_net_context server_fd;

//     mbedtls_ssl_init(&ssl);
//     mbedtls_x509_crt_init(&cacert);
//     mbedtls_ctr_drbg_init(&ctr_drbg);
//     // ESP_LOGI("https_get_task", "Seeding the random number generator");

//     mbedtls_ssl_config_init(&conf);

//     mbedtls_entropy_init(&entropy);
//     if((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
//                                     NULL, 0)) != 0)
//     {
//         ESP_LOGE("https_get_task", "mbedtls_ctr_drbg_seed returned %d", ret);
//         abort();
//     }

//     // ESP_LOGI("https_get_task", "Loading the CA root certificate...");

//     ret = mbedtls_x509_crt_parse(&cacert, server_root_cert_pem_start,
//                                  server_root_cert_pem_end-server_root_cert_pem_start);

//     if(ret < 0)
//     {
//         ESP_LOGE("https_get_task", "mbedtls_x509_crt_parse returned -0x%x\n\n", -ret);
//         abort();
//     }

//     // ESP_LOGI("https_get_task", "Setting hostname for TLS session...");

//      /* Hostname set here should match CN in server certificate */
//     if((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
//     {
//         ESP_LOGE("https_get_task", "mbedtls_ssl_set_hostname returned -0x%x", -ret);
//         abort();
//     }

//     // ESP_LOGI("https_get_task", "Setting up the SSL/TLS structure...");

//     if((ret = mbedtls_ssl_config_defaults(&conf,
//                                           MBEDTLS_SSL_IS_CLIENT,
//                                           MBEDTLS_SSL_TRANSPORT_STREAM,
//                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
//     {
//         ESP_LOGE("https_get_task", "mbedtls_ssl_config_defaults returned %d", ret);
//         goto exit;
//     }

//     /* MBEDTLS_SSL_VERIFY_OPTIONAL is bad for security, in this example it will print
//        a warning if CA verification fails but it will continue to connect.

//        You should consider using MBEDTLS_SSL_VERIFY_REQUIRED in your own code.
//     */
//     mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
//     mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
//     mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
// #ifdef CONFIG_MBEDTLS_DEBUG
//     mbedtls_esp_enable_debug_log(&conf, CONFIG_MBEDTLS_DEBUG_LEVEL);
// #endif

//     if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
//     {
//         ESP_LOGE("https_get_task", "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
//         goto exit;
//     }

//     while(1) 
//     {
//         mbedtls_net_init(&server_fd);

//         // ESP_LOGI("https_get_task", "Connecting to %s:%s...", WEB_SERVER, WEB_PORT);

//         if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER,
//                                       WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
//         {
//             ESP_LOGE("https_get_task", "mbedtls_net_connect returned -%x", -ret);
//             goto exit;
//         }

//         // ESP_LOGI("https_get_task", "Connected.");

//         mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

//         // ESP_LOGI("https_get_task", "Performing the SSL/TLS handshake...");

//         while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
//         {
//             if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
//             {
//                 ESP_LOGE("https_get_task", "mbedtls_ssl_handshake returned -0x%x", -ret);
//                 goto exit;
//             }
//         }

//         // ESP_LOGI("https_get_task", "Verifying peer X.509 certificate...");

//         if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
//         {
//             /* In real life, we probably want to close connection if ret != 0 */
//             ESP_LOGW("https_get_task", "Failed to verify peer certificate!");
//             bzero(buf, sizeof(buf));
//             mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);
//             ESP_LOGW("https_get_task", "verification info: %s", buf);
//         }
//         else
//         {
//             ESP_LOGI("https_get_task", "Certificate verified.");
//         }

//         ESP_LOGI("https_get_task", "Cipher suite is %s", mbedtls_ssl_get_ciphersuite(&ssl));

//         ESP_LOGI("https_get_task", "Writing HTTP request...");

//         size_t written_bytes = 0;
//         do {
//             ret = mbedtls_ssl_write(&ssl,
//                                     (const unsigned char *)REQUEST + written_bytes,
//                                     strlen(REQUEST) - written_bytes);
//             if (ret >= 0) {
//                 // ESP_LOGI("https_get_task", "%d bytes written", ret);
//                 written_bytes += ret;
//             } else if (ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ) {
//                 ESP_LOGE("https_get_task", "mbedtls_ssl_write returned -0x%x", -ret);
//                 goto exit;
//             }
//         } while(written_bytes < strlen(REQUEST));

//         ESP_LOGI("https_get_task", "Reading HTTP response...");

//         do
//         {
//             len = sizeof(buf) - 1;
//             bzero(buf, sizeof(buf));
//             ret = mbedtls_ssl_read(&ssl, (unsigned char *)buf, len);

//             if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
//                 continue;

//             if(ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
//             {
//                 ret = 0;
//                 ESP_LOGE("https_get_task", "MBEDTLS ERR SSL PEER CLOSE NOTIFY");
//                 break;
//             }

//             if(ret < 0)
//             {
//                 ESP_LOGE("https_get_task", "mbedtls_ssl_read returned -0x%x", -ret);
//                 break;
//             }

//             if(ret == 0)
//             {
//                 ESP_LOGI("https_get_task", "connection closed");
//                 break;
//             }

//             len = ret;
//             ESP_LOGD("https_get_task", "%d bytes read", len);
//             /* Print response directly to stdout as it is read */
// /*          for(int i = 0; i < len; i++)
//             {
//                 putchar(buf[i]);
//             }*/
//         } while(1);

//         mbedtls_ssl_close_notify(&ssl);

//     exit:
//         mbedtls_ssl_session_reset(&ssl);
//         mbedtls_net_free(&server_fd);

//         if(ret != 0)
//         {
//             mbedtls_strerror(ret, buf, 100);
//             ESP_LOGE("https_get_task", "Last error was: -0x%x - %s", -ret, buf);
//         }

// //        putchar('\n'); // JSON output doesn't have a newline at end

//         static int request_count;
//         ESP_LOGI("https_get_task", "Completed %d requests", ++request_count);

//         for(int countdown = 10; countdown >= 0; countdown--) {
//             ESP_LOGI("https_get_task", "%d...", countdown);
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//         }
//         ESP_LOGI("https_get_task", "Starting again!");
//     }
// }
/** 
 * https��get�����������ڴ������������Ϲ��������Ԥ����json����
 * @param[in]   pvParameters     :��ʾ���������ֵ�������񴫽������ǳ��е�ѡ��?
 * @retval      null            
 * @par         �޸���־ 
 *               Ver0.0.1:
                    Helon_Chan, 2018/06/29, ��ʼ���汾\n 
 */
static void https_request_by_get_task(void *pvParameters)
{
    https_request_by_GET(HTTPS_URL_BJ);
/*  switch (*((uint8_t *)pvParameters))
  {
  case 1:
    break;
  case 2:
    https_request_by_GET(HTTPS_URL_SH);
    break;
  case 3:
    https_request_by_GET(HTTPS_URL_GZ);
    break;
  case 4:
    https_request_by_GET(HTTPS_URL_SZ);
    break;
  default:
    break;
  }*/
  vTaskDelete(NULL);
}

static void event_handler(void* arg, esp_event_base_t event_base, 
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
	{//????????????wifi???????????????????????????????????smart config
		if(wifi_flag==1)
		{
			// ESP_LOGI("event_handler", "smart config start!");
			xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
            LCD_ShowString(0,line_count,16*22,16,16,"Start smart config...");
            line_count+=16;
            LCD_ShowString(0,line_count,16*27,16,16,"Use you phone to config...");
            line_count+=16;

			disconnected_count=0;
		}
		else
		{
			// ESP_LOGI("event_handler", "connect to last config wifi!");
			esp_wifi_connect();
            

		}
    }
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {//=========================================================//
		// ESP_LOGI("event_handler", "disconnected ,and connect again!");
		disconnected_count++;
		if(disconnected_count>=10)
		{
			// ESP_LOGI("event_handler", "10 errors while link to current wifi, use smart config!");
			xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
			disconnected_count=0;
		}
		else
		{
		esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
		}
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {//=========================================================//
		// ESP_LOGI("event_handler", "got an ip!");
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
		//https request
//		ESP_ERROR_CHECK(example_connect());
//		xTaskCreate(&https_get_task, "https_get_task", 8192, NULL, 5, NULL);
        LCD_ShowString(0,line_count,16*29,16,16,"Success to connect the wifi!");
        line_count+=16;
        int s_city_select=0;
        int err_code = xTaskCreate(https_request_by_get_task,
                                 "https_request_by_get_task",
                                 1024 * 8,
                                 &s_city_select,
                                 3,
                                 NULL);
      if (err_code != pdPASS)
      {
        // ESP_LOGI("event_handler", "https_request_by_get_task create failure,reason is %d\n", err_code);
      }
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) 
    {// smart config event handler
        // ESP_LOGI("event_handler", "Scan done");
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) 
    {
        // ESP_LOGI("event_handler", "Found channel");
    } 
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) 
    {
        // ESP_LOGI("event_handler", "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = { 0 };
        uint8_t password[65] = { 0 };

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true) {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        // ESP_LOGI("event_handler", "SSID:%s", ssid);
        // ESP_LOGI("event_handler", "PASSWORD:%s", password);

        ESP_ERROR_CHECK( esp_wifi_disconnect() );
        ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
        ESP_ERROR_CHECK( esp_wifi_connect() );
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        xEventGroupSetBits(wifi_event_group, ESPTOUCH_DONE_BIT);
    }

}
static void smartconfig_example_task(void * parm)
{
    EventBits_t uxBits;
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_smartconfig_start(&cfg) );
    while (1)
    {
        uxBits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY); 
        if(uxBits & CONNECTED_BIT) 
        {
           LCD_ShowString(0,line_count,16*25,16,16,"Start smart configed OK!");
           line_count+=16;
           // ESP_LOGI("smartconfig_example_task", "WiFi Connected to ap");
        }
        if(uxBits & ESPTOUCH_DONE_BIT)
        {
            // ESP_LOGI("smartconfig_example_task", "smartconfig over");
            LCD_ShowString(0,line_count,16*29,16,16,"Start smart configed FAILED!");
            line_count+=16;

            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}

static void initialise_wifi(void)
{

    unsigned int ca_pem_bytes = ca_pem_end - ca_pem_start;
    unsigned int client_crt_bytes = client_crt_end - client_crt_start;
    unsigned int client_key_bytes = client_key_end - client_key_start;
//    ESP_LOGI("initialise_wifi","begin......");
    LCD_ShowString(0,line_count,16*26,16,16,"Ininitialising wifi......");
    line_count+=16;
    tcpip_adapter_init();
    
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );// for smart config
//    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );//default is WIFI_STORAGE_FLASH

	wifi_config_t flash_config;
	ESP_ERROR_CHECK( esp_wifi_get_config(ESP_IF_WIFI_STA, &flash_config));
//	ESP_LOGI("initialise_wifi", "Get storaged config : SSID =%s, PASSWORD=%s...",flash_config.sta.ssid,flash_config.sta.password);
	wifi_config_t wifi_config={0};//����0���ṹ�壬��������
	if(flash_config.sta.ssid[0]!=0)
	{
		wifi_flag=0;//0������flash��ȡ������1����ʹ��smart config
		memcpy(wifi_config.sta.ssid,flash_config.sta.ssid,sizeof(wifi_config.sta.ssid)+1);
		memcpy(wifi_config.sta.password,flash_config.sta.password,sizeof(wifi_config.sta.password)+1);
//		ESP_LOGI("initialise_wifi", "copy storaged config : SSID =%s, PASSWORD=%s...",wifi_config.sta.ssid,wifi_config.sta.password);
		// ESP_LOGI("initialise_wifi", "Setting WiFi configuration SSID :%s,PASSWORD:%s...", wifi_config.sta.ssid,wifi_config.sta.password);
        char  str[255];
        sprintf(str,"Connect to %s",wifi_config.sta.ssid);
        LCD_ShowString(0,line_count,16*strlen(str),16,16,str);
        line_count+=16;
		ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
		ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
		ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_ca_cert(ca_pem_start, ca_pem_bytes) );
		ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_cert_key(client_crt_start, client_crt_bytes,\
			client_key_start, client_key_bytes, NULL, 0) );
		ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EXAMPLE_EAP_ID, strlen(EXAMPLE_EAP_ID)) );
		if (EXAMPLE_EAP_METHOD == EAP_PEAP || EXAMPLE_EAP_METHOD == EAP_TTLS)
        {
			ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EXAMPLE_EAP_USERNAME, strlen(EXAMPLE_EAP_USERNAME)) );
			ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EXAMPLE_EAP_PASSWORD, strlen(EXAMPLE_EAP_PASSWORD)) );
		}
		
		ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_enable() );
		ESP_ERROR_CHECK( esp_wifi_start() );
	}
	else
	{
		wifi_flag=1;
		ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
		ESP_ERROR_CHECK( esp_wifi_start() );
	}//*/
}
/*
static void wpa2_enterprise_example_task(void *pvParameters)
{
    tcpip_adapter_ip_info_t ip;
    memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    while (1) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        if (tcpip_adapter_get_ip_info(ESP_IF_WIFI_STA, &ip) == 0) {
            ESP_LOGI("wpa2_enterprise_example_task", "~~~~~~~~~~~");
            ESP_LOGI("wpa2_enterprise_example_task", "IP:"IPSTR, IP2STR(&ip.ip));
            ESP_LOGI("wpa2_enterprise_example_task", "MASK:"IPSTR, IP2STR(&ip.netmask));
            ESP_LOGI("wpa2_enterprise_example_task", "GW:"IPSTR, IP2STR(&ip.gw));
            ESP_LOGI("wpa2_enterprise_example_task", "~~~~~~~~~~~");
        }
    }
}*/

static void display_task()
{
    //int times = 0;

    char i = 0;
    int count=0;
    while(1)
    {
        LCD_ShowWeather(weather+i);
        i++;
        if(i>=weather_count)
        i=0;
        count++;
        if(count>=120)//10 minutes
        {
            count=0;
            i=0;
            int s_city_select=0;
            int err_code = xTaskCreate(https_request_by_get_task,
                                 "https_request_by_get_task",
                                 1024 * 8,
                                 &s_city_select,
                                 3,
                                 NULL);
            vTaskDelete(display_task_handle);
            break;
        }
        vTaskDelay(5000 / portTICK_RATE_MS);
    } 
}

void app_main(void)
{
    Lcd_Init();   //tft��ʼ��
    printf("lcd init OK!\n");
    
    LCD_Display_Dir(R2L_U2D);

    LCD_Clear(BLACK);BACK_COLOR = BLACK;POINT_COLOR = GREEN;
//    display_task();
//    xTaskCreate(display_task, "display_task", 4096, NULL, 3, NULL);
    ESP_ERROR_CHECK( nvs_flash_init() );
    initialise_wifi();
}

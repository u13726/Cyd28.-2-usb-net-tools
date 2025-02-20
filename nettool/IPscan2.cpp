/* ICMP echo example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
//#include "sdkconfig.h"
//#include "lwip/inet.h"
//#include "lwip/netdb.h"
//#include "lwip/sockets.h"
//#include "esp_console.h"
#include "esp_event.h"
#include "nvs_flash.h"
//#include "argtable3/argtable3.h"
 #include "ping/ping_sock.h"
bool waitforgreen=false;
#include "config.h"
String tab="";
static void cmd_ping_on_ping_success(esp_ping_handle_t hdl, void *args)
{
    uint8_t ttl;  
    uint16_t seqno;
    uint32_t elapsed_time, recv_len;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
   if (!(tab.indexOf(ipaddr_ntoa((ip_addr_t*)&target_addr))>=0))
   { tab=tab + ipaddr_ntoa((ip_addr_t*)&target_addr) +"\n";
    display->setCursor(0,10);
    display->printf("%s", tab.c_str());
   }/*esp_ping_delete_session(hdl);
    waitforgreen=false;
*/
}

static void cmd_ping_on_ping_timeout(esp_ping_handle_t hdl, void *args)
{
    uint16_t seqno;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
   // printf("From %s icmp_seq=%d timeout\n",ipaddr_ntoa((ip_addr_t*)&target_addr), seqno);
}

static void cmd_ping_on_ping_end(esp_ping_handle_t hdl, void *args)
{
    ip_addr_t target_addr;
    uint32_t transmitted;
    uint32_t received;
    uint32_t total_time_ms;
    uint32_t loss;

    esp_ping_get_profile(hdl, ESP_PING_PROF_REQUEST, &transmitted, sizeof(transmitted));
    esp_ping_get_profile(hdl, ESP_PING_PROF_REPLY, &received, sizeof(received));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_DURATION, &total_time_ms, sizeof(total_time_ms));

    if (transmitted > 0) {
        loss = (uint32_t)((1 - ((float)received) / transmitted) * 100);
    } else {
        loss = 0;
    }
#ifdef CONFIG_LWIP_IPV4
    if (IP_IS_V4(&target_addr)) {
        printf("\n--- %s ping statistics ---\n", inet_ntoa(*ip_2_ip4(&target_addr)));
    }
#endif
 //    printf("%" PRIu32 " packets transmitted, %" PRIu32 " received, %" PRIu32 "%% packet loss, time %" PRIu32 "ms\n",
 //          transmitted, received, loss, total_time_ms);
    // delete the ping sessions, so that we clean up all resources and can create a new ping session
    // we don't have to call delete function in the callback, instead we can call delete function from other tasks
    esp_ping_delete_session(hdl);
    waitforgreen=false;
}
 

static int do_ping_cmd(ip_addr_t target_addr)//int argc, char **argv)
{
    esp_ping_config_t config = ESP_PING_DEFAULT_CONFIG();

         config.timeout_ms = (uint32_t)(200); 
        config.interval_ms = (uint32_t)(200); 
        config.data_size = (uint32_t)(10);
         config.count = (uint32_t)(1);
        //config.tos = (uint32_t)(ping_args.tos->ival[0]);
    //     config.ttl = (uint32_t)(ping_args.ttl->ival[0]);
     //  config.interface = (uint32_t)(ping_args.interface->ival[0])
/*
    // parse IP address
    {
        struct addrinfo hint;
        struct addrinfo *res = NULL;
        memset(&hint, 0, sizeof(hint));
         if (getaddrinfo(t, NULL, &hint, &res) != 0) {
            printf("ping: unknown host %s\n",t);
            return 1;
        }
#ifdef CONFIG_LWIP_IPV4
        if (res->ai_family == AF_INET) {
            struct in_addr addr4 = ((struct sockaddr_in *) (res->ai_addr))->sin_addr;
            inet_addr_to_ip4addr(ip_2_ip4(&target_addr), &addr4);
        }
              #endif
                      freeaddrinfo(res);
                  }
                  */
                  config.target_addr = target_addr;
  //                printf("%s ==   %s\n",ipaddr_ntoa((ip_addr_t*)&target_addr),ipaddr_ntoa((ip_addr_t*)&config.target_addr) );
    /* set callback functions */
    esp_ping_callbacks_t cbs = {
        .cb_args = NULL,
        .on_ping_success = cmd_ping_on_ping_success,
        .on_ping_timeout = cmd_ping_on_ping_timeout,
        .on_ping_end = cmd_ping_on_ping_end
    };
    esp_ping_handle_t ping;
    waitforgreen=true;
    esp_ping_new_session(&config, &cbs, &ping);
    esp_ping_start(ping);
   /// delay(1000);
    //esp_ping_stop(ping);

    return 0;
}

 
  
  
 #include <WiFi.h>


void setupIPS(bool init) {
  if (init)
  {display->fillScreen(0xFFFFFF);
  display->setTextSize(1);
  display->setTextColor(0);
  display->setFont(&FreeMono8pt7b);
  display->setCursor(10,10);
  initWiFi();
  }
  else
  {WiFi.disconnect(true);
  }
    
}

void loopIPS()
 { static int t=2;
ip_addr_t target_addr;
 {char a[20];                                                                
    if(waitforgreen) return; 
    target_addr=IPADDR4_INIT_BYTES(192,168,0,t);
     do_ping_cmd(target_addr);
  display->setCursor(289,200);
  display->fillRect(289, 180 ,30,30,0Xffffff);
  display->setCursor(289,200);
  display->printf("%03d",t);
    t=t+1;
    if(t>254) t=2;
   }
}

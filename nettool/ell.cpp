#if defined(ESP32)
#include "WiFi.h"
#else
#include "ESP8266WiFi.h"
#define log_i(format, ...) Serial.printf(format, ##__VA_ARGS__)
#endif
#include "config.h"
 #define SCAN_INTERVAL 3000

int16_t w, h, text_size, banner_height, graph_baseline, graph_height, channel_width, signal_width;
// RSSI RANGE
#define RSSI_CEILING -40
#define RSSI_FLOOR -100
// Channel color mapping from channel 1 to 14
uint16_t channel_color[] = {
    RGB565_RED, RGB565_ORANGE, RGB565_YELLOW, RGB565_GREEN, RGB565_CYAN, RGB565_BLUE, RGB565_MAGENTA,
    RGB565_RED, RGB565_ORANGE, RGB565_YELLOW, RGB565_GREEN, RGB565_CYAN, RGB565_BLUE, RGB565_MAGENTA};

uint8_t scan_count = 0;

void setup2(bool init)
{if(init)
 {WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
   // Init Display
  w = display->width();
  h = display->height();
  text_size = (h < 200) ? 1 : 2;
  banner_height = text_size * 3 * 4;
  graph_baseline = h - 20;                            // minus 2 text lines
  graph_height = graph_baseline - banner_height - 30; // minus 3 text lines
  channel_width = w / 17;
  signal_width = channel_width * 2;
  display->setTextSize(1);
  display->fillScreen(RGB565_BLACK);
  display->setTextColor(RGB565_RED);
  display->setCursor(0, 10);
  display->print("ESP");
  display->setTextColor(RGB565_WHITE);
  display->print(" WiFi Analyzer");
 }
 else
 {WiFi.disconnect();
 }
}

bool matchBssidPrefix(uint8_t *a, uint8_t *b)
{
  for (uint8_t i = 0; i < 5; i++)
  { // only compare first 5 bytes
    if (a[i] != b[i])
    {
      return false;
    }
  }
  return true;
}

void loop2()
{
  uint8_t ap_count_list[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int32_t noise_list[] = {RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR};
  int32_t peak_list[] = {RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR};
  int16_t peak_id_list[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
  int32_t channel;
  int16_t idx;
  int32_t rssi;
  uint8_t *bssid;
  String ssid;
  uint16_t color;
  int16_t height, offset, text_width;

  // WiFi.scanNetworks will return the number of networks found
#if defined(ESP32)
  int n = WiFi.scanNetworks(false /* async */, true /* show_hidden */, true /* passive */, 500 /* max_ms_per_chan */);
#else
  int n = WiFi.scanNetworks(false /* async */, true /* show_hidden */);
#endif

  // clear old graph
  display->fillRect(0, banner_height, w, h - banner_height, RGB565_BLACK);
  display->setTextSize(0.8);

  if (n == 0)
  {
    display->setTextColor(RGB565_WHITE);
    display->setCursor(0, banner_height);
    display->println("no networks found");
  }
  else
  {
    for (int i = 0; i < n; i++)
    {
      channel = WiFi.channel(i);
      idx = channel - 1;
      rssi = WiFi.RSSI(i);
      bssid = WiFi.BSSID(i);

      // channel peak stat
      if (peak_list[idx] < rssi)
      {
        peak_list[idx] = rssi;
        peak_id_list[idx] = i;
      }

      // check signal come from same AP
      bool duplicate_SSID = false;
      for (int j = 0; j < i; j++)
      {
        if ((WiFi.channel(j) == channel) && matchBssidPrefix(WiFi.BSSID(j), bssid))
        {
          duplicate_SSID = true;
          break;
        }
      }

      if (!duplicate_SSID)
      {
        ap_count_list[idx]++;

        // noise stat
        int32_t noise = rssi - RSSI_FLOOR;
        noise *= noise;
        if (channel > 4)
        {
          noise_list[idx - 4] += noise;
        }
        if (channel > 3)
        {
          noise_list[idx - 3] += noise;
        }
        if (channel > 2)
        {
          noise_list[idx - 2] += noise;
        }
        if (channel > 1)
        {
          noise_list[idx - 1] += noise;
        }
        noise_list[idx] += noise;
        if (channel < 14)
        {
          noise_list[idx + 1] += noise;
        }
        if (channel < 13)
        {
          noise_list[idx + 2] += noise;
        }
        if (channel < 12)
        {
          noise_list[idx + 3] += noise;
        }
        if (channel < 11)
        {
          noise_list[idx + 4] += noise;
        }
      }
    }

    // plot found WiFi info
    for (int i = 0; i < n; i++)
    {
      channel = WiFi.channel(i);
      idx = channel - 1;
      rssi = WiFi.RSSI(i);
      color = channel_color[idx];
      height = constrain(map(rssi, RSSI_FLOOR, RSSI_CEILING, 1, graph_height), 1, graph_height);
      offset = (channel + 1) * channel_width;

      // trim rssi with RSSI_FLOOR
      if (rssi < RSSI_FLOOR)
      {
        rssi = RSSI_FLOOR;
      }

      // plot chart
      // display->drawLine(offset, graph_baseline - height, offset - signal_width, graph_baseline + 1, color);
      // display->drawLine(offset, graph_baseline - height, offset + signal_width, graph_baseline + 1, color);
      display->startWrite();
      display->writeEllipseHelper(offset, graph_baseline + 1, signal_width, height, 0b0011, color);
      display->endWrite();

      if (i == peak_id_list[idx])
      {
        // Print SSID, signal strengh and if not encrypted
        String ssid = WiFi.SSID(i);
           if (ssid.length() == 0)
        {
          ssid = WiFi.BSSIDstr(i);
        }
        text_width = (ssid.length() + 6) * 6;
        if (text_width > w)
        {
          offset = 0;
        }
        else
        {
          offset -= signal_width;
          if ((offset + text_width) > w)
          {
            offset = w - text_width;
          }
        }
        display->setTextColor(color);
        display->setCursor(offset, graph_baseline - 10 - height);
        display->print(ssid);
        display->print('(');
        display->print(rssi);
        display->print(')');
#if defined(ESP32)
        if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN)
#else
        if (WiFi.encryptionType(i) == ENC_TYPE_NONE)
#endif
        {
          display->print('*');
        }
      }
    }
  }

  // print WiFi stat
  display->setTextColor(RGB565_WHITE);
  display->setCursor(0, banner_height);
  display->print(n);
  display->print(" networks found, lesser noise channels: ");
  bool listed_first_channel = false;
  int32_t min_noise = noise_list[0];          // init with channel 1 value
  for (channel = 2; channel <= 11; channel++) // channels 12-14 may not available
  {
    idx = channel - 1;
    log_i("min_noise: %d, noise_list[%d]: %d", min_noise, idx, noise_list[idx]);
    if (noise_list[idx] < min_noise)
    {
      min_noise = noise_list[idx];
    }
  }

  for (channel = 1; channel <= 11; channel++) // channels 12-14 may not available
  {
    idx = channel - 1;
    // check channel with min noise
    if (noise_list[idx] == min_noise)
    {
      if (!listed_first_channel)
      {
        listed_first_channel = true;
      }
      else
      {
        display->print(", ");
      }
      display->print(channel);
    }
  }

  // draw graph base axle
  display->drawFastHLine(0, graph_baseline, display->width(), RGB565_WHITE);
  for (channel = 1; channel <= 14; channel++)
  {
    idx = channel - 1;
    offset = (channel + 1) * channel_width;
    display->setTextColor(channel_color[idx]);
    display->setCursor(offset - ((channel < 10) ? 3 : 6), graph_baseline + 2);
    display->print(channel);
    if (ap_count_list[idx] > 0)
    {
      display->setCursor(offset - ((ap_count_list[idx] < 10) ? 9 : 12), graph_baseline + 8 + 2);
      display->print('{');
      display->print(ap_count_list[idx]);
      display->print('}');
    }
  }

  // Wait a bit before scanning again
 // delay(SCAN_INTERVAL);

  
}

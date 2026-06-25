#pragma once

#ifdef ESP8266
#include <ESP8266WebServer.h>
typedef ESP8266WebServer tWebServer;
#else
#include <WebServer.h>
typedef WebServer tWebServer;
#endif

#include "mlrs-wireless-bridge-webui-html.h"

tWebServer webui_server(80);
bool webui_started = false;

String webui_js_escape(String s)
{
    s.replace("\\", "\\\\");
    s.replace("\"", "\\\"");
    s.replace("\r", "");
    s.replace("\n", "");
    return s;
}

String webui_render_page()
{
    String html = FPSTR(WEBUI_HTML);
    html.replace("%DEVNAME%", device_name);
    html.replace("%WIFIMODE%", String(g_wifi_mode));
    html.replace("%PROTO%", String(g_protocol));
    html.replace("%TCPPORT%", String(g_tcp_port));
    html.replace("%UDPPORT%", String(g_udp_port));
    html.replace("%APPASS%", webui_js_escape(g_password));
    html.replace("%STASSID%", webui_js_escape(g_network_ssid));
    html.replace("%STAPASS%", webui_js_escape(g_network_password));
    return html;
}

void webui_handle_root()
{
    webui_server.send(200, "text/html", webui_render_page());
}

void webui_handle_save()
{
    if (webui_server.hasArg("wifimode")) {
        int m = webui_server.arg("wifimode").toInt();
        if (m >= WIFIMODE_AP_OPEN && m <= WIFIMODE_CLIENT) {
            g_wifi_mode = m;
            preferences.putInt(G_WIFIMODE_STR, g_wifi_mode);
        }
    }

    if (webui_server.hasArg("appass")) {
        String v = webui_server.arg("appass");
        if (v.length() == 0 || (v.length() >= 8 && v.length() <= 63)) {
            g_password = v;
            preferences.putString(G_PASSWORD_STR, g_password);
        }
    }

    if (webui_server.hasArg("stassid")) {
        String v = webui_server.arg("stassid");
        if (v.length() <= 32) {
            g_network_ssid = v;
            preferences.putString(G_NETWORK_SSID_STR, g_network_ssid);
        }
    }

    if (webui_server.hasArg("stapass")) {
        String v = webui_server.arg("stapass");
        if (v.length() == 0 || (v.length() >= 8 && v.length() <= 63)) {
            g_network_password = v;
            preferences.putString(G_NETWORK_PASSWORD_STR, g_network_password);
        }
    }

    if (webui_server.hasArg("proto")) {
        int p = webui_server.arg("proto").toInt();
        if (p == WIRELESS_PROTOCOL_TCP || p == WIRELESS_PROTOCOL_UDP || p == WIRELESS_PROTOCOL_ESPNOW) {
            g_protocol = p;
            preferences.putInt(G_PROTOCOL_STR, g_protocol);
        }
    }

    if (webui_server.hasArg("tcpport")) {
        int v = webui_server.arg("tcpport").toInt();
        if (v >= 1 && v <= 65535) {
            g_tcp_port = v;
            preferences.putInt(G_TCPPORT_STR, g_tcp_port);
        }
    }

    if (webui_server.hasArg("udpport")) {
        int v = webui_server.arg("udpport").toInt();
        if (v >= 1 && v <= 65535) {
            g_udp_port = v;
            preferences.putInt(G_UDPPORT_STR, g_udp_port);
        }
    }

    if (g_wifi_mode == WIFIMODE_AP_PASSWORD && (g_password.length() < 8 || g_password.length() > 63)) {
        g_wifi_mode = WIFIMODE_AP_OPEN;
        preferences.putInt(G_WIFIMODE_STR, g_wifi_mode);
    }

    webui_server.send(200, "text/html", FPSTR(WEBUI_SAVED_HTML));
    delay(500);
    preferences.end();
    ESP.restart();
}

void webui_setup()
{
    webui_server.on("/", HTTP_GET, webui_handle_root);
    webui_server.on("/save", HTTP_POST, webui_handle_save);
    webui_server.onNotFound(webui_handle_root);
    webui_server.begin();
}

void webui_loop()
{
    webui_server.handleClient();
}

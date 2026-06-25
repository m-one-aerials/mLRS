#pragma once

const char WEBUI_HTML[] PROGMEM = R"HTMLPAGE(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>mLRS Wireless Bridge</title>
<style>
body { font-family: system-ui, sans-serif; background:#1b1f24; color:#e6e6e6; margin:0; padding:16px; }
.card { max-width:460px; margin:0 auto; background:#262b32; border-radius:10px; padding:20px; }
h1 { font-size:20px; margin:0 0 4px; }
.sub { color:#9aa4af; font-size:13px; margin-bottom:16px; }
label { display:block; margin:14px 0 4px; font-size:13px; color:#c7d0d9; }
select, input { width:100%; box-sizing:border-box; padding:9px; border-radius:6px; border:1px solid #3a4049; background:#1b1f24; color:#e6e6e6; font-size:14px; }
.hint { font-size:11px; color:#7f8a96; margin-top:3px; }
button { margin-top:20px; width:100%; padding:11px; border:0; border-radius:6px; background:#3b82f6; color:#fff; font-size:15px; font-weight:600; }
.row2 { display:flex; gap:12px; }
.row2 > div { flex:1; }
.hidden { display:none; }
</style>
</head>
<body>
<div class="card">
<h1>mLRS Wireless Bridge</h1>
<div class="sub">%DEVNAME%</div>
<form method="POST" action="/save">
<label>WiFi mode</label>
<select name="wifimode" id="wifimode" onchange="upd()">
<option value="0">Access Point (open)</option>
<option value="1">Access Point (password)</option>
<option value="2">WiFi Client</option>
</select>

<div id="ap_pass_box">
<label>AP password</label>
<input type="text" name="appass" id="appass" maxlength="63">
<div class="hint">8 to 63 characters</div>
</div>

<div id="sta_box">
<label>Network SSID</label>
<input type="text" name="stassid" id="stassid" maxlength="32">
<label>Network password</label>
<input type="text" name="stapass" id="stapass" maxlength="63">
<div class="hint">8 to 63 characters, or empty for an open network</div>
</div>

<label>MAVLink output</label>
<select name="proto" id="proto" onchange="upd()">
<option value="0">TCP</option>
<option value="1">UDP</option>
<option value="6">ESP-NOW</option>
</select>

<div class="row2">
<div id="tcp_box">
<label>TCP port</label>
<input type="number" name="tcpport" id="tcpport" min="1" max="65535">
</div>
<div id="udp_box">
<label>UDP port</label>
<input type="number" name="udpport" id="udpport" min="1" max="65535">
</div>
</div>
<div class="hint" id="espnow_hint">ESP-NOW has no WiFi network, so the web UI is only reachable in an Access Point or Client mode.</div>

<button type="submit">Save &amp; Reboot</button>
</form>
</div>
<script>
var cfg = { wifimode:%WIFIMODE%, proto:%PROTO%, tcpport:%TCPPORT%, udpport:%UDPPORT% };
function show(id, on) { document.getElementById(id).classList.toggle('hidden', !on); }
function upd() {
  var m = parseInt(document.getElementById('wifimode').value);
  var p = parseInt(document.getElementById('proto').value);
  var espnow = (p == 6);
  show('ap_pass_box', m == 1 && !espnow);
  show('sta_box', m == 2 && !espnow);
  show('tcp_box', p == 0);
  show('udp_box', p == 1);
  show('espnow_hint', espnow);
}
document.getElementById('wifimode').value = cfg.wifimode;
document.getElementById('proto').value = cfg.proto;
document.getElementById('tcpport').value = cfg.tcpport;
document.getElementById('udpport').value = cfg.udpport;
document.getElementById('appass').value = "%APPASS%";
document.getElementById('stassid').value = "%STASSID%";
document.getElementById('stapass').value = "%STAPASS%";
upd();
</script>
</body>
</html>
)HTMLPAGE";


const char WEBUI_SAVED_HTML[] PROGMEM = R"HTMLPAGE(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>mLRS Wireless Bridge</title>
<style>
body { font-family: system-ui, sans-serif; background:#1b1f24; color:#e6e6e6; text-align:center; padding-top:60px; }
.box { max-width:360px; margin:0 auto; background:#262b32; border-radius:10px; padding:24px; }
</style>
</head>
<body>
<div class="box">
<h2>Saved</h2>
<p>The bridge is rebooting with the new settings.</p>
<p>Reconnect to the bridge and reload this page.</p>
</div>
</body>
</html>
)HTMLPAGE";

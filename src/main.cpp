#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Preferences.h>
#include <time.h>

// --- CONFIGURACIÓN DE RED ---
#define WIFI_SSID "AndroidAP2421"
#define WIFI_PASSWORD "91e5cfaa61b8"

// --- CONFIGURACIÓN DE PINES ---
#define DHTPIN 4
#define DHTTYPE DHT22
#define RELAY_PIN 12
#define SOIL_PIN 34

// --- CONFIGURACIÓN DE MUESTREO Y GRÁFICOS ---
#define SENSOR_READ_INTERVAL_MS 2000 // Leer sensores para vista en vivo cada 2 segundos
#define DATA_SAMPLE_INTERVAL_MIN 5   // Guardar una muestra para el gráfico cada 5 minutos
#define MAX_DATA_POINTS (1440 / DATA_SAMPLE_INTERVAL_MIN) // Puntos para 24 horas (24*60/5 = 288)

// --- OBJETOS GLOBALES ---
DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);
Preferences prefs;

// --- ESTRUCTURA PARA PUNTOS DE DATOS DE GRÁFICOS ---
struct DataPoint {
  time_t timestamp;
  float temp;
  float hum;
  int soil;
};

// --- VARIABLES DE ESTADO Y CONTROL ---
float temperature = 0;
float ambientHumidity = 0; // <-- NUEVA VARIABLE
int soilMoisture = 0;
bool relayState = false;

// --- VARIABLES CONFIGURABLES (con valores por defecto) ---
int humidityThreshold = 500;
int irrigationDuration = 10;
int scheduleHour1 = 7, scheduleMin1 = 0;
int scheduleHour2 = 18, scheduleMin2 = 30;

// --- VARIABLES DE TEMPORIZACIÓN ---
time_t irrigationEndTime = 0;
unsigned long lastSensorReadTime = 0;
unsigned long lastDataSampleTime = 0;
unsigned long lastScheduleCheckTime = 0;

// --- ARRAYS CIRCULARES PARA GRÁFICOS DIARIOS ---
DataPoint dailyData[MAX_DATA_POINTS];
int dataIndex = 0;
bool bufferFilled = false;

// ------------------------------------------------------------
// FUNCIONES DE CONTROL DE RIEGO
// ------------------------------------------------------------

void desactivarRiego() {
  digitalWrite(RELAY_PIN, LOW);
  relayState = false;
  irrigationEndTime = 0;
  Serial.println("Riego DESACTIVADO.");
}

void activarRiego() {
  if (!relayState) {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = true;
    irrigationEndTime = time(nullptr) + (irrigationDuration * 60);
    Serial.print("Riego ACTIVADO por ");
    Serial.print(irrigationDuration);
    Serial.println(" minutos.");
  }
}

// ------------------------------------------------------------
// FUNCIONES DE CONFIGURACIÓN INICIAL (SETUP)
// ------------------------------------------------------------

void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. IP: " + WiFi.localIP().toString());
}

void setupNTP() {
  configTime(3600, 3600, "pool.ntp.org");
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error obteniendo hora NTP. Reintentando...");
    delay(2000);
    setupNTP();
  } else {
    Serial.println("NTP sincronizado y hora local obtenida.");
  }
}

void loadPreferences() {
  prefs.begin("riego", true);
  humidityThreshold = prefs.getInt("umbral", 500);
  irrigationDuration = prefs.getInt("duracion", 10);
  scheduleHour1 = prefs.getInt("h1", 7);
  scheduleMin1 = prefs.getInt("m1", 0);
  scheduleHour2 = prefs.getInt("h2", 18);
  scheduleMin2 = prefs.getInt("m2", 30);
  prefs.end();
  Serial.println("Configuración cargada desde la memoria.");
}

// ------------------------------------------------------------
// HANDLERS DEL SERVIDOR WEB
// ------------------------------------------------------------

void handleOn() { activarRiego(); server.sendHeader("Location", "/"); server.send(303); }
void handleOff() { desactivarRiego(); server.sendHeader("Location", "/"); server.send(303); }

void handleSetThreshold() {
  if (server.hasArg("threshold")) {
    humidityThreshold = server.arg("threshold").toInt();
    prefs.begin("riego", false); prefs.putInt("umbral", humidityThreshold); prefs.end();
  }
  server.sendHeader("Location", "/"); server.send(303);
}

void handleSetDuration() {
  if (server.hasArg("duration")) {
    irrigationDuration = server.arg("duration").toInt();
    prefs.begin("riego", false); prefs.putInt("duracion", irrigationDuration); prefs.end();
  }
  server.sendHeader("Location", "/"); server.send(303);
}

void handleSetSchedule() {
  if (server.hasArg("h1")) scheduleHour1 = server.arg("h1").toInt();
  if (server.hasArg("m1")) scheduleMin1 = server.arg("m1").toInt();
  if (server.hasArg("h2")) scheduleHour2 = server.arg("h2").toInt();
  if (server.hasArg("m2")) scheduleMin2 = server.arg("m2").toInt();
  prefs.begin("riego", false);
  prefs.putInt("h1", scheduleHour1); prefs.putInt("m1", scheduleMin1);
  prefs.putInt("h2", scheduleHour2); prefs.putInt("m2", scheduleMin2);
  prefs.end();
  server.sendHeader("Location", "/"); server.send(303);
}

// Envía los datos para los gráficos
void handleChartData() {
  String json = "[";
  int count = bufferFilled ? MAX_DATA_POINTS : dataIndex;
  for (int i = 0; i < count; i++) {
    int idx = (dataIndex + i) % MAX_DATA_POINTS;
    if (!bufferFilled && idx >= dataIndex) continue;
    DataPoint dp = dailyData[idx];
    struct tm timeinfo;
    localtime_r(&dp.timestamp, &timeinfo);
    char timeStr[6];
    strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
    json += "{";
    json += "\"time\":\"" + String(timeStr) + "\",";
    json += "\"temp\":" + String(dp.temp, 1) + ",";
    json += "\"hum\":" + String(dp.hum, 1) + ","; // <-- NUEVO DATO
    json += "\"soil\":" + String(dp.soil);
    json += "}";
    if (i < count - 1) json += ",";
  }
  json += "]";
  server.send(200, "application/json", json);
}

// Envía el estado actual del sistema (para actualizaciones en tiempo real)
void handleStatus() {
  long countdown = 0;
  if (relayState && irrigationEndTime > 0) {
    time_t now = time(nullptr);
    if (irrigationEndTime > now) {
      countdown = irrigationEndTime - now;
    }
  }
  String json = "{";
  json += "\"temp\":" + String(temperature, 1) + ",";
  json += "\"ambientHum\":" + String(ambientHumidity, 1) + ","; // <-- NUEVO DATO
  json += "\"soil\":" + String(soilMoisture) + ",";
  json += "\"relay\":" + String(relayState ? "true" : "false") + ",";
  json += "\"countdown\":" + String(countdown);
  json += "}";
  server.send(200, "application/json", json);
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Riego Automático Inteligente</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css" rel="stylesheet">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style> body { background-color: #f4f4f9; } .card { box-shadow: 0 4px 8px rgba(0,0,0,0.1); } </style>
</head>
<body class="p-4">
    <div class="container">
        <h2 class="mb-4 text-center text-primary">Panel de Control de Riego</h2>

        <div class="row">
            <div class="col-lg-4 mb-4"><div class="card"><div class="card-body"><canvas id="tempChart"></canvas></div></div></div>
            <div class="col-lg-4 mb-4"><div class="card"><div class="card-body"><canvas id="humidityChart"></canvas></div></div></div>
            <div class="col-lg-4 mb-4"><div class="card"><div class="card-body"><canvas id="soilChart"></canvas></div></div></div>
        </div>

        <div class="card p-3 mb-4">
            <h5 class="card-title">Estado Actual (en tiempo real)</h5>
            <p><strong>Temperatura:</strong> <span id="currentTemp">--</span> °C</p>
            <p><strong>Humedad Ambiente:</strong> <span id="currentAmbientHum">--</span> %</p>
            <p><strong>Humedad del Suelo:</strong> <span id="currentSoil">--</span></p>
            <p><strong>Estado del Riego:</strong> <span id="relayStatus" class="fw-bold">--</span></p>
            <p><strong>Tiempo de riego restante:</strong> <span id="countdown" class="fw-bold text-success">--</span></p>
        </div>

        <div class="card p-3 mb-4">
            <h5 class="card-title">Control Manual</h5>
            <a href="/on" class="btn btn-success me-2 mb-2">Encender Riego Manual</a>
            <a href="/off" class="btn btn-danger mb-2">Apagar Riego</a>
        </div>
        
        <div class="card p-3 mb-4">
            <h5 class="card-title">Configuración</h5>
            <div class="row">
                <div class="col-md-6">
                    <form action="/setThreshold" method="GET" class="mb-3">
                        <label for="threshold" class="form-label">Umbral de Humedad (riega si es menor):</label>
                        <input type="number" id="threshold" name="threshold" value=")rawliteral" + String(humidityThreshold) + R"rawliteral(" class="form-control mb-2">
                        <button type="submit" class="btn btn-primary">Guardar Umbral</button>
                    </form>
                </div>
                <div class="col-md-6">
                    <form action="/setDuration" method="GET" class="mb-3">
                        <label for="duration" class="form-label">Duración del Riego (minutos):</label>
                        <input type="number" id="duration" name="duration" value=")rawliteral" + String(irrigationDuration) + R"rawliteral(" class="form-control mb-2">
                        <button type="submit" class="btn btn-primary">Guardar Duración</button>
                    </form>
                </div>
            </div>
        </div>

        <div class="card p-3">
             <h5 class="card-title">Programación de Horarios</h5>
             <form action="/setSchedule" method="GET">
                <div class="row">
                    <div class="col-md-6 mb-3">
                        <label class="form-label">Horario 1:</label>
                        <div class="input-group">
                            <input type="number" name="h1" value=")rawliteral" + String(scheduleHour1) + R"rawliteral(" class="form-control" placeholder="HH" min="0" max="23">
                            <span class="input-group-text">:</span>
                            <input type="number" name="m1" value=")rawliteral" + String(scheduleMin1) + R"rawliteral(" class="form-control" placeholder="MM" min="0" max="59">
                        </div>
                    </div>
                    <div class="col-md-6 mb-3">
                        <label class="form-label">Horario 2:</label>
                        <div class="input-group">
                            <input type="number" name="h2" value=")rawliteral" + String(scheduleHour2) + R"rawliteral(" class="form-control" placeholder="HH" min="0" max="23">
                            <span class="input-group-text">:</span>
                            <input type="number" name="m2" value=")rawliteral" + String(scheduleMin2) + R"rawliteral(" class="form-control" placeholder="MM" min="0" max="59">
                        </div>
                    </div>
                </div>
                <button type="submit" class="btn btn-primary">Guardar Horarios</button>
            </form>
        </div>
    </div>

<script>
    let tempChart, humidityChart, soilChart;

    function formatCountdown(seconds) {
        if (seconds <= 0) return "Inactivo";
        const mins = Math.floor(seconds / 60);
        const secs = seconds % 60;
        return `${String(mins).padStart(2, '0')}:${String(secs).padStart(2, '0')}`;
    }

    async function updateStatus() {
        try {
            const response = await fetch('/status');
            if (!response.ok) return;
            const data = await response.json();
            
            document.getElementById('currentTemp').textContent = data.temp;
            document.getElementById('currentAmbientHum').textContent = data.ambientHum;
            document.getElementById('currentSoil').textContent = data.soil;
            
            const statusSpan = document.getElementById('relayStatus');
            if (data.relay) {
                statusSpan.textContent = 'ACTIVO';
                statusSpan.className = 'fw-bold text-success';
            } else {
                statusSpan.textContent = 'APAGADO';
                statusSpan.className = 'fw-bold text-danger';
            }
            document.getElementById('countdown').textContent = formatCountdown(data.countdown);
        } catch (error) {
            console.error('Error updating status:', error);
        }
    }

    async function updateCharts() {
        try {
            const response = await fetch('/chartdata');
            if (!response.ok) return;
            const data = await response.json();

            const labels = data.map(d => d.time);
            const tempData = data.map(d => d.temp);
            const humData = data.map(d => d.hum);
            const soilData = data.map(d => d.soil);
            
            tempChart.data.labels = labels;
            tempChart.data.datasets[0].data = tempData;
            tempChart.update();

            humidityChart.data.labels = labels;
            humidityChart.data.datasets[0].data = humData;
            humidityChart.update();

            soilChart.data.labels = labels;
            soilChart.data.datasets[0].data = soilData;
            soilChart.update();
        } catch (error) {
            console.error('Error updating charts:', error);
        }
    }

    document.addEventListener('DOMContentLoaded', () => {
        const createChart = (ctx, label, title, color) => {
            return new Chart(ctx, {
                type: 'line',
                data: { labels: [], datasets: [{ label: label, data: [], borderColor: color, tension: 0.1, fill: false }] },
                options: { responsive: true, plugins: { title: { display: true, text: title }}}
            });
        };

        tempChart = createChart(document.getElementById('tempChart').getContext('2d'), 'Temperatura (°C)', 'Temperatura 24h', 'rgb(255, 99, 132)');
        humidityChart = createChart(document.getElementById('humidityChart').getContext('2d'), 'Humedad Amb. (%)', 'Humedad Ambiente 24h', 'rgb(75, 192, 192)');
        soilChart = createChart(document.getElementById('soilChart').getContext('2d'), 'Humedad Suelo (ADC)', 'Humedad Suelo 24h', 'rgb(54, 162, 235)');
        
        updateStatus();
        updateCharts();
        setInterval(updateStatus, 2000);
        setInterval(updateCharts, 60000);
    });
</script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

// ------------------------------------------------------------
// SETUP
// ------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  setupWiFi();
  setupNTP();
  loadPreferences();

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/setThreshold", handleSetThreshold);
  server.on("/setDuration", handleSetDuration);
  server.on("/setSchedule", handleSetSchedule);
  server.on("/chartdata", handleChartData);
  server.on("/status", handleStatus);

  server.begin();
  Serial.println("Servidor web iniciado.");
}

// ------------------------------------------------------------
// LOOP PRINCIPAL (NO BLOQUEANTE)
// ------------------------------------------------------------
void loop() {
  server.handleClient();

  unsigned long currentMillis = millis();

  // --- Tarea 1: Lectura de sensores en "tiempo real" (cada 2 segundos) ---
  if (currentMillis - lastSensorReadTime >= SENSOR_READ_INTERVAL_MS) {
    lastSensorReadTime = currentMillis;

    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();

    if (!isnan(newTemp) && !isnan(newHum)) {
        temperature = newTemp;
        ambientHumidity = newHum;
    } else {
        Serial.println("Error leyendo del sensor DHT!");
    }
    soilMoisture = analogRead(SOIL_PIN);
  }

  // --- Tarea 2: Guardado de datos para gráficos (cada 5 minutos) ---
  if (currentMillis - lastDataSampleTime >= (DATA_SAMPLE_INTERVAL_MIN * 60 * 1000)) {
    lastDataSampleTime = currentMillis;
    
    // Usa los últimos valores leídos, no es necesario leer de nuevo
    if (!isnan(temperature)) {
      Serial.print("Muestra guardada para gráfico: Temp=");
      Serial.print(temperature);
      Serial.print("C, Hum.Amb=");
      Serial.print(ambientHumidity);
      Serial.print("%, Hum.Suelo=");
      Serial.println(soilMoisture);
      
      dailyData[dataIndex].timestamp = time(nullptr);
      dailyData[dataIndex].temp = temperature;
      dailyData[dataIndex].hum = ambientHumidity;
      dailyData[dataIndex].soil = soilMoisture;
      dataIndex = (dataIndex + 1) % MAX_DATA_POINTS;
      if (!bufferFilled && dataIndex == 0) {
        bufferFilled = true;
      }
    }
  }

  // --- Tarea 3: Lógica de riego automático (cada 30 segundos) ---
  if (currentMillis - lastScheduleCheckTime >= 30000) {
    lastScheduleCheckTime = currentMillis;
    
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 100)) {
      bool isScheduleTime1 = (timeinfo.tm_hour == scheduleHour1 && timeinfo.tm_min == scheduleMin1);
      bool isScheduleTime2 = (timeinfo.tm_hour == scheduleHour2 && timeinfo.tm_min == scheduleMin2);
      
      if (isScheduleTime1 || isScheduleTime2) {
        Serial.println("Activando riego por horario.");
        activarRiego();
      }

      if (!relayState && soilMoisture < humidityThreshold && soilMoisture > 0) {
        Serial.println("Activando riego por baja humedad de suelo.");
        activarRiego();
      }
    }
  }

  // --- Tarea 4: Control del temporizador de riego (se ejecuta constantemente) ---
  if (relayState && time(nullptr) >= irrigationEndTime) {
    Serial.println("Temporizador de riego finalizado.");
    desactivarRiego();
  }
}
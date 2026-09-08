#ifndef INDEX_H
#define INDEX_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Weather Dashboard</title>
    <style>
        :root {
            --bg-color: #0f172a;
            --card-bg: #1e293b;
            --text-primary: #f8fafc;
            --text-secondary: #94a3b8;
            --accent-local: #38bdf8;
            --accent-internet: #34d399;
            --border-color: #334155;
        }
        body {
            font-family: system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            background-color: var(--bg-color);
            color: var(--text-primary);
            margin: 0;
            padding: 20px;
            display: flex;
            justify-content: center;
            min-height: 100vh;
        }
        .container {
            width: 100%;
            max-width: 950px;
        }
        h1 {
            text-align: center;
            font-size: 1.8rem;
            margin-bottom: 5px;
            letter-spacing: 0.5px;
        }
        .location-sub {
            text-align: center;
            color: var(--text-secondary);
            margin-bottom: 25px;
            font-size: 0.95rem;
        }
        .section-title {
            font-size: 1.1rem;
            font-weight: 600;
            margin: 25px 0 10px 5px;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        .local-title { color: var(--accent-local); }
        .internet-title { color: var(--accent-internet); }
        
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
        }
        .card {
            background-color: var(--card-bg);
            border: 1px solid var(--border-color);
            border-radius: 12px;
            padding: 18px 12px;
            text-align: center;
            box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
        }
        .label {
            font-size: 0.82rem;
            color: var(--text-secondary);
            text-transform: uppercase;
            letter-spacing: 0.5px;
            margin-bottom: 8px;
        }
        .value {
            font-size: 1.6rem;
            font-weight: 700;
        }
        .forecast-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
            gap: 12px;
        }
        .forecast-card {
            background-color: var(--card-bg);
            border: 1px solid var(--border-color);
            border-radius: 12px;
            padding: 16px 10px;
            text-align: center;
        }
        .forecast-day {
            font-size: 0.85rem;
            color: var(--text-secondary);
            font-weight: 600;
            margin-bottom: 6px;
        }
        .forecast-icon {
            font-size: 1.8rem;
            margin: 6px 0;
        }
        .forecast-temps {
            font-size: 1rem;
            font-weight: 600;
        }
        .temp-max { color: #f87171; }
        .temp-min { color: #60a5fa; }
        .footer {
            text-align: center;
            font-size: 0.75rem;
            color: var(--text-secondary);
            margin-top: 35px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Weather Dashboard</h1>
        <div class="location-sub">&#128205; Tel Aviv, Israel</div>
        
        <div class="section-title local-title">Local Sensors</div>
        <div class="grid">
            <div class="card">
                <div class="label">&#127777;&#65039; Temperature</div>
                <div class="value" id="local-temp">-- &deg;C</div>
            </div>
            <div class="card">
                <div class="label">&#128167; Humidity</div>
                <div class="value" id="local-hum">-- %</div>
            </div>
            <div class="card">
                <div class="label">&#128200; Pressure</div>
                <div class="value" id="local-press">-- hPa</div>
            </div>
        </div>

        <div class="section-title internet-title">Current Conditions</div>
        <div class="grid">
            <div class="card">
                <div class="label">&#127777;&#65039; Ext. Temp</div>
                <div class="value" id="ext-temp">-- &deg;C</div>
            </div>
            <div class="card">
                <div class="label">&#128167; Ext. Humidity</div>
                <div class="value" id="ext-hum">-- %</div>
            </div>
            <div class="card">
                <div class="label">&#127788;&#65039; Wind Speed</div>
                <div class="value" id="ext-wind">-- km/h</div>
            </div>
        </div>

        <div class="section-title internet-title">5-Day Forecast</div>
        <div class="forecast-grid" id="forecast-container"></div>

        <div class="footer">ESP32-C3 Super Mini &bull; Auto-refreshing every 10m</div>
    </div>

    <script>
        function getWeatherIcon(code) {
            if (code === 0) return '&#9728;&#65039;';
            if (code === 1 || code === 2) return '&#9925;';
            if (code === 3) return '&#9729;&#65039;';
            if (code >= 51 && code <= 67) return '&#127783;&#65039;';
            if (code >= 71 && code <= 77) return '&#10052;&#65039;';
            if (code >= 95) return '&#9889;';
            return '&#127777;&#65039;';
        }

        function formatDay(dateStr) {
            const date = new Date(dateStr);
            return date.toLocaleDateString('en-US', { weekday: 'short', month: 'numeric', day: 'numeric' });
        }

        function updateData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('local-temp').innerHTML = data.local.temperature !== null ? data.local.temperature.toFixed(1) + " &deg;C" : "Disconnected";
                    document.getElementById('local-hum').innerHTML = data.local.humidity !== null ? data.local.humidity.toFixed(1) + " %" : "Disconnected";
                    document.getElementById('local-press').innerHTML = data.local.pressure !== null ? data.local.pressure.toFixed(1) + " hPa" : "Disconnected";
                    
                    if(data.internet && data.internet.current) {
                        document.getElementById('ext-temp').innerHTML = data.internet.current.external_temp !== undefined ? data.internet.current.external_temp.toFixed(1) + " &deg;C" : "-- &deg;C";
                        document.getElementById('ext-hum').innerHTML = data.internet.current.external_humidity !== undefined ? data.internet.current.external_humidity.toFixed(1) + " %" : "-- %";
                        document.getElementById('ext-wind').innerHTML = data.internet.current.wind_speed !== undefined ? data.internet.current.wind_speed.toFixed(1) + " km/h" : "-- km/h";
                    }

                    if(data.internet && data.internet.daily) {
                        const container = document.getElementById('forecast-container');
                        container.innerHTML = '';
                        data.internet.daily.forEach(day => {
                            const icon = getWeatherIcon(day.code);
                            const dayName = formatDay(day.date);
                            const card = document.createElement('div');
                            card.className = 'forecast-card';
                            card.innerHTML = `
                                <div class="forecast-day">${dayName}</div>
                                <div class="forecast-icon">${icon}</div>
                                <div class="forecast-temps">
                                    <span class="temp-max">${Math.round(day.max)}&deg;</span> / 
                                    <span class="temp-min">${Math.round(day.min)}&deg;</span>
                                </div>
                            `;
                            container.appendChild(card);
                        });
                    }
                })
                .catch(err => console.error('Error fetching data:', err));
        }

        setInterval(updateData, 600000);
        updateData();
    </script>
</body>
</html>
)rawliteral";

#endif
const ICONS = {
  Clear: 'icons/clear.png',
  Partly_Cloudy: 'icons/partly-cloudy.png',
  Clouds: 'icons/clouds.png',
  Rain: 'icons/rain.png',
  Snow: 'icons/snow.png',
  Thunderstorm: 'icons/thunderstorm.png',
  Mist: 'icons/mist.png'
};

const ICON_RASTER_SIZE = 16;

const elements = {
  city: document.getElementById('cityName'),
  condition: document.getElementById('weatherPreview'),
  temperature: document.getElementById('temperature'),
  high: document.getElementById('highTemperature'),
  low: document.getElementById('lowTemperature'),
  feelsLike: document.getElementById('feelsLike'),
  rainChance: document.getElementById('rainChance'),
  humidity: document.getElementById('humidity'),
  windSpeed: document.getElementById('windSpeed'),
  windDirection: document.getElementById('windDirection'),
  receipt: document.getElementById('receiptPreview')
};

const weatherData = window.weatherApiData;
const currentWeather = weatherData.current;
const forecastSamples = weatherData.forecast;

elements.city.value = weatherData.location;
elements.condition.value = currentWeather.condition;
elements.temperature.value = currentWeather.temperatureC;
elements.high.value = currentWeather.highC;
elements.low.value = currentWeather.lowC;
elements.feelsLike.value = currentWeather.feelsLikeC;
elements.rainChance.value = currentWeather.rainChancePercent;
elements.humidity.value = currentWeather.humidityPercent;
elements.windSpeed.value = currentWeather.windSpeedKmh;
elements.windDirection.value = currentWeather.windDirection;

function iconFor(condition) {
  return ICONS[condition] || ICONS.Mist;
}

function numericValue(element, fallback = 0) {
  const value = Number(element.value);
  return Number.isFinite(value) ? value : fallback;
}

function formatTemperature(value) {
  return `${Number(value).toFixed(1)}°C`;
}

function nextForecastTimes(now = new Date()) {
  return forecastSamples.map((_, index) => {
    const time = new Date(now);
    time.setMinutes(0, 0, 0);
    time.setHours(now.getHours() + (index + 1) * 3);
    return `${String(time.getHours()).padStart(2, '0')}:00`;
  });
}

function renderReceipt() {
  const now = new Date();
  const time = now.toLocaleTimeString('en-GB', {
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit',
    hour12: false
  });
  const date = `${String(now.getDate()).padStart(2, '0')}.${String(now.getMonth() + 1).padStart(2, '0')}.${now.getFullYear()}`;
  const condition = elements.condition.value;
  const times = nextForecastTimes(now);
  const forecasts = forecastSamples.map((forecast, index) => `
    <div class="forecast-row">
      <span class="forecast-time">${times[index]}</span>
      <img class="forecast-icon" src="${iconFor(forecast.condition)}" alt="${forecast.condition}" />
      <span class="forecast-temp">${formatTemperature(forecast.temperatureC)}</span>
      <span class="forecast-rain">${forecast.rainChancePercent}%</span>
    </div>
  `).join('');

  elements.receipt.innerHTML = `
    <p class="clock">${time}</p>
    <div class="receipt-location-date">
      <span class="receipt-location">${escapeHtml(elements.city.value || 'Location')}</span>
      <time class="receipt-date">${date}</time>
    </div>
    <hr class="rule" />
    <section class="current-weather" aria-label="Current weather">
      <img class="current-icon" src="${iconFor(condition)}" alt="${escapeHtml(condition)}" />
      <div class="current-readings">
        <div class="current-temperature">${formatTemperature(numericValue(elements.temperature))}</div>
        <div class="condition-label">${escapeHtml(condition)}</div>
        <div class="high-low">
          <div><span class="high-low-label">High</span><span class="high-low-value">${formatTemperature(numericValue(elements.high))}</span></div>
          <div><span class="high-low-label">Low</span><span class="high-low-value">${formatTemperature(numericValue(elements.low))}</span></div>
        </div>
      </div>
    </section>
    <hr class="rule" />
    <section class="metrics" aria-label="Weather details">
      <div class="metric"><span class="metric-label">Feels like</span><span class="metric-value">${formatTemperature(numericValue(elements.feelsLike))}</span></div>
      <div class="metric"><span class="metric-label">Rain</span><span class="metric-value">${numericValue(elements.rainChance)}%</span></div>
      <div class="metric"><span class="metric-label">Humidity</span><span class="metric-value">${numericValue(elements.humidity)}%</span></div>
      <div class="metric"><span class="metric-label">Wind</span><span class="metric-value">${numericValue(elements.windSpeed).toFixed(1)} km/h ${escapeHtml(elements.windDirection.value)}</span></div>
    </section>
    <hr class="rule" />
    <h2 class="forecast-heading">Next 12 hours · 3-hour steps</h2>
    ${forecasts}
    <hr class="rule" />
    <p class="challenge-title">HackClub Display Challange</p>
    <hr class="rule" />
  `;

  elements.receipt.querySelectorAll('img').forEach((image) => {
    image.addEventListener('error', () => {
      if (!image.src.endsWith('/mist.png')) image.src = 'icons/mist.png';
    }, { once: true });
  });
}

function escapeHtml(value) {
  return String(value).replace(/[&<>"']/g, (character) => ({
    '&': '&amp;',
    '<': '&lt;',
    '>': '&gt;',
    '"': '&quot;',
    "'": '&#39;'
  })[character]);
}

function loadIcon(path) {
  return new Promise((resolve) => {
    const image = new Image();
    image.onload = () => resolve(image);
    image.onerror = () => {
      if (path === ICONS.Mist) {
        resolve(null);
        return;
      }
      const fallback = new Image();
      fallback.onload = () => resolve(fallback);
      fallback.onerror = () => resolve(null);
      fallback.src = ICONS.Mist;
    };
    image.src = path;
  });
}

function drawReceiptIcon(context, image, x, y, size) {
  const raster = document.createElement('canvas');
  raster.width = ICON_RASTER_SIZE;
  raster.height = ICON_RASTER_SIZE;
  const rasterContext = raster.getContext('2d');
  if (!rasterContext) throw new Error('Your browser could not create an icon raster canvas.');

  const scale = Math.min(
    ICON_RASTER_SIZE / image.naturalWidth,
    ICON_RASTER_SIZE / image.naturalHeight
  );
  const width = image.naturalWidth * scale;
  const height = image.naturalHeight * scale;
  rasterContext.drawImage(
    image,
    (ICON_RASTER_SIZE - width) / 2,
    (ICON_RASTER_SIZE - height) / 2,
    width,
    height
  );

  context.imageSmoothingEnabled = false;
  context.drawImage(raster, x, y, size, size);
}

function drawRule(context, y) {
  context.beginPath();
  context.moveTo(18, y);
  context.lineTo(366, y);
  context.lineWidth = 2;
  context.strokeStyle = '#222';
  context.stroke();
}

async function exportReceiptPng() {
  const width = 384;
  const height = Math.ceil(elements.receipt.getBoundingClientRect().height);
  const canvas = document.createElement('canvas');
  canvas.width = width;
  canvas.height = height;
  const context = canvas.getContext('2d');
  if (!context) throw new Error('Your browser could not create an image canvas.');

  context.fillStyle = '#fff';
  context.fillRect(0, 0, width, height);
  context.fillStyle = '#17191c';
  context.textBaseline = 'alphabetic';

  const now = new Date();
  context.textAlign = 'center';
  context.font = '800 48px "Courier New"';
  context.fillText(now.toLocaleTimeString('en-GB', { hour12: false }), width / 2, 58);

  context.textAlign = 'left';
  context.font = '700 14px Arial';
  context.fillText(elements.city.value || 'Location', 18, 88, 245);
  context.textAlign = 'right';
  context.font = '14px Arial';
  context.fillText(`${String(now.getDate()).padStart(2, '0')}.${String(now.getMonth() + 1).padStart(2, '0')}.${now.getFullYear()}`, 366, 88);
  drawRule(context, 105);

  const currentIcon = await loadIcon(iconFor(elements.condition.value));
  if (currentIcon) drawReceiptIcon(context, currentIcon, 20, 118, 128);
  context.textAlign = 'left';
  context.font = '800 54px "Arial Narrow", Arial';
  context.fillText(formatTemperature(numericValue(elements.temperature)), 160, 174);
  context.font = '13px Arial';
  context.fillText(elements.condition.value, 162, 195, 190);
  context.font = '700 10px Arial';
  context.fillText('HIGH', 162, 222);
  context.fillText('LOW', 263, 222);
  context.font = '700 13px Arial';
  context.fillText(formatTemperature(numericValue(elements.high)), 162, 239);
  context.fillText(formatTemperature(numericValue(elements.low)), 263, 239);
  drawRule(context, 265);

  const metrics = [
    ['Feels like', formatTemperature(numericValue(elements.feelsLike))],
    ['Rain', `${numericValue(elements.rainChance)}%`],
    ['Humidity', `${numericValue(elements.humidity)}%`],
    ['Wind', `${numericValue(elements.windSpeed).toFixed(1)} km/h ${elements.windDirection.value}`]
  ];
  context.textAlign = 'center';
  metrics.forEach(([label, value], index) => {
    const x = 18 + index * 87;
    context.font = '700 10px Arial';
    context.fillText(label, x + 43, 288, 84);
    context.font = '700 13px Arial';
    context.fillText(value, x + 43, 308, 84);
  });
  drawRule(context, 318);

  context.textAlign = 'left';
  context.font = '800 11px "Arial Narrow", Arial';
  context.fillText('NEXT 12 HOURS · 3-HOUR STEPS', 18, 342);
  const times = nextForecastTimes(now);
  for (let index = 0; index < forecastSamples.length; index += 1) {
    const y = 352 + index * 38;
    context.beginPath();
    context.moveTo(18, y);
    context.lineTo(366, y);
    context.lineWidth = 1;
    context.strokeStyle = '#c8c8c8';
    context.stroke();
    const forecast = forecastSamples[index];
    const icon = await loadIcon(iconFor(forecast.condition));
    context.font = '700 12px Arial';
    context.textAlign = 'left';
    context.fillText(times[index], 24, y + 25);
    if (icon) drawReceiptIcon(context, icon, 101, y + 4, 30);
    context.textAlign = 'center';
    context.fillText(formatTemperature(forecast.temperatureC), 220, y + 25);
    context.fillText(`${forecast.rainChancePercent}%`, 320, y + 25);
  }

  drawRule(context, 517);
  context.textAlign = 'center';
  context.font = '800 13px "Trebuchet MS", Arial';
  context.fillText('HackClub Display Challange', width / 2, 545);
  drawRule(context, 560);

  const link = document.createElement('a');
  link.download = 'weather_receipt.png';
  link.href = canvas.toDataURL('image/png');
  link.click();
}

document.querySelectorAll('.sidebar input, .sidebar select').forEach((input) => {
  input.addEventListener('input', renderReceipt);
  input.addEventListener('change', renderReceipt);
});
document.getElementById('exportButton').addEventListener('click', () => {
  exportReceiptPng().catch((error) => window.alert(`Could not export receipt: ${error.message}`));
});

renderReceipt();
window.setInterval(renderReceipt, 1000);

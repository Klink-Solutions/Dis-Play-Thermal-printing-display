// Replace these example values with fields mapped from the weather API response.
window.weatherApiData = {
  location: 'Munich, Germany',
  current: {
    condition: 'Clear',
    temperatureC: 18.6,
    highC: 21.4,
    lowC: 14.2,
    feelsLikeC: 17.8,
    rainChancePercent: 20,
    humidityPercent: 61,
    windSpeedKmh: 12.4,
    windDirection: 'S'
  },
  forecast: [
    { condition: 'Partly Cloudy', temperatureC: 19.2, rainChancePercent: 10 },
    { condition: 'Clouds', temperatureC: 18.1, rainChancePercent: 20 },
    { condition: 'Rain', temperatureC: 16.8, rainChancePercent: 65 },
    { condition: 'Clouds', temperatureC: 15.9, rainChancePercent: 35 }
  ]
};

var Config = function(name){
  var load = function(){
    try {
      var config = localStorage.getItem(name);
      if(config !== null){
        return JSON.parse(config);
      }
    }catch(e){}
    return {};
  };
  var store = function(data){
    try{
      localStorage.setItem(name, JSON.stringify(data));
    }catch(e){}
  };
  return {
    load: load,
    store: store
  }
};

var Weather = function(pebble){
  var METHOD = 'GET';
  var BASE_URL = 'http://api.openweathermap.org/data/2.5/weather';
  var API_ID = 'b8c060512b6a117879f2636c41bbb67b';
  var ICONS = {
    '01d': 'a',
    '02d': 'b',
    '03d': 'c',
    '04d': 'd',
    '09d': 'e',
    '10d': 'f',
    '11d': 'g',
    '13d': 'h',
    '50d': 'i',
    '01n': 'A',
    '02n': 'B',
    '03n': 'C',
    '04n': 'D',
    '09n': 'E',
    '10n': 'F',
    '11n': 'G',
    '13n': 'H',
    '50n': 'I',
  };
  var LOCATION_OPTS = {
    'timeout': 5000,
    'maximumAge': 30 * 60 * 1000
  };

  var parseIcon = function(icon){
    return ICONS[icon].charCodeAt(0);
  }

  var fetchWeatherForLocation = function(location){
    var query = 'q=' + location;
    fetchWeather(query);
  }

  var fetchWeatherForCoordinates = function(latitude, longitude){
    var query = 'lat=' + latitude + '&lon=' + longitude;
    fetchWeather(query);
  }

  var fetchWeather = function(query) {
    var req = new XMLHttpRequest();
    query += '&cnt=1&appid=' + API_ID;
    req.open(METHOD, BASE_URL + '?' + query, true);
    req.onload = function () {
      if (req.readyState === 4) {
        if (req.status === 200) {
          var response = JSON.parse(req.responseText);
          var timestamp = response.dt;
          var temperature = Math.round(response.main.temp - 273.5);
          var icon = parseIcon(response.weather[0].icon);
          var city = response.name;
          var data = {
            'AppKeyWeatherIcon': icon,
            'AppKeyWeatherTemperature': temperature
          };
          Pebble.sendAppMessage(data);
        } else {
          Pebble.sendAppMessage({ 'AppKeyWeatherFailed': 1 });
        }
      }
    };
    req.send(null);
  }

  var locationSuccess = function(pos) {
    var coordinates = pos.coords;
    fetchWeatherForCoordinates(coordinates.latitude, coordinates.longitude);
  }

  var locationError = function(err) {
    pebble.sendAppMessage({
      'AppKeyWeatherFailed': 0
    });
  }

  pebble.addEventListener('appmessage', function (e) {
    var dict = e.payload;
    if(dict['AppKeyWeatherRequest']) {
      var config = Config('config');
      var location = config.load().location;
      if(location){
        fetchWeatherForLocation(location);
      }else{
        window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, LOCATION_OPTS);
      }
    }
  });
}(Pebble);


Pebble.addEventListener('ready', function (e) {
  var data = { 'AppKeyJsReady': 1 };
  Pebble.sendAppMessage(data);
});


Pebble.addEventListener('showConfiguration', function() {
  var URL = 'https://cdn.rawgit.com/groyoh/minimalin/ffd0da5fb45f0722dee6e59eb4b05fa63ca82136/config/index.html';
  var config = Config('config');
  var params = config.load();
  params.platform = Pebble.getActiveWatchInfo().platform;
  var query = '?config=' + encodeURIComponent(JSON.stringify(params));
  Pebble.openURL(URL + query);
});

Pebble.addEventListener('webviewclosed', function(e) {
  if(e.response){
    var config = Config('config');
    var configData = JSON.parse(decodeURIComponent(e.response));
    config.store(configData);
    var mapping = {
      minute_hand_color: 'AppKeyMinuteHandColor',
      hour_hand_color: 'AppKeyHourHandColor',
      background_color: 'AppKeyBackgroundColor',
      time_color: 'AppKeyTimeColor',
      info_color: 'AppKeyInfoColor',
      date_displayed: 'AppKeyDateDisplayed',
      health_enabled: 'AppKeyHealthEnabled',
      weather_enabled: 'AppKeyWeatherEnabled',
      bluetooth_icon: 'AppKeyBluetoothIcon',
      battery_displayed_at: 'AppKeyBatteryDisplayedAt',
      temperature_unit: 'AppKeyTemperatureUnit',
      refresh_rate: 'AppKeyRefreshRate',
      rainbow_mode: 'AppKeyRainbowMode',
      vibrate_on_the_hour: 'AppKeyVibrateOnTheHour',
      military_time: 'AppKeyMilitaryTime'
    };
    var dict = { AppKeyConfig: 1 };
    for(var key in mapping){
      if (mapping.hasOwnProperty(key)) {
        dict[mapping[key]] = configData[key];
      }
    }
    dict['AppKeyConfig'] = 1;
    Pebble.sendAppMessage(dict, function() {}, function() {});
  }
});

var API_ID = "b8c060512b6a117879f2636c41bbb67b";
var ICONS = {
  "01d": "a",
  "02d": "b",
  "03d": "c",
  "04d": "d",
  "09d": "e",
  "10d": "f",
  "11d": "g",
  "13d": "h",
  "50d": "i",
  "01n": "A",
  "02n": "B",
  "03n": "C",
  "04n": "D",
  "09n": "E",
  "10n": "F",
  "11n": "G",
  "13n": "H",
  "50n": "I",
};

function parseIcon(icon) {
  return ICONS[icon].charCodeAt(0);
}

function fetchWeatherForLocation(location){
  var query = "q=" + location;
  fetchWeather(query);
}

function fetchWeatherForCoordinates(latitude, longitude){
  var query = "lat=" + latitude + "&lon=" + longitude;
  fetchWeather(query);
}

function fetchWeather(query) {
  var req = new XMLHttpRequest();
  query += "&cnt=1&appid=" + API_ID;
  req.open("GET", "http://api.openweathermap.org/data/2.5/weather?" + query, true);
  req.onload = function () {
    if (req.readyState === 4) {
      if (req.status === 200) {
        var response = JSON.parse(req.responseText);
        var timestamp = response.dt;
        var temperature = Math.round(response.main.temp - 273.5);
        var icon = parseIcon(response.weather[0].icon);
        var city = response.name;
        var data = {
          "AppKeyWeatherIcon": icon,
          "AppKeyWeatherTemperature": temperature
        };
        console.log("weather sent: " + JSON.stringify(data));
        Pebble.sendAppMessage(data);
      } else {
        console.warn("weather error (" + req.code + ")");
        Pebble.sendAppMessage({
          "AppKeyWeatherFailed": 1
        });
      }
    }
  };
  req.send(null);
}

function locationSuccess(pos) {
  var coordinates = pos.coords;
  fetchWeatherFromCoordinates(coordinates.latitude, coordinates.longitude);
}

function locationError(err) {
  console.warn("location error (" + err.code + "): " + err.message);
  Pebble.sendAppMessage({
    "AppKeyWeatherFailed": 0
  });
}

var locationOptions = {
  "timeout": 5000,
  "maximumAge": 5 * 60000
};

Pebble.addEventListener("ready", function (e) {
  var data = {
    "AppKeyJsReady": 1
  };
  Pebble.sendAppMessage(data);
});

Pebble.addEventListener("appmessage", function (e) {
  var dict = e.payload;
  if(dict["AppKeyWeatherRequest"]) {
    var location = localStorage.getItem("Location");
    if(location){
      fetchWeatherForLocation(location);
    }else{
      window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
    }
  }
});

Pebble.addEventListener('showConfiguration', function() {
  var toQueryString = function(obj){
    var parts = [];
    for (var i in obj) {
      if (obj.hasOwnProperty(i) && obj[i] !== null) {
        parts.push(encodeURIComponent(i) + '=' + encodeURIComponent(obj[i]));
      }
    }
    return parts.join('&');
  };

  var getSavedColor = function(attr){
    var color = localStorage.getItem(attr + '_color');
    return color ? '#' + color.toString(16) : null;
  };
  var getSavedBool = function(attr){
    var savedValue = localStorage.getItem(attr + '_bool');
    if(savedValue){
      return savedValue.toLowerCase();
    }
    return null;
  };
  var getSavedInt = function(attr){
    return localStorage.getItem(attr + '_int');
  };
  var getSaved = function(attr){
    return localStorage.getItem(attr);
  }

  var url = 'https://cdn.rawgit.com/groyoh/minimalin/ab2ebe5e56e9568b055017ddf8672b7856e8d4e1/config/index.html?';
  var params = {
    minute_hand_color: getSavedColor('MinuteHand'),
    hour_hand_color: getSavedColor('HourHand'),
    date_displayed: getSavedBool('DateDisplayed'),
    health_enabled: getSavedBool('HealthEnabled'),
    battery_displayed_at: getSavedInt('BatteryDisplayedAt'),
    bluetooth_icon: getSavedInt('BluetoothIcon'),
    weather_enabled: getSavedBool('WeatherEnabled'),
    temperature_unit: getSavedInt('TemperatureUnit'),
    rainbow_mode: getSavedBool('RainbowMode'),
    dark_mode: getSavedBool('DarkMode'),
    background_color: getSavedColor('Background'),
    time_color: getSavedColor('Time'),
    info_color: getSavedColor('Info'),
    refresh_rate: getSavedInt('RefreshRate'),
    location: getSaved('Location'),
    platform: Pebble.getActiveWatchInfo().platform,
    vibrate_on_the_hour: getSavedBool('VibrateOnTheHour'),
    military_time: getSavedBool('MilitaryTime')
  };
  url += toQueryString(params);
  // console.log('Showing configuration page: ' + url);
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  if(e.response){
    var saveColor = function(dict, attr, color){
      color = color.replace('#', '').replace('0x','');
      localStorage.setItem(attr + '_color', color);
      dict['AppKey' + attr + 'Color'] = parseInt(color, 16);
    };
    var saveBool = function(dict, attr, bool){
      localStorage.setItem(attr + '_bool', bool);
      dict['AppKey' + attr] = bool;
    };
    var saveInt = function(dict, attr, int){
      localStorage.setItem(attr + '_int', int);
      dict['AppKey' + attr] = int;
    };
    var configData = JSON.parse(decodeURIComponent(e.response));
    // console.log('Configuration page returned: ' + JSON.stringify(configData));
    var dict = {};
    saveColor(dict, 'MinuteHand', configData.minute_hand_color);
    saveColor(dict, 'HourHand', configData.hour_hand_color);
    saveColor(dict, 'Background', configData.background_color);
    saveColor(dict, 'Time', configData.time_color);
    saveColor(dict, 'Info', configData.info_color);
    saveBool(dict, 'DateDisplayed', configData.date_displayed);
    saveBool(dict, 'HealthEnabled', configData.health_enabled);
    saveBool(dict, 'WeatherEnabled', configData.weather_enabled);
    saveInt(dict, 'BluetoothIcon', configData.bluetooth_icon);
    saveInt(dict, 'BatteryDisplayedAt', configData.battery_displayed_at);
    saveInt(dict, 'TemperatureUnit', configData.temperature_unit);
    saveInt(dict, 'RefreshRate', configData.refresh_rate);
    saveBool(dict, 'RainbowMode', configData.rainbow_mode);
    saveBool(dict, 'VibrateOnTheHour', configData.vibrate_on_the_hour);
    saveBool(dict, 'MilitaryTime', configData.military_time);
    localStorage.setItem('DarkMode_bool', configData.dark_mode);
    dict['AppKeyConfig'] = 1;
    localStorage.setItem("Location", configData.location);
    Pebble.sendAppMessage(dict, function() {
      // console.log('Send successful: ' + JSON.stringify(dict));
    }, function() {
      // console.log('Send failed!');
    });
  }
});

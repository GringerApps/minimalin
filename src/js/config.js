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

  var url = 'https://cdn.rawgit.com/groyoh/minimalin/9868954db64a617376d50c01ae636b6fcbc1f913/config/index.html?';
  var params = {
    minute_hand_color: getSavedColor('minute_hand'),
    hour_hand_color: getSavedColor('hour_hand'),
    date_displayed: getSavedBool('date_displayed'),
    bluetooth_icon: getSavedInt('bluetooth_icon'),
    rainbow_mode: getSavedBool('rainbow_mode'),
    background_color: getSavedColor('background'),
    date_color: getSavedColor('date'),
    time_color: getSavedColor('time'),
    platform: Pebble.getActiveWatchInfo().platform
  };
  url += toQueryString(params);
  console.log('Showing configuration page: ' + url);
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var saveColor = function(dict, attr, color){
    color = color.replace('#', '').replace('0x','');
    localStorage.setItem(attr + '_color', color);
    dict['KEY_' + attr.toUpperCase() + '_COLOR'] = parseInt(color, 16);
  };
  var saveBool = function(dict, attr, bool){
    localStorage.setItem(attr + '_bool', bool);
    dict['KEY_' + attr.toUpperCase()] = bool;
  };
  var saveInt = function(dict, attr, int){
    localStorage.setItem(attr + '_int', int);
    dict['KEY_' + attr.toUpperCase()] = int;
  };
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));
  var dict = {};
  saveColor(dict, 'minute_hand', configData.minute_hand_color);
  saveColor(dict, 'hour_hand', configData.hour_hand_color);
  saveColor(dict, 'background', configData.background_color);
  saveColor(dict, 'date', configData.date_color);
  saveColor(dict, 'time', configData.time_color);
  saveBool(dict, 'date_displayed', configData.date_displayed);
  saveInt(dict, 'bluetooth_icon', configData.bluetooth_icon);
  saveBool(dict, 'rainbow_mode', configData.rainbow_mode);
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
});

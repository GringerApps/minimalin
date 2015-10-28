Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function() {
  var toQueryString = function(obj){
    var parts = [];
    for (var i in obj) {
        if (obj.hasOwnProperty(i) && obj[i] !== null) {
            parts.push(encodeURIComponent(i) + "=" + encodeURIComponent(obj[i]));
        }
    }
    return parts.join("&");
  };
  
  var getSavedColor = function(attr){
    return localStorage.getItem(attr + '_color');
  };
  var getSavedBool = function(attr){
    return localStorage.getItem(attr + '_bool');
  };
	var url = 'https://cdn.rawgit.com/groyoh/minimalin/2898852f65e871eeed9fd77cb3ce879581f3172a/config/index.html?';
  var params = {
    minute_hand_color: getSavedColor('minute_hand'),
    hour_hand_color: getSavedColor('hour_hand'),
    display_date: getSavedBool('date_displayed')
  };
  url += toQueryString(params);
  console.log('Showing configuration page: ' + url);

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var colorKey = function(attr, color){
    return 'KEY_' + attr.toUpperCase() + '_COLOR_' + color.toUpperCase();
  };

  var saveColor = function(dict, attr, color){
  	localStorage.setItem(attr + '_color', color);
    attr = attr.toUpperCase();
    var red   = parseInt(color.substring(2, 4), 16);
    var green = parseInt(color.substring(4, 6), 16);
    var blue  = parseInt(color.substring(6), 16);
    var redKey   = colorKey(attr, 'red');
    var greenKey = colorKey(attr, 'green');
    var blueKey  = colorKey(attr, 'blue');
    dict[redKey]   = red;
    dict[greenKey] = green;
    dict[blueKey]  = blue;
  };
  var saveBool = function(dict, attr, bool){
    localStorage.setItem(attr + '_bool', bool);
    dict.KEY_DATE_DISPLAYED = bool;
  };
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));
  var dict = {};
  saveColor(dict, 'minute_hand', configData.minute_hand_color);
  saveColor(dict, 'hour_hand', configData.hour_hand_color);
  saveBool(dict, 'date_displayed', configData.date_displayed);
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
});

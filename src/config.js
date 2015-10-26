Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function() {
  var url = 'https://cdn.rawgit.com/groyoh/minimalin/12c25f3704144531f96c9116ab88d98418fa3f56/config/index.html';
  console.log('Showing configuration page: ' + url);

  Pebble.openURL(url);
});

var setColorToDict = function(dict, attr, color){
  dict['KEY_' + attr + '_COLOR_RED']   = parseInt(color.substring(2, 4), 16);
  dict['KEY_' + attr + '_COLOR_GREEN'] = parseInt(color.substring(4, 6), 16);
  dict['KEY_' + attr + '_COLOR_BLUE']  = parseInt(color.substring(6), 16);
};

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));

  var minuteHandColor = configData.minute_hand_color;
  var dict = {};
  setColorToDict(dict, 'MINUTE_HAND', minuteHandColor);
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
});

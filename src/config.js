Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function() {
  var url = 'https://cdn.rawgit.com/groyoh/minimalin/739d433b129f2ce5965460d161f8c08bcc16f077/config/index.html';
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
  var dict = {};
  setColorToDict(dict, 'MINUTE_HAND', configData.minute_hand_color);
  setColorToDict(dict, 'HOUR_HAND', configData.hour_hand_color);
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
});

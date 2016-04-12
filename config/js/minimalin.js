 Ractive.DEBUG = false;
     var query = function(variable, defaultVariable) {
       var query = location.search.substring(1);
       var vars = query.split('&');
       for (var i = 0; i < vars.length; i++) {
         var pair = vars[i].split('=');
         if (pair[0] === variable) {
           return decodeURIComponent(pair[1]);
         }
       }
       return defaultVariable || null;
     };

     var bool = function(name, defaultValue){
       var val = (query(name) || '').toLowerCase();
       switch(val){
         case 'true':
           return true;
         case 'false':
           return false;
         default:
           return defaultValue;
       }
     };

     var color = function(name, defaultValue){
       var val = query(name, defaultValue);
       if(val.match(/(?:#|0x)[0-9a-fA-F]{6}/)){
         return val;
       }
       return defaultValue;
     };

     var queryIn = function(name, authorizedValues, defaultValue){
       var val = query(name, defaultValue);
       for(var i in authorizedValues){
         if(authorizedValues[i] == val){
           return authorizedValues[i];
         }
       }
       return defaultValue;
     };

     var toRefreshRate = {
         0: 5,
         1: 20,
         2: 40
     };

     var fromRefreshRate = {
         5: 0,
         20: 1,
         40: 2
     };

     var defaults = {
       minute_hand_color: '#FFFFFF',
       hour_hand_color:  '#FF0000',
       date_displayed:  true,
       bluetooth_icon: 1,
       rainbow_mode: false,
       background_color: '#000000',
       date_color: '#555555',
       time_color: '#AAAAAA',
       info_color: '#555555',
       temperature_unit: 0,
       refresh_rate: fromRefreshRate[20],
       weather_enabled: true,
       location: ''
     };

     var data = {
       defaults: defaults,
       minute_hand_color: color('minute_hand_color', defaults.minute_hand_color),
       hour_hand_color:  color('hour_hand_color', defaults.hour_hand_color),
       date_displayed:  bool('date_displayed', defaults.date_displayed),
       bluetooth_icon: queryIn('bluetooth_icon', [0,1,2], defaults.bluetooth_icon),
       temperature_unit: queryIn('temperature_unit', [0,1], defaults.temperature_unit),
       refresh_rate: fromRefreshRate[queryIn('refresh_rate', [5,20,40], defaults.refresh_rate)],
       rainbow_mode: bool('rainbow_mode', defaults.rainbow_mode),
       background_color: color('background_color', defaults.background_color),
       date_color: color('date_color', defaults.date_color),
       time_color: color('time_color', defaults.time_color),
       info_color: color('info_color', defaults.info_color),
       weather_enabled: bool('weather_enabled', defaults.weather_enabled),
       location: query('location',defaults.location),
       platform: queryIn('platform', ['basalt', 'chalk'], 'basalt')
     };

     Ractive.load('template/ConfigView.html').then(function(ConfigView) {
       new ConfigView({
         el: 'body',
         data: data,
         oninit: function(){
           this.on('save', function() {
             var response_data = {
               minute_hand_color: this.get("minute_hand_color"),
               hour_hand_color: this.get("hour_hand_color"),
               date_displayed: this.get("date_displayed"),
               bluetooth_icon: this.get("bluetooth_icon"),
               rainbow_mode: this.get("rainbow_mode"),
               background_color: this.get("background_color"),
               date_color: this.get("date_color"),
               time_color: this.get("time_color"),
               info_color: this.get("info_color"),
               weather_enabled: this.get("weather_enabled"),
               refresh_rate: toRefreshRate[this.get("refresh_rate")],
               temperature_unit: this.get("temperature_unit"),
               location: this.get("location") || ""
             };
             var return_to = query('return_to', 'pebblejs://close#');
             document.location = return_to + encodeURIComponent(JSON.stringify(response_data));
           });
         }
       });
     });
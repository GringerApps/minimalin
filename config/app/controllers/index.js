import Ember from 'ember';
import _ from 'lodash/lodash';

export default Ember.Controller.extend({
  setupPreview: function() {
    const platform = this.get("platform");
    this.setProperties({
      dateDisplayed: true,
      rainbowMode: false,
      rect: true,
      round: false,
      circleColor: "#AA00FF",
      hourHandColor: "#AA00FF",
      minuteHandColor: "#AA00FF",
      timeColor: "#AA00FF",
      infoColor: "#AA00FF",
      dateColor: "#AA00FF",
      infoText: "z13°"
    });
  }.on("init")
});

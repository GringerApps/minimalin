/*jshint node:true*/
/* global require, module */
var EmberApp = require('ember-cli/lib/broccoli/ember-app');

module.exports = function(defaults) {
  var app = new EmberApp(defaults, {
    minifyJS: {
      enabled: true
    },
    minifyCSS: {
      enabled: true
    }
  });

  app.import('vendor/slate.min.css');

  return app.toTree();
};

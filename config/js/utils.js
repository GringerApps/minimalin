// Read the url
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

// ??
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

// Verify it's a color
var color = function(name, defaultValue){
 var val = query(name, defaultValue);
 if(val.match(/(?:#|0x)[0-9a-fA-F]{6}/)){
   return val;
 }
 return defaultValue;
};

//??
var queryIn = function(name, authorizedValues, defaultValue){
  var val = query(name, defaultValue);
  for(var i in authorizedValues){
   if(authorizedValues[i] == val){
     return authorizedValues[i];
   }
  }
  return defaultValue;
};



/*
//DOM manipulation
Object.prototype.hasClass = function(className) {
  return this.className.indexOf(className) !== -1;
}

Object.prototype.addClass = function(className) {
  if(this.hasClass(className)) {
    return false;
  }

  this.className = this.className += ' ' + className;
  return true;
}

Object.prototype.removeClass = function(className) {
  if(!this.hasClass(className)) {
    return false;
  }
  this.className = this.className.replace(className, '');
  return true;
}
*/
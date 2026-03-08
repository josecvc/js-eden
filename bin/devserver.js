/**
 * This script runs a simple webserver which is capable of passing through
 * requests to various ruby scripts we use. It's intended as an alternative to
 * running a vagrant box with apache.
 *
 * While running this a jseden instance should be visible at localhost:8000
 */
var connect = require('connect');
var serveStatic = require('serve-static');
var port = 8000;

var app = connect();

// I USE THIS FOR SHAREDARRAYBUFFER DON'T PANIC
app.use(function (req, res, next) {
  res.setHeader("Cross-Origin-Opener-Policy", "same-origin");
  res.setHeader("Cross-Origin-Embedder-Policy", "require-corp");
  next();
});


app.use(serveStatic('.')).listen(port, function () {
  console.log('JS-Eden instance running at http://localhost:'+port);
});



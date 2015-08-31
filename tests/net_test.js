
for (var count = 0; count < 1000000; count++) {
  var s0 = net.createServer({ a: 1 }, function(kde) { });
  s0 = null;
  Duktape.gc();
}

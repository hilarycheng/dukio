
for (var count = 0; count < 10; count++) {
  print("Count " + count);
  var s0 = net.createServer({ a: 1 }, function(kde) { });
//  s0.listen("0.0.0.0", 8000);
  s0 = null;
  Duktape.gc();
}

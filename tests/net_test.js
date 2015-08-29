print('Net Test ' + net.constructor);

var s0 = net.createServer({ a: 1 }, function(kde) { });
var s1 = net.createServer(function(abc) { });

print(s1.address());
s1.listen('0.0.0.0', 7000);
for (var s in s1) {
  print('-----' + s);
}

s0 = null;
s1 = null;

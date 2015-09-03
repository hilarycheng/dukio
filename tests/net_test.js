
var s0 = net.createServer(function(socket) {
  print('Create Server Callback at js');
  for (var s in socket) {
    print('Sock Name ' + s);
  }
  return 0;
});
s0.listen("0.0.0.0", 8000);

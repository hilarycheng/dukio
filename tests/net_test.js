
var s0 = net.createServer(function(socket) {
  print('Create Server Callback at js');
  for (var s in socket) {
    print('Sock Name ' + s);
  }
  print(socket.remoteAddress + ' ' + socket.remotePort + ' ' + socket.remoteFamily);
  print(socket.localAddress + ' ' + socket.localPort);

  socket.on('data', function(data) {
    print('Sockte Data ' + data);
  });

  socket.on('close', function(had_error) {
    print('Sockte Close ' + had_error);
  });

  return 0;
});
s0.listen("0.0.0.0", 8000);

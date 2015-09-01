
var s0 = net.createServer(function(res) {
  print('Create Server Callback at js');
  return 0;
});
s0.listen("0.0.0.0", 8000);

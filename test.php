<?php

/*
$t = new TCP();
$t->connect("127.0.0.1", 5432, function () use ($t) {
  echo "connected!\n";
  $t->write("gheh!", function() use ($t) {
    echo "written!\n";
  });
});
*/

$server = new TCP();
$server->listen(80, function ($client) {
  echo "connected!\n";
  var_dump($client);
  $client->write("HTTP 1.0 500 OK\r\n\r\nHello world!", function() use ($client) {
    echo "written!\n";
  });
});

uv_run();

?>
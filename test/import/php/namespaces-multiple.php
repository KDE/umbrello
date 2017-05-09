<?php
namespace MyProject {
const CONNECT_OK = 1;
class Connection1 { /* ... */ }
function connect() { /* ... */  }
}

namespace AnotherProject {
const CONNECT_OK = 1;
class Connection2 { /* ... */ }
function connect() { /* ... */  }
}

namespace { // global code
class Connection3 { /* ... */ }
session_start();
$a = MyProject\connect();
echo MyProject\Connection::start();
}
?>


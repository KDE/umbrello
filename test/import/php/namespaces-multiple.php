<?php
namespace MyProject {
const CONNECT_OK = 1;
class Connection1 { function test1() {} }
function connect() { /* ... */  }
}

namespace AnotherProject {
const CONNECT_OK = 1;
class Connection2 { function test2() {} }
function connect() { /* ... */  }
}

namespace { // global code
class Connection3 { function test3() {} }
session_start();
$a = MyProject\connect();
echo MyProject\Connection::start();
}

namespace Illuminate\Database\Eloquent\Model;

class TestModel {}

namespace Timetracker\Base\Models;

use Illuminate\Database\Eloquent\Model;
use ApiServer\Modules\Models\BaseModel;

class Hour {}

?>


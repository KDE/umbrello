<?php
//output tokens in a readable form
//uses the token_get_all php function (needs this extension installed)

if (!isset($_SERVER['argv'][1])) {
    die("Usage: {$_SERVER['argv'][0]} file.php\n");
}
$c = file_get_contents($_SERVER['argv'][1]);
$c = str_replace("\r", "", $c);
$tokens = token_get_all($c);

$maxLength = 0;
foreach ($tokens as $k=>$i) {
    $i[1] = $tokens[$k][1] = str_replace("\n", '\n', $i[1]);
    if (ord($i[1]) == 0) {
        $i[1] = $tokens[$k][1] = '';
    }
    if ($maxLength < strlen($i[1].$i[2])) $maxLength = strlen($i[1].$i[2]);
}
if ($maxLength > 50) $maxLength = 50;
foreach ($tokens as $i) {
    echo $i[2].'  ';
    if (!$i[2]) echo ' ';
    echo $i[1].str_repeat(' ', $maxLength-strlen($i[1].$i[2])).': ';
    if (is_int($i[0])) {
        echo token_name($i[0])."\n";
    } else {
        echo $i[0]."\n";
    }
}

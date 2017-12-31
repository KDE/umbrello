<?php
//compares the tokens returned by token_get_all with the lexer

if (!isset($_SERVER['argv'][1])) {
    die("Usage: {$_SERVER['argv'][0]} [FILE | --code CODE]\n");
}

if ( $_SERVER['argv'][1] == '--code' ) {
    echo "processing code:\n\n";
    processString($_SERVER['argv'][2]);
} else {
    unset($_SERVER['argv'][0]);
    foreach ($_SERVER['argv'] as $file) {
        echo "processing file $file:\n\n";
        processString(file_get_contents($file));
    }
}

function processString($c) {
    $tokens = array();
    $start = microtime(true);
    $c = str_replace("\r", "", $c);
    foreach (token_get_all($c) as $i) {
        if (is_string($i)) {
            $name = $i;
            $chars = $i;
        } else if (is_int($i[0])) {
            $name = token_name($i[0]);
            $chars = $i[1];
        } else {
            $name = $i[0];
            $chars = $i[1];
        }
        if (!$name) $name = $i[1];
        $tokens[] = str_replace(array("\r", "\n"), array('\r', '\n'), $chars) . ' ' . $name;
    }
    $phpTime = microtime(true) - $start;

    var_dump($tokens);

    $out = array();
    $start = microtime(true);
    exec("php-parser --print-tokens --code ".escapeshellarg($c), $out, $ret);
    $parserTime = microtime(true) - $start;
    if ($ret != 0) {
        echo "php-parser failed\n";
        exit(255);
    }

    unset($out[0]); //remove "Parsing file ..."
    array_pop($out); //remove "successfully parsed"
    array_pop($out); //remove "end of file"
    $diff = array_diff($tokens, $out);
    if (!$diff || (count($tokens) != count($out))) {
        echo "code correctly tokenized ($parserTime / $phpTime)...\n";
    } else {
        echo "******* parser output:\n";
        out($out);
        echo "******* expected:\n";
        out($tokens);
        echo "******* differences in code:\n";
        out($diff);
        exit(255);
    }
}

function out($a) {
    foreach ($a as $i) {
        echo "'$i'\n";
    }
}



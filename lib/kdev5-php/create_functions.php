<?php
/*
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/


if (!isset($_SERVER['argv'][1])) {
    $msg = "Usage:\n".$_SERVER['argv'][0]." [path to phpdoc]\n";
    $msg .= "you may checkout from the php svn server using this command:\n";
    $msg .= "svn checkout http://svn.php.net/repository/phpdoc/en/trunk ./phpdoc-en\n";
    $msg .= "\nTo debug files/directories use this: ".$_SERVER['argv'][0]." --debug PATH ...\n";
    file_put_contents('php://stderr', $msg);
    exit(-1);
}

$skipClasses = array();

$skipClasses[] = 'self';
$skipClasses[] = 'parent';
$skipClasses[] = '__php_incomplete_class';
$skipClasses[] = 'php_user_filter';
$skipClasses[] = 'static'; // O_o where does that come from?

$classes = array();
$constants = array();
$constants_comments = array();
$variables = array();
$existingFunctions = array();
$versions = array();

if ($_SERVER['argv'][1] == '--debug') {
    // only debug given file
    define('DEBUG', true);
    $dirs = array();
    foreach ( $_SERVER['argv'] as $i => $v ) {
        if ( $i <= 1 ) {
            continue;
        } else if ( is_dir($v) ) {
            $dirs[] = $v;
        } else if ( file_exists($v) ) {
            parseFile(new SplFileInfo($v));
        } else {
            trigger_error("bad argument: ".$v, E_USER_ERROR);
        }
    }
} else {
    define('DEBUG', false);

    if (!file_exists($_SERVER['argv'][1])) {
        file_put_contents('php://stderr', "phpdoc path not found");
        exit(-1);
    }

    $dirs = array(
        $_SERVER['argv'][1]."/reference",
        $_SERVER['argv'][1]."/features",
        $_SERVER['argv'][1]."/appendices",
        $_SERVER['argv'][1]."/language/predefined/"
    );
}

foreach ($dirs as $dir) {
    $dirIt = new RecursiveIteratorIterator( new RecursiveDirectoryIterator($dir));
    foreach ($dirIt as $file) {
        parseFile($file);
    }
}

unset($existingFunctions);

/*
 Here be dirty hacks! PHP's documentation isn't as good as could be wished for...
 */

// Clean the functions from imagick
foreach (array_keys($constants) as $c) {
    if ($pos = strpos($c, '::')) {
        $class = substr($c, 0, $pos);
        if ($class == 'imagick') $class = 'Imagick';
        newClassEntry($class);
    }
}

if ( !DEBUG ) {
    // The dir function, which lacks parseable documentation...
    $classes['global']['functions'][] = array(
        'name' => "dir",
        'params' => array(array('name' => "path", 'type' => "string", 'isRef' => false)),
        'type' => "Directory",
        'desc' => "Return an instance of the Directory class"
    );
}

$skipFunctions = array();
// remove delete() function which only acts as a pointer to unlink
// in the documentation but does not actually exist as a alias in PHP
$skipFunctions[] = 'delete';

// awesome uncallable functions - noone knows wth that should be...
$skipMethods = array();
$skipMethods[] = 'list';
$skipMethods[] = 'declare';
$skipMethods[] = 'do';
$skipMethods[] = 'echo';
$skipMethods[] = 'function';

/*
 Here ends the hackings...
 */

function constTypeValue($ctype) {
    if ($ctype == 'integer' || $ctype == 'int') {
        return "0";
    } else if ($ctype == 'string') {
        return "''";
    } else if ($ctype == 'bool') {
        return "false";
    } else if ($ctype == 'float') {
        return "0.0";
    } else {
        // default to integer const type
        return "0";
    }
}

function removeTag($xml, $tag) {
    $tag = preg_quote($tag, '#');
    return trim(preg_replace('#(^<'.$tag.'[^>]*>|</'.$tag.'>$)#s', '', trim($xml)));
}

function cleanupComment($comment) {
    // <function|parameter>...</> to {@link ...}
    $comment = preg_replace('#<(function|parameter)>(.+)</\1>#U', '{@link $2}', $comment);
    // remove <para> and other stuff
    ///TODO: support web-links, lists and tables
    $comment = strip_tags($comment);
    $comment = html_entity_decode($comment);

    // make sure no */ occurs in a comment...
    $comment = preg_replace('#\*/#', '* /', $comment);

    $comment = preg_replace('#(?<=[^\n])\n(?=[^\n])#s', ' ', $comment);

    $comment = preg_replace('#  +#', ' ', $comment);
    $comment = preg_replace('#^ | $#m', '', $comment);
    $comment = preg_replace("#\n{3,}#", "\n\n", $comment);

    $comment = trim($comment);
    return $comment;
}

function prepareComment($comment, array $more, $indent = '') {
    $comment = cleanupComment($comment);
    if (empty($comment) && empty($more)) {
        return '';
    }
    $comment = wordwrap($comment, 70, "\n", false);
    if ( !empty($more) ) {
        if ( !empty($comment) ) {
            $comment .= "\n\n";
        }
        foreach($more as $s) {
            $comment .= str_replace("\n", "\n  ", // indent
                            wordwrap(cleanupComment($s), 68, "\n", false)
                        )."\n";
        }
    }
    $comment = rtrim($comment);
    // add indentation and asterisk
    $comment = preg_replace("#^#m", $indent." * ", $comment);
    return $indent."/**\n".
                   $comment."\n".
           $indent." **/\n";
}

function sortByName($a, $b) {
    return strnatcasecmp($a['name'], $b['name']);
}

$fileHeader  = "<?php\n";
$fileHeader .= "// THIS FILE IS GENERATED\n";
$fileHeader .= "// WARNING! All changes made in this file will be lost!\n\n";

$declarationCount = 0;
$out = $fileHeader;

// make sure the output it somehow ordered to prevent huge svn diffs
uksort($variables, 'strnatcasecmp');
uksort($classes, 'strnatcasecmp');
uksort($constants, 'strnatcasecmp');

// put exception up front
$exception = $classes['exception'];
unset($classes['exception']);
$classes = array_merge(array('exception' => $exception), $classes);
reset($classes);

foreach ($variables as $name=>$var) {
    $declarationCount++;
    $moreDesc = array();
    if ($var['deprecated']) {
        $moreDesc[] = "@deprecated";
    }
    if (isset($var['superglobal']) && $var['superglobal']) {
        $moreDesc[] = "@superglobal";
    }
    $out .= prepareComment($var['desc'], $moreDesc);
    $out .= "$name = array();\n\n";
}

// make skipclasses lowercase
foreach ($skipClasses as &$name) {
    $name = strtolower($name);
}

foreach ($classes as $class => $i) {
    ///TODO: find proper fix for that
    $i['isInterface'] = $i['isInterface'] && empty($i['implements']);
    if (in_array($class, $skipClasses)) continue; //skip those as they are documented in spl.php
    if ($class != 'global') {
        if (isset($i['desc'])) {
            $out .= prepareComment($i['desc'], array());
        }
        $class = $i['prettyName'];
        $out .= ($i['isInterface'] ? 'interface' : 'class') . " " . $class;
        if (isset($i['extends'])) {
            $out .= " extends {$i['extends']}";
        }
        if (isset($i['implements'])) {
            $out .= " implements ".implode(", ", $i['implements']);
        }
        $out .= " {\n";
        $declarationCount++;
        foreach ($constants as $c=>$ctype) {
            if ($pos = strpos($c, '::')) {
                if (substr($c, 0, $pos) == $class) {
                    unset($constants[$c]);
                    $c = substr($c, $pos+2);
                    $out .= "    const $c = ".constTypeValue($ctype).";\n";
                    $declarationCount++;
                }
            }
        }
    }

    $indent = '';
    if ($class != 'global') $indent = '    ';

    usort($i['properties'], 'sortByName');
    foreach ($i['properties'] as $f) {
        $moreDesc = array();
        if ($f['type']) {
            $moreDesc[] = "@var {$f['type']}";
        }
        ///HACK the directory stuff has really bad documentation
        if ($class != 'directory') {
            $out .= prepareComment($f['desc'], $moreDesc, $indent);
        }
        $out .= "{$indent}var $".$f['name'].";\n";
        $declarationCount++;
    }

    usort($i['functions'], 'sortByName');
    foreach ($i['functions'] as $f) {
        if ( $class == 'global' && in_array($f['name'], $skipFunctions) ) {
            continue;
        } else if ( $class != 'global' && in_array($f['name'], $skipMethods) ) {
            continue;
        }
        $moreDesc = array();
        foreach ($f['params'] as $pi=>$param) {
            $desc = '';
            if ( isset($param['desc']) ) {
                $desc = trim($param['desc']);
            }
            $moreDesc[] = "@param {$param['type']} \${$param['name']} $desc";
        }
        if ($f['type']) {
            $moreDesc[] = "@return {$f['type']}";
        }
        $version_key = strtolower(($class == 'global' ? '' : $class.'::') . $f['name']);
        if (isset($versions[$version_key])) {
            $moreDesc[] = "@since {$versions[$version_key]}";
        }
        ///HACK the directory stuff has really bad documentation
        if ($class != 'directory') {
            $out .= prepareComment($f['desc'], $moreDesc, $indent);
        }
        $out .= "{$indent}function ".$f['name'];
        $out .= "(";
        $first = true;
        foreach ($f['params'] as $pi=>$param) {
            if (!$first) $out .= ", ";
            $first = false;
            if ($param['isRef']) $out .= "&";
            $out .= '$'.$param['name'];
        }
        $out .= ")";
        if ( !$i['isInterface'] ) {
            $out .= "{}";
        } else {
            $out .= ";";
        }
        $out .= "\n\n";
        $declarationCount++;
    }

    if ($class != 'global') $out .= "}\n";
}
foreach ($constants as $c=>$ctype) {
    if (strpos($c, '::')===false) {
        if ( isset($constants_comments[$c]) ) {
          $out .= prepareComment($constants_comments[$c], array());
        }
        $out .= "define('$c', ".constTypeValue($ctype).");\n";
        $declarationCount++;
    }
}
chdir(dirname(__FILE__));
if ( !DEBUG ) {
    echo "saving phpfunctions.php file\n";
    file_put_contents("phpfunctions.php", $out);

    if ( shell_exec("which php-parser") ) {
        echo "making sure phpfunctions file is valid...\n";
        system("php-parser phpfunctions.php", $ret);
        if ( $ret != 0 ) {
            die("could not parse file, aborting\n");
        }
    } else {
        echo "note: put php-parser in your path and I can check the generated file directly...\n";
    }

    echo "done\n";
} else {
    echo "phpfunctions.php\n~~~~\n$out\n~~~~\n";
}
echo "wrote ".$declarationCount." declarations\n";

/**
 * Parse file
 *
 * @param SplFileInfo  $file  File handler
 * @return  bool
 */
function parseFile($file, $funcOverload="") {
global $existingFunctions, $constants, $constants_comments, $variables, $classes, $isInterface, $versions;

    if (substr($file->getFilename(), -4) != '.xml') return false;
    if (substr($file->getFilename(), 0, 9) == 'entities.') return false;
    $string = file_get_contents($file->getPathname());
    $isInterface = strpos($string, '<phpdoc:classref') !== false &&
                   strpos($string, '&reftitle.interfacesynopsis;') !== false;

    $string = preg_replace('#(?:(&amp;|&gt;|&lt;)|&[A-Za-z\\.0-9-_]+;)#', '$1', $string);
    $removeSections = array();
    $removeSections[] = 'apd.installwin32';
    $removeSections[] = 'intl.intldateformatter-constants.calendartypes';
    foreach ($removeSections as $i) {
        $string = preg_replace('#'.preg_quote('<section xml:id="'.$i.'">').'.*?</section>#s', '', $string);
    }
    echo "reading documentation from {$file->getPathname()}\n";
    $xml = simplexml_load_string($string,  "SimpleXMLElement",  LIBXML_NOCDATA);

    if ( $file->getFilename() == 'versions.xml' ) {
        foreach ( $xml->xpath('/versions/function') as $f ) {
            $attrs = $f->attributes();
            $versions[strtolower($attrs['name'])] = (string) $attrs['from'];
        }
        return;
    }

    $xml->registerXPathNamespace('db', 'http://docbook.org/ns/docbook');
    $xml->registerXPathNamespace('phpdoc', 'http://php.net/ns/phpdoc');
    if ($vars = $xml->xpath('//phpdoc:varentry//db:refnamediv')) {
        foreach ($vars as $var) {
            foreach ($var->refname as $i) {
                $i = (string)$i;
                if ( isset($variables[$i]) ) {
                    $v = $variables[$i];
                } else {
                    $v = array();
                }
                if (substr($i, 0, 1) != '$') continue;

                if (substr($i, -13) == ' [deprecated]') {
                    $i = substr($i, 0, -13);
                    $v['deprecated'] = true;
                } else {
                    $v['deprecated'] = false;
                }
                $v['desc'] = (string)$var->refpurpose;
                $variables[$i] = $v;
            }
        }
    }
    if ($vars = $xml->xpath("//phpdoc:varentry[@xml:id='language.variables.superglobals']//db:member/db:varname")) {
        foreach ($vars as $var) {
            $variables[(string)$var]['superglobal'] = true;
        }
    }
    if (isset($xml->variablelist)) {
        foreach ($xml->variablelist->varlistentry as $i=>$varlistentry) {
            if ($c = (string)$varlistentry->term->constant) {
                if (!isset($constants[$c])) {
                    if (strpos($c, '=')) {
                        $c = substr($c, 0, strpos($c, '='));
                    }
                    $ctype = $varlistentry->term->type;
                    if (!$ctype) {
                        $ctype = $varlistentry->term->link;
                    }
                    $constants[$c] = (string)$ctype;
                }
            }
        }
    }
    // handle constants.xml with different layout as those above
    if ( !isset($xml->variablelist) && $file->getFilename() == 'constants.xml' && $xml->xpath("//db:constant") ) {
        $consts = $xml->xpath("//db:entry");
        foreach ( $consts as $i=>$p ) {
            if ( isset($p->constant) ) {
                if ( !isset($p->type) ) {
                    // default to integer constants
                    $p->type = 'integer';
                } else {
                    // check for comment
                    // next entry is the value of the constant which is followed by the comment
                    if ( isset($consts[$i+2]) && !$consts[$i+2]->children() ) {
                        $comment = $consts[$i+2]->asXml();
                        if ( !empty($comment) ) {
                            $constants_comments[(string)$p->constant] = $comment;
                        }
                    }
                }
                $constants[(string)$p->constant] = (string)$p->type;
            }
        }
    } else if (!isset($xml->variablelist) && $file->getFilename() == 'commandline.xml') {
        // yay for non-unified xml structures :-X
        $consts = $xml->xpath("//db:row");
        foreach ( $consts as $i=>$p ) {
            $constant = "";
                    // default to integer constants
            $type = "integer";
            if ( isset($p->entry[0]) && isset($p->entry[0]->constant) ) {
                $constant = trim((string) $p->entry[0]->constant);
                if ( isset($p->entry[0]->constant->type) ) {
                    $type = (string)$p->entry[0]->constant->type;
                }
            }
            if (empty($constant)) {
                continue;
            }
            // check for comment
            // next entry is the comment
            if ( isset($p->entry[1]) ) {
                $comment = $p->entry[1]->para->asXml();
                if ( !empty($comment) ) {
                    $constants_comments[$constant] = $comment;
                }
            }
            $constants[$constant] = $type;
        }
    }
    if ($list = $xml->xpath('//db:sect2[starts-with(@xml:id, "reserved.classes")]/db:variablelist/db:varlistentry')) {
        foreach ($list as $l) {
            $classname = newClassEntry((string)$l->term->classname);

            $classes[$classname]['desc'] = removeTag($l->listitem->asXML(), 'listitem');
        }
    }

    $cEls = $xml->xpath('//db:classsynopsis/db:classsynopsisinfo');
    if ($cEls) {
        foreach ($cEls as $class) {
            $class->registerXPathNamespace('db', 'http://docbook.org/ns/docbook');
            $className = (string)$class->ooclass->classname;
            if (!$className) continue;
            $className = newClassEntry($className);
            if ($extends = $class->xpath('//db:ooclass')) {
                foreach ($extends as $c) {
                    if ($c->modifier == 'extends') {
                        $classes[$className]['extends'] = (string)$c->classname;
                    }
                }
            }
            if ($interfaces = $class->xpath('//db:oointerface/db:interfacename')) {
                foreach ($interfaces as $if) {
                    $classes[$className]['implements'][] = (string)$if;
                }
            }
            if ($paras = $xml->xpath('//db:section[starts-with(@xml:id, "'.$className.'")]/db:para')) {
                foreach ($paras as $p) {
                    $classes[$className]['desc'] .= "\n".((string)$p);
                }
            }
        }
    }

    if (!isset($xml->refsect1)) return false;

    $desc = getDocumentation($xml);

    $addedSomething = false;
    // file could contain function + property
    if (isset($xml->refsect1->classsynopsis) && isset($xml->refsect1->classsynopsis->fieldsynopsis)) {
        $class = (string)$xml->refsect1->classsynopsis->ooclass->classname;

        foreach ( $xml->refsect1->classsynopsis->fieldsynopsis as $synopsis ) {
            newPropertyEntry($class, $synopsis->varname, $desc, $synopsis->type );
            $addedSomething = true;
        }
    }

    if (isset($xml->refsect1->methodsynopsis)) {
        foreach( $xml->refsect1->methodsynopsis as $synopsis ) {
            newMethodEntry('global', $synopsis->methodname, $funcOverload, $synopsis, $desc, $xml);
            $addedSomething = true;
        }
    }
    if (isset($xml->refsect1->classsynopsis) && isset($xml->refsect1->classsynopsis->methodsynopsis)) {
        $methodsynopsis = $xml->refsect1->classsynopsis->methodsynopsis;
        newMethodEntry($xml->refsect1->classsynopsis->ooclass->classname, $methodsynopsis->methodname, $funcOverload, $methodsynopsis, $desc, $xml);
        $addedSomething = true;
    }
    if ( !$addedSomething && isset($xml->refnamediv->refpurpose->function) ) {
        // This is function alias
        $functionName = (string)$xml->refnamediv->refname;
        $aliasName    = (string)$xml->refnamediv->refpurpose->function;
        $baseFileName = dirname($file->getPathname()).'/'.str_replace('_', '-', $aliasName).'.xml';
        if ( $baseFileName == $file->getPathname() || !file_exists($baseFileName) ) {
            return false;
        }
        parseFile(new SplFileInfo($baseFileName), $functionName);
        $addedSomething = true;
    }

    return $addedSomething;
} // end of function parseFile()

/**
 * Create a new class entry if it not exists.
 *
 * Key in $classes will be the lower-case @p $name.
 * The prettyName member will be @p $name, if it contains non-lowercase chars.
 *
 * Returns the lower-cased @p $name
 */
function newClassEntry($name) {
    global $classes, $isInterface;
    $lower = strtolower($name);
    if (!isset($classes[$lower])) {
        $classes[$lower] = array(
            'functions' => array(),
            'properties' => array(),
            'prettyName' => $name,
            'desc' => '',
            'isInterface' => $isInterface,
        );
    } else {
        if ( $lower != $name ) {
            $classes[$lower]['prettyName'] = $name;
        }
        if ( $isInterface ) {
            $classes[$lower]['isInterface'] = true;
        }
    }
    return $lower;
}

/**
 * get the documentation for an entry
 * @return string
 */
function getDocumentation(SimpleXMLElement $xml) {
    $descs = array();
    foreach ($xml->refsect1->para as $p ) {
        $p = removeTag($p->asXML(), 'para');
        if ( stripos($p, 'procedural style') !== false || stripos($p, 'procedure style') !== false
            || stripos($p, 'object oriented style') !== false ) {
            // uninteresting
            continue;
        }
        $descs[] = $p;
    }
    return implode("\n\n", $descs);
}

/**
 * create a new property entry for @p $class
 */
function newPropertyEntry($class, $name, $desc, $type) {
    global $classes;
    $class = newClassEntry($class);
    $classes[$class]['properties'][] = array(
        'name' => (string) $name,
        'desc' => (string) $desc,
        'type' => (string) $type
    );
}

/**
 * create a new method entry for @p $class
 */
function newMethodEntry($class, $function, $funcOverload, $methodsynopsis, $desc, SimpleXMLElement $xml) {
    global $existingFunctions, $classes;
    $class = (string) $class;
    $function = (string) $function;
    $funcOverload = (string) $funcOverload;

    if (strpos($function, '::')) {
        $class = substr($function, 0, strpos($function, '::'));
        $function = substr($function, strpos($function, '::')+2);
    } else if (strpos($function, '->')) {
        $class = substr($function, 0, strpos($function, '->'));
        $function = substr($function, strpos($function, '->')+2);
    } else {
        if ($function == '__halt_compiler') return false;
        if ($function == 'exit') return false;
        if ($function == 'die') return false;
        if ($function == 'eval') return false;
        if ($function == 'echo') return false;
        if ($function == 'print') return false;
        if ($function == 'array') return false;
        if ($function == 'list') return false;
        if ($function == 'isset') return false;
        if ($function == 'unset') return false;
        if ($function == 'empty') return false;
    }

    if (strpos($function, '-')) return false;
    if (strpos($class, '-')) return false;
    if ($function == 'isSet') return false; //todo: bug in lexer
    if ($function == 'clone') return false; //todo: bug in lexer
    if (substr($class, 0, 3) == 'DOM') $class = 'Dom'.substr($class, 3);
    $class = trim($class);
    if ($class == 'imagick') $class = 'Imagick';
    if (in_array($class.'::'.($funcOverload ? $funcOverload : $function), $existingFunctions)) return false;
    $existingFunctions[] = $class.'::'.($funcOverload ? $funcOverload : $function);

    $params = array();
    foreach ($methodsynopsis->methodparam as $param) {
        $paramName = $param->parameter;
        if (trim($paramName) == '...') continue;
        if (!trim($paramName)) continue;
        $paramName = str_replace('/', '', $paramName);
        $paramName = str_replace('-', '', $paramName);
        $paramName = trim(trim(trim($paramName), '*'), '&');
        if (is_numeric(substr($paramName, 0, 1))) $paramName = '_'.$paramName;
        $params[] = array(
            'name' => $paramName,
            'type' => (string)$param->type,
            'isRef' => isset($param->parameter->attributes()->role) ? ($param->parameter->attributes()->role == "reference") : false
        );
    }
    // get description of params
    if ( $param_descs = $xml->xpath('db:refsect1[@role="parameters"]//db:varlistentry') ) {
        $i = 0;
        foreach ( $param_descs as $d ) {
            if ( !isset($params[$i]) ) {
                ///TODO: support optional params (i.e. ... token)
                continue;
            }
            $paramName = (string) $d->term->parameter;
            $params[$i]['desc'] = '';
            foreach ( $d->listitem->para as $p ) {
                $p = removeTag($p->asXML(), 'para');
                $params[$i]['desc'] .= $p . "\n";
            }
            ++$i;
        }
    }

    $class = newClassEntry($class);
    $classes[$class]['functions'][] = array(
        'name'   => $funcOverload ? $funcOverload : $function,
        'params' => $params,
        'type'   => (string)$methodsynopsis->type,
        'desc'   => $funcOverload ? str_replace($function, $funcOverload, $desc) : $desc
    );
}

/* don't add a closing ?> here, we use this file in a benchmark as well */

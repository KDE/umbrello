<?
//various tests for lexer and parser
class foo {
    const bar = 'bar';
}
$asdf = new foo;
$asdf->somethingNew = true;

function test($a=foo::bar) { echo $bar; }
function test(array &$a=array()) {}
function test2(foo $x, $y='blub') {}
function test2($a=array("foo",1=>array(1,array()))) {}
final class foo extends bar implements asdf, xxx {
    const asdf = 'asdf', xxx='foo';
    const asde = foo::blah;
    public static $x = array('asdf',);
    public abstract function asdf();
    public static function asdf($a, $foo=array(1, 2)) {}
}
interface asdf extends asf, xxx {
}
echo array();
echo "$a\"\n";
"$f\"bar";
echo $i;
echo $i[1];
echo $i{1};
echo $i['asdf'];
echo $$i;
echo $$i[1];
echo $i[$j];
print "foo";
echo array(1,1,$i,'foo'=>$bar, 3=>&$x,);
exit;
exit();
exit(1);
die;
die();
echo (int)$i;
echo (string)$i;
echo (array)$i;
echo (object)$i;
echo (bool)$i;
echo (unset)$i;
echo isset($i);
echo isset($i, $j);
empty($i);
include 'asdf';
include_once 'asdf';
require 'asdf';
require_once 'asdf';
eval('foo');
echo @$j && @@$i;
echo print print @@$foo;
new a(1, 2==1, 1&&2);
$i++;
++$i;
$i+1;
$i-1;
2+3;
$i++;
1+1;
++$i;
$i << 2;
$i >> 3;
(int)$i;
(bool)$i + 1 / 3;
new a(&$a);
if ($i): echo $j; else: echo $i; endif;
if (1) {} elseif (2) {} else if (2) {} else {}
foo::bar;
foo::$bar[1];
bar(1);
$$bar(1);
$foo();
$foo[1]();
$foo[1]{1}();
switch(1) {
    case 1:
        break;
}
switch(1) {
}
foo() or bar();
($a - 1);
$foo->bar();
$foo->bar->bar()->foo()->asdf[1];
$i = !$i;
$i == $i;
$i != $i;
$i === $i;
$i !== $i;
$i < $i;
$i > $i;
$i <= $i;
$i >= $i;
$i || $i;
$i && $i;
$i += $i;
$i -= $i;
$i *= $i;
$i /= $i;
$i .= $i;
$i %= $i;
$i &= $i;
$i |= $i;
$i ^= $i;
$i <<= $i;
$i >>= $i;
$i->$i;
for (;;) { }
for (1;1;1,1) { }
"\\";
"\\\\";
"\\\"";
115.3e+1;
115.3e-1;
115.3e1;
115.3e1165165;
#comment
//foo
/** bar **/
/*** bar **/
@!$foo;
@list($a) = @foo($b);
self::${$bar};
${$bar};
$$bar;
$bar;
$${$bar};
$foo[1];
?>
some html
<?=$foo?>
<?=$i; echo $b; ?>
<?php if ( true ) : ?>
foo
<?php endif ?>
<?php
echo <<<EOD1
barx$i
asdf
EOD1 xx
EOD1;
?>
<?php echo "foo"; //bar ?>
<?php
$foo = ($bar);
$foo = (!$bar);
?>
<?Php echo "a"; ?>
<?php
$i = '-V';
echo `uptime $i`;
"{$foo[bar]}";
"\${$foo}";

//from php manual
echo "$beer's taste is great";
echo "He drank some $beers";
echo "He drank some ${beer}s";
echo "He drank some {$beer}s";
echo "A banana is $fruits[banana].";
echo "A banana is {$fruits['banana']}.";
echo "A banana is {$fruits[banana]}.";
echo "A banana is " . $fruits['banana'] . ".";
echo "This square is $square->width meters broad.";
echo "This is { $great}";
echo "This is {$great}";
echo "This is ${great}";
echo "This square is {$square->width}00 centimeters broad.";
echo "This works: {$arr[4][3]}";
echo "This is wrong: {$arr[foo][3]}";
echo "This works: {$arr['foo'][3]}";
echo "This works: " . $arr['foo'][3];
echo "This works too: {$obj->values[3]->name}";
echo "This is the value of the var named $name: {${$name}}";
echo "This is the value of the var named by the return value of getName(): {${getName()}}";
echo "This is the value of the var named by the return value of \$object->getName(): {${$object->getName()}}";

$foo =& new bar() || exit;
$foo =& bar() || bar();
"{$foo["bar"]}";
/*/*/

while (1) {
echo 'a' ?>jghjhfg
<?php }
class foo {
    var $settings = "foo \$bar";
}
?>
<? if (1) : ?><?=$foo?><? endif; ?>
<? if (1) { ?><?=$foo?><? } ?>
<?
"{$foo[$i.'bar']}";
"{$foo['bar'.$i]}";
$i =& $a();
$i =& new $a;
$i =& $a;
$i =& new $a(1,2,3);
"${foo[a]}";
"${foo}";
$blah = 'blah';
$foo =& $blah || die($i);
$foo =& $blah() || die($i);
if (true) { ?>
<? echo "foo"; ?>
<?="bar";?>
<? }


<?php
// Fibonacci sequence calculation
function fibonacci($n) {
    if ($n <= 1) {
        return $n;
    }
    return fibonacci($n - 1) + fibonacci($n - 2);
}

$target = 10;
$fib = fibonacci($target);
echo "Fibonacci result: ", $fib;
?>

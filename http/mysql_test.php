<?php
    //ini_set('error_reporting', E_ALL);
    //ini_set('display_errors', 1);
    //ini_set('display_startup_errors', 1);
    header('Content-Type: text/plain');
    $mysqli = mysqli_connect("localhost", "tt", "cern99", "tt");
    if(!$mysqli) echo "err mysqli";
    $res = mysqli_query($mysqli, "SELECT `id` FROM `mode_1_page`");
    if(!$res) echo "err res";
    $str = "";
    while ($row = $res->fetch_assoc()) 
        $str = $str . $row['id'] . "\n";
    echo $str;
?>

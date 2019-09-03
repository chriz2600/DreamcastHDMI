<?php
    if (session_status() == PHP_SESSION_NONE) {
        session_start();
    }

    if ($_SESSION["progress"] >= 100) {
        $_SESSION["progress"] = 0;
    }

    $tmp = $_SESSION["progress"] + 5 + rand(0, 17);
    $_SESSION["progress"] = $tmp > 100 ? 100 : $tmp;
    echo $_SESSION["progress"];
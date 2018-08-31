<?php
    if (session_status() == PHP_SESSION_NONE) {
        session_start();
    }

    if ($_SESSION["progress"] >= 100) {
        $_SESSION["progress"] = 0;
    }

    $_SESSION["progress"] = $_SESSION["progress"] + 10;
    echo $_SESSION["progress"];
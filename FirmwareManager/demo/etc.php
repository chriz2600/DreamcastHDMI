<?php
    if (session_status() == PHP_SESSION_NONE) {
        session_start();
    }

    if ($_SESSION["last_flash_md5"]) {
        echo $_SESSION["last_flash_md5"];
    } else {
        echo "5a203ca2507ead6f6488138a2e6618b7";
    }

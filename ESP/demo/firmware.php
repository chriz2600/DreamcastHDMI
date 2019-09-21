<?php
    if (session_status() == PHP_SESSION_NONE) {
        session_start();
    }

    if ($_SERVER['REQUEST_URI'] == "/firmware.bin.md5") {
        if ($_SESSION["firmware.bin.md5"]) {
            echo $_SESSION["firmware.bin.md5"];
        } else {
            echo "0f7c2058962c2f2d05b0d3c704852710";
        }
    } else if ($_SERVER['REQUEST_URI'] == "/firmware.dc.md5") {
        if ($_SESSION["firmware.dc.md5"]) {
            echo $_SESSION["firmware.dc.md5"];
        } else {
            echo "751bfdeb1793261853887dc11c876d40";
        }
    }

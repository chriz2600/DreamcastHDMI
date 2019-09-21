<?php
    if (session_status() == PHP_SESSION_NONE) {
        session_start();
    }

    if ($_SERVER['REQUEST_URI'] == "/flash/fpga") {
        $_SESSION["last_flash_md5"] = $_SESSION["firmware.dc.md5"];
    } else if ($_SERVER['REQUEST_URI'] == "/flash/esp") {
        $_SESSION["last_esp_flash_md5"] = $_SESSION["firmware.bin.md5"];
    } else if ($_SERVER['REQUEST_URI'] == "/flash/index") {
        $_SESSION["index_html_gz_md5"] = $_SESSION["esp_index_html_gz_md5"];
    }

    $_SESSION["progress"] = 0;

<?php
    if (session_status() == PHP_SESSION_NONE) {
        session_start();
    }

    if ($_SERVER['REQUEST_URI'] == "/etc/last_flash_md5") {
        if ($_SESSION["last_flash_md5"]) {
            echo $_SESSION["last_flash_md5"];
        } else {
            echo "5a203ca2507ead6f6488138a2e6618b7";
        }
    } else if ($_SERVER['REQUEST_URI'] == "/etc/last_esp_flash_md5") {
        if ($_SESSION["last_esp_flash_md5"]) {
            echo $_SESSION["last_esp_flash_md5"];
        } else {
            echo "635f6fe2eafcb7418bcbbada4060f82d";
        }
    }

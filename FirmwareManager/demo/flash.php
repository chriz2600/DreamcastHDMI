<?php
    if (session_status() == PHP_SESSION_NONE) {
        session_start();
    }

    $_SESSION["last_flash_md5"] = $_SESSION["firmware.dc.md5"] ? $_SESSION["firmware.dc.md5"] : "751bfdeb1793261853887dc11c876d40";
    $_SESSION["progress"] = 0;

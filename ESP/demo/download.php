<?php
    if (session_status() == PHP_SESSION_NONE) {
        session_start();
    }

    if ($_SESSION["config"]) {
        $j = json_decode($_SESSION["config"], true);
    }

    function _getMD5FileFPGA() {
        global $j;
        return (
              "https://" . $j["firmware_server"]
            + $j["firmware_server_path"]
            . "/fw/" . $j["firmware_version"]
            . "/DCxPlus-v2.dc.md5?cc=" . rand()
        );
    }
    function _getMD5FileESP() {
        global $j;
        return (
              "https://" . $j["firmware_server"]
            + $j["firmware_server_path"]
            . "/esp/" . $j["firmware_version"]
            . "/4MB-firmware.bin.md5?cc=" . rand()
        );
    }
    function _getMD5FileIndex() {
        global $j;
        return (
              "https://" . $j["firmware_server"]
            + $j["firmware_server_path"]
            . "/esp/" . $j["firmware_version"]
            . "/esp.index.html.gz.md5?cc=" . rand()
        );
    }
    $_SESSION["progress"] = 0;

    if ($_SERVER['REQUEST_URI'] == "/download/fpga") {
        $_SESSION["firmware.dc.md5"] = trim(file_get_contents(_getMD5FileFPGA()));
        echo "[" . _getMD5FileFPGA() . "]<br>[".$_SESSION["firmware.dc.md5"]."]";
    } else if ($_SERVER['REQUEST_URI'] == "/download/esp") {
        $_SESSION["firmware.bin.md5"] = trim(file_get_contents(_getMD5FileESP()));
        echo "[" . _getMD5FileESP() . "]<br>[".$_SESSION["firmware.bin.md5"]."]";
    } else if ($_SERVER['REQUEST_URI'] == "/download/index") {
        $_SESSION["esp_index_html_gz_md5"] = trim(file_get_contents(_getMD5FileIndex()));
        echo "[" . _getMD5FileIndex() . "]<br>[".$_SESSION["esp_index_html_gz_md5"]."]";
    }

    
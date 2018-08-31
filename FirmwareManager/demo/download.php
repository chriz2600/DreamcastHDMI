<?php
    if (session_status() == PHP_SESSION_NONE) {
        session_start();
    }

    if ($_SESSION["config"]) {
        $j = json_decode($_SESSION["config"], true);
    }

    function _getMD5File() {
        global $j;
        return (
              "https://" . $j["firmware_server"]
            . "/fw/" . $j["firmware_version"]
            . "/DCxPlus-" . $j["firmware_fpga"]
            . "-" . $j["firmware_format"]
            . ".dc.md5?cc=" . rand()
        );
    }
    $_SESSION["progress"] = 0;
    $_SESSION["firmware.dc.md5"] = trim(file_get_contents(_getMD5File()));

    echo "[" . _getMD5File() . "]<br>[".$_SESSION["firmware.dc.md5"]."]";
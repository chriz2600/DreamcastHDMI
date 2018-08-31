<?php
    function get($name, $defaultValue) {
        global $_POST, $j;
        if (isset($_POST[$name])) {
            if ($_POST[$name] == "") {
                $j[$name] = $defaultValue;
            } else {
                $j[$name] = $_POST[$name];
            }
        }
    }

    if (session_status() == PHP_SESSION_NONE) {
        session_start();
    }

    if ($_SESSION["config"]) {
        $j = json_decode($_SESSION["config"], true);
    }

    get("ssid", "");
    get("password", "");
    get("ota_pass", "");
    get("firmware_server", "dc.i74.de");
    get("firmware_version", "master");
    get("firmware_fpga", "10CL025");
    get("firmware_format", "VGA");
    get("http_auth_user", "Test");
    get("http_auth_pass", "testtest");

    $_SESSION["config"] = json_encode($j);
    echo "OK";
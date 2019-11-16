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
    get("firmware_server_path", "");
    get("firmware_version", "master");
    get("http_auth_user", "dchdmi");
    get("http_auth_pass", "");
    get("conf_ip_addr", "");
    get("conf_ip_gateway", "");
    get("conf_ip_mask", "");
    get("conf_ip_dns", "");
    get("hostname", "dc-firmware-manager");
    get("video_resolution", "VGA");
    get("video_mode", "CableDetect");
    get("reset_mode", "led");
    get("deinterlace_mode", "bob");
    get("protected_mode", "off");
    get("keyboard_layout", "us");

    $_SESSION["config"] = json_encode($j);
    echo "OK";
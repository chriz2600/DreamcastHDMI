<?php
    if (session_status() == PHP_SESSION_NONE) {
        session_start();
    }

    header('Content-Type: text/json');

    if (!$_SESSION["config"]) {
        $_SESSION["config"] = '{
            "ssid": "SomeSSID",
            "password": "SomePassword",
            "ota_pass": "SomeOTAPassword",
            "firmware_server": "dc.i74.de",
            "firmware_server_path": "",
            "firmware_version": "master",
            "http_auth_user": "dchdmi",
            "http_auth_pass": "testtest",
            "conf_ip_addr": "",
            "conf_ip_gateway": "",
            "conf_ip_mask": "",
            "conf_ip_dns": "",
            "hostname": "dc-firmware-manager",
            "video_resolution": "1080p",
            "video_mode": "ForceVGA",
            "reset_mode": "led",
            "deinterlace_mode": "bob",
            "protected_mode": "off",
            "keyboard_layout": "us",
            "fw_version": "v4.0-rlx",
            "flash_chip_size": "4194304"
        }';
    } else {
        error_log("c---> " . $_SESSION["config"]);
    }

    echo $_SESSION["config"];
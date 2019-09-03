<?php
    if (session_status() == PHP_SESSION_NONE) {
        session_start();
    }

    if ($_SESSION["esp_index_html_gz_md5"]) {
        echo $_SESSION["esp_index_html_gz_md5"];
    } else {
        echo "21987654321098765432109876543210";
    }

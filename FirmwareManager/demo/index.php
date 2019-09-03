<?php
    if (session_status() == PHP_SESSION_NONE) {
        session_start();
    }

    if ($_SESSION["index_html_gz_md5"]) {
        echo $_SESSION["index_html_gz_md5"];
    } else {
        echo "01234567890123456789012345678912";
    }

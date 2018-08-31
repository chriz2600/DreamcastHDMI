<?php
    if (session_status() == PHP_SESSION_NONE) {
        session_start();
    }

    $_SESSION["firmware.dc.md5"] = md5_file($_FILES['file']['tmp_name']);
    usleep(150000);


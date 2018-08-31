<?php
    if (session_status() == PHP_SESSION_NONE) {
        session_start();
    }

    echo "false\n";
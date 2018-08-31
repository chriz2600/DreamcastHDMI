// for browsers that don't support key property
keyboardeventKeyPolyfill.polyfill();

var FIRMWARE_FILE = "/firmware.dc";
var FIRMWARE_EXTENSION = "dc";
var ESP_FIRMWARE_FILE = "/firmware.bin";
var ESP_INDEX_STAGING_FILE = "/esp.index.html.gz";
var ESP_FIRMWARE_EXTENSION = "bin";

var process = {};
process.nextTick = (function () {
    var canSetImmediate = typeof window !== 'undefined' && window.setImmediate;
    var canPost = typeof window !== 'undefined' && window.postMessage && window.addEventListener;
    if (canSetImmediate) {
        return function (f) { return window.setImmediate(f) };
    }
    if (canPost) {
        var queue = [];
        window.addEventListener('message', function (ev) {
            var source = ev.source;
            if ((source === window || source === null) && ev.data === 'process-tick') {
                ev.stopPropagation();
                if (queue.length > 0) {
                    var fn = queue.shift();
                    fn();
                }
            }
        }, true);
        return function nextTick(fn) {
            queue.push(fn);
            window.postMessage('process-tick', '*');
        };
    }
    return function nextTick(fn) {
        setTimeout(fn, 0);
    };
})();

function typed(finish_typing) {
    return function(term, message, delay, finish) {
        anim = true;
        var prompt = term.get_prompt();
        var c = 0;
        if (message.length > 0) {
            term.set_prompt('');
            var new_prompt = '';
            var looper = function() {
                var chr = $.terminal.substring(message, c, c+1);
                new_prompt += chr;
                term.set_prompt(new_prompt);
                c++;
                if (c == length(message)) {
                    // execute in next interval
                    setTimeout(function() {
                        // swap command with prompt
                        finish_typing(term, message, prompt);
                        anim = false;
                        finish && finish();
                    }, delay);
                } else {
                    if (delay == 0) {
                        process.nextTick(looper);
                    } else {
                        setTimeout(looper, delay);
                    }
                }
                $('#term').scrollTop($('#term').prop('scrollHeight'));
            };
            setTimeout(looper, delay);
        }
    };
}
function length(string) {
    string = $.terminal.strip(string);
    return $('<span>' + string + '</span>').text().length;
}
var typed_prompt = typed(function(term, message, prompt) {
    term.set_prompt(message + ' ');
});
var typed_message = typed(function(term, message, prompt) {
    term.echo(message)
    term.set_prompt(prompt);
});
function fileInputChange() {
    var files = $("#fileInput").get(0).files;
    if (files && files[0]) {
        var reader = new FileReader();
        reader.onload = function(event) {
            var spark = new SparkMD5.ArrayBuffer();
            var md5 = spark.append(event.target.result);
            lastMD5 = spark.end();
            endTransaction("Selected file:\n"
                + "Name: " + files[0].name + "\n"
                + "Size: " + files[0].size + " Byte\n"
                + "MD5:  [[b;#fff;]" + lastMD5 + "]"
            );
        };
        reader.readAsArrayBuffer(files[0]);
    } else {
        endTransaction({ msg: "No file selected!", iserror: true });
    }
}

function startTransaction(msg, action) {
    finish = false;
    waiting = true;
    term.set_prompt('');
    term.find('.cursor').removeClass('blink');
    term.find('.cursor').hide();
    if ($.type(msg) === "object") {
        iserror = msg.iserror;
        msg = msg.msg || "";
    }
    if (msg) {
        typed_message(term, msg, 0, function() {
            finish = true;
        });
    } else {
        finish = true;
    }
    if (typeof(action) == "function") {
        action();
    }
}

function endTransaction(msg, iserror, cb) {
    (function wait() {
        if (finish) {
            if (msg) {
                if ($.type(msg) === "object") {
                    iserror = msg.iserror || false;
                    msg = msg.msg || "";
                }

                if (iserror) {
                    term.error(msg);
                } else {
                    term.echo(msg);
                }
            }
            term.find('.cursor').show();
            term.find('.cursor').addClass('blink');
            term.set_prompt('dc-hdmi> ');
            $('#term').scrollTop($('#term').prop('scrollHeight'));
            waiting = false;
            if (typeof(cb) == "function") {
                cb();
            }
        } else {
            setTimeout(wait, 350);
        }
    })();
}

var lastMD5;
var progressSize = 40;
var anim = false;
var waiting = false;
var finish = false;
var scanlines = $('.scanlines');
var tv = $('.tv');
var term = $('#term').terminal(function(command, term) {
    var match;
    if (command.match(/^\s*exit\s*$/)) {
        $('.tv').addClass('collapse');
        term.disable();
    } else if (command.match(/^\s*help\s*$/)) {
        help(true);
    } else if (command.match(/^\s*helpexpert\s*$/)) {
        help(true, true);
    } else if (command.match(/^\s*ls\s*$/)) {
        startTransaction(null, function() {
        listFiles();
    });
    } else if (command.match(/^\s*select\s*$/)) {
        startTransaction("Please select file to upload...", function() {
            $('#fileInput').click();
        });
        (function waitForDialog() {
            if (term.find('.cursor').hasClass('blink')) {
                setTimeout(function() {
                    fileInputChange();
                }, 150);
            } else {
                setTimeout(waitForDialog, 350);
            }
        })();
    } else if (command.match(/^\s*uploadfpga\s*$/)) {
        startTransaction(null, function() {
            uploadFPGA();
        });
    } else if (command.match(/^\s*uploadesp\s*$/)) {
        startTransaction(null, function() {
            uploadESP();
        });
    } else if (command.match(/^\s*uploadindex\s*$/)) {
        startTransaction(null, function() {
            uploadESPIndex();
        });
    } else if (command.match(/^\s*flash\s*$/)) {
        flashall(0);
    } else if (command.match(/^\s*flashfpga\s*$/)) {
        startTransaction(null, function() {
            flashFPGA();
        });
    } else if (command.match(/^\s*flashfpgasecure\s*$/)) {
        startTransaction(null, function() {
            flashFPGA(true);
        });
    } else if (command.match(/^\s*flashesp\s*$/)) {
        startTransaction(null, function() {
            flashESP();
        });
    } else if (command.match(/^\s*flashindex\s*$/)) {
        startTransaction(null, function() {
            flashESPIndex();
        });
    } else if (command.match(/^\s*reset\s*$/)) {
        resetall(0);
    } else if (command.match(/^\s*resetfpga\s*$/)) {
        startTransaction(null, function() {
            reset();
        });
    } else if (command.match(/^\s*resetesp\s*$/)) {
        startTransaction(null, function() {
            restartESP(function() {
                document.location.reload(true);
            });
        });
    } else if (command.match(/^\s*file\s*$/)) {
        startTransaction(null, function() {
            fileInputChange();
        });
    } else if (command.match(/^\s*check\s*$/)) {
        checkall(0);
    } else if (command.match(/^\s*checkfpga\s*$/)) {
        startTransaction(null, function() {
            getConfig(false, getFPGAFirmwareData);
        });
    } else if (command.match(/^\s*checkesp\s*$/)) {
        startTransaction(null, function() {
            getConfig(false, getESPFirmwareData);
        });
    } else if (command.match(/^\s*checkindex\s*$/)) {
        startTransaction(null, function() {
            getConfig(false, getESPIndexFirmwareData);
        });
    } else if (command.match(/^\s*setup\s*$/)) {
        startTransaction(null, function() {
            getConfig(false, setupMode);
        });
    } else if (command.match(/^\s*config\s*$/)) {
        startTransaction(null, function() {
            getConfig(true);
        });
    } else if (command.match(/^\s*cleanup\s*$/)) {
        startTransaction(null, function() {
            doDeleteFirmwareFile();
        });
    } else if (command.match(/^\s*flash_chip_size\s*$/)) {
        startTransaction(null, function() {
            getFlashChipSize();
        });
    } else if (command.match(/^\s*download\s*$/)) {
        downloadall(0);
    } else if (command.match(/^\s*downloadfpga\s*$/)) {
        startTransaction(null, function() {
            getConfig(false, downloadFPGA);
        });
    } else if (command.match(/^\s*downloadesp\s*$/)) {
        startTransaction(null, function() {
            getConfig(false, downloadESP);
        });
    } else if (command.match(/^\s*downloadindex\s*$/)) {
        startTransaction(null, function() {
            getConfig(false, downloadESPIndex);
        });
    } else if (command.match(/^\s*res_vga\s*$/)) {
        startTransaction(null, function() {
            setResolution("VGA");
        });
    } else if (command.match(/^\s*res_480p\s*$/)) {
        startTransaction(null, function() {
            setResolution("480p");
        });
    } else if (command.match(/^\s*res_960p\s*$/)) {
        startTransaction(null, function() {
            setResolution("960p");
        });
    } else if (command.match(/^\s*res_1080p\s*$/)) {
        startTransaction(null, function() {
            setResolution("1080p");
        });
    } else if (command.match(/^\s*resetpll\s*$/)) {
        startTransaction(null, function() {
            resetpll();
        });
    } else if (command.match(/^\s*details\s*$/)) {
        typed_message(term,
              getHelpDetailsFPGA()
            + getHelpDetailsESP()
            + getHelpDetailsIndex(),
        0);
    } else if (command.match(/^\s*detailsfpga\s*$/)) {
        typed_message(term, getHelpDetailsFPGA(), 0);
    } else if (command.match(/^\s*detailsesp\s*$/)) {
        typed_message(term, getHelpDetailsESP(), 0);
    } else if (command.match(/^\s*detailsindex\s*$/)) {
        typed_message(term, getHelpDetailsIndex(), 0);
    } else if (command.match(/^\s*banner\s*$/)) {
        term.clear();
        term.greetings();
    } else if (match = command.match(/^\s*osd\s*(on|off)\s*$/)) {
        startTransaction(null, function() {
            if (match[1] == "on") {
                osdControl("on");
            } else {
                osdControl("off");
            }
        });
    } else if (match = command.match(/^\s*osdwrite\s*([0-9]+)\s*([0-9]+)\s*"([^"]*)"\s*$/)) {
        startTransaction(null, function() {
            osdWrite(match[1], match[2], match[3]);
        });
    } else if (command !== '') {
        term.error('unkown command, try:');
        help();
    } else {
        term.echo('');
    }
}, {
    name: 'dc-hdmi',
    onResize: set_size,
    exit: false,
    onInit: function(term) {
        set_size();
        typed_message(term, 'Type [[b;#fff;]help] to get help!', 36, function() {
            startTransaction(null, function() {
                checkSetupStatus();
            });
        });
    },
    completion: function(s, cb) {
        if (!s) {
            cb([
                "help",
                "helpexpert",
                "get",
                "check",
                "download",
                "flash",
                "reset",
                "details",
                "setup",
                "config",
                "clear",
                "banner",
                "cleanup",
                "ls",
                "exit"
            ]);
            return;
        }
        cb([
            "help",
            "helpexpert",
            "get",
            "check",
                "checkfpga",
                "checkesp",
                "checkindex",
            "select",
            "file",
            /* upload */
                "uploadfpga",
                "uploadesp",
                "uploadindex",
            "download",
                "downloadfpga",
                "downloadesp",
                "downloadindex",
            "flash",
                "flashfpga",
                "flashfpgasecure",
                "flashesp",
                "flashindex",
            "reset",
                "resetfpga",
                "resetesp",
            "details",
                "detailsfpga",
                "detailsesp",
                "detailsindex",
            "setup",
            "config",
            "clear",
            "banner",
            //"restart",
            "cleanup",
            "ls",
            "exit"
        ]);
    },
    prompt: 'dc-hdmi> ',
    greetings: [
        '       __                                        __ ',
        '   ___/ /___ _____ ___   ______ ____ ___   ____ / /_',
        '  / _  // _// _  // _ \\ /     // __// _ \\ /  _// __/',
        ' / // // / / ___// // // / / // /_ / // /_\\ \\ / /__ ',
        ' \\___//_/ /____/ \\__\\_/_/ /_//___/ \\__\\_/___/ \\___/',
        '                            __  __ __   ______ __',
        '                           / __/ //  \\ /     // /',
        '     @citrus3000psi       / __  // / // / / // /',
        '       @chriz2600        /_/ /_//___//_/ /_//_/',
        ''
    ].join('\n'),
    keydown: function(e) {
        //disable keyboard when animating or waiting
        if (waiting || anim) {
            return false;
        }
    }
});
function set_size() {
    // for window heihgt of 170 it should be 2s
    var height = $(window).height();
    var time = (height * 2) / 170;
    scanlines[0].style.setProperty("--time", time);
    tv[0].style.setProperty("--width", $(window).width());
    tv[0].style.setProperty("--height", height);
}

function getHelpDetailsFPGA() {
    var msg = "";
    msg = "[[b;#fff;]FPGA firmware upgrade procedure:]\n"
        + "  ________              ___________              _________\n"
        + " /        \\            /           \\ flashfpga  /         \\\n"
        + " | jQuery |            |  esp-07s  |----------->|  FPGA   |\n"
        + " |  Term  | uploadfpga |  staging  | resetfpga  |  flash  |\n"
        + " | (this) |----------->|   flash   |----------->|  (SPI)  |\n"
        + " \\________/            \\___________/            \\_________/\n"
        + "   |   /|\\                  /|\\\n"
        + "   |    |       downloadfpga |\n"
        + "  \\|/   |               _____|_____\n"
        + "   select              /           \\\n"
        + "   (from               | dc.i74.de |\n"
        + "     HD)               \\___________/\n"
        + " \n";
    return msg;
}

function getHelpDetailsESP() {
    var msg = "";
    msg = "[[b;#fff;]ESP firmware upgrade procedure:]\n"
        + "  ________             ___________\n"
        + " /        \\           /           \\\n"
        + " | jQuery |           |  esp-07s  |───╮flashesp\n"
        + " |  Term  | uploadesp |  staging  |<──┫\n"
        + " | (this) |---------->|   flash   |───╯resetesp\n"
        + " \\________/           \\___________/\n"
        + "   |   /|\\                 /|\\\n"
        + "   |    |       downloadesp |\n"
        + "  \\|/   |             ______|______\n"
        + "   select            /             \\\n"
        + "   (from             | esp.i74.de  |\n"
        + "     HD)             \\_____________/\n"
        + " \n";
    return msg;
}

function getHelpDetailsIndex() {
    var msg = "";
    msg = "[[b;#fff;]index.html upgrade procedure:]\n"
        + "  ________               ___________\n"
        + " /        \\             /           \\\n"
        + " | jQuery |             |  esp-07s  |───╮flashindex\n"
        + " |  Term  | uploadindex |  staging  |<──╯\n"
        + " | (this) |------------>|   flash   |--->(browser) reload\n"
        + " \\________/             \\___________/\n"
        + "   |   /|\\                   /|\\\n"
        + "   |    |       downloadindex |\n"
        + "  \\|/   |               ______|______\n"
        + "   select              /             \\\n"
        + "   (from               | esp.i74.de  |\n"
        + "     HD)               \\_____________/\n"
        + " \n";
    return msg;
}

function help(full, expert) {
    var msg = "";
    if (full) {
        msg = " \n";
        if (!expert) {
            msg += "[[b;#fff;]System upgrade procedure:]\n";
            msg += " \n";
            msg += "1) Use [[b;#fff;]check] to see, if any updates are available.\n";
            msg += "2) If updates are available, use [[b;#fff;]download] to stage the updates.\n";
            msg += "3) Use [[b;#fff;]flash] to apply the updates.\n";
            msg += "4) Use [[b;#fff;]reset] to restart DCHDMI with the updates.\n";
            msg += " \n";
            msg += "- Use [[b;#fff;]config] to display current configuration.\n";
            msg += "- Use [[b;#fff;]setup] to update the configuration.\n";
            msg += " \n";
        }
    }
    if (expert) {
        msg += "[[i;#fff;]Expert command set:]\n";
    } else {
        msg += "[[i;#fff;]Basic commands:]\n";
    }
    msg += "[[b;#fff;]check] [[;#666;].........] check if new firmware is available\n";
    if (expert) {
        msg += "[[b;#fff;]check][[bi;#fff000;]type] [[;#666;].....] check if new firmware is available for [[bi;#fff000;]type]\n";
    }

    if (expert) {
        msg += "[[b;#fff;]select] [[;#666;]........] select file to upload\n";
        msg += "[[b;#fff;]file] [[;#666;]..........] show information on selected file\n";
        msg += "[[b;#fff;]upload][[bi;#fff000;]type] [[;#666;]....] upload selected file as [[bi;#fff000;]type]\n";
    }

    msg += "[[b;#fff;]download] [[;#666;]......] download latest firmware set\n"
    if (expert) {
        msg += "[[b;#fff;]download][[bi;#fff000;]type] [[;#666;]..] download latest [[bi;#fff000;]type] from server\n";
    }

    msg += "[[b;#fff;]flash] [[;#666;].........] flash firmware set from staging area\n";
    if (expert) {
        msg += "[[b;#fff;]flash][[bi;#fff000;]type] [[;#666;].....] flash [[bi;#fff000;]type] from staging area\n";
    }
    msg += "[[b;#fff;]reset] [[;#666;].........] full DCHDMI reset\n";
    if (expert) {
        msg += "[[b;#fff;]reset][[bi;#fff000;]type] [[;#666;].....] reset [[bi;#fff000;]type] \n";
    }
    msg += "[[b;#fff;]setup] [[;#666;].........] enter setup mode\n";
    msg += "[[b;#fff;]config] [[;#666;]........] get current setup\n";
    msg += "[[b;#fff;]clear] [[;#666;].........] clear terminal screen\n";

    if (expert) {
        msg += "[[b;#fff;]cleanup] [[;#666;].......] remove staged firmware files\n";
        msg += "[[b;#fff;]ls] [[;#666;]............] list files on ESP flash\n";
    }

    msg += "[[b;#fff;]exit] [[;#666;]..........] end terminal\n";

    if (expert) {
        msg += "\n";
        msg += "Available [[bi;#fff000;]type]s:\n";
        msg += "[[b;#fff;]fpga] [[;#666;]..........] FPGA\n";
        msg += "[[b;#fff;]esp] [[;#666;]...........] ESP\n";
        msg += "[[b;#fff;]index] [[;#666;].........] index.html\n";
        msg += "\n";
        msg += "Special command:\n";
        msg += "[[b;#fff;]flashfpgasecure] flash FPGA from staging area, while disabling fpga\n";
    }

    //typed_message(term, msg, 0);
    term.echo(msg);
}

function progress(percent, width) {
    var size = Math.round(width*percent/100);
    var left = '', taken = '', i;
    for (i=size; i--;) {
        taken += '=';
    }
    if (taken.length > 0 && percent < 100) {
        taken = taken.replace(/=$/, '>');
    }
    for (i=width-size; i--;) {
        left += ' ';
    }
    return '[' + taken + left + '] ' + percent + '%';
}

function uploadFPGA(isRetry) {
    upload(false, "/upload/fpga", FIRMWARE_FILE);
}

function uploadESP(isRetry) {
    upload(false, "/upload/esp", ESP_FIRMWARE_FILE);
}

function uploadESPIndex(isRetry) {
    upload(false, "/upload/index", ESP_INDEX_STAGING_FILE);
}

function upload(isRetry, uri, dataFile, successCallback) {
    var file = $("#fileInput").get(0).files[0];
    if (!file) {
        endTransaction("No file selected!", true);
        return;
    }
    var formData = new FormData();
    var client = new XMLHttpRequest();

    formData.append("file", file);

    client.onerror = function(e) {
        if (isRetry) {
            endTransaction("Error during upload!", true);
        } else {
            upload(true, uri, dataFile, successCallback);
        }
    };

    client.onload = function(e) {
        if (client.status >= 200 && client.status < 400) {
            $.ajax(dataFile + ".md5").done(function (data) {
                endTransactionWithMD5Check(lastMD5, data, "Please try to re-upload.", successCallback);
            }).fail(function() {
                endTransaction('Error reading checksum', true);
            });
        } else {
            endTransaction('Error uploading: ' + client.status, true);
        }
    };

    client.upload.onprogress = function(e) {
        var p = Math.round(100 / e.total * e.loaded);
        term.set_prompt(progress(p - 1, progressSize));
    };

    client.onabort = function(e) {
        endTransaction("Abort!", true);
    };

    term.set_prompt(progress(0, progressSize));

    client.open("POST", uri + "?size=" + file.size);
    client.send(formData);
}

var setupData = {};
var currentConfigData = {};
var setupDataMapping = {
    ssid:             [ "WiFi SSID        ", "empty" ],
    password:         [ "WiFi Password    ", "empty" ],
    ota_pass:         [ "OTA Password     ", "empty" ],
    firmware_server:  [ "Firmware Server  ", "dc.i74.de" ],
    firmware_version: [ "Firmware Version ", "master" ],
    http_auth_user:   [ "HTTP User        ", "Test" ],
    http_auth_pass:   [ "HTTP Password    ", "testtest" ],
    conf_ip_addr:     [ "IP address       ", "empty" ],
    conf_ip_gateway:  [ "Gateway          ", "empty" ],
    conf_ip_mask:     [ "Netmask          ", "empty" ],
    conf_ip_dns:      [ "DNS              ", "empty" ],
    hostname:         [ "Hostname         ", "dc-firmware-manager" ],
    video_resolution: [ "Video output     ", "1080p" ],
    video_mode:       [ "Video mode       ", "CableDetect" ]
};
var dataExcludeMap = {
    "flash_chip_size":"", 
    "fw_version":""
};

function setupDataDisplayToString(data, isSafe) {
    var value = " \n";
    for (x in data) {
        if (x in dataExcludeMap) {
            continue;
        }
        var t = setupDataMapping[x][0] || x;
        value += t + ": " 
            + (
                data[x] 
                ? ('[[b;#fff;]' + data[x] + ']')
                : (isSafe ? '[[b;yellow;]reset]' : '[[b;red;]not yet set]')
            )
             + " \n";
    }
    return value;
}

function prepareQuestion(pos, total, field) {
    return { 
        q : pos + '/' + total + ': ' + setupDataMapping[field][0] 
        + " \n    (default: [[b;yellow;]" + setupDataMapping[field][1] + "])"
        + " \n    (current value: " 
        + (currentConfigData[field] 
            ? "[[b;green;]" + currentConfigData[field] + "]"
            : "[[b;red;]not yet set]"
        )
        + ")"
        + " \n    New value? ", 
        cb: function(value) { 
            setupData[field] = value; 
        }
    };
}

function doDeleteFirmwareFile() {
    $.ajax("/cleanup");
    endTransaction('firmware file removed.');
}

function listFiles() {
    $.ajax("/list-files").done(function (data) {
        endTransaction(createListing(data));
    }).fail(function() {
        endTransaction('Error listing files.', true);
    });
}

String.prototype.paddingLeft = function(paddingValue) {
    return String(paddingValue + this).slice(-paddingValue.length);
};

String.prototype.paddingRight = function(paddingValue) {
    return String(this + paddingValue).substring(0, paddingValue.length);
};

function createListing(data) {
    var msg = "";
    var maxFilenameLen = 0;

    data.files.sort(function(a, b){
        return (
            (a.size > b.size) 
            ? -1 
            : (
                (a.size < b.size) 
                ? 1 
                : (a.name < b.name ? -1 : (a.name > b.name ? 1 : 0))
            )
        );
    });

    for (x in data.files) {
        var name = data.files[x].name;
        msg += (""+data.files[x].size).paddingLeft("       ") + " " + name + "\n";
        if (name.length > maxFilenameLen) maxFilenameLen = name.length;
    }
    return (
       "Size    Filename\n"
       + Array(maxFilenameLen + 9).join('-') + "\n"
       + msg
       + Array(maxFilenameLen + 9).join('-') + "\n"
       + data.usedBytes + " of " + data.totalBytes + " bytes used\n"
    );
}

function getFlashChipSize() {
    $.ajax("/flash_size").done(function (data) {
        endTransaction("Flash chip size: " + $.trim(data) + " Bytes");
    }).fail(function() {
        endTransaction('Error getting current config.', true);
    });
}

function osdControl(state) {
    $.ajax("/osd/" + state).done(function (data) {
        endTransaction("OSD is " + state);
    }).fail(function() {
        endTransaction('Error switching OSD to ' + state, true);
    });
}

function osdWrite(column, row, text) {
    var data = {
        'column': column,
        'row': row,
        'text': text
    };
    $.ajax({
        'type': "POST",
        'url': "/osdwrite",
        'data': $.param(data, true)
    }).done(function (data) {
        endTransaction('[[b;#fff;]Done].\n');
    }).fail(function() {
        endTransaction('Error.', true);
    });
}

function setResolution(type) {
    $.ajax("/res/" + type).done(function (data) {
        endTransaction("Switched resolution to: " + type);
    }).fail(function() {
        endTransaction('Error switching resolution.', true);
    });
}

function resetpll() {
    $.ajax("/reset/pll").done(function (data) {
        endTransaction("Reset PLL done.");
    }).fail(function() {
        endTransaction('Error resetting PLL.', true);
    });
}

function getConfig(show, cb) {
    $.ajax("/config").done(function (data) {
        currentConfigData = data;
        endTransaction(
            show ? " \n-- Current config: ----------------------------------"
            + setupDataDisplayToString(currentConfigData, false)
            + "-----------------------------------------------------" : ""
        , null, cb);
    }).fail(function() {
        endTransaction('Error getting current config.', true);
    });
}

function setupMode() {
    term.history().disable();
    var questions = [
        {
            pc: function() {
                if (JSON.stringify(setupData) == JSON.stringify({})) {
                    term.error("no changes made.");
                    return false;
                }
                return true;
            },
            q : function() {
                return " \n-- Changes to save: ---------------------------------"
                    + setupDataDisplayToString(setupData, true)
                    + "-----------------------------------------------------"
                    + ' \nAfter saving changes, you will have to'
                    + ' \nreset this application by typing: [[b;#fff;]reset].'
                    + ' \nSave changes (y)es/(n)o? ';
            },
            cb: function(value) {
                if (value.match(/^(y|yes)$/)) {
                    // start saving transaction
                    startTransaction("saving setup...", function() {
                        $.ajax({
                            type: "POST",
                            url: "/setup",
                            data: $.param(setupData, true)
                        }).done(function (data) {
                            endTransaction('[[b;#fff;]Done] Setup data saved.\n');
                        }).fail(function() {
                            endTransaction('Error saving setup data.', true);
                        });
                    });
                } else {
                    term.error("discarded setup");
                }
                // reset setupData
                setupData = {};
            }
        }
    ];
    var keyz = Object.keys(setupDataMapping);
    var size = keyz.length;
    for (var i = size - 1 ; i >= 0 ; i--) {
        questions.unshift(prepareQuestion(i + 1, size, keyz[i]));
    }
    var next = function() {
        var n = questions.shift();
        var v;
        if (n) {
            n.pc = n.pc || function() { return true; };
            if (n.pc()) {
                term.push(function(command) {
                    term.pop();
                    var lm = command.match(/^(.+)$/i);
                    if (lm) {
                        lm[0] = $.trim(lm[0]);
                        if (n.cb) {
                            n.cb(lm[0]);
                        }
                    }
                    next();
                }, {
                    prompt: typeof(n.q) == "function" ? n.q() : n.q
                });
            } else {
                term.history().enable();
            }
        } else {
            term.history().enable();
        }
    }

    term.echo(" \n[[b;#fff;]This will guide you through the setup process:]\n"
        + "- Just hit return to leave the value unchanged.\n"
        + "- Enter a single space to reset value to firmware default.\n"
        + "- CTRL-D to abort.\n"
    );
    next();
}

function checkSetupStatus() {
    $.ajax("/issetupmode").done(function (data) {
        var setupStatus = $.trim(data);
        if (setupStatus === "false") {
            endTransaction(null, null, function() {
                getConfig(false, function() {
                    typed_message(term, "Firmware version: [[b;#fff;]" + currentConfigData["fw_version"] + "]\n", 36);
                });
            });
        } else {
            endTransaction(null, null, function() {
                getConfig(false, setupMode);
            });
        }
    }).fail(function() {
        endTransaction("Error checking setup status!", true);
    });
}

function _getFPGAMD5File() {
    return (
          "//" + currentConfigData["firmware_server"]
        + "/fw/" + currentConfigData["firmware_version"]
        + "/DCxPlus-default"
        + "." + FIRMWARE_EXTENSION + ".md5?cc=" + Math.random()
    );
}

function _getESPMD5File() {
    return (
          "//esp.i74.de"
        + "/" + currentConfigData["firmware_version"]
        + "/" + currentConfigData["flash_chip_size"] / 1024 / 1024 + "MB"
        + "-" + "firmware"
        + "." + ESP_FIRMWARE_EXTENSION + ".md5?cc=" + Math.random()
    );
}

function _getESPIndexMD5File() {
    return (
          "//esp.i74.de"
        + "/" + currentConfigData["firmware_version"]
        + "/" + ESP_INDEX_STAGING_FILE + ".md5?cc=" + Math.random()
    );
}

function createCheckResult(lastFlashMd5, stagedMd5, origMd5, title) {
    return '[[b;#fff;]'+title+']:\n'
    + ' Installed: ' + (lastFlashMd5 == "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" ? "[[b;yellow;]No previously flashed version found]" : '[[b;#fff;]' + lastFlashMd5 + ']') + '\n'
    + ' Staged:    ' + (stagedMd5 == "00000000000000000000000000000000" ? "[[b;yellow;]No staged version found]" : '[[b;#fff;]' + stagedMd5 + ']') + '\n'
    + ' Official:  ' + '[[b;#fff;]' + origMd5 + ']\n'
    + ' Result:    ' + ((lastFlashMd5 == origMd5)
        ? 'Currently installed '+title+'\n            is already the latest version.'
        : 'New '+title+' available.')
    + '\n';
}

function checkall(step) {
    switch(step) {
        case 0:
            startTransaction(null, function() {
                getConfig(false, function() {
                    checkall(step + 1);
                });
            });
            break;
        case 1:
            startTransaction(null, function() {
                getFPGAFirmwareData(function() {
                    checkall(step + 1);
                });
            });
            break;
        case 2:
            startTransaction(null, function() {
                getESPFirmwareData(function() {
                    checkall(step + 1);
                });
            });
            break;
        case 3:
            startTransaction(null, function() {
                getESPIndexFirmwareData(function() {
                    checkall(step + 1);
                });
            });
            break;
        default:
            break;
    }
}

function getFPGAFirmwareData(successCallback, resultCallback) {
    getFirmwareData(
        "/etc/last_flash_md5",
        FIRMWARE_FILE,
        _getFPGAMD5File(),
          resultCallback 
        ? function(lastFlashMd5, stagedMd5, origMd5) { resultCallback(lastFlashMd5, stagedMd5, origMd5); }
        : function(lastFlashMd5, stagedMd5, origMd5) {
            endTransaction(
                createCheckResult(lastFlashMd5, stagedMd5, origMd5, "FPGA firmware"),
                false,
                successCallback
            );
        }
    );
}

function getESPFirmwareData(successCallback, resultCallback) {
    getFirmwareData(
        "/etc/last_esp_flash_md5",
        ESP_FIRMWARE_FILE,
        _getESPMD5File(),
          resultCallback 
        ? function(lastFlashMd5, stagedMd5, origMd5) { resultCallback(lastFlashMd5, stagedMd5, origMd5); }
        : function(lastFlashMd5, stagedMd5, origMd5) {
            endTransaction(
                createCheckResult(lastFlashMd5, stagedMd5, origMd5, "ESP firmware"),
                false,
                successCallback
            );
        }
    );
}

function getESPIndexFirmwareData(successCallback, resultCallback) {
    getFirmwareData(
        "/index.html.gz.md5",
        ESP_INDEX_STAGING_FILE,
        _getESPIndexMD5File(),
          resultCallback 
        ? function(lastFlashMd5, stagedMd5, origMd5) { resultCallback(lastFlashMd5, stagedMd5, origMd5); }
        : function(lastFlashMd5, stagedMd5, origMd5) {
            endTransaction(
                createCheckResult(lastFlashMd5, stagedMd5, origMd5, "ESP index.html"),
                false,
                successCallback
            );
        }
    );
}

function getFirmwareData(lastMD5Uri, file, origMD5Uri, loadCallback) {
    $.ajax(lastMD5Uri).done(function (data) {
        var lastFlashMd5 = $.trim(data);
        $.ajax(file + ".md5").done(function (data) {
            var stagedMd5 = $.trim(data);
            $.ajax(origMD5Uri).done(function (data) {
                var origMd5 = $.trim(data);
                loadCallback(lastFlashMd5, stagedMd5, origMd5);
            }).fail(function() {
                endTransaction('Error reading original checksum', true);
            });
        }).fail(function() {
            endTransaction("Error reading checksum!", true);
        });
    }).fail(function() {
        endTransaction("Error reading checksum!", true);
    });

}
function checkFirmware() {
    $.ajax("/etc/last_flash_md5").done(function (data) {
        var lastFlashMd5 = $.trim(data);
        $.ajax(_getFPGAMD5File()).done(function (data) {
            var origMd5 = $.trim(data);
            if (lastFlashMd5 == origMd5) {
                endTransaction('Currently installed firmware is already the latest version.');
            } else {
                endTransaction('New firmware available.');
            }
        }).fail(function() {
            endTransaction('Error reading original checksum', true);
        });
    }).fail(function() {
        endTransaction("Error resetting fpga!", true);
    });
}

function resetall(step) {
    switch (step) {
        case 0:
            startTransaction(null, function() {
                reset_all(function() {
                    resetall(step + 1);
                }, true);
            });
            break;
        // case 1:
        //     startTransaction(null, function() {
        //         restartESP(function() {
        //             resetall(step + 1);
        //         });
        //     });
        //     break;
        default:
            document.location.reload(true);
            break;
    }
}

function reset(successCallback, noNewline) {
    $.ajax("/reset/fpga").done(function (data) {
        endTransaction('FPGA reset [[b;green;]OK]' + (noNewline ? '' : '\n'), false, successCallback);
    }).fail(function() {
        endTransaction("Error resetting fpga!", true);
    });
}

var retryTimeout;

function reset_all(successCallback, noNewline) {
    $.ajax({ url: "/reset/all", timeout: 1000 });
    term.echo('Reset [[b;green;]OK]');
    retryTimeout = 100;
    pingESP(successCallback);
}

function restartESP(successCallback) {
    $.ajax({ url: "/reset/esp", timeout: 1000 });
    term.echo('ESP reset [[b;green;]OK]');
    retryTimeout = 100;
    pingESP(successCallback);
}

function pingESP(successCallback) {
    $.ajax({
        url: "/ping",
        timeout: 1000
    }).done(function (data) {
        term.set_prompt('Done, reloading page...');
        successCallback();
    }).fail(function() {
        term.set_prompt("Waiting for ESP to restart (" + (retryTimeout) + ")");
        $('#term').scrollTop($('#term').prop('scrollHeight'));
        retryTimeout--;
        if (retryTimeout > 0) {
            pingESP(successCallback);
        } else {
            endTransaction('ping within timeout failed.', true);
        }
    });
}

function doProgress(successCallback) {
    function progressPoll() {
        $.ajax("/progress").done(function (data) {
            var pgrs = $.trim(data);
            if (pgrs.indexOf("ERROR") == 0) {
                endTransaction(pgrs, true)
            } else {
                term.set_prompt(progress(pgrs, progressSize));
                if (pgrs == "100") {
                    successCallback();
                } else {
                    setTimeout(progressPoll, 1500);
                }
            }
        }).fail(function () {
            endTransaction('Error reading progress', true);
        })
    };
    setTimeout(progressPoll, 1500);
}

function flashall(step) {
    switch (step) {
        case 0:
            startTransaction(null, function() {
                getConfig(false, function() {
                    flashall(step + 1);
                });
            });
            break;
        case 1:
            term.echo("[[b;#fff;]Step 1/3:]\nChecking FPGA firmware");
            startTransaction(null, function() {
                getFPGAFirmwareData(null, function(lastFlashMd5, stagedMd5, origMd5) {
                    if (lastFlashMd5 == stagedMd5) {
                        endTransaction('FPGA firmware is the same as the staged version.', false, function() { flashall(step + 2); });
                    } else {
                        endTransaction('FPGA firmware is older than the staged version.', false, function() { flashall(step + 1); });
                    }
                });
            });
            break;
        case 2:
            term.echo("Flashing FPGA firmware");
            startTransaction(null, function() {
                flashFPGA(false, function() {
                    flashall(step + 1);
                });
            });
            break;
        case 3:
            term.echo("[[b;#fff;]Step 2/3:]\nChecking ESP firmware");
            startTransaction(null, function() {
                getESPFirmwareData(null, function(lastFlashMd5, stagedMd5, origMd5) {
                    if (lastFlashMd5 == stagedMd5) {
                        endTransaction('ESP firmware is the same as the staged version.', false, function() { flashall(step + 2); });
                    } else {
                        endTransaction('ESP firmware is older than the staged version.', false, function() { flashall(step + 1); });
                    }
                });
            });
            break;
        case 4:
            term.echo("Flashing ESP firmware");
            startTransaction(null, function() {
                flashESP(function() {
                    flashall(step + 1);
                });
            });
            break;
        case 5:
            term.echo("[[b;#fff;]Step 3/3:]\nChecking ESP index.html");
            startTransaction(null, function() {
                getESPIndexFirmwareData(null, function(lastFlashMd5, stagedMd5, origMd5) {
                    if (lastFlashMd5 == stagedMd5) {
                        endTransaction('ESP index.html is the same as the staged version.', false, function() { flashall(step + 2); });
                    } else {
                        endTransaction('ESP index.html is older than the staged version.', false, function() { flashall(step + 1); });
                    }
                });
            });
            break;
        case 6:
            term.echo("Flashing ESP index.html");
            startTransaction(null, function() {
                flashESPIndex(function() {
                    flashall(step + 1);
                });
            });
            break;
        default:
            endTransaction("[[b;#fff;]Done!]\n");
            break;
    }
}

function downloadall(step) {
    switch(step) {
        case 0:
            startTransaction(null, function() {
                getConfig(false, function() {
                    downloadall(step + 1);
                });
            });
            break;
        case 1:
            term.echo("[[b;#fff;]Step 1/3:]\nChecking FPGA firmware");
            startTransaction(null, function() {
                getFPGAFirmwareData(null, function(lastFlashMd5, stagedMd5, origMd5) {
                    if (stagedMd5 == origMd5) {
                        endTransaction('Staged FPGA firmware is already the newest version.', false, function() { downloadall(step + 2); });
                    } else {
                        endTransaction('Staged FPGA firmware is older than the newest version.', false, function() { downloadall(step + 1); });
                    }
                });
            });
            break;
        case 2:
            term.echo("Downloading FPGA firmware");
            startTransaction(null, function() {
                downloadFPGA(function() {
                    downloadall(step + 1);
                });
            });
            break;
        case 3:
            term.echo("[[b;#fff;]Step 2/3:]\nChecking ESP firmware");
            startTransaction(null, function() {
                getESPFirmwareData(null, function(lastFlashMd5, stagedMd5, origMd5) {
                    if (stagedMd5 == origMd5) {
                        endTransaction('Staged ESP firmware is already the newest version.', false, function() { downloadall(step + 2); });
                    } else {
                        endTransaction('Staged ESP firmware is older than the newest version.', false, function() { downloadall(step + 1); });
                    }
                });
            });
            break;
        case 4:
            term.echo("Downloading ESP firmware");
            startTransaction(null, function() {
                downloadESP(function() {
                    downloadall(step + 1);
                });
            });
            break;
        case 5:
            term.echo("[[b;#fff;]Step 3/3:]\nChecking ESP index.html");
            startTransaction(null, function() {
                getESPIndexFirmwareData(null, function(lastFlashMd5, stagedMd5, origMd5) {
                    if (stagedMd5 == origMd5) {
                        endTransaction('Staged ESP index.html is already the newest version.', false, function() { downloadall(step + 2); });
                    } else {
                        endTransaction('Staged ESP index.html is older than the newest version.', false, function() { downloadall(step + 1); });
                    }
                });
            });
            break;
        case 6:
            term.echo("Downloading ESP index.html");
            startTransaction(null, function() {
                downloadESPIndex(function() {
                    downloadall(step + 1);
                });
            });
            break;
        default:
            endTransaction("[[b;#fff;]Done!]\n");
            break;
    }
}

function downloadFPGA(successCallback) {
    download("/download/fpga", FIRMWARE_FILE, _getFPGAMD5File(), successCallback);
}

function downloadESP(successCallback) {
    download("/download/esp", ESP_FIRMWARE_FILE, _getESPMD5File(), successCallback);
}

function downloadESPIndex(successCallback) {
    download("/download/index", ESP_INDEX_STAGING_FILE, _getESPIndexMD5File(), successCallback);
}

function download(uri, file, origMD5File, successCallback) {
    //startSpinner(term, spinners["shark"]);
    term.set_prompt(progress(0, progressSize));
    $.ajax(uri).done(function (data) {
        doProgress(
            function() {
                $.ajax(file + ".md5").done(function (data) {
                    var calcMd5 = $.trim(data);
                    $.ajax(origMD5File).done(function (data) {
                        var origMd5 = $.trim(data);
                        endTransactionWithMD5Check(calcMd5, origMd5, "Please try to re-download file.", successCallback);
                    }).fail(function() {
                        endTransaction('Error reading original checksum', true);
                    });
                }).fail(function() {
                    endTransaction('Error reading checksum', true);
                });
            }
        );
    }).fail(function() {
        endTransaction("Error starting download!", true);
    });
}

function flashFPGA(secure, successCallback) {
    flash(
        (secure ? "/flash/secure/fpga" : "/flash/fpga"),
        FIRMWARE_FILE,
        "/etc/last_flash_md5",
        successCallback
    );
}

function flashESP(successCallback) {
    flash(
        "/flash/esp",
        ESP_FIRMWARE_FILE,
        "/etc/last_esp_flash_md5",
        successCallback
    );
}

function flashESPIndex(successCallback) {
    flash(
        "/flash/index",
        ESP_INDEX_STAGING_FILE,
        "/index.html.gz.md5",
        successCallback
    );
}

function flash(uri, file, md5File, successCallback) {
    //startSpinner(term, spinners["shark"]);
    term.set_prompt(progress(0, progressSize));
    $.ajax(uri).done(function (data) {
        doProgress(
            function() {
                $.ajax(file + ".md5").done(function (data) {
                    var calcMd5 = $.trim(data);
                    $.ajax(md5File).done(function (data) {
                        var lastFlashMd5 = $.trim(data);
                        endTransactionWithMD5Check(calcMd5, lastFlashMd5, "Please try to re-flash.", successCallback);
                    }).fail(function() {
                        endTransaction('Error reading checksum', true);
                    });
                }).fail(function() {
                    endTransaction('Error reading checksum', true);
                });
            }
        );
    }).fail(function() {
        endTransaction("Error starting flash process!", true);
    });
}

function endTransactionWithMD5Check(chk1, chk2, msg, cb) {
    endTransaction(
        progress(100, progressSize) + ' [[b;green;]OK]\n'
        + "MD5 Check:\n"
        + (chk1 == chk2
            ? "    " + chk1 + "\n == " + chk2 + " [[b;green;]OK]"
            : "    " + chk1 + "\n[[b;red;] != ]" + chk2 + " [[b;red;]FAIL]")
        + (chk1 == chk2 ? "" : "\n" + msg), 
        null, 
        (chk1 == chk2 ? cb : null)
    );
}
